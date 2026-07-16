#include "transport_catalogue.h"

#include <cassert>
#include <string>
#include <unordered_set>
#include <utility>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string name, geo::Coordinates coords) {
        auto it = name_to_stops_.find(name);

        if (it != name_to_stops_.end()) {
            it->second->coords = coords;
            return;
        }

        stops_.push_back(Stop{std::move(name), coords});

        Stop& stop = stops_.back();
        name_to_stops_.emplace(stop.name, &stop);
    }

    void TransportCatalogue::AddBus(std::string name, const std::vector<std::string_view>& stop_names) {
        std::vector<const Stop*> route;
        route.reserve(stop_names.size());

        for (std::string_view stop_name : stop_names) {
            const Stop* stop = FindStop(stop_name);
            assert(stop != nullptr);
            route.push_back(stop);
        }

        buses_.push_back(Bus{std::move(name), std::move(route)});

        Bus& bus = buses_.back();
        name_to_bus_.emplace(bus.name, &bus);

        for (const Stop* stop : bus.route) {
            stop_to_bus_[stop->name].insert(bus.name);
        }
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it = name_to_stops_.find(name);

        if (it == name_to_stops_.end()) {
            return nullptr;
        }

        return it->second;
    }

    const std::unordered_set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        static const std::unordered_set<std::string_view> empty_buses;

        auto it = stop_to_bus_.find(stop_name);
        if (it == stop_to_bus_.end()) {
            return empty_buses;
        }

        return it->second;
    }

    const Bus* TransportCatalogue::FindBus(std::string_view name) const {
        auto it = name_to_bus_.find(name);

        if (it == name_to_bus_.end()) {
            return nullptr;
        }

        return it->second;
    }

    std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view name) const {
        const Bus* bus = FindBus(name);
        if (bus == nullptr) {
            return std::nullopt;
        }

        size_t stops = bus->route.size();
        std::unordered_set<const Stop*> unique_stops(bus->route.begin(), bus->route.end());

        double route_length = 0.0;
        for (size_t i = 1; i < bus->route.size(); ++i) {
            route_length += geo::ComputeDistance(bus->route[i - 1]->coords, bus->route[i]->coords);
        }

        return BusInfo{route_length, stops, unique_stops.size()};
    }

}
