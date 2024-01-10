#pragma once
#include "geo.h"
#include<deque>
#include<set>
#include<string>
#include<string_view>
#include<vector>
namespace domain {
	struct Stop
	{
		std::string name;
		geo::Coordinates lat_lng;
		std::set<std::string_view> buses; //автобусы, проходящие через остановку
		size_t index;//индекс остановки в графе
	};

	struct Bus
	{
		std::string number;
		std::vector<Stop*> route;
		bool is_roundtrip;
		size_t index;//индекс автобуса в графе
	};

	class Domain {
	public:
		Domain() = default;
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);
		std::deque<Stop>& GetStops();
		std::deque<Bus>& GetBuses();
		const std::deque<Bus>& GetBuses() const;
		const std::deque<Stop>& GetStops() const;
	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
	};
}