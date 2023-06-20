#include "bosch-assign.hpp"

#define WRITER_THREAD   0
#define READER_THREAD   1

/**
 * @brief The main function.
 *
 * This function demonstrates the usage of the Queue class by creating two threads:
 * one writer thread and one reader thread. The writer thread pushes integers into
 * the queue, while the reader thread pops integers from the queue. The main function
 * waits for the threads to complete, retrieves their return values, and outputs them.
 *
 * @return 0 on successful execution.
 */
int main() {
    Queue<int> *thread_safe_queue = new Queue<int>(2);
    std::cout << "queue capacity: " << thread_safe_queue->queue_size() << std::endl;
    pthread_t threads[2];
	int check = 0;
    int *ptr[2];

	check = pthread_create(&threads[WRITER_THREAD], NULL, writer, (void*)thread_safe_queue);
	if (check) {
		fprintf(stderr, "unable to create the thread\n");
        return -1;
	}

	check = pthread_create(&threads[READER_THREAD], NULL, reader, (void*)thread_safe_queue);
	if (check) {
		fprintf(stderr, "unable to create the thread\n");
        return -1;
	}

    pthread_join(threads[WRITER_THREAD], (void**)&(ptr[WRITER_THREAD]));
    pthread_join(threads[READER_THREAD], (void**)&(ptr[READER_THREAD]));

    std::cout << "writer thread returned: " << *ptr[WRITER_THREAD] << std::endl;
    std::cout << "reader thread returned: " << *ptr[READER_THREAD] << std::endl;
    delete thread_safe_queue;

    return 0;
}