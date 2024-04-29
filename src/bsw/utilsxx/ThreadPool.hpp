// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <thread>
#include <utility>

#include "NonCopyable.hpp"
#include "SyncQueue.hpp"

const size_t MaxTaskCount = 20;

class ThreadPool : public NonCopyable {
 public:
    using Task = std::function<void()>;
    explicit ThreadPool(int numThreads = std::thread::hardware_concurrency()) : m_queue(MaxTaskCount) {
        Start(numThreads);
    }
    ~ThreadPool(void) { Stop(); }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    void Stop() {
        std::call_once(m_flag, [this] { StopThreadGroup(); });  // StopThreadGroup
    }

    void AddTask(Task &&task) { m_queue.Put(std::forward<Task>(task)); }
    void AddTask(const Task &task) { m_queue.Put(task); }

 private:
    void Start(int numThreads) {
        m_running = true;
        // create
        for (int i = 0; i < numThreads; ++i) {
            m_threadgroup.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
        }
    }

    void RunInThread() {
        while (m_running) {
            std::list<Task> list;
            m_queue.Take(list);

            for (auto &task : list) {
                if (!m_running)
                    return;

                task();
            }
        }
    }

    void StopThreadGroup() {
        m_queue.Stop();     // stop task queue
        m_running = false;  // stop thread

        // waiting stop
        for (auto thread : m_threadgroup) {
            if (thread)
                thread->join();
        }
        m_threadgroup.clear();
    }

    std::list<std::shared_ptr<std::thread>> m_threadgroup;
    SyncQueue<Task> m_queue;  // Task queue
    std::atomic_bool m_running;
    // std::condition_variable m_conditional_lock;  // lock
    std::once_flag m_flag;
};
