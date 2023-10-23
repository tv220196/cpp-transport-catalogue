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
		std::set<std::string_view> buses; //��������, ���������� ����� ���������
	};

	enum StopSearchResults {
		NOT_FOUND,
		FOUND_NO_BUSES,
		FOUND_WITH_BUSES,
	};

	struct StopSearchResult
	{
		StopSearchResults result;
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
		double route_length;
	};

	class TransportCatalogue {
		// ���������� ����� ��������������
	public:
		void AddStop(const std::string& stop_name, geo::Coordinates lat_lng); //���������� ���������
		void AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route); //���������� ��������
		const BusSearchResult BusSearch(std::string_view bus_name) const; //��������� ������� �� ����� ����������� ��������
		const StopSearchResult StopSearch(std::string_view stop_name) const; //��������� ������� �� ����� ���������

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stop_lookup_table_; //���-������� ��� �������� ������ ���������
		std::deque<Bus> buses_;
		std::deque<BusSearchResult> bus_search_results_;
		std::unordered_map<std::string_view, std::pair<Bus*, BusSearchResult*>> bus_lookup_table_; //���-������� ��� �������� ������ ����������� ��������
	};
}