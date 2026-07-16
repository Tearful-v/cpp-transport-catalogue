#pragma once

#include "geo.h"

#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

    struct Stop {
        explicit Stop(std::string name) : name(std::move(name)) {}
        Stop(std::string name, geo::Coordinates coords) : name(std::move(name)), coords(coords) {}

        std::string name = "dummy";
        geo::Coordinates coords = {0, 0};
    };

    struct Bus {
        Bus(std::string name, std::vector<Stop*> route) : name(std::move(name)), route(std::move(route)) {}

        std::string name;
        std::vector<Stop*> route;
    };

    struct BusInfo {
        BusInfo(double length, size_t stops_count, size_t unique_stops_count)
            : route_length(length), stops(stops_count), unique_stops(unique_stops_count) {}

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
        Stop* GetOrCreateStop(std::string_view name);

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop*> name_to_stops_;
        std::unordered_map<std::string_view, Bus*> name_to_bus_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_bus_;
    };

}
