#pragma once
#include "domain.h"
#include<unordered_map>

namespace transport_catalogue {
	struct BusSearchResult
	{
		size_t stops_on_route;
		size_t unique_stops;
		int actual_route_length;
		double curvature;
	};

	class TwoStopsHasher {
	public:
		size_t operator()(const std::pair<std::string_view, std::string_view> stops) const {
			return std::hash<unsigned long long>{}(static_cast<unsigned long long>(std::hash<std::string_view>{}(stops.first)) + static_cast<unsigned long long>(std::hash<std::string_view>{}(stops.second)));
		}
	};

	class TransportCatalogue {
		// –еализуйте класс самосто€тельно
	public:
		void AddStop(const std::string& stop_name, geo::Coordinates lat_lng); //добавление остановки
		void AddDistance(const std::string& first_stop, const std::string& second_stop, int distance); //добавление рассто€ни€ между остановками
		void AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route, bool is_roundtrip); //добавление автобуса
		const BusSearchResult SearchBus(std::string_view bus_name) const; //обработка запроса на поиск автобусного маршрута
		const domain::Stop* SearchStop(std::string_view stop_name) const; //обработка запроса на поиск остановки
		const std::deque<domain::Bus>& GetBuses() const;
		const std::deque<domain::Stop>& GetStops() const;
	private:
		domain::Domain stops_and_buses_;
		std::unordered_map<std::string_view, domain::Stop*> stop_lookup_table_; //хеш-таблица дл€ быстрого поиска остановки
		std::deque<BusSearchResult> bus_search_results_;
		std::unordered_map<std::string_view, std::pair<domain::Bus*, BusSearchResult*>> bus_lookup_table_; //хеш-таблица дл€ быстрого поиска автобусного маршрута
		std::unordered_map<std::pair<std::string_view, std::string_view>, int, TwoStopsHasher> distances_; //хеш-таблица дл€ быстрого поиска рассто€ний между остановками
	};
}