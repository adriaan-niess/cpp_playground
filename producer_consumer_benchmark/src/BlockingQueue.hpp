#ifndef BLOCKINGQUEUE_HPP
#define BLOCKINGQUEUE_CPP

#include "ConcurrentQueue.hpp"

#include <mutex>
#include <boost/circular_buffer.hpp>
#include <condition_variable>

template<typename T>
class BlockingQueue : public ConcurrentQueue<T>
{
private:
    std::mutex mMutex;
    boost::circular_buffer<T> mQueue;
    std::condition_variable mNotFull;
    std::condition_variable mNotEmpty;
public:
    BlockingQueue(uint64_t maxSize = 100);
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;
    virtual ~BlockingQueue() {};
    virtual void pop(T& value) override;
    virtual void push(const T& value) override;
};

template<typename T>
BlockingQueue<T>::BlockingQueue(uint64_t maxSize)
    : mQueue(maxSize)
{
}

template<typename T>
void BlockingQueue<T>::pop(T& value)
{
    std::unique_lock<std::mutex> lock(mMutex);
    while (mQueue.empty()) {
        mNotEmpty.wait(lock);
    }
    value = mQueue.front();
    mQueue.pop_front();
    lock.unlock();
    mNotFull.notify_one();
}

template<typename T>
void BlockingQueue<T>::push(const T& value)
{
    std::unique_lock<std::mutex> lock(mMutex);
    while (mQueue.full()) {
        mNotFull.wait(lock);
    }
    mQueue.push_back(value);
    lock.unlock();
    mNotEmpty.notify_one();
}

#endif
