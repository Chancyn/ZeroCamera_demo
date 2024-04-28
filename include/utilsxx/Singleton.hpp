// Copyright(c) 2024-present, zhoucc zhoucc2008@outlook.com contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>

template<typename T>
class Singleton {
 public:
    static T &GetInstance() {
        if (t_ == nullptr) {
            std::unique_lock<std::mutex> unique_locker(mtx_);
            if (t_ == nullptr) {
                t_ = std::unique_ptr<T>(new T);
                printf("zhoucc Constructor [%p]\n", &(*t_));
            }

            return *t_;
        }
        // printf("zhoucc GetInstance [%p]\n", &(*t_));
        return *t_;
    }

 protected:
    Singleton() = default;
    virtual ~Singleton() = default;

 private:
    explicit Singleton(T &&) = delete;
    explicit Singleton(const T &) = delete;
    void operator=(const T &) = delete;

 private:
    static std::unique_ptr<T> t_;
    static std::mutex mtx_;
};

template<typename T>
std::unique_ptr<T> Singleton<T>::t_;

template<typename T>
std::mutex Singleton<T>::mtx_;

template<typename T>
class SingletonArgs {
 public:
    template<typename... Args>
    static T &GetInstance(Args &&...args) {
        if (t_ == nullptr) {
            std::unique_lock<std::mutex> unique_locker(mtx_);
            if (t_ == nullptr) {
                t_ = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
                printf("zhoucc Args Constructor [%p]\n", &(*t_));
            }

            return *t_;
        }
        // printf("zhoucc Args GetInstance [%p]\n", &(*t_));
        return *t_;
    }

    // Destory Instance,normal do not use it
    static void DesInstance() {
        std::unique_lock<std::mutex> unique_locker(mtx_);
        if (t_) {
            t_.reset();
            // t_ = nullptr;
        }
    }

 protected:
    SingletonArgs() = default;
    virtual ~SingletonArgs() = default;

 private:
    explicit SingletonArgs(T &&) = delete;
    explicit SingletonArgs(const T &) = delete;
    void operator=(const T &) = delete;

 private:
    static std::unique_ptr<T> t_;
    static std::mutex mtx_;
};

template<typename T>
std::unique_ptr<T> SingletonArgs<T>::t_;

template<typename T>
std::mutex SingletonArgs<T>::mtx_;
