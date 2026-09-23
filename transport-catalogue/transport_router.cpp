#include "transport_router.h"

namespace transport_router {

namespace {

constexpr double METERS_PER_KILOMETER = 1000.0;
constexpr double MINUTES_PER_HOUR = 60.0;

}  // namespace

TransportRouter::Graph TransportRouter::BuildGraph(
    const transport_catalogue::TransportCatalogue& catalogue) {
    AssignVertexIds(catalogue);

    Graph graph(catalogue.GetAllStops().size());
    AddBusEdges(graph, catalogue);
    return graph;
}

void TransportRouter::AssignVertexIds(
    const transport_catalogue::TransportCatalogue& catalogue) {
    graph::VertexId id = 0;
    for (const auto& stop : catalogue.GetAllStops()) {
        stop_to_vertex_.emplace(stop.name, id++);
    }
}

void TransportRouter::AddBusEdges(
    Graph& graph, const transport_catalogue::TransportCatalogue& catalogue) {
    const double meters_per_minute =
        settings_.bus_speed * METERS_PER_KILOMETER / MINUTES_PER_HOUR;

    for (const auto& bus : catalogue.GetAllBuses()) {
        const auto& route = bus.route;
        for (size_t from = 0; from < route.size(); ++from) {
            int total_distance = 0;
            for (size_t to = from + 1; to < route.size(); ++to) {
                total_distance += catalogue.GetStopsDistance(route[to - 1]->name,
                                                            route[to]->name);
                const double ride_time = total_distance / meters_per_minute;
                const double total_time = settings_.wait_time + ride_time;
                const graph::EdgeId edge_id = graph.AddEdge({
                    stop_to_vertex_.at(route[from]->name),
                    stop_to_vertex_.at(route[to]->name),
                    total_time,
                });

                info_.emplace(edge_id,
                            EdgeInfo{bus.name,
                                    route[from]->name,
                                    static_cast<int>(to - from),
                                    ride_time});
            }
        }
    }
}

std::optional<transport_router::RouteResult> TransportRouter::BuildRoute(
    std::string_view from, std::string_view to) const {
    const auto it_from = stop_to_vertex_.find(from);
    const auto it_to = stop_to_vertex_.find(to);
    if (it_from == stop_to_vertex_.end() || it_to == stop_to_vertex_.end()) {
        return std::nullopt;
    }

    const auto route = router_.BuildRoute(it_from->second, it_to->second);
    if (!route) {
        return std::nullopt;
    }

    transport_router::RouteResult result;
    result.total_time = route->weight;
    result.items.reserve(route->edges.size() * 2);
    for (graph::EdgeId edge_id : route->edges) {
        const EdgeInfo& edge = info_.at(edge_id);
        result.items.emplace_back(
            transport_router::WaitItem{edge.wait_stop, settings_.wait_time});
        result.items.emplace_back(
            transport_router::BusItem{edge.bus_name, edge.span_count, edge.ride_time});
    }

    return result;
}

}  //transport_router
