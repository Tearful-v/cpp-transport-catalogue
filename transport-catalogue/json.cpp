#include "json.h"
#include <iostream>
#include <string_view>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    if (!(input >> c)) {
        throw ParsingError("Array not closed");
    }

    if (c == ']') {
        return Node(std::move(result));
    }

    input.putback(c);

    while (true) {
        result.push_back(LoadNode(input));
        if (!(input >> c)) {
            throw ParsingError("Array not closed");
        }
        if (c == ']') {
            return Node(std::move(result));
        }
        if (c != ',') {
            throw ParsingError("Expected coma");
        }
    }
}

Node LoadNum(istream& input) {
    string number;

    if (input.peek() == '-') {
        number += input.get();
    }

    while (isdigit(input.peek())) {
        number += input.get();
    }

    bool is_double = false;

    if (input.peek() == '.') {
        is_double = true;
        number += input.get();

        while (isdigit(input.peek())) {
            number += input.get();
        }
    }

    if (input.peek() == 'e' || input.peek() == 'E') {
        is_double = true;
        number += input.get();

        if (input.peek() == '+' || input.peek() == '-') {
            number += input.get();
        }

        while (isdigit(input.peek())) {
            number += input.get();
        }
    }

    if (is_double) {
        return Node(stod(number));
    }

    return Node(stoi(number));
}

Node LoadString(istream& input) {
    std::string result;
    char ch;
    while (input.get(ch)) {
        if (ch == '"') {
            return Node(std::move(result));
        }
        if (ch == '\\') {
            if (!(input.get(ch))) {
                throw ParsingError("String is not closed");
            }
            if (ch == 'n') {
                result += '\n';
            } else if (ch == 'r') {
                result += '\r';
            } else if (ch == 't') {
                result += '\t';
            } else if (ch == '"') {
                result += '"';
            } else if (ch == '\\') {
                result += '\\';
            } else {
                throw ParsingError("unknown combination");
            }
        } else {
            result += ch;
        }
    }
    throw ParsingError("string not closed");
}

Node LoadDict(istream& input) {
    Dict result;
    char c;

    if (!(input >> c)) {
        throw ParsingError("Dict not closed");
    }

    if (c == '}') {
        return Node(std::move(result));
    }

    while (true) {
        if (c != '"') {
            throw ParsingError("key must be string");
        }
        std::string key = LoadString(input).AsString();
        if (!(input >> c) || c != ':') {
            throw ParsingError("Expected :");
        }
        result.insert({std::move(key), LoadNode(input)});

        if (!(input >> c)) {
            throw ParsingError("Dict not closed");
        }
        if (c == '}') {
            return Node(std::move(result));
        }
        if (c != ',') {
            throw ParsingError("Expected ,");
        }
        if (!(input >> c)) {
            throw ParsingError("Dict not closed");
        }
    }
}
void ReadLiteral(istream& input, string_view expected) {
    for (char expected_char : expected) {
        char actual_char;

        if (!input.get(actual_char) || actual_char != expected_char) {
            throw ParsingError("Invalid literal");
        }
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't') {
        ReadLiteral(input, "rue");
        return Node(true);
    }
    else if (c == 'f') {
        ReadLiteral(input, "alse");
        return Node(false);
    }
    else if (c == 'n') {
        ReadLiteral(input, "ull");
        return Node();
    } else if (c == '-' || std::isdigit(c)) {
        input.putback(c);
        return LoadNum(input);
    } else {
        throw ParsingError("wrong input");
    }
}

}  // namespace


//======================================================

Node::Node()
    : value_(nullptr) {
}

Node::Node(Array array) {
    value_ = std::move(array);
}

Node::Node(Dict map) {
    value_ = std::move(map);
}

Node::Node(int value) {
    value_ = value;
}

Node::Node(double value) {
    value_ = value;
}

Node::Node(std::string value) {
    value_ = std::move(value);
}

Node::Node(bool value){
    value_ = value;
}

Node::Node(std::nullptr_t) {
    value_ = nullptr;
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Not array");
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Not dict");
    }
    return std::get<Dict>(value_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Not int");
    }
    return std::get<int>(value_);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Not string");
    }
    return std::get<std::string>(value_);
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(value_);
    }

    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }

    throw std::logic_error("Not double");
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Not bool");
    }
    return std::get<bool>(value_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

bool Node::operator ==(const Node& other) const {
    return value_ == other.value_;
}

bool Node::operator !=(const Node& other) const {
    return !(*this == other);
}

//==================================================================

void PrintString(const string& str, ostream& output) {
    output << '"';

    for (char c : str) {
        if (c == '\n') {
            output << "\\n";
        } else if (c == '\r') {
            output << "\\r";
        } else if (c == '\t') {
            output << "\\t";
        } else if (c == '"') {
            output << "\\\"";
        } else if (c == '\\') {
            output << "\\\\";
        } else {
            output << c;
        }
    }

    output << '"';
}

void PrintNode(const Node& node, ostream& output) {
    if (node.IsNull()) {
        output << "null";
    } else if (node.IsInt()) {
        output << node.AsInt();
    } else if (node.IsPureDouble()) {
        output << node.AsDouble();
    } else if (node.IsBool()) {
        output << (node.AsBool() ? "true" : "false");
    } else if (node.IsString()) {
        PrintString(node.AsString(), output);
    } else if (node.IsArray()) {
        output << '[';

        bool first = true;
        for (const Node& value : node.AsArray()) {
            if (!first) {
                output << ',';
            }

            PrintNode(value, output);
            first = false;
        }

        output << ']';
    } else if (node.IsMap()) {
        output << '{';

        bool first = true;
        for (const auto& [key, value] : node.AsMap()) {
            if (!first) {
                output << ',';
            }

            PrintString(key, output);
            output << ':';
            PrintNode(value, output);

            first = false;
        }

        output << '}';
    }
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    Node root = LoadNode(input);
    char c;
    if (input >> c) {
        throw ParsingError("Unexpected data after root");
    }
    return Document{(root)};
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

void Print(const Document& doc, ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json
