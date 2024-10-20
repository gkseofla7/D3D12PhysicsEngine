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
#include "ThreadPool.h"
#include <iostream>
namespace hlab {

    ThreadPool::ThreadPool(size_t num_threads)
        : num_threads_(num_threads), stop_all(false) {
        worker_threads_.reserve(num_threads_);
        for (size_t i = 0; i < num_threads_; ++i) {
            worker_threads_.emplace_back([this]() { this->WorkerThread(); });
        }

        m_renderThread = std::thread(([this]() { this->RenderThread(); }));
    }
    void ThreadPool::WorkerThread() {
        while (true) {
            std::unique_lock<std::mutex> lock(m_job_q_);
            cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
            if (stop_all && this->jobs_.empty()) {
                return;
            }

            // 맨 앞의 job 을 뺀다.
            std::function<void()> job = std::move(jobs_.front());
            jobs_.pop();
            lock.unlock();

            // 해당 job 을 수행한다 :)
            job();
        }
    }

    void ThreadPool::RenderThread() {
        while (true) {
            
            std::unique_lock<std::mutex> lock(m_render_job_q_);
            // renderjob이 있으면 깨움
            cv_render_job_q_.wait(lock, [this]() { return (!this->m_renderJobs_.empty()&& !m_isMainThreadUsingRendering) || stop_all; });
            if (stop_all && this->m_renderJobs_.empty()) {
                return;
            }
            m_finishRenderThread = false;
            std::function<void()> job = std::move(m_renderJobs_.front());
            assert(job);
            m_renderJobs_.pop();
            lock.unlock();
            job();

            if (m_renderJobs_.empty())
            {
                m_finishRenderThread = true;
                cv_render_job_q_.notify_one();
            }
        }
    }
    ThreadPool::~ThreadPool() {
        stop_all = true;
        cv_job_q_.notify_all();

        for (auto& t : worker_threads_) {
            t.join();
        }
        m_renderThread.join();
    }


}  // namespace ThreadPool
