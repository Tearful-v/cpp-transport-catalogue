#include "map_renderer.h"
#include "domain.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace {

    void AddBusLabel(svg::Document& doc, svg::Point point, const std::string& bus_name,
                     const svg::Color& color, const map_render::RenderSettings& settings) {
        svg::Text underlayer;
        underlayer.SetPosition(point)
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus_name)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetPosition(point)
            .SetOffset(settings.bus_label_offset)
            .SetFontSize(settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus_name)
            .SetFillColor(color);

        doc.Add(std::move(underlayer));
        doc.Add(std::move(label));
    }

    void AddStopLabel(svg::Document& doc, svg::Point point, const std::string& stop_name,
                      const map_render::RenderSettings& settings) {
        svg::Text underlayer;
        underlayer.SetPosition(point)
            .SetOffset(settings.stop_label_offset)
            .SetFontSize(settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop_name)
            .SetFillColor(settings.underlayer_color)
            .SetStrokeColor(settings.underlayer_color)
            .SetStrokeWidth(settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text label;
        label.SetPosition(point)
            .SetOffset(settings.stop_label_offset)
            .SetFontSize(settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop_name)
            .SetFillColor("black");

        doc.Add(std::move(underlayer));
        doc.Add(std::move(label));
    }

    void FillPolyline(const std::vector<const domain::Bus*> &buses, const map_render::RenderSettings &settings,
                    svg::Document &doc, const map_render::SphereProjector &projector) {
        int index = 0;
        for (auto bus : buses) {
            if (bus->route.empty()) {
                continue;
            }

            svg::Polyline line;

            const svg::Color& color = settings.color_palette[index % settings.color_palette.size()];
            ++index;
            line.SetFillColor(svg::NoneColor)
                .SetStrokeColor(color)
                .SetStrokeWidth(settings.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            for (auto stop : bus->route) {
                line.AddPoint(projector(stop->coords));
            }
            doc.Add(std::move(line));
        }
    }

    void FillBusName(const std::vector<const domain::Bus*> &buses, const map_render::RenderSettings &settings,
                    svg::Document &doc, const map_render::SphereProjector &projector) {
        int index = 0;
        for (auto bus : buses) {
            if (bus->route.empty()) {
                continue;
            }
            const svg::Color& color = settings.color_palette[index % settings.color_palette.size()];
            ++index;
            AddBusLabel(doc, projector(bus->route.front()->coords), bus->name, color, settings);

            if (!bus->is_roundtrip && bus->route.front()->name != bus->route[bus->route.size() / 2]->name) {
                AddBusLabel(doc, projector(bus->route[bus->route.size() / 2]->coords), bus->name, color, settings);
            }
        }
    }

    std::vector<const domain::Stop*> CollectStops(const std::vector<const domain::Bus*> &buses) {
        std::map<std::string_view, const domain::Stop*> stops;
        for (auto bus : buses) {
            if (bus->route.empty()) {
                continue;
            }

            for (auto stop : bus->route) {
                stops[stop->name] = stop;
            }
        }

        std::vector<const domain::Stop*> result;
        result.reserve(stops.size());
        for (const auto& [name, stop] : stops) {
            result.push_back(stop);
        }

        return result;
    }

    void FillStopName(const std::vector<const domain::Stop*> &stops, const map_render::RenderSettings &settings,
                    svg::Document &doc, const map_render::SphereProjector &projector) {
        for (auto stop : stops) {
            AddStopLabel(doc, projector(stop->coords), stop->name, settings);
        }
    }

    void FillCircle(const std::vector<const domain::Stop*> &stops, const map_render::RenderSettings &settings,
                    svg::Document &doc, const map_render::SphereProjector &projector) {
        for (auto stop : stops) {
            svg::Circle circle;
            circle.SetCenter(projector(stop->coords))
                .SetRadius(settings.stop_radius)
                .SetFillColor("white");
            doc.Add(std::move(circle));
        }
    }

} //namespace

namespace map_render {

    svg::Document MapRender::RenderMap(const transport_catalogue::TransportCatalogue& catalogue) const {
        svg::Document doc;
        std::vector<const domain::Bus*> buses = catalogue.GetAllBuses();
        std::vector<geo::Coordinates> coords;
        for (auto bus : buses) {
            if (bus->route.empty()) {
                continue;
            }

            for (auto stop : bus->route) {
                coords.push_back(stop->coords);
            }
        }
        SphereProjector projector(coords.begin(), coords.end(), settings_.width,
                                 settings_.height, settings_.padding);
        std::vector<const domain::Stop*> stops = CollectStops(buses);

        FillPolyline(buses, settings_, doc, projector);
        FillBusName(buses, settings_, doc, projector);
        FillCircle(stops, settings_, doc, projector);
        FillStopName(stops, settings_, doc, projector);

        return doc;
    }

} //map_render
