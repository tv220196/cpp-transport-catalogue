#pragma once
#include<deque>
#include<set>
#include<string>
#include<string_view>
#include<tuple>
#include<unordered_map>
#include<vector>
#include "geo.h"

namespace transport_catalogue {
	struct Stop //название остановки, координаты остановки, автобусы, проходящие через остановку
	{
		std::string name;
		geo::Coordinates lat_lng;
		std::set<std::string_view> buses;
	};

	struct Bus //номер автобуса, маршрут, кол-во уникальных остановок в маршруте, длина маршрута
	{
		std::string number;
		std::vector<Stop*> route;
		size_t unique_stops_count;
		double length;
	};

	class TransportCatalogue {
		// Реализуйте класс самостоятельно
	public:
		void AddStop(std::string stop_name, geo::Coordinates lat_lng); //добавление остановки
		void AddBus(std::string bus_number, std::vector<std::string_view> bus_route); //добавление автобуса
		const std::tuple<size_t, size_t, double> BusSearch(std::string_view bus_name) const; //обработка запроса на поиск автобусного маршрута
		const std::tuple<std::string, std::set<std::string_view>> StopSearch(std::string_view stop_name) const; //обработка запроса на поиск остановки

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stop_lookup_table_; //хеш-таблица для быстрого поиска остановки
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> bus_lookup_table_; //хеш-таблица для быстрого поиска автобусного маршрута
	};
}