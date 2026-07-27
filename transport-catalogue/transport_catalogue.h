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
        int route_length = 0;
        size_t stops = 0;
        size_t unique_stops = 0;
        double curvature = 0.0;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string name, geo::Coordinates coords);
        void AddBus(std::string name, const std::vector<std::string_view>& stop_names);
        void SetStopsDistance(std::string_view from, std::string_view to, int distance);

        const Stop* FindStop(std::string_view name) const;
        const Bus* FindBus(std::string_view name) const;
        const std::unordered_set<std::string_view>& GetBusesForStop(std::string_view stop_name) const;
        int GetStopsDistance(std::string_view from, std::string_view to) const;
        std::optional<BusInfo> GetBusInfo(std::string_view name) const;


    private:
        struct StopsPairHasher {
            size_t operator()(std::pair<std::string_view, std::string_view> stops) const {
                return std::hash<std::string_view>{}(stops.first) * 37
                    + std::hash<std::string_view>{}(stops.second);
            }
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop*> name_to_stops_;
        std::unordered_map<std::string_view, const Bus*> name_to_bus_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_bus_;
        std::unordered_map<std::pair<std::string_view, std::string_view>, int, StopsPairHasher> stops_distance_;
    };

}
