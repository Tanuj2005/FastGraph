#include "net/server.hpp"
#include <cstdio>

int main() {
    Server server(6379);
    server.start();
    return 0;
}