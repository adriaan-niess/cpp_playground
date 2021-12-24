#include "ConcurrentQueue.hpp"
#include "BlockingQueue.hpp"
#include "LockfreeQueue.hpp"

#include <iostream>
#include <cstdlib>
#include <thread>
#include <functional>
#include <chrono>
#include <fstream>

#define ITERATIONS 10000
#define PRODUCE_INTERVAL 2ms
#define PRODUCER_LOGFILE "producer.csv"
#define CONSUMER_LOGFILE "consumer.csv"

inline std::chrono::nanoseconds now()
{
    auto time_point = std::chrono::system_clock::now();
    auto duration = time_point.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return nanoseconds;
}

void produce(ConcurrentQueue<int>& queue, const std::string& logfile)
{
    using namespace std::chrono_literals;
    std::ofstream file;
    file.open(logfile);
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        file << i << "," << now().count() << std::endl;
        queue.push(i);
        std::this_thread::sleep_for(PRODUCE_INTERVAL);
    }
    file.close();
}

void consume(ConcurrentQueue<int>& queue, const std::string& logfile)
{
    using namespace std::chrono_literals;
    std::ofstream file;
    file.open(logfile);
    int value;
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        queue.pop(value);
        file << value << "," << now().count() << std::endl;
    }
    file.close();
}

void measureLatency(ConcurrentQueue<int>& queue, const std::string& prefix)
{
    std::string prodLogfile = prefix + PRODUCER_LOGFILE;
    std::string consumerLogfile = prefix + CONSUMER_LOGFILE;
    std::thread producer(std::bind(&produce, std::ref(queue), std::ref(prodLogfile)));
    std::thread consumer(std::bind(&consume, std::ref(queue), std::ref(consumerLogfile)));
    producer.join();
    consumer.join();
}

int main(int argc, char** argv)
{
    LockfreeQueue<int> lockfreeQueue;
    measureLatency(lockfreeQueue, "results/lockfree_");
    
    BlockingQueue<int> blockingQueue;
    measureLatency(blockingQueue, "results/blocking_");

    return EXIT_SUCCESS;
}
