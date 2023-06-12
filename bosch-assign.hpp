#include <iostream>
#include <queue>
#include <pthread.h>
#define TIME_OUT    110
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
         * @param element The element to push into the queue.
         */
        void push(T element) {
            int timeout_ms = 500;
            write_counts++;
            if (write_counts >= 10) {
                this->set_is_done();
            }

            pthread_mutex_lock(&mutex_lock);
            std::cout << "push: locked" << std::endl;
            bool success = true;
            if (count() >= queue_size()) {
                if (timeout_ms > 0) {
                    struct timespec time_spec;
                    clock_gettime(CLOCK_REALTIME, &time_spec);
                    time_spec.tv_sec += timeout_ms / 1000;
                    time_spec.tv_nsec += (timeout_ms % 1000) * 1000000;
                    if (time_spec.tv_nsec > 1000000000) {
                        time_spec.tv_sec += 1;
                        time_spec.tv_nsec -= 1000000000;
                    }

                    if (pthread_cond_timedwait(&cond_read, &mutex_lock, &time_spec) == TIME_OUT) {
                        std::cout << "push: timed out" << std::endl;
                        success = false;
                    }
                } else {
                    std::cout << "push: queue full" << std::endl;
                    pthread_cond_wait(&cond_read, &mutex_lock);
                }
            }

            if (success) {
                this->m_queue.push(element);
                std::cout << "push: pushed " << element << std::endl;
                if (1 == count()) {
                    pthread_cond_signal(&cond_write);
                    std::cout << "push: signaled to pop" << std::endl;
                }
            }

            pthread_mutex_unlock(&mutex_lock);
            std::cout << "push: released lock" << std::endl;
        }

        /**
         * @brief Pops an element from the queue.
         *
         * @return The element popped from the queue.
         */
        T pop() {
            T item = 0;
            int timeout_ms = 500;
            pthread_mutex_lock(&mutex_lock);

            bool success = true;
            if (this->m_queue.size() == 0) {
                if (timeout_ms > 0) {
                    struct timespec time_spec;
                    clock_gettime(CLOCK_REALTIME, &time_spec);
                    time_spec.tv_sec += timeout_ms / 1000;
                    time_spec.tv_nsec += (timeout_ms % 1000) * 1000000;
                    if (time_spec.tv_nsec > 1000000000) {
                        time_spec.tv_sec += 1;
                        time_spec.tv_nsec -= 1000000000;
                    }

                    if (pthread_cond_timedwait(&cond_write, &mutex_lock, &time_spec) == TIME_OUT) {
                        std::cout << "pop: timed out" << std::endl;
                        success = false;
                    }
                } else {
                    std::cout << "pop: queue empty" << std::endl;
                    pthread_cond_wait(&cond_write, &mutex_lock);
                }
            }

            if (success) {
                item = this->m_queue.front();
                this->m_queue.pop();
                pthread_cond_signal(&cond_read);
            }

            pthread_mutex_unlock(&mutex_lock);
            std::cout << "pop: released lock\n";

            return item;
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

    Queue<int> *my_queue = static_cast<Queue<int>*>(arg);
    int count = 0;
    while(true) {
        count++;
        std::cout << "writer: queue count is " << my_queue->count() << std::endl;
        my_queue->push(count);
        if (count >= 10) {
            w_ret = 100;
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

    Queue<int> *my_queue = static_cast<Queue<int>*>(arg);
    while(true) {
        std::cout << my_queue->pop() << std::endl;
        if (my_queue->get_is_done()) {
            std::cout << "reader: queue count is " << my_queue->count() << std::endl;
            if (my_queue->count() == 0) {
                r_ret = 200;
                pthread_exit(&r_ret);
            }
        }
    }
}