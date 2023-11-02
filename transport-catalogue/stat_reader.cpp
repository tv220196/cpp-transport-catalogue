#include "stat_reader.h"

namespace request {
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output) {
        // Реализуйте самостоятельно
        if (request.find("Bus ") != std::string_view::npos) {
            std::string_view bus_number = Trim(request.substr(request.find(' '), request.size() - request.find(' ')));
            transport_catalogue::BusSearchResult bus_search_result = transport_catalogue.SearchBus(bus_number);
            if (bus_search_result.stops_on_route == 0) {
                output << "Bus " << bus_number << ": not found\n";
                return;
            }
            output << "Bus " << bus_number << ": " << bus_search_result.stops_on_route << " stops on route, " << bus_search_result.unique_stops <<
                " unique stops, " << bus_search_result.actual_route_length << " route length, " << bus_search_result.curvature << " curvature\n";
            return;
        }
        
        if (request.find("Stop ") != std::string_view::npos) {
            std::string_view stop_name = Trim(request.substr(request.find(' '), request.size() - request.find(' ')));
            transport_catalogue::StopSearchResult stop_search_result = transport_catalogue.SearchStop(stop_name);
            switch (stop_search_result.result)
            {
            case transport_catalogue::NOT_FOUND:
                output << "Stop " << stop_name << ": not found\n";
                break;
            case transport_catalogue::FOUND_NO_BUSES:
                output << "Stop " << stop_name << ": no buses\n";
                break;
            case transport_catalogue::FOUND_WITH_BUSES:
                output << "Stop " << stop_name << ": buses ";
                size_t count = 0;
                for (const auto& bus : stop_search_result.buses) {
                    if (count != stop_search_result.buses.size() - 1) {
                        output << bus << " ";
                        ++count;
                        continue;
                    }
                    output << bus << '\n';
                }
                break;
            }
        }
    }
}
