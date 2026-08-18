#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

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
        JsonReader(const json::Document& doc) : doc_(doc) {}

        void FillCatalogue(transport_catalogue::TransportCatalogue &catalogue) const;
        json::Document ProcessRequests(const transport_catalogue::TransportCatalogue &catalogue) const;
        map_render::RenderSettings GetRenderSettings() const;

    private:
        const json::Document &doc_;
    };

} //json_reader
