#pragma once

#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace transport_router {

struct RouterSettings {
    double wait_time;
    double bus_speed;
};

struct WaitItem {
    std::string_view stop_name;
    double time = 0.0;
};

struct BusItem {
    std::string_view bus_name;
    int span_count = 0;
    double time = 0.0;
};

using RouteItem = std::variant<WaitItem, BusItem>;

struct RouteResult {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

class TransportRouter {
public:
    using Weight = double;
    using Graph = graph::DirectedWeightedGraph<Weight>;
    using Router = graph::Router<Weight>;

    explicit TransportRouter(RouterSettings settings,
                             const transport_catalogue::TransportCatalogue& catalogue)
        : settings_(std::move(settings))
        , graph_(BuildGraph(catalogue))
        , router_(graph_) {
    }

    std::optional<RouteResult> BuildRoute(std::string_view from, std::string_view to) const;

private:
    struct EdgeInfo {
        std::string_view bus_name;
        std::string_view wait_stop;
        int span_count = 0;
        double ride_time = 0.0;
    };

    Graph BuildGraph(const transport_catalogue::TransportCatalogue& catalogue);
    void AssignVertexIds(const transport_catalogue::TransportCatalogue& catalogue);
    void AddBusEdges(Graph& graph, const transport_catalogue::TransportCatalogue& catalogue);

    RouterSettings settings_;
    std::unordered_map<std::string_view, graph::VertexId> stop_to_vertex_;
    std::unordered_map<graph::EdgeId, EdgeInfo> info_;

    Graph graph_;
    Router router_;
};

}  // namespace transport_router
