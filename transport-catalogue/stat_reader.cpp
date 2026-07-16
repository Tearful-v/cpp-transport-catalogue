#include "stat_reader.h"

#include "geo.h"

#include <algorithm>
#include <iomanip>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

static void PrintBusStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view bus_name,
              std::ostream& output) {
    auto bus_info = transport_catalogue.GetBusInfo(bus_name);
    if (!bus_info) {
        output << "Bus " << bus_name << ": not found\n";
        return;
    }

    output << std::setprecision(6);
    output << "Bus " << bus_name << ": " << bus_info->stops << " stops on route, "
           << bus_info->unique_stops << " unique stops, " << bus_info->route_length << " route length\n";
}

static void PrintStopStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view stop_name,
               std::ostream& output) {
    const transport_catalogue::Stop* stop = transport_catalogue.FindStop(stop_name);
    if (stop == nullptr) {
        output << "Stop " << stop_name << ": not found\n";
        return;
    }

    const auto& buses = transport_catalogue.GetBusesForStop(stop->name);
    if (buses.empty()) {
        output << "Stop " << stop_name << ": no buses\n";
        return;
    }

    std::vector<std::string_view> sorted_buses(buses.begin(), buses.end());
    std::sort(sorted_buses.begin(), sorted_buses.end());

    output << "Stop " << stop_name << ": buses";
    for (std::string_view bus_name : sorted_buses) {
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
        PrintBusStat(transport_catalogue, name, output);
    } else if (command == "Stop") {
        PrintStopStat(transport_catalogue, name, output);
    }
}

void ReadAndPrintStats(std::istream& input, std::ostream& output,
                       const transport_catalogue::TransportCatalogue& transport_catalogue) {
    int stat_request_count = 0;
    input >> stat_request_count >> std::ws;

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(transport_catalogue, line, output);
    }
}
