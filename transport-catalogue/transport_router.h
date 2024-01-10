#pragma once
#include <cstdint>
#include <map>
#include <string_view>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

using namespace graph;

namespace transport_router {

	class RoutingSettings {
	public:
		RoutingSettings() = default;
		void SetRoutingSettings(int bus_wait_time, int bus_velocity);
		uint16_t GetBusWaitTime() const;
		uint16_t GetBusVelocity() const;
	private:
		uint16_t bus_wait_time_ = 0; // ����� �������� �������� �� ��������� � �������
		uint16_t bus_velocity_ = 0; //�������� �������� � ��/�
	};

	struct Item {
		std::size_t index_bus_;
		std::size_t span_count_;
	};

	class BusGraph : public graph::DirectedWeightedGraph<double> {
	public:
		BusGraph(const transport_catalogue::TransportCatalogue& catalogue, const RoutingSettings& routing_settings);

		const std::string_view GetBusNumber(EdgeId edge_id) const;
		int GetSpanCount(EdgeId edge_id) const;

	private:
		const transport_catalogue::TransportCatalogue* catalogue_ = nullptr;
		const RoutingSettings* routing_settings_ = nullptr;
		std::map<graph::EdgeId, Item> items_;
		int m_in_km_ = 1000;
		int sec_in_min = 60;

		void BuildGraph();
		double CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to);
	};
}



	


