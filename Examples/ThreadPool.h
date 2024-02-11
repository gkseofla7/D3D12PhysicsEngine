#pragma once

//#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace hlab {

    class ThreadPool {
    public:
        static ThreadPool& getInstance()
        {
            static ThreadPool threadPool(5);
            return threadPool;
        }
        ThreadPool(size_t num_threads);
        ~ThreadPool();

        // job 을 추가한다.
        template <class F, class... Args>
        std::future<typename std::result_of<F(Args...)>::type> EnqueueJob(
            F&& f, Args&&... args);

    private:
        // 총 Worker 쓰레드의 개수.
        size_t num_threads_;
        // Worker 쓰레드를 보관하는 벡터.
        std::vector<std::thread> worker_threads_;
        // 할일들을 보관하는 job 큐.
        std::queue<std::function<void()>> jobs_;
        // 위의 job 큐를 위한 cv 와 m.
        std::condition_variable cv_job_q_;
        std::mutex m_job_q_;

        // 모든 쓰레드 종료
        bool stop_all;

        // Worker 쓰레드
        void WorkerThread();
    };
    template <class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueJob(
        F&& f, Args&&... args) {
        if (stop_all) {
            throw std::runtime_error("ThreadPool 사용 중지됨");
        }

        using return_type = typename std::result_of<F(Args...)>::type;
        auto job = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> job_result_future = job->get_future();
        {
            std::lock_guard<std::mutex> lock(m_job_q_);
            jobs_.push([job]() { (*job)(); });
        }
        cv_job_q_.notify_one();

        return job_result_future;
    }
}  // namespace ThreadPool
//#include "ThreadPool.hpp"