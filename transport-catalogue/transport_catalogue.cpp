#include "transport_catalogue.h"

#include <string>

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string name, geo::Coordinates coords) {
        auto it = name_to_stops_.find(name);

        if (it != name_to_stops_.end()) {
            it->second->ChangeCoords(coords);
            return;
        }

        stops_.emplace_back(std::move(name), coords);

        Stop& stop = stops_.back();
        name_to_stops_.emplace(stop.GetName(), &stop);
    }

    Stop* TransportCatalogue::GetOrCreateStop(std::string_view name) {
        auto it = name_to_stops_.find(name);

        if (it != name_to_stops_.end()) {
            return it->second;
        }

        stops_.emplace_back(std::string(name));

        Stop& stop = stops_.back();
        name_to_stops_.emplace(stop.GetName(), &stop);

        return &stop;
    }

    void TransportCatalogue::AddBus(std::string name, const std::vector<std::string_view>& stop_names) {
        std::vector<Stop*> route;
        route.reserve(stop_names.size());

        for (std::string_view stop_name : stop_names) {
            route.push_back(GetOrCreateStop(stop_name));
        }

        buses_.emplace_back(std::move(name), std::move(route));

        Bus& bus = buses_.back();
        name_to_bus_.emplace(bus.GetName(), &bus);

        for (Stop* stop : bus.GetRoute())
            stop_to_bus_[stop->GetName()].insert(bus.GetName());
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it = name_to_stops_.find(name);

        if (it == name_to_stops_.end()) {
            return nullptr;
        }

        return it->second;
    }

    const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
        static const std::set<std::string_view> empty_buses;

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

}
