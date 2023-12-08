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
		// ���������� ����� ��������������
	public:
		void AddStop(const std::string& stop_name, geo::Coordinates lat_lng); //���������� ���������
		void AddDistance(const std::string& first_stop, const std::string& second_stop, int distance); //���������� ���������� ����� �����������
		void AddBus(const std::string& bus_number, const std::vector<std::string_view>& bus_route, bool is_roundtrip); //���������� ��������
		const BusSearchResult SearchBus(std::string_view bus_name) const; //��������� ������� �� ����� ����������� ��������
		const domain::Stop* SearchStop(std::string_view stop_name) const; //��������� ������� �� ����� ���������
		const std::deque<domain::Bus>& GetBuses() const;
		const std::deque<domain::Stop>& GetStops() const;
	private:
		domain::Domain stops_and_buses_;
		std::unordered_map<std::string_view, domain::Stop*> stop_lookup_table_; //���-������� ��� �������� ������ ���������
		std::deque<BusSearchResult> bus_search_results_;
		std::unordered_map<std::string_view, std::pair<domain::Bus*, BusSearchResult*>> bus_lookup_table_; //���-������� ��� �������� ������ ����������� ��������
		std::unordered_map<std::pair<std::string_view, std::string_view>, int, TwoStopsHasher> distances_; //���-������� ��� �������� ������ ���������� ����� �����������
	};
}