#include "map_renderer.h"

namespace map_render {
	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	void MapRender::SetVisual(const VisualSettings& settings) {
		width_ = settings.width;
		height_ = settings.height;
		padding_ = settings.padding;
		line_width_ = settings.line_width;
		stop_radius_ = settings.stop_radius;
		bus_label_font_size_ = settings.bus_label_font_size;
		bus_label_offset_ = settings.bus_label_offset;
		stop_label_font_size_ = settings.stop_label_font_size;
		stop_label_offset_ = settings.stop_label_offset;
		underlayer_width_ = settings.underlayer_width;
	}

	void MapRender::SetColor(ColorSettingType color_type, const std::string& color) {
		if (color_type == ColorSettingType::UNDERLAYER) {
			underlayer_color_ = color;
		}
		else {
			color_palette_.push_back(color);
		}
	}
	void MapRender::SetColor(ColorSettingType color_type, int color_r, int color_g, int color_b) {
		svg::Rgb color{static_cast<uint8_t>(color_r), static_cast<uint8_t>(color_g), static_cast<uint8_t>(color_b)};
		if (color_type == ColorSettingType::UNDERLAYER) {
			underlayer_color_ = color;
		}
		else {
			color_palette_.push_back(color);
		}
	}
	void MapRender::SetColor(ColorSettingType color_type, int color_r, int color_g, int color_b, double color_a) {
		svg::Rgba color{static_cast<uint8_t>(color_r), static_cast<uint8_t>(color_g), static_cast<uint8_t>(color_b), color_a};
		if (color_type == ColorSettingType::UNDERLAYER) {
			underlayer_color_ = color;
		}
		else {
			color_palette_.push_back(color);
		}
	}

	void MapRender::CreateSphereProjector(const std::deque<domain::Stop>& bus_stops) {
		std::vector<geo::Coordinates> bus_stops_geo_coords;
		for (const auto& stop : bus_stops) {
			if (!stop.buses.empty()) {
				bus_stops_geo_coords.push_back(stop.lat_lng);
			}
		}
		sphere_projector_ = { bus_stops_geo_coords.begin(), bus_stops_geo_coords.end(), width_, height_, padding_ };
	}

	void MapRender::DrawRoutes(const std::deque<domain::Bus>& bus_routes) {
		std::deque<domain::Bus> buses(bus_routes);
		std::sort(buses.begin(), buses.end(), [](domain::Bus& left, domain::Bus& right) {return left.number < right.number; });

		size_t color_index = 0;
		for (const auto& bus : buses) {
			if (!bus.route.empty()) {
				std::vector<geo::Coordinates> bus_route_geo_coords;
				for (const auto& stop : bus.route) {
					bus_route_geo_coords.push_back(stop->lat_lng);
				}
				svg::Polyline bus_route;
				bus_route.SetStrokeColor(color_palette_[color_index]);
				bus_route.SetFillColor(svg::NoneColor);
				bus_route.SetStrokeWidth(line_width_);
				bus_route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				bus_route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				for (const auto geo_coord : bus_route_geo_coords) {
					const svg::Point screen_coord = sphere_projector_(geo_coord);
					bus_route.AddPoint(screen_coord);
				}
				map_.Add(bus_route);
				if (color_index < color_palette_.size() - 1) {
					++color_index;
				}
				else {
					color_index = 0;
				}
			}
		}
	}

	void MapRender::DrawBusNames(const std::deque<domain::Bus>& bus_routes) {
		std::deque<domain::Bus> buses(bus_routes);
		std::sort(buses.begin(), buses.end(), [](domain::Bus& left, domain::Bus& right) {return left.number < right.number; });

		size_t color_index = 0;
		for (const auto& bus : buses) {
			if (!bus.route.empty()) {
				svg::Text bus_name_underlayer;
				bus_name_underlayer.SetData(bus.number);
				bus_name_underlayer.SetPosition(sphere_projector_(bus.route[0]->lat_lng));
				bus_name_underlayer.SetOffset(bus_label_offset_);
				bus_name_underlayer.SetFontSize(bus_label_font_size_);
				bus_name_underlayer.SetFontFamily("Verdana");
				bus_name_underlayer.SetFontWeight("bold");
				bus_name_underlayer.SetFillColor(underlayer_color_);
				bus_name_underlayer.SetStrokeColor(underlayer_color_);
				bus_name_underlayer.SetStrokeWidth(underlayer_width_);
				bus_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				bus_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				map_.Add(bus_name_underlayer);

				svg::Text bus_name;
				bus_name.SetData(bus.number);
				bus_name.SetPosition(sphere_projector_(bus.route[0]->lat_lng));
				bus_name.SetOffset(bus_label_offset_);
				bus_name.SetFontSize(bus_label_font_size_);
				bus_name.SetFontFamily("Verdana");
				bus_name.SetFontWeight("bold");
				bus_name.SetFillColor(color_palette_[color_index]);
				map_.Add(bus_name);

				if (bus.is_roundtrip == false && bus.route[0] != bus.route[bus.route.size()/2]) {
					svg::Text bus_name_duplicate_underlayer;
					bus_name_duplicate_underlayer.SetData(bus.number);
					bus_name_duplicate_underlayer.SetPosition(sphere_projector_(bus.route[bus.route.size() / 2]->lat_lng));
					bus_name_duplicate_underlayer.SetOffset(bus_label_offset_);
					bus_name_duplicate_underlayer.SetFontSize(bus_label_font_size_);
					bus_name_duplicate_underlayer.SetFontFamily("Verdana");
					bus_name_duplicate_underlayer.SetFontWeight("bold");
					bus_name_duplicate_underlayer.SetFillColor(underlayer_color_);
					bus_name_duplicate_underlayer.SetStrokeColor(underlayer_color_);
					bus_name_duplicate_underlayer.SetStrokeWidth(underlayer_width_);
					bus_name_duplicate_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
					bus_name_duplicate_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
					map_.Add(bus_name_duplicate_underlayer);

					svg::Text bus_name_duplicate;
					bus_name_duplicate.SetData(bus.number);
					bus_name_duplicate.SetPosition(sphere_projector_(bus.route[bus.route.size() / 2]->lat_lng));
					bus_name_duplicate.SetOffset(bus_label_offset_);
					bus_name_duplicate.SetFontSize(bus_label_font_size_);
					bus_name_duplicate.SetFontFamily("Verdana");
					bus_name_duplicate.SetFontWeight("bold");
					bus_name_duplicate.SetFillColor(color_palette_[color_index]);
					map_.Add(bus_name_duplicate);
				}
				if (color_index < color_palette_.size() - 1) {
					++color_index;
				}
				else {
					color_index = 0;
				}
			}
		}
	}

	void MapRender::DrawStops(const std::deque<domain::Stop>& bus_stops) {
		std::deque<domain::Stop> stops(bus_stops);
		std::sort(stops.begin(), stops.end(), [](domain::Stop& left, domain::Stop& right) {return left.name < right.name; });

		for (const auto& stop : stops) {
			if (!stop.buses.empty()) {
				svg::Circle bus_stop;
				bus_stop.SetCenter(sphere_projector_(stop.lat_lng));
				bus_stop.SetRadius(stop_radius_);
				bus_stop.SetFillColor("white");
				map_.Add(bus_stop);
			}
		}
	}

	void MapRender::DrawStopNames(const std::deque<domain::Stop>& bus_stops) {
		std::deque<domain::Stop> stops(bus_stops);
		std::sort(stops.begin(), stops.end(), [](domain::Stop& left, domain::Stop& right) {return left.name < right.name; });

		for (const auto& stop : stops) {
			if (!stop.buses.empty()) {
				svg::Text stop_name_underlayer;
				stop_name_underlayer.SetData(stop.name);
				stop_name_underlayer.SetPosition(sphere_projector_(stop.lat_lng));
				stop_name_underlayer.SetOffset(stop_label_offset_);
				stop_name_underlayer.SetFontSize(stop_label_font_size_);
				stop_name_underlayer.SetFontFamily("Verdana");
				stop_name_underlayer.SetFillColor(underlayer_color_);
				stop_name_underlayer.SetStrokeColor(underlayer_color_);
				stop_name_underlayer.SetStrokeWidth(underlayer_width_);
				stop_name_underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				stop_name_underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				map_.Add(stop_name_underlayer);

				svg::Text stop_name;
				stop_name.SetData(stop.name);
				stop_name.SetPosition(sphere_projector_(stop.lat_lng));
				stop_name.SetOffset(stop_label_offset_);
				stop_name.SetFontSize(stop_label_font_size_);
				stop_name.SetFontFamily("Verdana");
				stop_name.SetFillColor("black");
				map_.Add(stop_name);
			}
		}
	}

	void MapRender::DrawMap(const std::deque<domain::Stop>& bus_stops, const std::deque<domain::Bus>& bus_routes) {
		CreateSphereProjector(bus_stops);
		DrawRoutes(bus_routes);
		DrawBusNames(bus_routes);
		DrawStops(bus_stops);
		DrawStopNames(bus_stops);
	}
	const svg::Document& MapRender::GetMap() const {
		return map_;
	}
}