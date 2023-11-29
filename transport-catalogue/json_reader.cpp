#include "json_reader.h"

#include <sstream>

using namespace std::literals;
namespace json_reader {

    json::Document LoadJSON(std::istream& in) {
        std::string doc;
        while (in) {
            std::string s;
            if (std::getline(in, s) && !s.empty()) {
                doc += s;
            }
            else {
                break;
            }
        }
        std::istringstream strm(doc);
        return json::Load(strm);
    }

    namespace input {

        void Input::FormTransportCatalogue(const json::Document& requests, transport_catalogue::TransportCatalogue& catalogue) {
            auto base_requests = requests.GetRoot().AsMap().at("base_requests"s).AsArray();
            for (const auto& base_request : base_requests) {
                ParseRequest(base_request);
            }
            ApplyCommands(catalogue);
        }

        CommandDescription ParseCommandDescription(const json::Node& request) {
            json::Dict description;
            for (const auto& descr : request.AsMap()) {
                if (descr.first != "type"s && descr.first != "name"s) {
                    description.insert(descr);
                }
            }
            return { request.AsMap().at("type"s).AsString(), request.AsMap().at("name"s).AsString(), description };
        }

        void Input::ParseRequest(const json::Node& request) {
            auto command_description = ParseCommandDescription(request);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        std::vector<std::string_view> ParseRoute(const json::Dict& route_description) {
            std::vector<std::string_view> results;
            if (route_description.at("is_roundtrip"s).AsBool()) {
                for (const auto& stop : route_description.at("stops"s).AsArray()) {
                    results.push_back(stop.AsString());
                }
                return results;
            }
            else {
                for (const auto& stop : route_description.at("stops"s).AsArray()) {
                    results.push_back(stop.AsString());
                }
                if (route_description.at("stops"s).AsArray().size() >= 2) {
                    for (int i = static_cast<int>(route_description.at("stops"s).AsArray().size()) - 2; i >= 0; --i) {
                        results.push_back(route_description.at("stops"s).AsArray()[i].AsString());
                    }
                }
                return results;
            }

        }

        void Input::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
            for (const auto& command_ : commands_) {
                if (command_.name == "Stop"s) {
                    catalogue.AddStop(command_.id, { command_.description.at("latitude"s).AsDouble(), command_.description.at("longitude"s).AsDouble() });
                    continue;
                }
                if (command_.name == "Bus"s) {
                    continue;
                }
            }
            for (const auto& command_ : commands_) {
                if (command_.name == "Stop"s) {
                    if (command_.description.count("road_distances"s)) {
                        for (const auto& road_distance : command_.description.at("road_distances"s).AsMap()) {
                            catalogue.AddDistance(command_.id, road_distance.first, road_distance.second.AsInt());
                        }
                    }
                    continue;
                }
                if (command_.name == "Bus"s) {
                    continue;
                }
            }
            for (const auto& command_ : commands_) {
                if (command_.name == "Stop"s) {
                    continue;
                }
                if (command_.name == "Bus"s) {
                    std::vector<std::string_view> route = ParseRoute(command_.description);
                    catalogue.AddBus(command_.id, route, command_.description.at("is_roundtrip"s).AsBool());
                }
            }
        }

    }

    namespace output {
        json::Node FormOutputStop(const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Dict result;
            transport_catalogue::BusSearchResult bus_search_result = catalogue.SearchBus(stat_request.AsMap().at("name"s).AsString());
            if (bus_search_result.stops_on_route == 0) {
                result["request_id"s] = stat_request.AsMap().at("id"s);
                result["error_message"s] = json::Node{ "not found"s };
            }
            else {
                result["stop_count"s] = json::Node{ static_cast<int>(bus_search_result.stops_on_route) };
                result["unique_stop_count"s] = json::Node{ static_cast<int>(bus_search_result.unique_stops) };
                result["route_length"s] = json::Node{ bus_search_result.actual_route_length };
                result["curvature"s] = json::Node{ bus_search_result.curvature };
                result["request_id"s] = stat_request.AsMap().at("id"s);
            }
            return json::Node(result);
        }

        json::Node FormOutputBus(const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Dict result;
            auto stop_search_result = catalogue.SearchStop(stat_request.AsMap().at("name"s).AsString());
            if (!stop_search_result) {
                result["request_id"s] = stat_request.AsMap().at("id"s);
                result["error_message"s] = json::Node{ "not found"s };
            }
            else {
                json::Array buses;
                for (const auto& bus : stop_search_result->buses) {
                    buses.push_back(json::Node(std::string{bus}));
                }
                result["buses"s] = json::Node{ buses };
                result["request_id"s] = stat_request.AsMap().at("id"s);
            }
            return json::Node(result);
        }

        json::Node FormOutputMap(const json::Document& requests, const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Dict result;
            result["request_id"s] = stat_request.AsMap().at("id"s);
            map_render::MapRender map;
            visual_settings::SetVisual(requests, map);
            map.DrawMap(catalogue.GetStops(), catalogue.GetBuses());
            std::ostringstream out;
            map.GetMap().Render(out);
            result["map"s] = json::Node{ out.str() };
            return json::Node(result);
        }

        json::Document FormOutput(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Array results;
            for (const auto& stat_request : requests.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
                std::string stat_request_type = stat_request.AsMap().at("type"s).AsString();
                if (stat_request_type == "Bus"s) {
                    results.push_back(FormOutputStop(stat_request, catalogue));
                }
                if (stat_request_type == "Stop"s) {
                    results.push_back(FormOutputBus(stat_request, catalogue));
                }
                if (stat_request_type == "Map"s) {
                    results.push_back(FormOutputMap(requests, stat_request, catalogue));
                }
            }
            return { json::Node(results) };
        }
    }

    namespace visual_settings {
        void SetColor(map_render::ColorSettingType color_type, const json::Node& color, map_render::MapRender& map) {
            if (color.IsString()) {
                map.SetColor(color_type, color.AsString());
            }
            else {
                if (color.AsArray().size() == 3) {
                    map.SetColor(color_type, color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
                }
                else {
                    map.SetColor(color_type, color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
                }
            }
        }

        void SetVisual(const json::Document& requests, map_render::MapRender& map) {
            auto render_settings = requests.GetRoot().AsMap().at("render_settings"s).AsMap();
            map_render::VisualSettings settings {
                render_settings.at("width"s).AsDouble(),
                render_settings.at("height"s).AsDouble(),
                render_settings.at("padding"s).AsDouble(),
                render_settings.at("line_width"s).AsDouble(),
                render_settings.at("stop_radius"s).AsDouble(),
                render_settings.at("bus_label_font_size"s).AsInt(),
                { render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble(), render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble() },
                render_settings.at("stop_label_font_size"s).AsInt(),
                { render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble(), render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble() },
                render_settings.at("underlayer_width"s).AsDouble()
            };
            map.SetVisual(settings);
            SetColor(map_render::ColorSettingType::UNDERLAYER, render_settings.at("underlayer_color"s), map);
            for (const auto& color : render_settings.at("color_palette"s).AsArray()) {
                SetColor(map_render::ColorSettingType::PALETTE, color, map);
            }
        }
    }
}

std::istream& operator>> (std::istream& in, json::Document& document) {
    document = json_reader::LoadJSON(in);
    return in;
}
std::ostream& operator<< (std::ostream& out, const json::Document& document) {
    json::Print(document, out);
    return out;
}
