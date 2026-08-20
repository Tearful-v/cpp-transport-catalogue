#include "svg.h"

namespace svg {

using namespace std::literals;

namespace {

void RenderEscapedText(std::ostream& out, std::string_view text) {
    for (char symbol : text) {
        switch (symbol) {
            case '"':
                out << "&quot;";
                break;
            case '\'':
                out << "&apos;";
                break;
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out.put(symbol);
                break;
        }
    }
}

struct ColorPrint {

    std::ostream& out;

    void operator()(std::monostate) const {
        out << "none";
    }

    void operator()(const std::string& str) const {
        out << str;
    }

    void operator()(const Rgb& color) const {
        out << "rgb("
        << static_cast<int>(color.red) << ","
        << static_cast<int>(color.green) << ","
        << static_cast<int>(color.blue) << ")";
    }

    void operator()(const Rgba& color) const {
        out << "rgba("
            << static_cast<int>(color.red) << ","
            << static_cast<int>(color.green) << ","
            << static_cast<int>(color.blue) << ","
            << color.opacity << ")";
    }

};

} // namespace

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<circle cx=\""sv << center_.x
        << "\" cy=\""sv << center_.y
        << "\" r=\""sv << radius_ << '"';

    RenderAttrs(out);

    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<polyline points=\"";

    bool first = true;

    for (const auto& point : points_) {
        if (!first) {
            out << ' ';
        }

        out << point.x << ',' << point.y;
        first = false;
    }

    out << '"';

    RenderAttrs(out);

    out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";

    RenderAttrs(out);

    out << " x=\"" << pos_.x << '"'
        << " y=\"" << pos_.y << '"'
        << " dx=\"" << offset_.x << '"'
        << " dy=\"" << offset_.y << '"'
        << " font-size=\"" << font_size_ << '"';

    if (font_family_) {
        out << " font-family=\"";
        RenderEscapedText(out, *font_family_);
        out << '"';
    }

    if (font_weight_) {
        out << " font-weight=\"";
        RenderEscapedText(out, *font_weight_);
        out << '"';
    }

    out << '>';

    RenderEscapedText(out, data_);

    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    RenderContext context(out, 2, 2);

    for (const auto& object : objects_) {
        object->Render(context);
    }

    out << "</svg>";
}

// ---------- Ostream ------------------

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;

        case StrokeLineCap::ROUND:
            out << "round";
            break;

        case StrokeLineCap::SQUARE:
            out << "square";
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
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
    }

    return out;
}

//----------------Color---------------

std::ostream& operator <<(std::ostream& out, const Color& color) {
    std::visit(ColorPrint{out}, color);
    return out;
}


}  // namespace svg
