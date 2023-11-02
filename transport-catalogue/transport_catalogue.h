#pragma once
#include<deque>
#include<set>
#include<string>
#include<string_view>
#include<unordered_map>
#include<vector>
#include "geo.h"

namespace transport_catalogue {
	struct Stop
	{
		std::string name;
		geo::Coordinates lat_lng;
		std::set<std::string_view> buses; //автобусы, проходящие через остановку
	};

	enum StopSearchResults {
		NOT_FOUND,
		FOUND_NO_BUSES,
		FOUND_WITH_BUSES,
	};

	struct StopSearchResult
	{
		StopSearchResults result; //result - а зачем это? - использую как флаг для определения формата вывода в request::ParseAndPrintStat
		std::set<std::string_view> buses;
	};

	struct Bus
	{
		std::string number;
		std::vector<Stop*> route;
	};

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
		// Реализуйте класс самостоятельно
	public:
		void AddStop(const std::string& stop_name, geo::Coordinates lat_lng); //добавление остановки
		void AddDistance(std::string_view first_stop, const std::string& second_stop, int distance); //добавление расстояния между остановками
		void AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route); //добавление автобуса
		const BusSearchResult SearchBus(std::string_view bus_name) const; //обработка запроса на поиск автобусного маршрута
		const StopSearchResult SearchStop(std::string_view stop_name) const; //обработка запроса на поиск остановки

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stop_lookup_table_; //хеш-таблица для быстрого поиска остановки
		std::deque<Bus> buses_;
		std::deque<BusSearchResult> bus_search_results_;
		std::unordered_map<std::string_view, std::pair<Bus*, BusSearchResult*>> bus_lookup_table_; //хеш-таблица для быстрого поиска автобусного маршрута
		std::unordered_map<std::pair<std::string_view, std::string_view>, int, TwoStopsHasher> distances_; //хеш-таблица для быстрого поиска расстояний между остановками
	};
}