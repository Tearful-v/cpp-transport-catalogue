#pragma once

#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace transport_router {

    struct RouterSettings {
        double wait_time;
        double bus_speed;
    };

    struct RouteResult {
        double total_time = 0.0;
        std::vector<graph::EdgeInfo> edges;
    };

    class TransportRouter {
    public:
        using Weight = double;
        using Graph = graph::DirectedWeightedGraph<Weight>;
        using Router = graph::Router<Weight>;

        explicit TransportRouter (RouterSettings settings,
                const transport_catalogue::TransportCatalogue& catalogue) :
                settings_(std::move(settings)), graph_(BuildGraph(catalogue)),
                router_(graph_) {}

        std::optional<RouteResult> BuildRoute(std::string_view from,
                                std::string_view to) const;

        double GetWaitTime() const {
            return settings_.wait_time;
        }

    private:
        RouterSettings settings_;
        Graph BuildGraph(const transport_catalogue::TransportCatalogue& catalogue);

        std::unordered_map<std::string_view, graph::VertexId> stop_to_vertex_;
        std::vector<graph::EdgeInfo> info_;

        Graph graph_;
        Router router_;
    };

} //transport_router
