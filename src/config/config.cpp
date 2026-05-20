#include "config/config.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>

Config Config::from_file(const std::string& path) {
    Config cfg;
    std::ifstream f(path);
    if (!f.is_open()) return cfg;  // defaults

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string key, val;
        if (!(ss >> key >> val)) continue;

        if (key == "port")              cfg.port              = std::stoi(val);
        else if (key == "threads")      cfg.num_threads       = std::stoul(val);
        else if (key == "snapshot_interval")
                                        cfg.snapshot_interval = std::stoi(val);
        else if (key == "rdb_path")     cfg.rdb_path          = val;
        else if (key == "max_connections")
                                        cfg.max_connections   = std::stoul(val);
    }
    return cfg;
}

void Config::apply_args(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; i++) {
        std::string key = argv[i];
        std::string val = argv[i + 1];
        if (key == "--port")     { port        = std::stoi(val); i++; }
        if (key == "--threads")  { num_threads = std::stoul(val); i++; }
        if (key == "--rdb")      { rdb_path    = val; i++; }
    }
}

void Config::print() const {
    printf("  port              : %d\n",   port);
    printf("  threads           : %zu\n",  num_threads);
    printf("  snapshot_interval : %ds\n",  snapshot_interval);
    printf("  rdb_path          : %s\n",   rdb_path.c_str());
    printf("  max_connections   : %zu\n",  max_connections);
}