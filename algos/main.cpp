#include <iostream>
#include <utility>

#include "Graph/Dijkstra.h"

typedef int vertex;
typedef std::vector<const vertex*> adj_list;

size_t costF(const vertex& f, const vertex& s) {
	return 1;
}

std::vector<vertex> verts{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

#define VERT(n) &verts.at(n - 1)
adj_list v1{ VERT(2), VERT(3) };
adj_list v2{ VERT(3) };
adj_list v3{ VERT(4), VERT(5), VERT(6) };
adj_list v4{ VERT(7), VERT(8) };
adj_list v5{ VERT(8) };
adj_list v6{ VERT(9) };
adj_list v7{ VERT(9) };
adj_list v8{ };
adj_list v9{ VERT(1), VERT(2) };
std::vector<std::vector<const vertex*>> adj{ v1, v2, v3, v4, v5, v6, v7, v8, v9 };

std::unordered_map<vertex, adj_list> graph{ {1, v1}, {2, v2}, {3, v3}, {4, v4}, {5, v5},{6, v6}, {7, v7}, {8, v8}, {9, v9} };

int main(void) {
	std::vector<Dijkstra::Node<vertex, costF>> nodes = Dijkstra::Node<vertex, costF>::to_nodes(verts, adj);

	vertex* node = &verts.at(0);
	while (true) {
		std::cout << *node << std::endl;
		if (graph.at(*node).empty()) break;
		node = (vertex*)graph.at(*node).at(0);
	}

	return 0;
}