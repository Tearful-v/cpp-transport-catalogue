#pragma once
#include "geo.h"

#include <cstddef>
#include <string>
#include <vector>

namespace domain {

    struct Stop {
        std::string name;
        geo::Coordinates coords = {0, 0};
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> route;
        bool is_roundtrip = false;
    };

    struct BusInfo {
        int route_length = 0;
        size_t stops = 0;
        size_t unique_stops = 0;
        double curvature = 0.0;
    };

}// domain
