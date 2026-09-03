#include "json_builder.h"

namespace json {

//------------------BaseContext-------------

    Builder::KeyContext Builder::Key(std::string key) {
        if (opened_container_.empty() || !opened_container_.top()->IsMap()) {
            throw std::logic_error("Key outside dict");
        }
        if (!key_.has_value()) {
            key_ = std::move(key);
        } else {
            throw std::logic_error("Key isnt empty, expected value");
        }
        return KeyContext{*this};
    }

    json::Node* Builder::AddNode(Node node) {
        if (opened_container_.empty()) {
            if (root_.has_value()) {
                throw std::logic_error("root already exist");
            }
            root_ = std::move(node);
            return &root_.value();
        }

        auto* container = opened_container_.top();
        if (container->IsArray()) {
            auto& array = container->AsArray();
            array.push_back(std::move(node));
            return &array.back();
        }

        if (container->IsMap()) {
            if (!key_.has_value()) {
                throw std::logic_error("Expected key before value");
            }
            auto& dict = container->AsMap();
            auto& it = dict[*key_] = std::move(node);
            key_.reset();
            return &it;
        }

        throw std::logic_error("call in wrong context");
    }

    json::Node Builder::Build() {
        if (!root_.has_value()) {
            throw std::logic_error("JSON is empty");
        } if (!opened_container_.empty()) {
            throw  std::logic_error("JSON is not ready yet");
        }
        Node result = std::move(root_.value());
        root_.reset();
        return result;
    }

    Builder& Builder::Value(json::Value value) {
        json::Node node{std::move(value)};
        (void) AddNode(std::move(node));
        return *this;
    }

    Builder& Builder::NodeValue(json::Node node) {
        (void) AddNode(std::move(node));
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        StartContainer(Node{Dict{}});
        return DictItemContext{*this};
    }

    Builder::ArrayItemContext Builder::StartArray() {
        StartContainer(Node{Array{}});
        return ArrayItemContext{*this};
    }

    json::Node* Builder::StartContainer(Node node) {
        Node* new_container = AddNode(std::move(node));
        opened_container_.push(new_container);
        return new_container;
    }

    Builder& Builder::EndDict() {
        if (opened_container_.empty()) {
            throw std::logic_error("Nothing to close");
        }
        if (!opened_container_.top()->IsMap()) {
            throw std::logic_error("Expected dict");
        }
        if (key_.has_value()) {
            throw std::logic_error("Expected value after key");
        }
        opened_container_.pop();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (opened_container_.empty()) {
            throw std::logic_error("Nothing to close");
        }
        if (!opened_container_.top()->IsArray()) {
            throw std::logic_error("Expected array");
        }
        opened_container_.pop();
        return *this;
    }

    //------------ BaseContext ---------------

    Builder::DictItemContext Builder::KeyContext::Value(json::Value value) {
        builder_.Value(std::move(value));
        return DictItemContext{builder_};
    }

    Builder::DictItemContext Builder::KeyContext::NodeValue(json::Node node) {
        builder_.NodeValue(std::move(node));
        return DictItemContext{builder_};
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(json::Value value) {
        builder_.Value(std::move(value));
        return ArrayItemContext{builder_};
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::NodeValue(json::Node node) {
        builder_.NodeValue(std::move(node));
        return ArrayItemContext{builder_};
    }

    json::Node Builder::BaseContext::Build() {
        return builder_.Build();
    }
    Builder::KeyContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }
    Builder& Builder::BaseContext::Value(json::Value value) {
        return builder_.Value(std::move(value));
    }
    Builder& Builder::BaseContext::NodeValue(json::Node node) {
        return builder_.NodeValue(std::move(node));
    }
    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }
    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }
    Builder& Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }

} //json
