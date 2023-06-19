#include <gtest/gtest.h>
#include <pthread.h>
#include "../bosch-assign.hpp"

pthread_t w_thread;
pthread_t r_thread;
Queue<int> *queue; 
            
/**
 * @brief Fixture for testing the Queue class with delay functionality.
 *
 * This fixture sets up the Queue object before running each test case and cleans it up after each test case.
 */
class QueueTestDelay : public ::testing::Test {
protected:
    void SetUp() override {
        queue = new Queue<int>(2);
    }

    void TearDown() override {
        delete queue;
    }
};

/**
 * @brief Test case for reading and writing threads.
 *
 * This test case validates the behavior of the reading and writing threads working concurrently with delay functionality.
 * It creates and joins the writer and reader threads, and checks the return values of both threads.
 * The test expects the writer thread to return 100 and the reader thread to return 200.
 */
TEST_F(QueueTestDelay, ReadingAndWritingThread) {
    void* w_ret;
    void* r_ret;
    pthread_create(&w_thread, NULL, writer, queue);
    pthread_create(&r_thread, NULL, reader, queue);
    pthread_join(w_thread, &w_ret);
    pthread_join(r_thread, &r_ret);

    int w_ret_val = *(static_cast<int*>(w_ret));
    std::cout << "writer thread returned: " << w_ret_val << std::endl;
    int r_ret_val = *(static_cast<int*>(r_ret));
    std::cout << "reader thread returned: " << r_ret_val << std::endl;
    ASSERT_EQ(100, w_ret_val);
    ASSERT_EQ(200, r_ret_val);
}

/**
 * @brief Test case for pushing and popping elements.
 *
 * This test case verifies the functionality of pushing elements into the queue and popping them back with delay functionality.
 * It checks the count of the queue before and after pushing elements, and also validates the popped items.
 * The test expects successful pushes and pops of elements, and an empty queue at the end.
 */
TEST_F(QueueTestDelay, PushAndPopElement) {
    int item;
    EXPECT_EQ(0, queue->count());
    ASSERT_TRUE(queue->push(1, 100));
    ASSERT_TRUE(queue->push(2, 100));
    EXPECT_EQ(2, queue->count());
    ASSERT_TRUE(queue->pop(item, 100));
    ASSERT_EQ(1, item);
    ASSERT_TRUE(queue->pop(item, 100));
    ASSERT_EQ(2, item);
    EXPECT_EQ(0, queue->count());
}

/**
 * @brief Test case for is_done flag.
 *
 * This test case verifies the behavior of the is_done flag in the Queue class with delay functionality.
 * It creates and joins the reader and writer threads and checks if the is_done flag is set to true at the end.
 * The test expects the is_done flag to be true after both threads have finished.
 */
TEST_F(QueueTestDelay, IsDoneFlag) {
    void* w_ret;
    void* r_ret;
    pthread_create(&r_thread, NULL, reader, queue);
    pthread_create(&w_thread, NULL, writer, queue);
    pthread_join(r_thread, &r_ret);
    pthread_join(w_thread, &w_ret);

    ASSERT_TRUE(queue->get_is_done());
}

/**
 * @brief Test case for popping an item from an empty queue with a timeout.
 *
 * This test case validates the behavior of popping an item from an empty queue with a timeout.
 * It checks if the pop operation returns false when the queue is empty and a timeout is specified.
 * The test expects the pop operation to fail (returning false).
 */
TEST_F(QueueTestDelay, PopFromEmptyQueue) {
    int item;
    EXPECT_EQ(0, queue->count());
    ASSERT_FALSE(queue->pop(item, 500));
}


/**
 * Previous test cases below 
*/

/**
 * @brief Test case for reading and writing thread functions.
 *
 * This test case validates the behavior of the reading and writing threads working concurrently without delay functionality.
 * It creates and joins the writer and reader threads, and checks the return values of both threads.
 * The test expects the writer thread to return 100 and the reader thread to return 200.
 */
TEST(QueueTest, ReadAndWrite) {
    Queue<int> myQueue(5);
    pthread_t writerThread;
    pthread_create(&writerThread, NULL, writer, &myQueue);

    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader, &myQueue);

    void* w_ret_val;
    pthread_join(writerThread, &w_ret_val);

    void* r_ret_val;
    pthread_join(readerThread, &r_ret_val);

    int w_ret = *(static_cast<int*>(w_ret_val));
    int r_ret = *(static_cast<int*>(r_ret_val));
    EXPECT_EQ(100, w_ret);
    EXPECT_EQ(200, r_ret);
}


/**
 * @brief Test case for verifying the order of elements.
 *
 * This test case validates the order of the popped elements which were pushed to the queue; without delay functionality.
 * It creates the queue with the capacity of five elements than pushes five elements in a row.
 * The test expects the popped elements in the order they are pushed and queue size to be zero in the end.
 */
TEST(QueueTest, PushAndPop) {
    Queue<int> queue(5);
    int item;

	queue.push(10);
	queue.push(20);
	queue.push(30);
	queue.push(40);
	queue.push(50);

    queue.pop(item);
	EXPECT_EQ(10, item);
	queue.pop(item);
    EXPECT_EQ(20, item);
	queue.pop(item);
    EXPECT_EQ(30, item);
	queue.pop(item);
    EXPECT_EQ(40, item);
	queue.pop(item);
    EXPECT_EQ(50, item);

	EXPECT_EQ(0, queue.count());
}

// Test case to check 'is_done' flag when the producer is done
TEST(QueueTest, IsDoneFlagAfterProcessing) {
	Queue<int> queue(5);

    pthread_t writerThread;
    pthread_create(&writerThread, NULL, writer, &queue);

    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader, &queue);

	void* w_retval;
	void* r_retval;
	pthread_join(writerThread, &w_retval);
	pthread_join(readerThread, &r_retval);

	ASSERT_EQ(queue.get_is_done(), 1);
}

/**
 * @brief Test case to verify the delay functionality of push funciton.
 * 
 * This test case validates the return 'false' funcitonality when trying to push an element into the full queue, with delay.
 * The test expects the push function to return false when it could not push the element within the set time.
*/
TEST(QueueTest, QueueFull) {
    Queue<int> queue(2);
	ASSERT_TRUE(queue.push(1));
	ASSERT_TRUE(queue.push(2));

	ASSERT_FALSE(queue.push(3,1000));
}

/**
 * @brief Test case to verify delay funcitonality of the pop function.
 * 
 * This test case validates the return 'false' funcitonality when trying to pop an element from the empty queue, with delay.
 * The test expects the pop function to return false when it could not pop the element within the set time.
 * The test case also keeps an eye over the queue size when pushing and popping elements.
*/
TEST(QueueTest, QueueEmpty) {
    Queue<int> queue(2);
    int item;
    // Push 2 elements into the Queue
	ASSERT_TRUE(queue.push(1));
	ASSERT_TRUE(queue.push(2));
	EXPECT_EQ(2, queue.count());

	ASSERT_TRUE(queue.pop(item));
    EXPECT_EQ(1, item);
	ASSERT_TRUE(queue.pop(item));
    EXPECT_EQ(2, item);
	EXPECT_EQ(0, queue.count());

	// Try to pop element from the empty Queue
	ASSERT_FALSE(queue.pop(item,1000));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}