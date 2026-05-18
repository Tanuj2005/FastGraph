#pragma once
#include "net/reactor.hpp"
#include "net/conn.hpp"
#include "kv/kv_engine.hpp"
#include "dispatch/dispatcher.hpp"
#include <unordered_map>
#include "graph/graph_engine.hpp"

class Server {
public:
    explicit Server(int port);
    void start();

private:
    void on_accept();
    void on_io(int fd, uint32_t events);
    void do_read(Conn* c);
    void do_write(Conn* c);
    void try_parse(Conn* c);
    void close_conn(Conn* c);
    void schedule_tick();

    static int  make_listen_fd(int port);
    static void set_nonblocking(int fd);

    int         port_;
    int         listen_fd_;
    Reactor     reactor_;
    KVEngine    kv_;
    GraphEngine graph_;
    Dispatcher  dispatcher_;
    std::unordered_map<int, Conn*> conns_;
};