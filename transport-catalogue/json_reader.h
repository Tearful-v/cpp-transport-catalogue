#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <map>
#include <string>
#include <vector>

namespace json_reader {

    struct StopCommand {
        std::string name;
        geo::Coordinates coords;
        std::map<std::string, int> road_distances;
    };

    struct BusCommand {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip = false;
    };

    class JsonReader {
    public:
        explicit JsonReader(const json::Document& doc) : doc_(doc) {}

        void FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) const;
        json::Document ProcessRequests(
            const transport_catalogue::TransportCatalogue& catalogue) const;
        map_render::RenderSettings GetRenderSettings() const;
        transport_router::RouterSettings GetRouterSettings() const;

    private:
        void ApplyStopCommands(
            const std::vector<StopCommand>& commands,
            transport_catalogue::TransportCatalogue& catalogue) const;
        void ApplyBusCommands(
            const std::vector<BusCommand>& commands,
            transport_catalogue::TransportCatalogue& catalogue) const;
        json::Node MakeStopResponse(
            const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue) const;
        json::Node MakeBusResponse(
            const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue) const;
        json::Node MakeMapResponse(
            const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue,
            const map_render::MapRender& renderer) const;
        json::Node MakeRouterResponce(
            const json::Dict& request,
            const transport_router::TransportRouter& router) const;

        const json::Document& doc_;
    };

} //json_reader
