#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace input {
    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return { nan, nan };
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return { lat, lng };
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return { std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1)) };
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
        // Реализуйте метод самостоятельно
        std::unordered_map<std::string_view, std::vector<std::string_view>> stops_descriptions;
        for (auto& command_ : commands_) {
            if (command_.command == "Stop") {
                stops_descriptions[command_.id] = Split(command_.description, ',');
                std::string coordinates = std::string(stops_descriptions.at(command_.id).at(0)) + ", " + std::string(stops_descriptions.at(command_.id).at(1));
                geo::Coordinates lat_lng = ParseCoordinates(coordinates);
                catalogue.AddStop(command_.id, lat_lng);
                continue;
            }
            if (command_.command == "Bus") {
                continue;
            }
        }
        for (auto& stop_description : stops_descriptions) {
            if (stop_description.second.size() < 3) {
                continue;
            }
            for (size_t i = 2; i < stop_description.second.size(); ++i) {
                int meters = std::stoi(std::string(stop_description.second.at(i).substr(0, stop_description.second.at(i).find_first_of('m'))));
                std::string destination(stop_description.second.at(i).substr(stop_description.second.at(i).find("m to ")+5, stop_description.second.at(i).size()));
                catalogue.AddDistance(stop_description.first, destination, meters);
            }
        }
        for (auto& command_ : commands_) {
            if (command_.command == "Stop") {
                continue;
            }
            if (command_.command == "Bus") {
                std::vector<std::string_view> route = ParseRoute(command_.description);
                catalogue.AddBus(command_.id, route);
            }
        }
    }
}