#ifndef THREADQUEUE_H_
#define THREADQUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadQueue {
 public:
  T pop() {
    std::unique_lock<std::mutex> mlock(mutex_);
    while(queue_.empty()) {
      cond_.wait(mlock);
    }
    auto val = queue_.front();
    queue_.pop();
    return val;
  }

  void pop(T& item){
    std::unique_lock<std::mutex> mlock(mutex_);
    while(queue_.empty()){
      cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop();
  }

  void push(const T& item) {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }

  ThreadQueue() = default;
  ThreadQueue(const ThreadQueue&) = delete;
  ThreadQueue& operator = (const ThreadQueue&) = delete;

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif
