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
            auto base_requests = requests.GetRoot().AsDict().at("base_requests"s).AsArray();
            for (const auto& base_request : base_requests) {
                ParseRequest(base_request);
            }
            ApplyCommands(catalogue);
        }

        CommandDescription ParseCommandDescription(const json::Node& request) {
            json::Dict description;
            for (const auto& descr : request.AsDict()) {
                if (descr.first != "type"s && descr.first != "name"s) {
                    description.insert(descr);
                }
            }
            return { request.AsDict().at("type"s).AsString(), request.AsDict().at("name"s).AsString(), description };
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
                        for (const auto& road_distance : command_.description.at("road_distances"s).AsDict()) {
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

        void Input::FormMap(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue, map_render::MapRender& map) {
            visual_settings::SetVisual(requests, map);
            map.DrawMap(catalogue.GetStops(), catalogue.GetBuses());
        }

        /*void Input::FormRoutingSettings(const json::Document& requests, transport_router::RoutingSettings& routing_settings) {
            auto settings = requests.GetRoot().AsDict().at("routing_settings"s).AsDict();
            //routing_settings.SetRoutingSettings(settings.at("bus_wait_time").AsInt(), settings.at("bus_velocity").AsInt());
            routing_settings.bus_wait_time_ = static_cast<uint16_t>(settings.at("bus_wait_time").AsInt());
            routing_settings.bus_velocity_ = static_cast<uint16_t>(settings.at("bus_velocity").AsInt());
        }*/

        void Input::FormBusGraph(const json::Document& requests, /*const transport_catalogue::TransportCatalogue& catalogue,*/ transport_router::BusGraph& bus_graph) {
            transport_router::RoutingSettings routing_settings;
            routing_settings.bus_wait_time = static_cast<uint16_t>(requests.GetRoot().AsDict().at("routing_settings"s).AsDict().at("bus_wait_time").AsInt());
            routing_settings.bus_velocity = static_cast<uint16_t>(requests.GetRoot().AsDict().at("routing_settings"s).AsDict().at("bus_velocity").AsInt());
            bus_graph.BuildGraph(routing_settings);
        }

    }

    namespace output {
        json::Node FormOutputStop(const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Dict result;
            transport_catalogue::BusSearchResult bus_search_result = catalogue.SearchBus(stat_request.AsDict().at("name"s).AsString());
            if (bus_search_result.stops_on_route == 0) {
                result["request_id"s] = stat_request.AsDict().at("id"s);
                result["error_message"s] = json::Node{ "not found"s };
            }
            else {
                result["stop_count"s] = json::Node{ static_cast<int>(bus_search_result.stops_on_route) };
                result["unique_stop_count"s] = json::Node{ static_cast<int>(bus_search_result.unique_stops) };
                result["route_length"s] = json::Node{ bus_search_result.actual_route_length };
                result["curvature"s] = json::Node{ bus_search_result.curvature };
                result["request_id"s] = stat_request.AsDict().at("id"s);
            }
            return json::Node(result);
        }

        json::Node FormOutputBus(const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue) {
            json::Dict result;
            auto stop_search_result = catalogue.SearchStop(stat_request.AsDict().at("name"s).AsString());
            if (!stop_search_result) {
                result["request_id"s] = stat_request.AsDict().at("id"s);
                result["error_message"s] = json::Node{ "not found"s };
            }
            else {
                json::Array buses;
                for (const auto& bus : stop_search_result->buses) {
                    buses.push_back(json::Node(std::string{bus}));
                }
                result["buses"s] = json::Node{ buses };
                result["request_id"s] = stat_request.AsDict().at("id"s);
            }
            return json::Node(result);
        }

        json::Node FormOutputMap(const json::Node& stat_request, map_render::MapRender& map) {
            json::Dict result;
            result["request_id"s] = stat_request.AsDict().at("id"s);
            std::ostringstream out;
            map.GetMap().Render(out);
            result["map"s] = json::Node{ out.str() };
            return json::Node(result);
        }

        std::pair<size_t, size_t> FindStopsIndex(const transport_catalogue::TransportCatalogue& catalogue, const std::string& from, const std::string& to) {
            size_t index_from = catalogue.SearchStop(from)->index;
            size_t index_to = catalogue.SearchStop(to)->index;
            return { index_from, index_to };
        }

        json::Array GetArrayItems(const transport_catalogue::TransportCatalogue& catalogue, const transport_router::BusGraph& bus_graph, 
                                  const std::vector<graph::EdgeId>& edges) {
            const auto& stops = catalogue.GetStops();
            json::Array items;
            for (const auto& edge : edges) {
                auto it = bus_graph.GetEdge(edge);
                if (it.weight == 0) {
                    return items;
                }
                std::string stop = stops.at(it.from).name;
                int time_for_wait = bus_graph.GetBusWaitTime();
                json::Node item_wait = json::Builder{}
                    .StartDict()
                    .Key("stop_name"s).Value(std::string(stop))
                    .Key("time"s).Value(double(time_for_wait))
                    .Key("type"s).Value("Wait"s)
                    .EndDict()
                    .Build();
                double time_for_bus = it.weight - time_for_wait;
                std::string bus(bus_graph.GetBusNumber(it.from));
                int span_count = bus_graph.GetSpanCount(it.from);
                json::Node item_bus = json::Builder{}
                    .StartDict()
                    .Key("bus"s).Value(std::string(bus))
                    .Key("span_count").Value(span_count)
                    .Key("time").Value(double(time_for_bus))
                    .Key("type").Value("Bus"s)
                    .EndDict()
                    .Build();
                items.push_back(item_wait);
                items.push_back(item_bus);
            }

            return items;
        }

        json::Node FormOutputRoute(const json::Node& stat_request, const transport_catalogue::TransportCatalogue& catalogue, 
                                   const transport_router::BusGraph& bus_graph) {
            auto [from, to] = FindStopsIndex(catalogue, stat_request.AsDict().at("from").AsString(), stat_request.AsDict().at("to").AsString());
            std::optional<graph::Router<double>::RouteInfo> route = bus_graph.BuildRoute(from, to);
            if (route) {
                auto& info = route.value();
                json::Array items = GetArrayItems(catalogue, bus_graph, info.edges);
                json::Node dict_node = json::Builder{}
                    .StartDict()
                    .Key("total_time"s).Value(info.weight)
                    .Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
                    .Key("items"s).Value(items)
                    .EndDict()
                    .Build();
                return dict_node;
            }
            else {
                json::Node dict_node = json::Builder{}
                    .StartDict()
                    .Key("error_message"s).Value("not found"s)
                    .Key("request_id"s).Value(stat_request.AsDict().at("id"s).AsInt())
                    .EndDict()
                    .Build();
                return dict_node;
            }
        }

        json::Document FormOutput(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue, map_render::MapRender& map, 
                                  const transport_router::BusGraph& bus_graph) {
            json::Array results;
            for (const auto& stat_request : requests.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
                std::string stat_request_type = stat_request.AsDict().at("type"s).AsString();
                if (stat_request_type == "Bus"s) {
                    results.push_back(FormOutputStop(stat_request, catalogue));
                }
                if (stat_request_type == "Stop"s) {
                    results.push_back(FormOutputBus(stat_request, catalogue));
                }
                if (stat_request_type == "Map"s) {
                    results.push_back(FormOutputMap(stat_request, map));
                }
                if (stat_request_type == "Route"s) {
                    results.push_back(FormOutputRoute(stat_request, catalogue, bus_graph));
                }
            }
            json::Node result;
            result.GetValue() = results;
            return json::Document(result);
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
            auto render_settings = requests.GetRoot().AsDict().at("render_settings"s).AsDict();
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
