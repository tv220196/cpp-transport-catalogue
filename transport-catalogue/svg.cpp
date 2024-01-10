#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<< (std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // ƒелегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << "\n"s;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // ¬ыводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " />"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            if (i == points_.size() - 1) {
                out << points_[i].x << ',' << points_[i].y;
            }
            else {
                out << points_[i].x << ","sv << points_[i].y << " "sv;
            }
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << " />"sv;
    }

    std::ostream& operator<< (std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<< (std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        default:
            break;
        }
        return out;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv;
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
            << "\" font-size=\""sv << font_size_ << "\""sv;
        if (font_family_ != "не выводитс€") {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_ != "не выводитс€") {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        for (const auto ch : data_) {
            if (ch == '\"') {
                out << "&quot;"sv;
            }
            else if (ch == '\'') {
                out << "&apos;"sv;
            }
            else if (ch == '<') {
                out << "&lt;"sv;
            }
            else if (ch == '>') {
                out << "&gt;"sv;
            }
            else if (ch == '&') {
                out << "&amp;"sv;
            }
            else {
                out << ch;
            }
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << "\n"s;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << "\n"s;
        for (const auto& object : objects_) {
            object->Render(out);
        }
        out << "</svg>"sv;
    }

    // ---------- Drawable ------------------

    void Drawable::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }
}