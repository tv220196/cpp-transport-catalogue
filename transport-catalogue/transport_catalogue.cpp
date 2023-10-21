#include "transport_catalogue.h"
#include <algorithm>

namespace transport_catalogue {

	void TransportCatalogue::AddStop(std::string stop_name, geo::Coordinates lat_lng) {
		stops_.push_back({ stop_name, lat_lng, {} });
		stop_lookup_table_[stops_.back().name] = &stops_.back();
	}

	void TransportCatalogue::AddBus(std::string bus_number, std::vector<std::string_view> bus_route) {
		std::vector<Stop*> route;
		for (const auto& stop : bus_route) {
			route.push_back(stop_lookup_table_.at(stop));
		}

		double length = 0.0;
		for (int i = 1; i < route.size(); ++i) {
			length += geo::ComputeDistance(route[i - 1]->lat_lng, route[i]->lat_lng);
		}

		std::sort(bus_route.begin(), bus_route.end());
		size_t unique_stops_count = std::distance(bus_route.begin(), std::unique(bus_route.begin(), bus_route.end()));

		buses_.push_back({ bus_number, route, unique_stops_count, length });
		bus_lookup_table_[buses_.back().number] = &buses_[buses_.size() - 1];

		for (const auto& stop : bus_route) {
			stop_lookup_table_.at(stop)->buses.insert(buses_.back().number);
		}
	}

	const std::tuple<size_t, size_t, double> TransportCatalogue::BusSearch(std::string_view bus_name) const {
		if (bus_lookup_table_.count(bus_name) == 0) {
			return std::tuple<size_t, size_t, double>{0, 0, 0.0};
		}
		return std::tuple<size_t, size_t, double>{bus_lookup_table_.at(bus_name)->route.size(), bus_lookup_table_.at(bus_name)->unique_stops_count, bus_lookup_table_.at(bus_name)->length};
	}

	const std::tuple<std::string, std::set<std::string_view>> TransportCatalogue::StopSearch(std::string_view stop_name) const {
		if (stop_lookup_table_.count(stop_name) == 0) {
			return std::tuple<std::string, std::set<std::string_view>>{"not found", {}};
		}
		if (stop_lookup_table_.at(stop_name)->buses.size() == 0) {
			return std::tuple<std::string, std::set<std::string_view>>{"no buses", stop_lookup_table_.at(stop_name)->buses};
		}
		return std::tuple<std::string, std::set<std::string_view>>{"", stop_lookup_table_.at(stop_name)->buses};
	}
}