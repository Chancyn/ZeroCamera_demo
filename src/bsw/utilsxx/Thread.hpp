// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace zc {
class Thread {
 public:
    explicit Thread(std::string name = "zcThreadDef");
    virtual ~Thread();

    enum State_e {
        Stoped,   // stoped or ide
        Running,  // running
        Paused    // Paused
    };

    State_e State() const;
    void Start();
    void Stop();
    void Pause();
    void Resume();

 protected:
    virtual int process() = 0;

 private:
    void _run();

 private:
    std::string m_name;
    std::thread *_thread;
    std::mutex _mutex;
    std::condition_variable _condition;
    std::atomic_bool _pauseFlag;
    std::atomic_bool _stopFlag;
    State_e _state;
};

}  // namespace zc
