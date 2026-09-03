#pragma once

#include "json.h"

#include <optional>
#include <stack>
#include <string>
#include <stdexcept>
#include <utility>

namespace json {

class Builder {
public:
    class BaseContext;
    class DictItemContext;
    class ArrayItemContext;
    class KeyContext;

    json::Node Build();

    KeyContext Key(std::string key);
    Builder& Value(json::Value value);
    Builder& NodeValue(json::Node node);

    DictItemContext StartDict();
    ArrayItemContext StartArray();

    Builder& EndDict();
    Builder& EndArray();

private:
    json::Node* AddNode(Node node);
    json::Node* StartContainer(Node node);

    std::optional<json::Node> root_;
    std::optional<std::string> key_;
    std::stack<json::Node*> opened_container_;
};


class Builder::BaseContext {
public:
    explicit BaseContext(Builder& builder)
        : builder_(builder) {
    }

    json::Node Build();
    KeyContext Key(std::string key);
    Builder& Value(json::Value value);
    Builder& NodeValue(json::Node node);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();

protected:
    Builder& builder_;
};


class Builder::KeyContext : public Builder::BaseContext {
public:
    using BaseContext::BaseContext;
    DictItemContext Value(json::Value value);
    DictItemContext NodeValue(json::Node node);

    json::Node Build() = delete;
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};


class Builder::DictItemContext : public Builder::BaseContext {
public:
    using BaseContext::BaseContext;

    json::Node Build() = delete;
    Builder& Value(json::Value value) = delete;
    Builder& NodeValue(json::Node node) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
};


class Builder::ArrayItemContext : public Builder::BaseContext {
public:
    using BaseContext::BaseContext;
    ArrayItemContext Value(json::Value value);
    ArrayItemContext NodeValue(json::Node node);

    json::Node Build() = delete;
    KeyContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
};

}// json
