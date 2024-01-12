#include "transport_router.h"

namespace transport_router {

	BusGraph::BusGraph(const transport_catalogue::TransportCatalogue& catalogue, RoutingSettings routing_settings)
		:graph::DirectedWeightedGraph<double>(catalogue.GetStops().size() * 2)
		,catalogue_(&catalogue)
		,routing_settings_(routing_settings) {
		BuildGraph();
	}

	std::optional<graph::Router<double>::RouteInfo> BusGraph::BuildRoute(const std::string& from, const std::string& to) const {
		size_t index_from = catalogue_->SearchStop(from)->index;
		size_t index_to = catalogue_->SearchStop(to)->index;
		return router_->BuildRoute(index_from, index_to);
	}

	int BusGraph::GetBusWaitTime() const {
		return static_cast<int>(routing_settings_.bus_wait_time);
	}

	const std::string_view BusGraph::GetBusNumber(EdgeId edge_id) const {
		size_t index = items_.at(edge_id).index_bus_;
		return catalogue_->GetBuses().at(index).number;
	}

	int BusGraph::GetSpanCount(EdgeId edge_id) const {
		return static_cast<int>(items_.at(edge_id).span_count_);
	}

	double BusGraph::CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to) {
		int distance;
		if (catalogue_->GetDistances().count({stop_from.name, stop_to.name})) {
			distance = catalogue_->GetDistances().at({ stop_from.name, stop_to.name });
		}
		else if (catalogue_->GetDistances().count({ stop_to.name, stop_from.name })) {
			distance = catalogue_->GetDistances().at({ stop_to.name, stop_from.name });
		}
		else { 
			return 0.0; 
		}
		double speed = 1.0 * m_in_km_ * routing_settings_.bus_velocity / sec_in_min;
		double time = 1.0 * distance / speed;
		return time;
	}

	void BusGraph::BuildGraph() {
		for (const auto& bus : catalogue_->GetBuses()) {
			for (size_t from = 0; from < bus.route.size() - 1; ++from) {
				double time = static_cast<double>(routing_settings_.bus_wait_time);
				for (size_t to = from + 1; to < bus.route.size(); ++to) {
					graph::VertexId vertex_from = bus.route.at(from)->index;
					graph::VertexId vertex_to = bus.route.at(to)->index;
					time += CalcTime(*bus.route.at(to - 1), *bus.route.at(to));
					EdgeId key = this->AddEdge({ vertex_from, vertex_to, time });
					size_t span_count = to - from;
					items_[key] = { bus.index, span_count };
				}
				time = routing_settings_.bus_wait_time;
			}
			if (!bus.is_roundtrip) {
				double time = static_cast<double>(routing_settings_.bus_wait_time);
				size_t span_count = 0;
				graph::VertexId vertex_last_stop = bus.route[bus.route.size()/2 + 1]->index;
				EdgeId key_last_stop = this->AddEdge({ vertex_last_stop, vertex_last_stop, time });
				items_[key_last_stop] = { bus.index, span_count };
			}
		}
		router_ = new graph::Router<double>{*this};
	}
}