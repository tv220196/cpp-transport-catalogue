#include <algorithm>
#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates lat_lng) {
		stops_and_buses_.AddStop({ stop_name, lat_lng, {} });
		stop_lookup_table_[stops_and_buses_.GetStops().back().name] = &stops_and_buses_.GetStops().back();
	}

	void TransportCatalogue::AddDistance(const std::string& first_stop, const std::string& second_stop, int distance) {
		distances_[{stop_lookup_table_.find(first_stop)->first, stop_lookup_table_.find(second_stop)->first}] = distance;
	}

	void TransportCatalogue::AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route, bool is_roundtrip) {
		stops_and_buses_.AddBus({ bus_number, {}, is_roundtrip});
		std::vector<domain::Stop*> route;
		int actual_route_length = 0;
		for (size_t i = 0; i < bus_route.size(); ++i) {
			route.push_back(stop_lookup_table_.at(bus_route[i]));
			stop_lookup_table_.at(bus_route[i])->buses.insert(stops_and_buses_.GetBuses().back().number);
			if (i != 0) {
				if (distances_.count({ bus_route[i - 1], bus_route[i] }) != 0) {
					actual_route_length += distances_.at({ bus_route[i - 1], bus_route[i] });
				}
				else {
					actual_route_length += distances_.at({ bus_route[i], bus_route[i - 1] });
				}
			}
		}
		stops_and_buses_.GetBuses().back().route = route;
		size_t stops_on_route = route.size();
		double length = 0.0;
		for (size_t i = 1; i < route.size(); ++i) {
			length += geo::ComputeDistance(route[i - 1]->lat_lng, route[i]->lat_lng);
		}
		double curvature = static_cast<double>(actual_route_length) / length;

		std::sort(route.begin(), route.end());
		size_t unique_stops_count = std::distance(route.begin(), std::unique(route.begin(), route.end()));
		bus_search_results_.push_back({ stops_on_route, unique_stops_count, actual_route_length, curvature });

		bus_lookup_table_[stops_and_buses_.GetBuses().back().number] = {&stops_and_buses_.GetBuses().back(), &bus_search_results_.back()};
	}

	const BusSearchResult TransportCatalogue::SearchBus(std::string_view bus_name) const {
		if (bus_lookup_table_.count(bus_name) == 0) {
			return { 0, 0, 0, 0.0 };
		}
		return *bus_lookup_table_.at(bus_name).second;
	}

	const domain::Stop* TransportCatalogue::SearchStop(std::string_view stop_name) const {
		if (stop_lookup_table_.count(stop_name) == 0) {
			return nullptr;
		}
		else {
			return stop_lookup_table_.at(stop_name);
		}
	}

	const std::deque<domain::Bus>& TransportCatalogue::GetBuses() const {
		return stops_and_buses_.GetBuses();
	}

	const std::deque<domain::Stop>& TransportCatalogue::GetStops() const {
		return stops_and_buses_.GetStops();
	}
}