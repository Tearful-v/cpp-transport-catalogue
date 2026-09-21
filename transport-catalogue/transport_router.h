#pragma once

#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

namespace transport_router {

    struct RouterSettings {
        double wait_time;
        double bus_speed;
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


    private:
        RouterSettings settings_;
        Graph BuildGraph(const transport_catalogue::TransportCatalogue& catalogue);

        Graph graph_;
        Router router_;
    };

} //transport_router
