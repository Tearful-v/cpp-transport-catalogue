#pragma once

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <iosfwd>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

class Node final : private Value {
public:
   /* Реализуйте Node, используя std::variant */

    using Value::Value;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    double AsDouble() const;
    bool AsBool() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    bool operator ==(const Node& other) const;
    bool operator !=(const Node& other) const;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
