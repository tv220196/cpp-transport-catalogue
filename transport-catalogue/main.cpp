#include "json_reader.h"
using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    map_render::MapRender map;
    transport_router::RoutingSettings routing_settings;
    json::Document requests;
    std::cin >> requests;
    json_reader::input::Input base_requests;
    base_requests.FormTransportCatalogue(requests, catalogue);
    base_requests.FormRoutingSettings(requests, routing_settings);
    base_requests.FormMap(requests, catalogue, map);
    transport_router::BusGraph bus_graph(catalogue, routing_settings);
    graph::Router<double> router(bus_graph);
    json::Document results = json_reader::output::FormOutput(requests, catalogue, map, routing_settings, bus_graph, router);
    json::Print(results, std::cout);
}