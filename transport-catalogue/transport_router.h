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

	struct RoutingSettings {
		uint16_t bus_wait_time = 0; // время ожидания автобуса на остановке в минутах
		uint16_t bus_velocity = 0; //скорость автобуса в км/ч
	};

	struct Item {
		std::size_t index_bus_;
		std::size_t span_count_;
	};

	class BusGraph : public graph::DirectedWeightedGraph<double> {
	public:
		explicit BusGraph(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings routing_settings);
		std::optional<graph::Router<double>::RouteInfo> BuildRoute(const std::string& from, const std::string& to) const;
		int GetBusWaitTime() const;
		const std::string_view GetBusNumber(EdgeId edge_id) const;
		int GetSpanCount(EdgeId edge_id) const;

	private:
		const transport_catalogue::TransportCatalogue* catalogue_ = nullptr;
		RoutingSettings routing_settings_;
		graph::Router<double>* router_ = nullptr;
		std::map<graph::EdgeId, Item> items_;
		int m_in_km_ = 1000;
		int sec_in_min = 60;

		void BuildGraph();
		double CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to);
	};
}



	


