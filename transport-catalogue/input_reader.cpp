#include "input_reader.h"

#include <algorithm>
#include <istream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

namespace input_reader {
namespace {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

struct StopDistance { //вспомогательная структура
    std::string_view stop_name;
    int distance = 0;
};

// парсит расстояния до остановок
std::vector<StopDistance> ParseDistanceToStops(const CommandDescription& command) {
    std::vector<StopDistance> result;
    std::string_view str = command.description;

    size_t comma_pos = str.find(',');
    if (comma_pos == std::string::npos) {
        throw std::runtime_error("Wrong format");
    }
    comma_pos = str.find(',' , comma_pos + 1);
    if (comma_pos == std::string::npos) {
        return result;
    }
    str.remove_prefix(comma_pos + 1);

    while (!str.empty()) {
        size_t space = str.find_first_not_of(' ');
        if (space == std::string::npos) {
            break;
        }
        str.remove_prefix(space);

        size_t pos = str.find('m');
        if (pos == std::string::npos) {
            throw std::runtime_error("Wrong format");
        }

        int distance = std::stoi(std::string(str.substr(0, pos)));
        str.remove_prefix(pos + 1); // убрал m

        std::string_view search = " to ";
        size_t s_pos = str.find(search);
        if (s_pos == std::string::npos) {
            throw std::runtime_error("Wrong format");
        }
        str.remove_prefix(s_pos + search.size());

        size_t comma = str.find(',');
        std::string_view stop_name;
        if (comma == std::string::npos) {
            stop_name = str;
            str.remove_prefix(str.size());
        } else {
            stop_name = str.substr(0, comma);
            str.remove_prefix(comma + 1);
        }

        result.push_back({Trim(stop_name), distance});
    }

    return result;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

} //безымянный namspace

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        if (command_description.command == "Stop") {
            stop_commands_.push_back(std::move(command_description));
        } else if (command_description.command == "Bus") {
            bus_commands_.push_back(std::move(command_description));
        }
    }
}

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
    for (const auto& com : stop_commands_) {
        catalogue.AddStop(com.id, ParseCoordinates(com.description));
    }

    for (const auto& com : stop_commands_) {
        for (const auto& dist : ParseDistanceToStops(com)) {
            catalogue.SetStopsDistance(com.id, dist.stop_name, dist.distance);
        }
    }

    for (const auto& com : bus_commands_) {
        catalogue.AddBus(com.id, ParseRoute(com.description));
    }
}

void ReadAndApplyCommands(std::istream& input, transport_catalogue::TransportCatalogue& catalogue) {
    int base_request_count = 0;
    input >> base_request_count >> std::ws;

    InputReader reader;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        reader.ParseLine(line);
    }

    reader.ApplyCommands(catalogue);
}

}
