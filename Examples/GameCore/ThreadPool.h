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
#include <cassert>
#include <iostream>
namespace hlab {

    class ThreadPool {
    public:
        static ThreadPool& getInstance()
        {
            static ThreadPool threadPool(10);
            return threadPool;
        }
        ThreadPool(size_t num_threads);
        ~ThreadPool();

        bool IsRenderThreadDone() { return m_finishRenderThread; }
        void SetUsingMainThreadUsingRendering(bool bInUse) { m_isMainThreadUsingRendering = bInUse; }
        // job �� �߰��Ѵ�.
        template <class F, class... Args>
        std::future<typename std::result_of<F(Args...)>::type> EnqueueJob(
            F&& f, Args&&... args);
        // job �� �߰��Ѵ�.
        template <class F, class... Args>
        std::future<typename std::result_of<F(Args...)>::type> EnqueueRenderJob(
            F&& f, Args&&... args);
        // queue�� �ְ� ���� lock
        std::mutex m_render_job_q_;
        std::condition_variable cv_render_job_q_;
        // ���ν����忡�� ������ ���� ������ ������
        // ���� ������� ���ߵ����Ѵ�.
    private:
        // �� Worker �������� ����.
        size_t num_threads_;
        // Worker �����带 �����ϴ� ����.
        std::vector<std::thread> worker_threads_;
        // ���ϵ��� �����ϴ� job ť.
        std::queue<std::function<void()>> jobs_;
        // ���� job ť�� ���� cv �� m.
        std::condition_variable cv_job_q_;
        std::mutex m_job_q_;

        std::thread m_renderThread;
        std::queue<std::function<void()>> m_renderJobs_;

        // ��� ������ ����
        bool stop_all;

        std::atomic<bool> m_finishRenderThread = false;
        // TODO. Atomic ���� �߰� �ʿ�
        std::atomic<bool> m_isMainThreadUsingRendering = false;
        // Worker ������
        void WorkerThread();

        void RenderThread();
    };
    template <class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueJob(
        F&& f, Args&&... args) {
        if (stop_all) {
            throw std::runtime_error("ThreadPool ��� ������");
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

    template <class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> ThreadPool::EnqueueRenderJob(
        F&& f, Args&&... args) {
        if (stop_all) {
            throw std::runtime_error("ThreadPool ��� ������");
        }
        //assert(f != nullptr);
        using return_type = typename std::result_of<F(Args...)>::type;
        auto job = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> job_result_future = job->get_future();
        {
            std::unique_lock<std::mutex> lock(m_render_job_q_);
            std::function<void()> pJob = [job]() { (*job)(); };
            assert(pJob);
            m_renderJobs_.push(pJob);
        }
        cv_render_job_q_.notify_one();

        return job_result_future;
    }
}