#include "json_reader.h"

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
    std::cout << results;
}