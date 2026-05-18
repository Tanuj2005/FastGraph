#pragma once
#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <unistd.h>
#include <chrono>
#include <queue>

using Handler = std::function<void( int, uint32_t )> ;
using Ms = int64_t ;
using Clock = std::chrono::steady_clock ;
using TimePoint = Clock::time_point ;

struct Timer {
    TimePoint when ;
    std::function<void()> cb ;
    bool operator>(const Timer& o) const { return when > o.when; }

};

class Reactor {
public:
    Reactor() {
        epfd_ = epoll_create1( EPOLL_CLOEXEC ) ;
        if ( epfd_ < 0 ) throw std::runtime_error( "epoll_create1 failed" ) ;

    }

    ~Reactor() { close( epfd_ ) ; }

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

    void remove( int fd, uint32_t events ) {
        handlers_.erase( fd ) ;
        epoll_ctl( epfd_, EPOLL_CTL_DEL, fd, nullptr ) ;

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
            auto it = handlers_.find( fd ) ;
            if ( it != handlers_.end() )
                it->second( fd, events[i].events ) ;
        }
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
    std::unordered_map<int, Handler> handlers_ ;
    std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer>> timers_ ;

};
