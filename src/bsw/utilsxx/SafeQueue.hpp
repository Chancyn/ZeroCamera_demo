// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <memory>

// Thread safe implementation of a Queue using a std::queue
template<typename T>
class SafeQueue {
 public:
    SafeQueue() {}
    SafeQueue(SafeQueue &&other) {}
    ~SafeQueue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);  // 互斥信号变量加锁，防止m_queue被改变
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);  // 互斥信号变量加锁，防止m_queue被改变
        return m_queue.size();
    }

    // add
    void enqueue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }

    // pop first
    bool dequeue(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);  // 队列加锁
        if (m_queue.empty())
            return false;
        t = std::move(m_queue.front());  // 取出队首元素，返回队首元素值，并进行右值引用
        m_queue.pop();                   // 弹出入队的第一个元素

        return true;
    }

 private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

class ThreadPools {
 private:
    class ThreadWorker {
     private:
        ThreadPools *m_pool;  // pool ptr
        int m_id;             // work id

     public:
        ThreadWorker(ThreadPools *pool, const int id) : m_pool(pool), m_id(id) {}
        // 重载()操作
        void operator()() {
            std::function<void()> func;  // 定义基础函数类func
            bool dequeued = false;
            while (!m_pool->m_shutdown) {
                {
                    // 为线程环境加锁，互访问工作线程的休眠和唤醒
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);

                    // task queue empty,wait
                    if (m_pool->m_queue.empty()) {
                        m_pool->m_conditional_lock.wait(lock);  // 等待条件变量通知，开启线程
                    }

                    // pop task
                    dequeued = m_pool->m_queue.dequeue(func);
                }

                if (dequeued)
                    func();
            }
        }
    };

    bool m_shutdown;
    SafeQueue<std::function<void()>> m_queue;    // 执行函数安全队列，即任务队列
    std::vector<std::thread> m_threads;          // 工作线程队列
    std::mutex m_conditional_mutex;              // 线程休眠锁互斥变量
    std::condition_variable m_conditional_lock;  // 线程环境锁，可以让线程处于休眠或者唤醒状态

 public:
    explicit ThreadPools(const int n_threads = 4) : m_shutdown(false), m_threads(std::vector<std::thread>(n_threads)) {}

    // Inits thread pool
    void init() {
        for (size_t i = 0; i < m_threads.size(); ++i) {
            m_threads.at(i) = std::thread(ThreadWorker(this, i));  // 分配工作线程
        }
    }

    // Waits until threads finish their current task and shutdowns the pool
    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();  // 通知，唤醒所有工作线程

        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (m_threads.at(i).joinable()) {
                m_threads.at(i).join();  // 将线程加入到等待队列
            }
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template<typename F, typename... Args>
    auto AddTask(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameter ready to execute
        std::function<decltype(f(args...))()> func = std::bind(
            std::forward<F>(f), std::forward<Args>(args)...);  // 连接函数和参数定义，特殊函数类型，避免左右值错误

        // Encapsulate it into a shared pointer in order to be able to copy construct
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]() { (*task_ptr)(); };

        // 队列通用安全封包函数，并压入安全队列
        m_queue.enqueue(warpper_func);

        // 唤醒一个等待中的线程
        m_conditional_lock.notify_one();

        // 返回先前注册的任务指针
        return task_ptr->get_future();
    }
};
