#include "json_builder.h"

namespace json {

//------------------BaseContext-------------

    KeyContext Builder::Key(std::string key) {
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
            if (root_is_ready_) {
                throw std::logic_error("root already exist");
            }
            root_ = std::move(node);
            root_is_ready_ = true;
            return &root_;
        }

        auto* container = opened_container_.top();
        if (container->IsArray()) {
            auto& array = const_cast<json::Array&>(container->AsArray());
            array.push_back(std::move(node));
            return &array.back();
        }

        if (container->IsMap()) {
            if (!key_.has_value()) {
                throw std::logic_error("Expected key before value");
            }
            auto& dict = const_cast<json::Dict&>(container->AsMap());
            auto& it = dict[*key_] = std::move(node);
            key_.reset();
            return &it;
        }

        throw std::logic_error("call in wrong context");
    }

    json::Node Builder::Build() {
        if (!root_is_ready_) {
            throw std::logic_error("JSON is empty");
        } if (!opened_container_.empty()) {
            throw  std::logic_error("JSON is not ready yet");
        }
        return std::move(root_);
    }

    Builder& Builder::Value(json::Value value) {
        json::Node node = std::visit([](auto&& value){
            return Node{std::move(value)};
        }, std::move(value));
        (void) AddNode(std::move(node));
        return *this;
    }

    DictItemContext Builder::StartDict() {
        Node* new_dict = AddNode(Node{Dict{}});
        opened_container_.push(new_dict);
        return DictItemContext{*this};
    }

    ArrayItemContext Builder::StartArray() {
        Node* new_array = AddNode(Node{Array{}});
        opened_container_.push(new_array);
        return ArrayItemContext{*this};
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

    DictItemContext KeyContext::Value(json::Value value) {
        builder_.Value(std::move(value));
        return DictItemContext{builder_};
    }

    ArrayItemContext ArrayItemContext::Value(json::Value value) {
        builder_.Value(std::move(value));
        return ArrayItemContext{builder_};
    }

    json::Node BaseContext::Build() {
        return builder_.Build();
    }
    KeyContext BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }
    Builder& BaseContext::Value(json::Value value) {
        return builder_.Value(std::move(value));
    }
    DictItemContext BaseContext::StartDict() {
        return builder_.StartDict();
    }
    ArrayItemContext BaseContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& BaseContext::EndDict() {
        return builder_.EndDict();
    }
    Builder& BaseContext::EndArray() {
        return builder_.EndArray();
    }

} //json
