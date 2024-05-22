// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include "Thread.hpp"
#include <string>

#include "zc_log.h"

// using namespace std;
namespace zc {

Thread::Thread(std::string name)
    : m_name(name), _thread(nullptr), _pauseFlag(false), _stopFlag(false), _state(Stoped) {}

Thread::~Thread() {
    Stop();
}

Thread::State_e Thread::State() const {
    return _state;
}

void Thread::Start() {
    if (_thread == nullptr) {
        _thread = new std::thread(&Thread::_run, this);
        _pauseFlag = false;
        _stopFlag = false;
        _state = Running;
    }
}

void Thread::Stop() {
    if (_thread != nullptr) {
        _pauseFlag = false;
        _stopFlag = true;
        _state = Stoping;
        _condition.notify_all();  // Notify one waiting thread, if there is one.
        _thread->join();          // wait for thread finished
        delete _thread;
        _thread = nullptr;
        _state = Stoped;
    }
}

void Thread::Pause() {
    if (_thread != nullptr) {
        _pauseFlag = true;
        _state = Paused;
    }
}

void Thread::Resume() {
    if (_thread != nullptr) {
        _pauseFlag = false;
        _condition.notify_all();
        _state = Running;
    }
}

void Thread::_run() {
    LOG_INFO("enter thread:,%s %d", m_name.c_str(), std::this_thread::get_id());
    // 线程外设置
    // pthread_setname_np(_thread->native_handle(), m_name.substr(0, 15).c_str());
    // 线程内设置
    pthread_setname_np(pthread_self(), m_name.substr(0, 15).c_str());
    while (!_stopFlag) {
        if (process() < 0) {
            LOG_ERROR("process error, pause thread:,%s %d", m_name.c_str(), std::this_thread::get_id());
            _pauseFlag = true;
        }

        if (!_stopFlag && _pauseFlag) {
            std::unique_lock<std::mutex> locker(_mutex);
            while (_pauseFlag) {
                _condition.wait(locker);  // Unlock _mutex and wait to be notified
            }
            locker.unlock();
        }
    }
    _pauseFlag = false;
    _stopFlag = false;

    LOG_INFO("exit thread:%s %d", m_name.c_str(), std::this_thread::get_id());
}

}  // namespace zc
