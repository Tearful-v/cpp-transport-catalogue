#include "stat_reader.h"

#include "geo.h"

#include <iomanip>
#include <ostream>
#include <unordered_set>

static void ParseBus(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name,
              std::ostream& output) {
    const transport_catalogue::Bus* bus = transport_catalogue.FindBus(bus_name);
    if (bus == nullptr) {
        output << "Bus " << bus_name << ": not found\n";
        return;
    }

    const auto& route = bus->GetRoute();
    std::unordered_set<const transport_catalogue::Stop*> unique_stops(route.begin(), route.end());

    double route_length = 0.0;
    for (size_t i = 1; i < route.size(); ++i) {
        route_length += geo::ComputeDistance(route[i - 1]->GetCoordinates(), route[i]->GetCoordinates());
    }

    output << std::setprecision(6);
    output << "Bus " << bus_name << ": "
           << route.size() << " stops on route, "
           << unique_stops.size() << " unique stops, "
           << route_length << " route length\n";
}

static void ParseStop(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name,
               std::ostream& output) {
    const transport_catalogue::Stop* stop = transport_catalogue.FindStop(stop_name);
    if (stop == nullptr) {
        output << "Stop " << stop_name << ": not found\n";
        return;
    }

    const auto& buses = transport_catalogue.GetBusesForStop(stop->GetName());
    if (buses.empty()) {
        output << "Stop " << stop_name << ": no buses\n";
        return;
    }

    output << "Stop " << stop_name << ": buses";
    for (std::string_view bus_name : buses) {
        output << ' ' << bus_name;
    }
    output << '\n';
}


void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    const auto space_pos = request.find(' ');
    const std::string_view command = request.substr(0, space_pos);
    const std::string_view name = request.substr(space_pos + 1);

    if (command == "Bus") {
        ParseBus(transport_catalogue, name, output);
    } else if (command == "Stop") {
        ParseStop(transport_catalogue, name, output);
    }
}
