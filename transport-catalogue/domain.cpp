#include "domain.h"

namespace domain {
	void Domain::AddStop(const Stop& stop) {
		stops_.push_back(stop);
	}
	void Domain::AddBus(const Bus& bus) {
		buses_.push_back(bus);
	}
	std::deque<Stop>& Domain::GetStops() {
		return stops_;
	}
	std::deque<Bus>& Domain::GetBuses() {
		return buses_;
	}
	const std::deque<Bus>& Domain::GetBuses() const {
		return buses_;
	}
	const std::deque<Stop>& Domain::GetStops() const {
		return stops_;
	}
}