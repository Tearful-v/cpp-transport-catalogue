#include "json_reader.h"
#include "json.h"

#include <algorithm>
#include <string>
#include <sstream>
#include <string_view>
#include <cstdint>
#include <vector>

namespace json_reader {
namespace {

    svg::Point ParsePoint(const json::Node& node) {
        const json::Array& array = node.AsArray();
        return {array[0].AsDouble(), array[1].AsDouble()};
    }

    svg::Color ParseColor(const json::Node& node) {
        if (node.IsString()) {
            return node.AsString();
        }

        const json::Array& color = node.AsArray();

        if (color.size() == 3) {
            return svg::Rgb{
                static_cast<uint8_t>(color[0].AsInt()),
                static_cast<uint8_t>(color[1].AsInt()),
                static_cast<uint8_t>(color[2].AsInt())
            };
        }

        return svg::Rgba{
            static_cast<uint8_t>(color[0].AsInt()),
            static_cast<uint8_t>(color[1].AsInt()),
            static_cast<uint8_t>(color[2].AsInt()),
            color[3].AsDouble()
        };
    }

} // namespace

    void JsonReader::ApplyStopCommands(
        const std::vector<StopCommand>& commands,
        transport_catalogue::TransportCatalogue& catalogue) const {
        for (const auto& com : commands) {
            catalogue.AddStop(com.name, com.coords);
        }
        for (const auto& com: commands) {
            for (const auto& stopinfo : com.road_distances) {
                catalogue.SetStopsDistance(com.name, stopinfo.first, stopinfo.second);
            }
        }
    }

    void JsonReader::ApplyBusCommands(
        const std::vector<BusCommand>& commands,
        transport_catalogue::TransportCatalogue& catalogue) const {
        for (const BusCommand& bus : commands) {
            catalogue.AddBus(bus.name, bus.stops, bus.is_roundtrip);
        }
    }

    map_render::RenderSettings JsonReader::GetRenderSettings() const {
        map_render::RenderSettings settings;
        const json::Node& root = doc_.GetRoot();
        const json::Dict& root_dict = root.AsMap();
        const json::Dict& render_request = root_dict.at("render_settings").AsMap();

        settings.height = render_request.at("height").AsDouble();
        settings.padding = render_request.at("padding").AsDouble();
        settings.width = render_request.at("width").AsDouble();
        settings.line_width = render_request.at("line_width").AsDouble();
        settings.stop_radius = render_request.at("stop_radius").AsDouble();
        settings.stop_label_font_size = render_request.at("stop_label_font_size").AsInt();
        settings.bus_label_font_size = render_request.at("bus_label_font_size").AsInt();
        settings.underlayer_width = render_request.at("underlayer_width").AsDouble();
        settings.bus_label_offset = ParsePoint(render_request.at("bus_label_offset"));
        settings.stop_label_offset = ParsePoint(render_request.at("stop_label_offset"));
        settings.underlayer_color = ParseColor(render_request.at("underlayer_color"));
        const json::Array& palette = render_request.at("color_palette").AsArray();
        settings.color_palette.reserve(palette.size());

        for (const json::Node& color_node : palette) {
            settings.color_palette.push_back(ParseColor(color_node));
        }

        return settings;
    }


    void JsonReader::FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) const {
        std::vector<StopCommand> stops;
        std::vector<BusCommand> buses;

        const json::Node& root = doc_.GetRoot();
        const json::Dict& root_dict = root.AsMap();
        const json::Array& fill_requests = root_dict.at("base_requests").AsArray();

        for (const auto& request : fill_requests) {
            const json::Dict& command = request.AsMap();
            if (command.at("type").AsString() == "Stop") {
                StopCommand stop;
                stop.name = command.at("name").AsString();
                stop.coords.lat = command.at("latitude").AsDouble();
                stop.coords.lng = command.at("longitude").AsDouble();

                std::map<std::string, int> tmp;
                const json::Dict& dicts = command.at("road_distances").AsMap();
                for (const auto& [name, dist] : dicts) {
                    tmp[name] = dist.AsInt();
                }
                stop.road_distances = std::move(tmp);
                stops.push_back(std::move(stop));
            } else {
                BusCommand bus;
                bus.name = command.at("name").AsString();
                bus.is_roundtrip = command.at("is_roundtrip").AsBool();

                const json::Array& stops_array = command.at("stops").AsArray();
                for (const json::Node& stop_node : stops_array) {
                    bus.stops.push_back(stop_node.AsString());
                }

                if (!bus.is_roundtrip && !bus.stops.empty()) {
                    const size_t stops_count = bus.stops.size();
                    for (size_t i = stops_count - 1; i > 0; --i) {
                        bus.stops.push_back(bus.stops[i - 1]);
                    }
                }
                buses.push_back(std::move(bus));
            }
        }

        ApplyStopCommands(stops, catalogue);
        ApplyBusCommands(buses, catalogue);
    }

    json::Dict JsonReader::MakeStopResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) const {
        json::Dict answer;
        answer["request_id"] = request.at("id").AsInt();

        const std::string& name = request.at("name").AsString();
        const domain::Stop* stop = catalogue.FindStop(name);
        if (stop == nullptr) {
            answer["error_message"] = std::string("not found");
            return answer;
        }

        const auto& buses = catalogue.GetBusesForStop(name);
        std::vector<std::string_view> sorted_buses(buses.begin(), buses.end());
        std::sort(sorted_buses.begin(), sorted_buses.end());

        json::Array buses_array;
        for (std::string_view bus_name : sorted_buses) {
            buses_array.push_back(std::string(bus_name));
        }

        answer["buses"] = std::move(buses_array);
        return answer;
    }

    json::Dict JsonReader::MakeBusResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) const {
        json::Dict answer;
        answer["request_id"] = request.at("id").AsInt();

        const std::string& name = request.at("name").AsString();
        auto bus_info = catalogue.GetBusInfo(name);
        if (!bus_info) {
            answer["error_message"] = std::string("not found");
            return answer;
        }

        answer["curvature"] = bus_info->curvature;
        answer["route_length"] = bus_info->route_length;
        answer["stop_count"] = static_cast<int>(bus_info->stops);
        answer["unique_stop_count"] = static_cast<int>(bus_info->unique_stops);
        return answer;
    }

    json::Dict JsonReader::MakeMapResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) const {
        json::Dict answer;
        answer["request_id"] = request.at("id").AsInt();

        map_render::RenderSettings settings = GetRenderSettings();
        map_render::MapRender renderer(settings);
        svg::Document map = renderer.RenderMap(catalogue.GetAllBuses());

        std::ostringstream buf;
        map.Render(buf);

        std::string map_str = buf.str();
        if (!map_str.empty() && map_str.back() == '\n') {
            map_str.pop_back();
        }
        answer["map"] = std::move(map_str);
        return answer;
    }

    json::Document JsonReader::ProcessRequests(
        const transport_catalogue::TransportCatalogue& catalogue) const {
        const json::Node& root = doc_.GetRoot();
        const json::Dict& root_dict = root.AsMap();
        const json::Array& stat_requests = root_dict.at("stat_requests").AsArray();

        json::Array answers;

        for (const json::Node& request : stat_requests) {
            const json::Dict& com = request.AsMap();

            if (com.at("type").AsString() == "Stop") {
                answers.push_back(MakeStopResponse(com, catalogue));
            } else if (com.at("type").AsString() == "Bus") {
                answers.push_back(MakeBusResponse(com, catalogue));
            } else if (com.at("type").AsString() == "Map") {
                answers.push_back(MakeMapResponse(com, catalogue));
            }
        }

        return json::Document{answers};
    }

} //json_reader
