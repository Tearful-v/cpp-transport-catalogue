#include "json_reader.h"
#include "json_builder.h"
#include "json.h"

#include <algorithm>
#include <optional>
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

    transport_router::RouterSettings JsonReader::GetRouterSettings() const {
        transport_router::RouterSettings settings;
        const json::Node &root = doc_.GetRoot();
        const json::Dict &root_dict = root.AsMap();
        const json::Dict &router_request = root_dict.at("routing_settings").AsMap();
        settings.wait_time = router_request.at("bus_wait_time").AsDouble();
        settings.bus_speed = router_request.at("bus_velocity").AsDouble();
        return settings;
    }


    void JsonReader::FillCatalogue(
        transport_catalogue::TransportCatalogue& catalogue) const {
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

    json::Node JsonReader::MakeStopResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) const {
        const std::string& name = request.at("name").AsString();
        const domain::Stop* stop = catalogue.FindStop(name);
        if (stop == nullptr) {
            return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(request.at("id").AsInt())
                    .Key("error_message").Value(std::string("not found"))
                .EndDict()
                .Build();
        }

        const auto& buses = catalogue.GetBusesForStop(name);
        std::vector<std::string_view> sorted_buses(buses.begin(), buses.end());
        std::sort(sorted_buses.begin(), sorted_buses.end());

        json::Array buses_array;
        for (std::string_view bus_name : sorted_buses) {
            buses_array.push_back(std::string(bus_name));
        }

        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(request.at("id").AsInt())
                .Key("buses").Value(std::move(buses_array))
            .EndDict()
            .Build();
    }

    json::Node JsonReader::MakeBusResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) const {
        const std::string& name = request.at("name").AsString();
        auto bus_info = catalogue.GetBusInfo(name);
        if (!bus_info) {
            return json::Builder{}
                .StartDict()
                    .Key("request_id").Value(request.at("id").AsInt())
                    .Key("error_message").Value(std::string("not found"))
                .EndDict()
                .Build();
        }

        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(request.at("id").AsInt())
                .Key("curvature").Value(bus_info->curvature)
                .Key("route_length").Value(bus_info->route_length)
                .Key("stop_count").Value(static_cast<int>(bus_info->stops))
                .Key("unique_stop_count").Value(static_cast<int>(bus_info->unique_stops))
            .EndDict()
            .Build();
    }

    json::Node JsonReader::MakeMapResponse(
        const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue,
        const map_render::MapRender& renderer) const {
        svg::Document map = renderer.RenderMap(catalogue.GetAllBuses());

        std::ostringstream buf;
        map.Render(buf);

        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(request.at("id").AsInt())
                .Key("map").Value(buf.str())
            .EndDict()
            .Build();
    }

    json::Node JsonReader::MakeRouterResponce(
            const json::Dict& request,
            const transport_router::TransportRouter& router) const {
        const auto res = router.BuildRoute(
            request.at("from").AsString(),
            request.at("to").AsString()
        );

        if (!res) {
            return json::Builder{}
            .StartDict()
                .Key("request_id").Value(request.at("id").AsInt())
                .Key("error_message").Value("not found")
            .EndDict()
            .Build();
        }

        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request.at("id").AsInt())
            .Key("total_time").Value(res->total_time)
            .Key("items").StartArray();

        for (const auto& edge : res->edges) {
        builder.StartDict()
            .Key("type").Value(std::string("Wait"))
            .Key("stop_name").Value(std::string(edge.wait_stop))
            .Key("time").Value(router.GetWaitTime())
        .EndDict()
        .StartDict()
            .Key("type").Value(std::string("Bus"))
            .Key("bus").Value(std::string(edge.bus_name))
            .Key("span_count").Value(edge.span_count)
            .Key("time").Value(edge.ride_time)
        .EndDict();
    }

    return builder
        .EndArray()
        .EndDict()
        .Build();
    }

    json::Document JsonReader::ProcessRequests(
        const transport_catalogue::TransportCatalogue& catalogue) const {
        const json::Node& root = doc_.GetRoot();
        const json::Dict& root_dict = root.AsMap();
        const json::Array& stat_requests = root_dict.at("stat_requests").AsArray();
        std::optional<map_render::MapRender> renderer;
        std::optional<transport_router::TransportRouter> router;

        json::Builder builder;
        auto answers = builder.StartArray();

        for (const json::Node& request : stat_requests) {
            const json::Dict& com = request.AsMap();

            if (com.at("type").AsString() == "Stop") {
                answers.NodeValue(MakeStopResponse(com, catalogue));
            } else if (com.at("type").AsString() == "Bus") {
                answers.NodeValue(MakeBusResponse(com, catalogue));
            } else if (com.at("type").AsString() == "Map") {
                if (!renderer.has_value()) {
                    renderer.emplace(GetRenderSettings());
                }
                answers.NodeValue(MakeMapResponse(com, catalogue, *renderer));
            } else if (com.at("type").AsString() == "Route") {
                if (!router.has_value()) {
                    router.emplace(GetRouterSettings(), catalogue);
                }
                answers.NodeValue(MakeRouterResponce(com, *router));
            }
        }

        return json::Document{answers.EndArray().Build()};
    }

} //json_reader
