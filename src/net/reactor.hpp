#pragma once
#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <chrono>
#include <queue>
#include <sys/eventfd.h>
#include <mutex>

using Handler = std::function<void( int, uint32_t )> ;
using Ms = int64_t ;
using Clock = std::chrono::steady_clock ;
using TimePoint = Clock::time_point ;

struct Timer {
    TimePoint when ;
    std::function<void()> cb ;
    bool operator>(const Timer& o) const { return when > o.when ; }

};

class Reactor {
public:

    Reactor() {

        epfd_ = epoll_create1( EPOLL_CLOEXEC ) ;
        event_fd_ = eventfd(0, EFD_NONBLOCK) ;

        // Register it with epoll
        epoll_event ev{} ;
        ev.events  = EPOLLIN ;
        ev.data.fd = event_fd_ ;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, event_fd_, &ev) ;
        if ( epfd_ < 0 ) throw std::runtime_error( "epoll_create1 failed" ) ;

    }

    ~Reactor() { close( epfd_ ) ; close(event_fd_) ; }

    // Disable copying
    Reactor( const Reactor& ) = delete ;
    Reactor& operator = ( const Reactor& ) = delete ;

    void add( int fd, uint32_t events, Handler h ) {
        handlers_[fd] = std::move( h ) ;
        epoll_event ev{} ;
        ev.events = events ;
        ev.data.fd = fd ;
        epoll_ctl( epfd_, EPOLL_CTL_ADD, fd, &ev ) ;
    }

    void modify( int fd, uint32_t events ) {
        epoll_event ev{} ;
        ev.events = events ;
        ev.data.fd = fd ;
        epoll_ctl( epfd_, EPOLL_CTL_MOD, fd, &ev ) ;
    }

    void remove(int fd) {
        handlers_.erase(fd) ;
        epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) ;
    }

    void add_timer( int delay_ms, std::function<void()> cb ) {
        timers_.push( {Clock::now() + std::chrono::milliseconds( delay_ms ), std::move( cb )} ) ;
    }

    void poll( int timeout_ms = -1 ) {
        if ( !timers_.empty() ) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timers_.top().when - Clock::now() ).count() ;
            int t = (int) std::max<int64_t>(0, ms) ;
            if ( timeout_ms < 0 || t < timeout_ms ) timeout_ms = t ;
        }

        epoll_event events[64] ;
        int n = epoll_wait( epfd_ , events, 64, timeout_ms ) ;
        fire_timers() ;
        for ( int i = 0 ; i <  n ; i++ ) {
            int fd = events[i].data.fd ;
            if ( fd == event_fd_ ) {
                // Drain eventfd
                uint64_t val;
                read(event_fd_, &val, sizeof(val));
                // Run all posted callbacks
                std::queue<std::function<void()>> pending ;
                {
                    std::unique_lock<std::mutex> lock(post_mutex_) ;
                    std::swap(pending, posted_) ;
                }
                while (!pending.empty()) {
                    pending.front()() ;
                    pending.pop() ;
                }
                continue ;
            }
            auto it = handlers_.find( fd ) ;
            if ( it != handlers_.end() )
                it->second( fd, events[i].events ) ;
        }
    }
    // Post a callback to run on the event loop thread (thread-safe)
    void post(std::function<void()> cb) {
        {
            std::unique_lock<std::mutex> lock(post_mutex_) ;
            posted_.push(std::move(cb)) ;
        }
        // Wake epoll
        uint64_t val = 1;
        write(event_fd_, &val, sizeof(val));
    }
    void run() { running_ = true; while ( running_ ) poll() ; }
    void stop() { running_ = false ; }

private:
    void fire_timers() {
        auto now = Clock::now() ;
        while ( !timers_.empty() && timers_.top().when <= now ) {
            auto cb = timers_.top().cb ;
            timers_.pop() ;
            cb() ;
        }
    }

    int epfd_ ;
    bool running_ = false ;
    int event_fd_ = -1 ;
    std::queue<std::function<void()>> posted_ ;
    mutable std::mutex post_mutex_ ;
    std::unordered_map<int, Handler> handlers_ ;
    std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer>> timers_ ;

};
