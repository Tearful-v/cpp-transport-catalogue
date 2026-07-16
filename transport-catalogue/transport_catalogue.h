#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coords = {0, 0};
    };

    struct Bus {
        std::string name;
        std::vector<const Stop*> route;
    };

    struct BusInfo {
        double route_length = 0.0;
        size_t stops = 0;
        size_t unique_stops = 0;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string name, geo::Coordinates coords);
        void AddBus(std::string name, const std::vector<std::string_view>& stop_names);

        const Stop* FindStop(std::string_view name) const;
        const Bus* FindBus(std::string_view name) const;
        const std::unordered_set<std::string_view>& GetBusesForStop(std::string_view stop_name) const;
        std::optional<BusInfo> GetBusInfo(std::string_view name) const;

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop*> name_to_stops_;
        std::unordered_map<std::string_view, Bus*> name_to_bus_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_bus_;
    };

}
