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
            auto [stops_count, unique_stops_count, length] = transport_catalogue.BusSearch(bus_number);
            if (stops_count == 0) {
                output << "Bus " << bus_number << ": not found\n";
                return;
            }
            output << "Bus " << bus_number << ": " << stops_count << " stops on route, " << unique_stops_count <<
                " unique stops, " << length << " route length\n";
            return;
        }

        if (request.find("Stop ") != std::string_view::npos) {
            std::string_view stop_name = Trim(request.substr(request.find(' '), request.size() - request.find(' ')));
            auto [stop_search_result, buses] = transport_catalogue.StopSearch(stop_name);

            if (stop_search_result == "not found") {
                output << "Stop " << stop_name << ": not found\n";
                return;
            }

            if (stop_search_result == "no buses") {
                output << "Stop " << stop_name << ": no buses\n";
                return;
            }

            output << "Stop " << stop_name << ": buses ";
            size_t count = 0;
            for (const auto& bus : buses) {
                if (count != buses.size() - 1) {
                    output << bus << " ";
                    ++count;
                    continue;
                }
                output << bus << '\n';
            }
        }
    }
}
