#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        void AddStop(std::string name, geo::Coordinates coords);
        void AddBus(std::string name, const std::vector<std::string>& stop_names, bool is_roundtrip = false);
        void SetStopsDistance(std::string_view from, std::string_view to, int distance);

        const domain::Stop* FindStop(std::string_view name) const;
        const domain::Bus* FindBus(std::string_view name) const;
        const std::unordered_set<std::string_view>& GetBusesForStop(std::string_view stop_name) const;
        int GetStopsDistance(std::string_view from, std::string_view to) const;
        std::optional<domain::BusInfo> GetBusInfo(std::string_view name) const;
        const std::deque<domain::Bus>& GetAllBuses() const;


    private:
        struct StopsPairHasher {
            size_t operator()(std::pair<std::string_view, std::string_view> stops) const {
                return std::hash<std::string_view>{}(stops.first) * 37
                    + std::hash<std::string_view>{}(stops.second);
            }
        };

        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> buses_;

        std::unordered_map<std::string_view, const domain::Stop*> name_to_stops_;
        std::unordered_map<std::string_view, const domain::Bus*> name_to_bus_;
        std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_bus_;
        std::unordered_map<std::pair<std::string_view, std::string_view>, int, StopsPairHasher> stops_distance_;
    };

}
