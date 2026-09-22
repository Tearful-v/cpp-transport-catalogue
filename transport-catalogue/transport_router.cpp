#include "transport_router.h"

namespace transport_router {
    using Weight = double;
    using Graph = graph::DirectedWeightedGraph<Weight>;
    using Router = graph::Router<Weight>;
    using VertexId = size_t;

    Graph TransportRouter::BuildGraph(const transport_catalogue::TransportCatalogue& catalogue) {
        const auto& stops = catalogue.GetAllStops();
        Graph graph(stops.size());
        VertexId id = 0;

        for (const auto& stop : stops) {
            stop_to_vertex_[stop.name] = id;
            ++id;
        }

        const double meters_per_minute = settings_.bus_speed * 1000.0 / 60.0;

        const auto& buses = catalogue.GetAllBuses();
        for (const auto& bus : buses) {
            const auto& route = bus.route;

            for (size_t from = 0; from < route.size(); ++from) {
                int total_distance = 0;

                for (size_t to = from + 1; to < route.size(); ++to) {
                    total_distance += catalogue.GetStopsDistance(route[to - 1]->name, route[to]->name);
                    const double ride_time = total_distance / meters_per_minute;
                    const double total_time = settings_.wait_time + ride_time;
                    graph.AddEdge({stop_to_vertex_[route[from]->name],
                                stop_to_vertex_[route[to]->name],
                                total_time});

                    info_.push_back({
                        bus.name,
                        route[from]->name,
                        static_cast<int>(to - from),
                        ride_time
                    });
                }
            }
        }

        return graph;
    }

    std::optional<RouteResult> TransportRouter::BuildRoute(
                            std::string_view from, std::string_view to) const {
        auto it_from = stop_to_vertex_.find(from);
        auto it_to = stop_to_vertex_.find(to);
        if (it_from == stop_to_vertex_.end()
            || it_to == stop_to_vertex_.end()) {
                return std::nullopt;
            }

        const auto route = router_.BuildRoute(
            it_from->second,
            it_to->second
        );

        if(!route) {
            return std::nullopt;
        }

        RouteResult result;
        result.total_time = route->weight;
        result.edges.reserve(route->edges.size());
        for (graph::EdgeId edge_id : route->edges) {
            result.edges.push_back(info_[edge_id]);
        }

        return result;
    }

}
