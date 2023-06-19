#include "bosch-assign.hpp"

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
    Queue<int> *q = new Queue<int>(2);
    std::cout << "queue capacity: " << q->queue_size() << std::endl;
    pthread_t threads[2];
	int check = 0;
    int *ptr[2];

	check = pthread_create(&threads[0], NULL, writer, (void*)q);
	if (check) {
		fprintf(stderr, "unable to create the thread\n");
        return -1;
	}

	check = pthread_create(&threads[1], NULL, reader, (void*)q);
	if (check) {
		fprintf(stderr, "unable to create the thread\n");
        return -1;
	}

    pthread_join(threads[0], (void**)&(ptr[0]));
    pthread_join(threads[1], (void**)&(ptr[1]));

    std::cout << "writer thread returned: " << *ptr[0] << std::endl;
    std::cout << "reader thread returned: " << *ptr[1] << std::endl;
    delete q;

    return 0;
}