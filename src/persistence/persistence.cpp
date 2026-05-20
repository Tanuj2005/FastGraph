#include "persistence/persistence.hpp"
#include "persistence/rdb.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <thread>

bool Persistence::save() {
    saving_ = true;
    bool ok = RDB::save(path_, kv_, graph_);
    saving_ = false;
    if (ok) printf("[persistence] saved to %s\n", path_.c_str());
    else    printf("[persistence] save FAILED\n");
    return ok;
}

bool Persistence::load() {
    bool ok = RDB::load(path_, kv_, graph_);
    if (ok) printf("[persistence] loaded from %s\n", path_.c_str());
    else    printf("[persistence] no snapshot found, starting fresh\n");
    return ok;
}

bool Persistence::bgsave() {
    if (saving_.load()) {
        printf("[persistence] bgsave already in progress\n");
        return false;
    }
    pid_t pid = fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // Child — save and exit
        bool ok = RDB::save(path_, kv_, graph_);
        _exit(ok ? 0 : 1);
    }

    // Parent — reap child asynchronously via SIGCHLD or just detach
    // Simple approach: detach (child becomes zombie briefly, OS cleans it)
    saving_ = true;
    // Register SIGCHLD handler to reset saving_ flag
    // For simplicity just spawn a waiter thread
    std::thread([this, pid]() {
        int status = 0;
        waitpid(pid, &status, 0);
        saving_ = false;
        printf("[persistence] bgsave complete, status=%d\n",
               WEXITSTATUS(status));
    }).detach();

    return true;
}