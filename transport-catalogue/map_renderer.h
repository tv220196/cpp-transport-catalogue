#pragma once
#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <optional>

namespace map_render {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        SphereProjector() = default;
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // ���� ����� ����������� ����� �� ������, ��������� ������
            if (points_begin == points_end) {
                return;
            }

            // ������� ����� � ����������� � ������������ ��������
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // ������� ����� � ����������� � ������������ �������
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // ��������� ����������� ��������������� ����� ���������� x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // ��������� ����������� ��������������� ����� ���������� y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // ������������ ��������������� �� ������ � ������ ���������,
                // ���� ����������� �� ���
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *height_zoom;
            }
        }

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_ = 0.0;
        double min_lon_ = 0.0;
        double max_lat_ = 0.0;
        double zoom_coeff_ = 0.0;
    };

    class MapRender {
	public:
		MapRender() = default;
		void SetWidth(double width);
		void SetHeight(double height);
		void SetPadding(double padding);
		void SetLineWidth(double line_width);
		void SetStopRadius(double stop_radius);
		void SetBusLabelFontSize(int bus_label_font_size);
		void SetBusLabelOffset(svg::Point bus_label_offset);
		void SetStopLabelFontSize(int stop_label_font_size);
		void SetStopLabelOffset(svg::Point stop_label_offset);
		void SetUnderlayerColor(const std::string& underlayer_color);
		void SetUnderlayerColor(int underlayer_color_r, int underlayer_color_g, int underlayer_color_b);
		void SetUnderlayerColor(int underlayer_color_r, int underlayer_color_g, int underlayer_color_b, double underlayer_color_a);
		void SetUnderlayerWidth(double underlayer_width);
		void SetColorPallete(const std::string& color);
		void SetColorPallete(int color_r, int color_g, int color_b);
		void SetColorPallete(int color_r, int color_g, int color_b, double color_a);

        void CreateSphereProjector(const std::deque<domain::Stop>& bus_stops);
        void DrawRoutes(const std::deque<domain::Bus>& bus_routes);
        void DrawBusNames(const std::deque<domain::Bus>& bus_routes);
        void DrawStops(const std::deque<domain::Stop>& bus_stops);
        void DrawStopNames(const std::deque<domain::Stop>& bus_stops);
        void DrawMap(const std::deque<domain::Stop>& bus_stops, const std::deque<domain::Bus>& bus_routes);
        const svg::Document& GetMap() const;
	private:
		double width_ = 0.0;//������ ����������� � ��������
		double height_ = 0.0;//������ ����������� � ��������
		double padding_ = 0.0;//������ ����� ����� �� ������� SVG-���������
		double line_width_ = 0.0;//������� �����, �������� �������� ���������� ��������
		double stop_radius_ = 0.0;//������ �����������, �������� ������������ ���������
		int bus_label_font_size_ = 0;//������ ������, ������� �������� �������� ���������� ���������
		svg::Point bus_label_offset_;//�������� ������� � ��������� �������� ������������ ��������� �������� ��������� �� �����
		int stop_label_font_size_ = 0;//������ ������, ������� ������������ �������� ���������
		svg::Point stop_label_offset_;//�������� �������� ��������� ������������ � ��������� �� �����
		svg::Color underlayer_color_;//���� �������� ��� ���������� ��������� � ���������
		double underlayer_width_ = 0.0;//������� �������� ��� ���������� ��������� � ���������
		std::vector<svg::Color> color_palette_;//�������� �������
        SphereProjector sphere_projector_;
		svg::Document map_;
	};

}
