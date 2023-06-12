#include <gtest/gtest.h>
#include <pthread.h>
#include "../bosch-assign.hpp"

// Test case for reader/writer
TEST(QueueTest, ReadAndWrite) {
    // Create a queue with a capacity of 5
    Queue<int> myQueue(5);

    // Create writer thread
    pthread_t writerThread;
    pthread_create(&writerThread, NULL, writer, &myQueue);

    // Create reader thread
    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader, &myQueue);

    // Wait for writer thread to finish
    void* w_ret_val;
    pthread_join(writerThread, &w_ret_val);

    // Wait for reader thread to finish
    void* r_ret_val;
    pthread_join(readerThread, &r_ret_val);

    // Verify the return values of the threads
    int w_ret = *(static_cast<int*>(w_ret_val));
    int r_ret = *(static_cast<int*>(r_ret_val));
    EXPECT_EQ(100, w_ret);
    EXPECT_EQ(200, r_ret);
}


// Test case to verify the push() and pop() functions
TEST(QueueTest, PushAndPop) {
    // Create a queue with a capacity of 5
    Queue<int> queue(5);

	// Push five elements into the queue
	queue.push(10);
	queue.push(20);
	queue.push(30);
	queue.push(40);
	queue.push(50);

	// Pop the elements and verify their order
	EXPECT_EQ(10, queue.pop());
	EXPECT_EQ(20, queue.pop());
	EXPECT_EQ(30, queue.pop());
	EXPECT_EQ(40, queue.pop());
	EXPECT_EQ(50, queue.pop());

	// After popping all elements, the queue should be empty
	EXPECT_EQ(0, queue.count());
}

// Test case to check 'is_done' flag when the producer is done
TEST(QueueTest, IsDoneFlagAfterProcessing) {
	Queue<int> queue(5);
    // Create writer thread
    pthread_t writerThread;
    pthread_create(&writerThread, NULL, writer, &queue);

    // Create reader thread
    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader, &queue);

    // Wait for the threads to finish
	void* w_retval;
	void* r_retval;
	pthread_join(writerThread, &w_retval);
	pthread_join(readerThread, &r_retval);

	// Check the is_done flag for producer done
	ASSERT_EQ(queue.get_is_done(), 1);
}

#if modify_push_func
// Test case for pushing and popping elements from the Queue
TEST(QueueTest, QueueFull) {
    Queue<int> queue(2);
    // Push 2 elements into the Queue
	ASSERT_TRUE(queue.push(1));
	ASSERT_TRUE(queue.push(2));

	// Try to push another element into the full Queue.
	ASSERT_FALSE(queue.push(3));

}
#endif

#if modify_pop_func
// Test case for the empty Queue
TEST(QueueTest, QueueEmpty) {
    Queue<int> queue(2);
    // Push 2 elements into the Queue
	ASSERT_TRUE(queue.push(1));
	ASSERT_TRUE(queue.push(2));
	EXPECT_EQ(2, queue.count());

	ASSERT_TRUE(queue.pop());
	ASSERT_TRUE(queue.pop());
	EXPECT_EQ(0, queue.count());

	// Try to pop element from the empty Queue
	ASSERT_FALSE(queue.pop());
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}