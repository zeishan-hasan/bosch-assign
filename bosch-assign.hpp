#include <iostream>
#include <queue>
#include <pthread.h>

#define TIMEOUT     500
#define R_RET       200
#define W_RET       100
#define W_LIMIT     10

static int w_ret = 0, r_ret = 0;

/**
 * @brief A thread-safe queue implementation.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T>
class Queue {
    private:
        std::queue<T> m_queue;     /**< The underlying queue */
        pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER; /**< Mutex lock for thread safety */
        pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;    /**< Condition variable for reader threads */
        pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;   /**< Condition variable for writer threads */
        int write_counts = 0;       /**< Number of write operations performed */
        int queue_capacity = 0;     /**< Maximum capacity of the queue */
        int is_done = 0;            /**< Flag indicating whether the producer is done */

    public:
        /**
         * @brief Constructs a new Queue object.
         *
         * @param n The maximum capacity of the queue.
         */
        Queue(int n) {
            this->queue_capacity = n;
            if (pthread_mutex_init(&mutex_lock, NULL)) {
                fprintf(stderr, "Unable to initialize the mutex\n");
            }
        }

        /**
         * @brief Destroys the Queue object.
         */
        ~Queue() {
            pthread_mutex_destroy(&mutex_lock);
            pthread_cond_destroy(&cond_read);
            pthread_cond_destroy(&cond_write);
        }

        /**
         * @brief Gets the number of elements in the queue.
         *
         * @return The number of elements in the queue.
         */
        int count() {
            return this->m_queue.size();
        }

        /**
         * @brief Sets the "is_done" flag to indicate the producer is done.
         */
        void set_is_done() {
            this->is_done = 1;
        }

        /**
         * @brief Gets the value of the "is_done" flag.
         *
         * @return The value of the "is_done" flag.
         */
        int get_is_done() {
            return this->is_done;
        }

        /**
         * @brief Gets the maximum capacity of the queue.
         *
         * @return The maximum capacity of the queue.
         */
        int queue_size() {
            return this->queue_capacity;
        }

        /**
         * @brief Pushes an element into the queue.
         *
         * This function pushes the given element into the queue, blocking if the queue is full.
         *
         * @param element The element to be pushed into the queue.
         * @param timeout_ms The maximum time in milliseconds to wait if the queue is full. Default is 0 (no timeout).
         * @return True if the element was successfully pushed into the queue, false if the push operation timed out or if the maximum number of elements (10) has been reached.
         *
         * @details
         * - If the current number of elements in the queue is equal to the queue's capacity, the function waits for the read condition to be signaled (indicating that a reader has consumed an element from the queue).
         * - If a timeout (specified by `timeout_ms`) is provided and the read condition is not signaled within the given time, the push operation times out and returns false.
         * - If no timeout is specified (`timeout_ms` = 0) and the queue is full, the function waits indefinitely until the read condition is signaled.
         * - After successfully pushing the element into the queue, the function signals the write condition to notify potential readers that there is data available for consumption.
         * - The function also increments the `write_counts` variable to keep track of the number of successful push operations. If `write_counts` reaches 10, indicating that the maximum number of elements has been reached, the `is_done` flag is set to true, and subsequent push operations will return false.
         *
         * @note This function should be called by the writer thread.
         */
        bool push(T element, int timeout_ms = 0) {
            bool status = true;
            write_counts++;
            if (W_LIMIT < write_counts) {
                this->is_done = true;
                status = false;
                return status;
            } 
            
            pthread_mutex_lock(&mutex_lock);
            std::cout << "push: acquired lock\n";
            if (this->count() == this->queue_size()) {
                std::cout << "push: queue is full\n";
                if (timeout_ms > 0) {
                    struct timespec time_spec;
                    clock_gettime(CLOCK_REALTIME, &time_spec);
                    time_spec.tv_sec += timeout_ms / 1000;
                    time_spec.tv_nsec += (timeout_ms % 1000) * 1000000;
                    if (time_spec.tv_nsec > 1000000000) {
                        time_spec.tv_sec += 1;
                        time_spec.tv_nsec -= 1000000000;
                    }
                    if (pthread_cond_timedwait(&cond_read, &mutex_lock, &time_spec) == ETIMEDOUT) {
                        status = false;
                        std::cout << "push: timeout\n";
                    }
                } else {
                    pthread_cond_wait(&cond_read, &mutex_lock);
                }
            } 
            
            if (status == true) {
                this->m_queue.push(element);
                std::cout << "push: pushed " << element << " into the queue\n";
                if (1 == this->count()) {
                    pthread_cond_signal(&cond_write);
                    std::cout << "push: signaled to pop\n";
                }
            }

            pthread_mutex_unlock(&mutex_lock);
            std::cout << "push: released lock\n";

            return status;
        }

        /**
         * @brief Pops an element from the queue.
         *
         * This function pops an element from the queue, blocking if the queue is empty.
         *
         * @param item A reference to the variable where the popped element will be stored.
         * @param timeout_ms The maximum time in milliseconds to wait if the queue is empty. Default is 0 (no timeout).
         * @return True if an element was successfully popped from the queue, false if the pop operation timed out or if the queue is empty.
         *
         * @details
         * - If the current number of elements in the queue is 0, indicating that the queue is empty, the function waits for the write condition to be signaled (indicating that a writer has pushed an element into the queue).
         * - If a timeout (specified by `timeout_ms`) is provided and the write condition is not signaled within the given time, the pop operation times out and returns false.
         * - If no timeout is specified (`timeout_ms` = 0) and the queue is empty, the function waits indefinitely until the write condition is signaled.
         * - After successfully popping an element from the queue, the function assigns the popped element to the `item` parameter and removes it from the queue.
         * - The function also signals the read condition if the number of elements in the queue becomes (queue_capacity - 1). This indicates to potential writers that there is space available in the queue for pushing new elements.
         *
         * @note This function should be called by the reader thread.
         */
        bool pop(T& item, int timeout_ms = 0) {
            bool status = true;
            pthread_mutex_lock(&mutex_lock);
            if (0 == this->count()) {
                std::cout << "pop: queue is empty\n";
                if (timeout_ms > 0) {
                    struct timespec time_spec;
                    clock_gettime(CLOCK_REALTIME, &time_spec);
                    time_spec.tv_sec += timeout_ms / 1000;
                    time_spec.tv_nsec += (timeout_ms % 1000) * 1000000;
                    if (time_spec.tv_nsec > 1000000000) {
                        time_spec.tv_sec += 1;
                        time_spec.tv_nsec -= 1000000000;
                    }
                    if (0 != pthread_cond_timedwait(&cond_write, &mutex_lock, &time_spec)) {
                        status = false;
                        std::cout << "pop: timeout\n";
                        
                    }
                } else {
                    pthread_cond_wait(&cond_write, &mutex_lock);
                }
            }
            
            if (true == status) {
                item = this->m_queue.front();
                this->m_queue.pop();
                std::cout << "pop: element popped out\n";
                if (this->count() == (this->queue_capacity - 1)) {
                    pthread_cond_signal(&cond_read);
                    std::cout << "pop: signalled push\n";
                }
            }

            pthread_mutex_unlock(&mutex_lock);
            std::cout << "pop: released lock\n";

            return status;
        }

};

/**
 * @brief Writer thread function.
 *
 * @param arg A pointer to the Queue object.
 * @return void* The return value of the thread.
 */
void *writer(void *arg) {
    if (arg == NULL) {
        return nullptr;
    }

    Queue<int> *m_queue = static_cast<Queue<int>*>(arg);
    int count = 0;
    while(true) {
        count++;
        std::cout << "writer: queue count is " << m_queue->count() << std::endl;
        // m_queue->push(count);
        m_queue->push(count, TIMEOUT);
        if (m_queue->get_is_done()) {
            w_ret = W_RET;
            pthread_exit(&w_ret);
        }
    }
}

/**
 * @brief Reader thread function.
 *
 * @param arg A pointer to the Queue object.
 * @return void* The return value of the thread.
 */
void *reader(void *arg) {
    if (arg == NULL) {
        return nullptr;
    }

    Queue<int> *m_queue = static_cast<Queue<int>*>(arg);
    int item = 0;
    while(true) {
        // m_queue->pop(item);
        m_queue->pop(item, TIMEOUT);
        std::cout << "reader: popped " << item << std::endl;
        if (m_queue->get_is_done()) {
            std::cout << "reader: queue count is " << m_queue->count() << std::endl;
            if (m_queue->count() == 0) {
                r_ret = R_RET;
                pthread_exit(&r_ret);
            }
        }
    }
}