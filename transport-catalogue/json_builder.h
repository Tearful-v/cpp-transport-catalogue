#pragma once

#include "json.h"

#include <optional>
#include <stack>
#include <string>
#include <stdexcept>
#include <utility>

namespace json {

class Builder;
class BaseContext;
class DictItemContext;
class ArrayItemContext;
class KeyContext;


class Builder {
public:
    json::Node Build();

    KeyContext Key(std::string key);
    Builder& Value(json::Value value);

    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& EndDict();
    Builder& EndArray();

private:
    json::Node* AddNode(Node node);

    json::Node root_;
    std::optional<std::string> key_;
    std::stack<json::Node*> opened_container_;
    bool root_is_ready_ = false;
};


class BaseContext {
public:
    explicit BaseContext(Builder& builder)
        : builder_(builder) {
    }

    json::Node Build();
    KeyContext Key(std::string key);
    Builder& Value(json::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

protected:
    Builder& builder_;
};


class KeyContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    DictItemContext Value(json::Value value);

    json::Node Build() = delete;
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};


class DictItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;

    json::Node Build() = delete;
    Builder& Value(json::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};


class ArrayItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;
    ArrayItemContext Value(json::Value value);

    json::Node Build() = delete;
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
};

}// json
