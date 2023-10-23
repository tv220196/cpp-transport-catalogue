#include "transport_catalogue.h"
#include <algorithm>

namespace transport_catalogue {

	void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates lat_lng) {
		stops_.push_back({ stop_name, lat_lng, {} });
		stop_lookup_table_[stops_.back().name] = &stops_.back();
	}

	void TransportCatalogue::AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route) {
		buses_.push_back({ bus_number, {} });
		std::vector<Stop*> route;
		for (const auto& stop : bus_route) {
			route.push_back(stop_lookup_table_.at(stop));
			stop_lookup_table_.at(stop)->buses.insert(buses_.back().number);
		}
		buses_.back().route = route;

		size_t stops_on_route = route.size();
		double length = 0.0;
		for (int i = 1; i < route.size(); ++i) {
			length += geo::ComputeDistance(route[i - 1]->lat_lng, route[i]->lat_lng);
		}
		std::sort(route.begin(), route.end());
		size_t unique_stops_count = std::distance(route.begin(), std::unique(route.begin(), route.end()));
		bus_search_results_.push_back({stops_on_route, unique_stops_count, length});

		bus_lookup_table_[buses_.back().number] = { &buses_.back(), &bus_search_results_.back() };
	}

	const BusSearchResult TransportCatalogue::BusSearch(std::string_view bus_name) const {
		if (bus_lookup_table_.count(bus_name) == 0) {
			return {0, 0, 0.0};
		}
		return *bus_lookup_table_.at(bus_name).second;
	}

	const StopSearchResult TransportCatalogue::StopSearch(std::string_view stop_name) const {
		if (stop_lookup_table_.count(stop_name) == 0) {
			return {NOT_FOUND, {}};
		}
		if (stop_lookup_table_.at(stop_name)->buses.size() == 0) {
			return {FOUND_NO_BUSES, stop_lookup_table_.at(stop_name)->buses};
		}
		return {FOUND_WITH_BUSES, stop_lookup_table_.at(stop_name)->buses};
	}
}