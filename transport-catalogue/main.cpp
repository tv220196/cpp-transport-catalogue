#include "json_reader.h"
using namespace std::literals;

/*int main() {
    transport_catalogue::TransportCatalogue catalogue;
    
    json::Document requests;
    std::cin >> requests;

    json_reader::input::Input base_requests;
    base_requests.FormTransportCatalogue(requests, catalogue);

    map_render::MapRender map;
    json_reader::visual_settings::SetVisual(requests, map);
    map.DrawMap(catalogue.GetStops(), catalogue.GetBuses());
    map.GetMap().Render(std::cout);
    
}*/

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json::Document requests;
    std::cin >> requests;
    json_reader::input::Input base_requests;
    base_requests.FormTransportCatalogue(requests, catalogue);
    json::Document results = json_reader::output::FormOutput(requests, catalogue);
    json::Print(results, std::cout);
}

/*int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json::Document requests{
        json::Builder{}
        .StartDict()
            .Key("base_requests"s)
            .StartArray()
            .StartDict()
            .Key("type"s).Value("Bus"s)
            .Key("name"s).Value("114"s)
            .Key("stops"s).StartArray().Value("Морской вокзал"s).Value("Ривьерский мост"s).EndArray()
            .Key("is_roundtrip"s).Value(false)
            .EndDict()
            .StartDict()
            .Key("type"s).Value("Stop"s)
            .Key("name"s).Value("Ривьерский мост"s)
            .Key("latitude"s).Value(43.587795)
            .Key("longitude"s).Value(39.716901)
            .Key("road_distances"s).StartDict().Key("Морской вокзал"s).Value(850).EndDict()
            .EndDict()
            .StartDict()
            .Key("type"s).Value("Stop"s)
            .Key("name"s).Value("Морской вокзал"s)
            .Key("latitude"s).Value(43.581969)
            .Key("longitude"s).Value(39.719848)
            .Key("road_distances"s).StartDict().Key("Ривьерский мост"s).Value(850).EndDict()
            .EndDict()
            .EndArray()
            .Key("render_settings"s)
            .StartDict()
            .Key("width"s).Value(200)
            .Key("height"s).Value(200)
            .Key("padding"s).Value(30)
            .Key("stop_radius"s).Value(5)
            .Key("line_width"s).Value(14)
            .Key("bus_label_font_size"s).Value(20)
            .Key("bus_label_offset"s).StartArray().Value(7).Value(15).EndArray()
            .Key("stop_label_font_size"s).Value(20)
            .Key("stop_label_offset"s).StartArray().Value(7).Value(-3).EndArray()
            .Key("underlayer_color"s).StartArray().Value(255).Value(255).Value(255).Value(0.85).EndArray()
            .Key("underlayer_width"s).Value(3)
            .Key("color_palette"s).StartArray().Value("green"s).StartArray().Value(255).Value(160).Value(0).EndArray().Value("red"s).EndArray()
            .EndDict()
            .Key("stat_requests"s)
            .StartArray()
            .StartDict().Key("id"s).Value(1).Key("type"s).Value("Map"s).EndDict()
            .StartDict().Key("id"s).Value(2).Key("type"s).Value("Stop"s).Key("name"s).Value("Ривьерский мост"s).EndDict()
            .StartDict().Key("id"s).Value(3).Key("type"s).Value("Bus"s).Key("name"s).Value("114"s).EndDict()
            .EndArray()
            .EndDict()
            .Build()
    };
    json_reader::input::Input base_requests;
    base_requests.FormTransportCatalogue(requests, catalogue);
    json::Document results = json_reader::output::FormOutput(requests, catalogue);
    json::Print(results, std::cout);
}*/