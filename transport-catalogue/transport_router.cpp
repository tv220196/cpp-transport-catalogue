#include "transport_router.h"

namespace transport_router {

	void RoutingSettings::SetRoutingSettings(int bus_wait_time, int bus_velocity) {
		bus_wait_time_ = static_cast<uint16_t>(bus_wait_time);
		bus_velocity_ = static_cast<uint16_t>(bus_velocity);
	}

	uint16_t RoutingSettings::GetBusWaitTime() const {
		return bus_wait_time_;
	}

	uint16_t RoutingSettings::GetBusVelocity() const {
		return bus_velocity_;
	}

	BusGraph::BusGraph(const transport_catalogue::TransportCatalogue& catalogue, const RoutingSettings& routing_settings)
		:graph::DirectedWeightedGraph<double>(catalogue.GetStops().size() * 2)
	{
		catalogue_ = &catalogue;
		routing_settings_ = &routing_settings;
		BuildGraph();
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
		double speed = 1.0 * m_in_km_ * routing_settings_->GetBusVelocity() / sec_in_min;
		double time = 1.0 * distance / speed;
		return time;
	}

	void BusGraph::BuildGraph() {
		for (const auto& bus : catalogue_->GetBuses()) {
			for (size_t from = 0; from < bus.route.size() - 1; ++from) {
				double time = static_cast<double>(routing_settings_->GetBusWaitTime());
				for (size_t to = from + 1; to < bus.route.size(); ++to) {
					graph::VertexId vertex_from = bus.route.at(from)->index;
					graph::VertexId vertex_to = bus.route.at(to)->index;
					time += CalcTime(*bus.route.at(to - 1), *bus.route.at(to));
					EdgeId key = this->AddEdge({ vertex_from, vertex_to, time });
					size_t span_count = to - from;
					items_[key] = { bus.index, span_count };
				}
				time = routing_settings_->GetBusWaitTime();
			}
			if (!bus.is_roundtrip) {
				double time = static_cast<double>(routing_settings_->GetBusWaitTime());
				size_t span_count = 0;
				graph::VertexId vertex_last_stop = bus.route[bus.route.size()/2 + 1]->index;
				EdgeId key_last_stop = this->AddEdge({ vertex_last_stop, vertex_last_stop, time });
				items_[key_last_stop] = { bus.index, span_count };
			}
		}
	}
}