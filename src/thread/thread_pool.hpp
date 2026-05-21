#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
#include <atomic>
#include <stdexcept>

class ThreadPool {
public:
    explicit ThreadPool( size_t num_threads ) {
        if ( num_threads == 0 )
            throw std::runtime_error( "need at least 1 thread" ) ;
        workers_.reserve( num_threads ) ;

        for ( size_t i = 0 ; i < num_threads ; i++ )
            workers_.emplace_back( [this] { worker_loop() ; }) ;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_) ;
            stop_ = true ;
        }
        cv_.notify_all() ;
        for (auto& t : workers_) t.join() ;
    }

    ThreadPool(const ThreadPool&) = delete ;
    ThreadPool& operator=(const ThreadPool&) = delete ;

    void submit( std::function<void()> task ) {
        {
            std::unique_lock<std::mutex> lock(mutex_) ;
            if (stop_) throw std::runtime_error("pool is stopped") ;
            queue_.push(std::move(task)) ;
        }
        cv_.notify_one() ;
    }

    size_t queue_size() const {
        std::unique_lock<std::mutex> lock(mutex_) ;
        return queue_.size() ; 
    }

private:   
    void worker_loop() {
        while ( true ) {
            std::function<void()> task ;
            {
                std::unique_lock<std::mutex> lock( mutex_ ) ;
                cv_.wait( lock, [this] {
                    return stop_ || !queue_.empty() ;
                }) ;
                if ( stop_ && queue_.empty()) return ;
                task = std::move( queue_.front()) ;
                queue_.pop() ;
            }

            task() ;
        }
    }

    std::vector<std::thread> workers_ ;
    std::queue<std::function<void()>> queue_ ;
    mutable std::mutex mutex_ ;
    std::condition_variable cv_ ;
    bool stop_ = false ;

};