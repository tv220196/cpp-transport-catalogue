#include "json_reader.h"
using namespace std;

int main() {
    json::Document requests;
    std::cin >> requests;
    json_reader::input::Input base_requests;
    transport_catalogue::TransportCatalogue catalogue;
    base_requests.FormTransportCatalogue(requests, catalogue);
    map_render::MapRender map;
    base_requests.FormMap(requests, catalogue, map);
    transport_router::BusGraph bus_graph(catalogue, base_requests.FormRoutingSettings(requests));
    json::Document results = json_reader::output::FormOutput(requests, catalogue, map, bus_graph);
    json::Print(results, std::cout);
}