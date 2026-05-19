#include "dispatch/dispatcher.hpp"
#include "protocol/encoder.hpp"
#include "graph/bfs.hpp"
#include "graph/dfs.hpp"
#include "graph/dijkstra.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

static std::string to_upper(std::string s) {
    for (auto& c : s) c = toupper(c);
    return s;
}

std::string Dispatcher::dispatch(const std::vector<std::string>& args) {
    if (args.empty()) return RespEncoder::error("empty command");
    std::string cmd = to_upper(args[0]);
    if (cmd.substr(0, 6) == "GRAPH.") return handle_graph(cmd, args);
    if (cmd[0] == 'Z')               return handle_zset(cmd, args);

    return handle_kv(cmd, args);
}

std::string Dispatcher::handle_kv(const std::string& cmd,
                                   const std::vector<std::string>& args) {
    if (cmd == "PING")
        return RespEncoder::simple_string(
            args.size() >= 2 ? args[1] : "PONG");

    if (cmd == "SET" && args.size() >= 3) {
        Ms ttl_ms = -1;
        for (size_t i = 3; i + 1 < args.size(); i++) {
            std::string opt = to_upper(args[i]);
            if (opt == "EX")  ttl_ms = std::stoll(args[i+1]) * 1000;
            if (opt == "PX")  ttl_ms = std::stoll(args[i+1]);
        }
        kv_.set(args[1], args[2], ttl_ms);
        return RespEncoder::simple_string("OK");
    }

    if (cmd == "GET" && args.size() >= 2) {
        auto v = kv_.get(args[1]);
        return v ? RespEncoder::bulk_string(*v) : RespEncoder::null_bulk();
    }

    if (cmd == "DEL" && args.size() >= 2) {
        return RespEncoder::integer(kv_.del(args[1]) ? 1 : 0);
    }

    if (cmd == "EXISTS" && args.size() >= 2) {
        return RespEncoder::integer(kv_.exists(args[1]) ? 1 : 0);
    }

    if (cmd == "EXPIRE" && args.size() >= 3) {
        bool ok = kv_.expire(args[1], std::stoll(args[2]) * 1000);
        return RespEncoder::integer(ok ? 1 : 0);
    }

    if (cmd == "PEXPIRE" && args.size() >= 3) {
        bool ok = kv_.expire(args[1], std::stoll(args[2]));
        return RespEncoder::integer(ok ? 1 : 0);
    }

    if (cmd == "TTL" && args.size() >= 2) {
        Ms ms = kv_.ttl(args[1]);
        if (ms == -2) return RespEncoder::integer(-2);
        if (ms == -1) return RespEncoder::integer(-1);
        return RespEncoder::integer((ms + 999) / 1000);
    }

    if (cmd == "PTTL" && args.size() >= 2) {
        return RespEncoder::integer(kv_.ttl(args[1]));
    }

    if (cmd == "PERSIST" && args.size() >= 2) {
        return RespEncoder::integer(kv_.persist(args[1]) ? 1 : 0);
    }

    if (cmd == "DBSIZE")
        return RespEncoder::integer((long long)kv_.size());

    if (cmd == "COMMAND")
        return RespEncoder::simple_string("OK");

    return RespEncoder::error("unknown command '" + args[0] + "'");
}


std::string Dispatcher::handle_graph(const std::string& cmd,
                                      const std::vector<std::string>& args) {
    try {

        // GRAPH.ADD_NODE id label props
        if (cmd == "GRAPH.ADD_NODE" && args.size() >= 4) {
            int id = std::stoi(args[1]);
            graph_.add_node(id, args[2], args[3]);
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.ADD_NODE id label  (props optional)
        if (cmd == "GRAPH.ADD_NODE" && args.size() == 3) {
            int id = std::stoi(args[1]);
            graph_.add_node(id, args[2], "{}");
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.DEL_NODE id
        if (cmd == "GRAPH.DEL_NODE" && args.size() >= 2) {
            graph_.remove_node(std::stoi(args[1]));
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.ADD_EDGE from to rel [weight]
        if (cmd == "GRAPH.ADD_EDGE" && args.size() >= 4) {
            int   from   = std::stoi(args[1]);
            int   to     = std::stoi(args[2]);
            float weight = args.size() >= 5 ? std::stof(args[4]) : 1.0f;
            graph_.add_edge(from, to, args[3], weight);
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.DEL_EDGE from to rel
        if (cmd == "GRAPH.DEL_EDGE" && args.size() >= 4) {
            graph_.remove_edge(std::stoi(args[1]),
                               std::stoi(args[2]), args[3]);
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.NODE id — get node metadata
        if (cmd == "GRAPH.NODE" && args.size() >= 2) {
            auto* meta = graph_.node_meta(std::stoi(args[1]));
            if (!meta) return RespEncoder::null_bulk();
            std::vector<std::string> out = {"label", meta->label,
                                            "props", meta->props};
            return RespEncoder::array(out);
        }

        // GRAPH.HAS_NODE id
        if (cmd == "GRAPH.HAS_NODE" && args.size() >= 2) {
            return RespEncoder::integer(
                graph_.has_node(std::stoi(args[1])) ? 1 : 0);
        }

        // GRAPH.HAS_EDGE from to
        if (cmd == "GRAPH.HAS_EDGE" && args.size() >= 3) {
            return RespEncoder::integer(
                graph_.has_edge(std::stoi(args[1]),
                                std::stoi(args[2])) ? 1 : 0);
        }

        // GRAPH.NEIGHBORS id
        if (cmd == "GRAPH.NEIGHBORS" && args.size() >= 2) {
            auto& edges = graph_.neighbors(std::stoi(args[1]));
            std::vector<std::string> out;
            for (auto& e : edges) {
                out.push_back(std::to_string(e.to));
                out.push_back(e.rel_type);
                out.push_back(std::to_string(e.weight));
            }
            return RespEncoder::array(out);
        }

        // GRAPH.REVNEIGHBORS id
        if (cmd == "GRAPH.REVNEIGHBORS" && args.size() >= 2) {
            auto& edges = graph_.reverse_neighbors(std::stoi(args[1]));
            std::vector<std::string> out;
            for (auto& e : edges) {
                out.push_back(std::to_string(e.to));
                out.push_back(e.rel_type);
            }
            return RespEncoder::array(out);
        }

        // GRAPH.LABEL_NODES label
        if (cmd == "GRAPH.LABEL_NODES" && args.size() >= 2) {
            auto ids = graph_.nodes_by_label(args[1]);
            std::vector<std::string> out;
            for (int id : ids) out.push_back(std::to_string(id));
            return RespEncoder::array(out);
        }

        // GRAPH.PATH src dst [rel]  — BFS unweighted
        if (cmd == "GRAPH.PATH" && args.size() >= 3) {
            int src = std::stoi(args[1]);
            int dst = std::stoi(args[2]);
            std::string rel = args.size() >= 4 ? args[3] : "";
            auto path = bfs_path(graph_.graph_ref(), src, dst, rel);
            if (path.empty()) return RespEncoder::null_bulk();
            std::vector<std::string> out;
            for (int id : path.nodes) out.push_back(std::to_string(id));
            return RespEncoder::array(out);
        }

        // GRAPH.WPATH src dst [rel]  — Dijkstra weighted
        if (cmd == "GRAPH.WPATH" && args.size() >= 3) {
            int src = std::stoi(args[1]);
            int dst = std::stoi(args[2]);
            std::string rel = args.size() >= 4 ? args[3] : "";
            auto path = dijkstra_path(graph_.graph_ref(), src, dst, rel);
            if (path.empty()) return RespEncoder::null_bulk();
            std::vector<std::string> out;
            out.push_back("cost");
            out.push_back(std::to_string(path.cost));
            out.push_back("path");
            for (int id : path.nodes) out.push_back(std::to_string(id));
            out.push_back("rels");
            for (auto& r : path.rels) out.push_back(r);
            return RespEncoder::array(out);
        }

        // GRAPH.NEIGHBORHOOD src hops [rel]
        if (cmd == "GRAPH.NEIGHBORHOOD" && args.size() >= 3) {
            int src  = std::stoi(args[1]);
            int hops = std::stoi(args[2]);
            std::string rel = args.size() >= 4 ? args[3] : "";
            auto results = bfs_neighborhood(graph_.graph_ref(),
                                            src, hops, rel);
            std::vector<std::string> out;
            for (auto& r : results) {
                out.push_back(std::to_string(r.node));
                out.push_back(std::to_string(r.hop));
                out.push_back(std::to_string(r.via));
                out.push_back(r.rel_type);
            }
            return RespEncoder::array(out);
        }

        // GRAPH.COMPONENT src
        if (cmd == "GRAPH.COMPONENT" && args.size() >= 2) {
            auto comp = bfs_component(graph_.graph_ref(),
                                      std::stoi(args[1]));
            std::vector<std::string> out;
            for (int id : comp) out.push_back(std::to_string(id));
            return RespEncoder::array(out);
        }

        // GRAPH.HAS_CYCLE
        if (cmd == "GRAPH.HAS_CYCLE") {
            bool cycle = dfs_has_cycle(graph_.graph_ref(),
                                       graph_.node_count());
            return RespEncoder::integer(cycle ? 1 : 0);
        }

        // GRAPH.DISTANCES src
        if (cmd == "GRAPH.DISTANCES" && args.size() >= 2) {
            auto dmap = dijkstra_all(graph_.graph_ref(),
                                     std::stoi(args[1]));
            std::vector<std::string> out;
            for (auto& [node, dist] : dmap.dist) {
                out.push_back(std::to_string(node));
                out.push_back(std::to_string(dist));
            }
            return RespEncoder::array(out);
        }

        // GRAPH.INFO
        if (cmd == "GRAPH.INFO") {
            std::vector<std::string> out = {
                "nodes", std::to_string(graph_.node_count()),
                "edges", std::to_string(graph_.edge_count())
            };
            return RespEncoder::array(out);
        }

        // GRAPH.INDEX_PROP id label prop value
        if (cmd == "GRAPH.INDEX_PROP" && args.size() >= 5) {
            graph_.index_property(args[2], args[3],
                                std::stod(args[4]),
                                std::stoi(args[1]));
            return RespEncoder::simple_string("OK");
        }

        // GRAPH.RANGE label prop min max
        if (cmd == "GRAPH.RANGE" && args.size() >= 5) {
            auto ids = graph_.range_query(args[1], args[2],
                                        std::stod(args[3]),
                                        std::stod(args[4]));
            std::vector<std::string> out;
            for (int id : ids) out.push_back(std::to_string(id));
            return RespEncoder::array(out);
        }

        // GRAPH.EXACT label prop value
        if (cmd == "GRAPH.EXACT" && args.size() >= 4) {
            auto ids = graph_.exact_query(args[1], args[2], std::stod(args[3]));
            std::vector<std::string> out;
            for (int id : ids) out.push_back(std::to_string(id));
            return RespEncoder::array(out);
        }

    } catch (const std::exception& e) {
        return RespEncoder::error(std::string("bad args: ") + e.what());
    }

    return RespEncoder::error("unknown graph command '" + args[0] + "'");
}

std::string Dispatcher::handle_zset(const std::string& cmd,
                                     const std::vector<std::string>& args) {
    try {
        // ZADD key score member
        if (cmd == "ZADD" && args.size() >= 4) {
            int added = zsets_.zadd(args[1], std::stod(args[2]), args[3]);
            return RespEncoder::integer(added);
        }

        // ZREM key member
        if (cmd == "ZREM" && args.size() >= 3) {
            return RespEncoder::integer(
                zsets_.zrem(args[1], args[2]) ? 1 : 0);
        }

        // ZSCORE key member
        if (cmd == "ZSCORE" && args.size() >= 3) {
            auto sc = zsets_.zscore(args[1], args[2]);
            return sc ? RespEncoder::bulk_string(std::to_string(*sc))
                      : RespEncoder::null_bulk();
        }

        // ZRANGE key start stop [WITHSCORES]
        if (cmd == "ZRANGE" && args.size() >= 4) {
            bool with_scores = args.size() >= 5 &&
                               to_upper(args[4]) == "WITHSCORES";
            auto results = zsets_.zrange(args[1],
                                          std::stoi(args[2]),
                                          std::stoi(args[3]));
            std::vector<std::string> out;
            for (auto& [score, member] : results) {
                out.push_back(member);
                if (with_scores) out.push_back(std::to_string(score));
            }
            return RespEncoder::array(out);
        }

        // ZRANGEBYSCORE key min max
        if (cmd == "ZRANGEBYSCORE" && args.size() >= 4) {
            auto results = zsets_.zrangebyscore(args[1],
                                                 std::stod(args[2]),
                                                 std::stod(args[3]));
            std::vector<std::string> out;
            for (auto& [score, member] : results) {
                out.push_back(member);
                out.push_back(std::to_string(score));
            }
            return RespEncoder::array(out);
        }

        // ZCARD key
        if (cmd == "ZCARD" && args.size() >= 2)
            return RespEncoder::integer(zsets_.zcard(args[1]));

        // ZRANK key member
        if (cmd == "ZRANK" && args.size() >= 3)
            return RespEncoder::integer(
                zsets_.zrank(args[1], args[2]));

    } catch (const std::exception& e) {
        return RespEncoder::error(std::string("bad args: ") + e.what());
    }

    return RespEncoder::error("unknown command '" + args[0] + "'");
}