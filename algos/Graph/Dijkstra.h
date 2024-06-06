#pragma once

#include <optional>
#include <vector>
#include <functional>
#include <queue>
#include <unordered_map>

namespace Dijkstra {
    enum COLOR { NONE = 0x0, WHITE = 0x1, GRAY = 0x2, BLACK = 0x4 };

    template<typename T, size_t(*costFunction)(const T&, const T&)>
    struct Node {
        const T data;
        const std::vector<const T*> adj;

        COLOR col = WHITE;
        int dist = -1;
        Node* prev = nullptr;


        void Relax(Node& other) const {
            auto dist_via_this = this->dist + costFunction(*this, other);
            if (other.dist > dist_via_this) {
                other->dist = dist_via_this;
                other->prev = this;
            }
        }


        static std::vector<Node<T, costFunction>> to_nodes(const std::vector<T>& input, const std::vector<std::vector<const T*>>& adj) {
            std::vector<Node<T, costFunction>> out;
            out.reserve(input.size());
            for (size_t i = 0; i < input.size(); i++) out.push_back(Node{ input[i], adj[i] });
            return out;
        }
    };



    /*template<typename T, size_t(*costFunction)(const T&, const T&)>
    std::vector<Node<T, costFunction>> dijkstra(const std::vector<Node<T, costFunction>>& nodes, const T& start) {

        std::unordered_map<AffiliationID, Node> nodes = {};
        for (const auto& affiliation : get_all_affiliations()) nodes[affiliation] = Node{ affiliation };

        struct NodeCompare {
            bool operator()(const Node* l, const Node* r) const { return l->dist > r->dist; }
        };
        std::priority_queue<Node*, std::vector<Node*>, NodeCompare> queue;

        Node* s = &nodes[start];
        *s = Node{ start, 0x01, 0, nullptr };

        Node* u;
        Node* v;

        queue.push(s);

        while (!queue.empty()) {
            u = queue.top();
            queue.pop();
            if (!u->state) continue;

            for (const auto& adj_data : get_connected(u->affiliation)) {
                v = &nodes[adj_data];
                Relax(u, v);
                if (v->state == 0x01) {
                    v->state = 0x02;
                    queue.push(v);
                }
            }
            u->state = 0x0;
        }

        std::optional<std::vector<AffiliationID>> result;
        std::vector<AffiliationID> path = {};
        Node tmp = nodes[target];
        while (tmp.prev) {
            path.push_back(tmp.affiliation);
            tmp = *tmp.prev;
        }
        path.push_back(start);

        return (result = path);
    }*/

}
