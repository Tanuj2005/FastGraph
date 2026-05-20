#include "net/server.hpp"
#include "protocol/encoder.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

Server::Server(int port, size_t num_threads)
    : port_(port), listen_fd_(-1), dispatcher_(kv_, graph_, zsets_), pool_(num_threads)  {}

void Server::start() {
    listen_fd_ = make_listen_fd(port_);

    reactor_.add(listen_fd_, EPOLLIN, [this](int, uint32_t) {
        on_accept();
    });

    schedule_tick();
    printf("FastGraph listening on :%d\n", port_);
    reactor_.run();
}

void Server::on_accept() {
    while (true) {
        int fd = accept(listen_fd_, nullptr, nullptr);
        if (fd < 0) break;
        set_nonblocking(fd);
        Conn* c = new Conn(fd);
        conns_[fd] = c;
        reactor_.add(fd, EPOLLIN, [this](int fd, uint32_t events) {
            on_io(fd, events);
        });
    }
}

void Server::on_io(int fd, uint32_t events) {
    auto it = conns_.find(fd);
    if (it == conns_.end()) return;
    Conn* c = it->second;
    if (events & EPOLLIN)  do_read(c);
    if (events & EPOLLOUT) do_write(c);
    if (c->state == ConnState::Closing) close_conn(c);
}

void Server::do_read(Conn* c) {
    char buf[4096];
    while (true) {
        ssize_t n = read(c->fd, buf, sizeof(buf));
        if (n > 0) {
            c->read_buf.append(buf, n);
        } else if (n == 0) {
            c->state = ConnState::Closing; return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            c->state = ConnState::Closing; return;
        }
    }
    try_parse(c);
}

void Server::try_parse(Conn* c) {
    while (true) {
        std::vector<std::string> args;
        auto res = c->parser.parse(c->read_buf, args);
        if (res == ParseResult::Incomplete) break;
        if (res == ParseResult::Error) {
            c->write_buf += RespEncoder::error("protocol error");
            c->state = ConnState::Closing;
            break;
        }

        // Offload heavy graph traversals to thread pool
        std::string cmd = args.empty() ? "" : args[0];
        for (auto& ch : cmd) ch = toupper(ch);

        bool heavy = (cmd == "GRAPH.PATH"   ||
                      cmd == "GRAPH.WPATH"  ||
                      cmd == "GRAPH.DISTANCES" ||
                      cmd == "GRAPH.COMPONENT" ||
                      cmd == "GRAPH.NEIGHBORHOOD");

        if (heavy) {
            int fd = c->fd;
            pool_.submit([this, args, c]() {
                std::string result = dispatcher_.dispatch(args);
                reactor_.post([this, c, result]() {
                    // Check conn still alive
                    if (conns_.find(c->fd) == conns_.end()) return;
                    c->write_buf += result;
                    c->state = ConnState::Writing;
                    reactor_.modify(c->fd, EPOLLIN | EPOLLOUT);
                });
            });
            // Don't add to write_buf here — async
            continue;
        }

        c->write_buf += dispatcher_.dispatch(args);
    }
    if (!c->write_buf.empty()) {
        c->state = ConnState::Writing;
        reactor_.modify(c->fd, EPOLLIN | EPOLLOUT);
    }
}

void Server::do_write(Conn* c) {
    while (!c->write_buf.empty()) {
        ssize_t n = write(c->fd, c->write_buf.data(), c->write_buf.size());
        if (n > 0) {
            c->write_buf.erase(0, n);
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            c->state = ConnState::Closing; return;
        }
    }
    if (c->state != ConnState::Closing) {
        c->state = ConnState::Reading;
        reactor_.modify(c->fd, EPOLLIN);
    }
}

void Server::close_conn(Conn* c) {
    reactor_.remove(c->fd);
    close(c->fd);
    conns_.erase(c->fd);
    delete c;
}

void Server::schedule_tick() {
    reactor_.add_timer(100, [this]() {
        kv_.tick();
        schedule_tick();
    });
}

int Server::make_listen_fd(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);
    bind(fd, (sockaddr*)&addr, sizeof(addr));
    listen(fd, 128);
    set_nonblocking(fd);
    return fd;
}

void Server::set_nonblocking(int fd) {
    int f = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, f | O_NONBLOCK);
}