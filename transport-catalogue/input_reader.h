#pragma once
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

class InputReader {
public:
    void ParseLine(std::string_view line);

    void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> stop_commands_;
    std::vector<CommandDescription> bus_commands_;
};

void ReadAndApplyCommands(std::istream& input, transport_catalogue::TransportCatalogue& catalogue);
