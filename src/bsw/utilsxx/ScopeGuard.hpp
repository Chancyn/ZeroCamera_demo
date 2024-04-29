// Copyright(c) 2024-present, zhoucc contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once
template<typename F>
class CGuard {
 public:
    explicit CGuard(F &&f) : m_func(std::move(f)), m_dismiss(false) {}
    explicit CGuard(const F &f) : m_func(f), m_dismiss(false) {}

    ~CGuard() {
        if (!m_dismiss && m_func != nullptr)
            m_func();
    }

    CGuard(CGuard &&rhs) : m_func(std::move(rhs.m_func)), m_dismiss(rhs.m_dismiss) { rhs.Dismiss(); }

    void Dismiss() { m_dismiss = true; }

 private:
    F m_func;
    bool m_dismiss;

    CGuard();
    CGuard(const CGuard &);
    CGuard &operator=(const CGuard &);
};

template<typename F>
CGuard<typename std::decay<F>::type> MakeGuard(F &&f) {
    return CGuard<typename std::decay<F>::type>(std::forward<F>(f));
}
