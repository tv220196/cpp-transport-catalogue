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
                if (command_.command == "Stop"s) {
                    catalogue.AddStop(command_.id, { command_.description.at("latitude"s).AsDouble(), command_.description.at("longitude"s).AsDouble() });
                    continue;
                }
                if (command_.command == "Bus"s) {
                    continue;
                }
            }
            for (const auto& command_ : commands_) {
                if (command_.command == "Stop"s) {
                    if (command_.description.count("road_distances"s)) {
                        for (const auto& road_distance : command_.description.at("road_distances"s).AsMap()) {
                            catalogue.AddDistance(command_.id, road_distance.first, road_distance.second.AsInt());
                        }
                    }
                    continue;
                }
                if (command_.command == "Bus"s) {
                    continue;
                }
            }
            for (const auto& command_ : commands_) {
                if (command_.command == "Stop"s) {
                    continue;
                }
                if (command_.command == "Bus"s) {
                    std::vector<std::string_view> route = ParseRoute(command_.description);
                    catalogue.AddBus(command_.id, route, command_.description.at("is_roundtrip"s).AsBool());
                }
            }
        }

    }

    namespace output {
        json::Document FormOutput(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Array results;
            for (const auto& stat_request : requests.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
                json::Dict result;
                if (stat_request.AsMap().at("type"s).AsString() == "Bus"s) {
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
                    results.push_back(result);
                }
                if (stat_request.AsMap().at("type"s).AsString() == "Stop"s) {
                    auto stop_search_result = catalogue.SearchStop(stat_request.AsMap().at("name"s).AsString());
                    if (!stop_search_result) {
                        result["request_id"s] = stat_request.AsMap().at("id"s);
                        result["error_message"s] = json::Node{ "not found"s };
                    }
                    else {
                        json::Array buses;
                        for (const auto& bus : stop_search_result->buses) {
                            buses.push_back(std::string{bus});
                        }
                        result["buses"s] = json::Node{ buses };
                        result["request_id"s] = stat_request.AsMap().at("id"s);
                    }
                    results.push_back(result);
                }
                if (stat_request.AsMap().at("type"s).AsString() == "Map"s) {
                    result["request_id"s] = stat_request.AsMap().at("id"s);
                    map_render::MapRender map;
                    visual_settings::SetVisual(requests, map);
                    map.DrawMap(catalogue.GetStops(), catalogue.GetBuses());
                    std::ostringstream out;
                    map.GetMap().Render(out);
                    result["map"s] = json::Node{ out.str() };
                    results.push_back(result);
                }
            }
            return { results };
        }
    }

    namespace visual_settings {
        void SetVisual(const json::Document& requests, map_render::MapRender& map) {
            auto render_settings = requests.GetRoot().AsMap().at("render_settings"s).AsMap();
            map.SetWidth(render_settings.at("width"s).AsDouble());
            map.SetHeight(render_settings.at("height"s).AsDouble());
            map.SetPadding(render_settings.at("padding"s).AsDouble());
            map.SetLineWidth(render_settings.at("line_width"s).AsDouble());
            map.SetStopRadius(render_settings.at("stop_radius"s).AsDouble());
            map.SetBusLabelFontSize(render_settings.at("bus_label_font_size"s).AsInt());
            map.SetBusLabelOffset({ render_settings.at("bus_label_offset"s).AsArray()[0].AsDouble(), render_settings.at("bus_label_offset"s).AsArray()[1].AsDouble()});
            map.SetStopLabelFontSize(render_settings.at("stop_label_font_size"s).AsInt());
            map.SetStopLabelOffset({ render_settings.at("stop_label_offset"s).AsArray()[0].AsDouble(), render_settings.at("stop_label_offset"s).AsArray()[1].AsDouble() });
            if (render_settings.at("underlayer_color"s).IsString()) {
                map.SetUnderlayerColor(render_settings.at("underlayer_color"s).AsString());
            }
            else {
                if (render_settings.at("underlayer_color"s).AsArray().size() == 3) {
                    map.SetUnderlayerColor(render_settings.at("underlayer_color"s).AsArray()[0].AsInt(), render_settings.at("underlayer_color"s).AsArray()[1].AsInt(), 
                        render_settings.at("underlayer_color"s).AsArray()[2].AsInt());
                }
                else {
                    map.SetUnderlayerColor(render_settings.at("underlayer_color"s).AsArray()[0].AsInt(), render_settings.at("underlayer_color"s).AsArray()[1].AsInt(),
                        render_settings.at("underlayer_color"s).AsArray()[2].AsInt(), render_settings.at("underlayer_color"s).AsArray()[3].AsDouble());
                }
            }
            map.SetUnderlayerWidth(render_settings.at("underlayer_width"s).AsDouble());
            for (const auto& color : render_settings.at("color_palette"s).AsArray()) {
                if (color.IsString()) {
                    map.SetColorPallete(color.AsString());
                }
                else {
                    if (color.AsArray().size() == 3) {
                        map.SetColorPallete(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
                    }
                    else {
                        map.SetColorPallete(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
                    }
                }
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
