#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace transport_catalogue {

    class Stop {
    public:
        Stop() = default;

        explicit Stop(std::string name) : name_(std::move(name)) {}

        Stop(std::string name, geo::Coordinates coords) : name_(std::move(name)), coords_(coords) {}

        void ChangeCoords(geo::Coordinates coords) noexcept {
            coords_ = coords;
        }

        std::string_view GetName() const noexcept {
            return name_;
        }

        const geo::Coordinates& GetCoordinates() const noexcept {
            return coords_;
        }

    private:
        std::string name_ = "dummy";
        geo::Coordinates coords_ = {0, 0};
    };

    class Bus {
    public:
        Bus(std::string name, std::vector<Stop*> route) : name_(std::move(name)), route_(std::move(route)) {}

        std::string_view GetName() const noexcept {
            return name_;
        }

        const std::vector<Stop*>& GetRoute() const noexcept {
            return route_;
        }

    private:
        std::string name_ = "dummy";
        std::vector<Stop*> route_;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string name, geo::Coordinates coords);
        void AddBus(std::string name, const std::vector<std::string_view>& stop_names);

        const Stop* FindStop(std::string_view name) const;
        const Bus* FindBus(std::string_view name) const;
        const std::set<std::string_view>& GetBusesForStop(std::string_view stop_name) const;

    private:
        Stop* GetOrCreateStop(std::string_view name);

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, Stop*> name_to_stops_;
        std::unordered_map<std::string_view, Bus*> name_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_bus_;
    };

}
