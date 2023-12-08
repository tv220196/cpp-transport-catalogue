#include "json_builder.h"
#include <memory>
#include <stdexcept>
#include <utility>
namespace json {

	KeyContext Builder::Key(std::string key) {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		if (!now_building_.empty()) {
			if (now_building_.back() == NOW_BUILDING::DICT && nodes_stack_.back()->IsString()) {
				throw std::logic_error("key has already been entered");
			}
			if (now_building_.back() == NOW_BUILDING::ARRAY) {
				throw std::logic_error("building object is not dictionary");
			}
		}
		if (now_building_.empty()) {	
			throw std::logic_error("building object is not dictionary");
		}
		Node* key_ptr = new Node;
		key_ptr->GetValue() = std::move(key);
		nodes_stack_.push_back(std::move(key_ptr));
		return *this;
	}
	Builder& Builder::Value(Node::Value val) {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		if (nodes_stack_.empty()) {
			root_.GetValue() = std::move(val);
			root_initialized_ = true;
		}
		else if (nodes_stack_.size() > 1 && nodes_stack_[nodes_stack_.size() - 2]->IsDict() && nodes_stack_.back()->IsString()) {
			std::string key = nodes_stack_.back()->AsString();
			nodes_stack_.pop_back();
			Dict dict = nodes_stack_.back()->AsDict();
			nodes_stack_.pop_back();
			Node value;
			value.GetValue() = std::move(val);
			dict[std::move(key)] = std::move(value);
			Node* dict_ptr = new Node;
			dict_ptr->GetValue() = dict;
			nodes_stack_.push_back(std::move(dict_ptr));
		}
		else if (nodes_stack_.back()->IsArray()) {
			Array array = nodes_stack_.back()->AsArray();
			nodes_stack_.pop_back();
			Node value;
			value.GetValue() = std::move(val);
			array.push_back(std::move(value));
			Node* array_ptr = new Node;
			array_ptr->GetValue() = array;
			nodes_stack_.push_back(std::move(array_ptr));
		}
		else {
			throw std::logic_error("invalid call");
		}
		return *this;
	}
	DictContext Builder::StartDict() {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		Node* dict_ptr = new Node;
		dict_ptr->GetValue() = Dict();
		nodes_stack_.push_back(std::move(dict_ptr));
		now_building_.push_back(NOW_BUILDING::DICT);
		return *this;
	}
	ArrayContext Builder::StartArray() {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		Node* array_ptr = new Node;
		array_ptr->GetValue() = Array();
		nodes_stack_.push_back(std::move(array_ptr));
		now_building_.push_back(NOW_BUILDING::ARRAY);
		return *this;
	}
	Builder& Builder::EndDict() {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		if (now_building_.back() != NOW_BUILDING::DICT) {
			throw std::logic_error("building object is not dictionary");
		}
		if (now_building_.size() == 1) {
			root_.GetValue() = nodes_stack_.back()->AsDict();
			root_initialized_ = true;
			nodes_stack_.pop_back();
			now_building_.pop_back();
		}
		else if (now_building_.size() > 1 && 
			    (now_building_[now_building_.size() - 2] == NOW_BUILDING::DICT || now_building_[now_building_.size() - 2] == NOW_BUILDING::ARRAY)) {
			Dict dict = nodes_stack_.back()->AsDict();
			nodes_stack_.pop_back();
			this->Value(dict);
			now_building_.pop_back();
		}
		return *this;
	}
	Builder& Builder::EndArray() {
		if (root_initialized_) {
			throw std::logic_error("object has already completed");
		}
		if (now_building_.back() != NOW_BUILDING::ARRAY) {
			throw std::logic_error("building object is not array");
		}
		if (now_building_.size() == 1) {
			root_.GetValue() = nodes_stack_.back()->AsArray();
			root_initialized_ = true;
			nodes_stack_.pop_back();
			now_building_.pop_back();
		}
		else if (now_building_.size() > 1 &&
			    (now_building_[now_building_.size() - 2] == NOW_BUILDING::ARRAY || now_building_[now_building_.size() - 2] == NOW_BUILDING::DICT)) {
			Array array = nodes_stack_.back()->AsArray();
			nodes_stack_.pop_back();
			this->Value(array);
			now_building_.pop_back();
		}
		return *this;
	}
	Node Builder::Build() {
		if (!root_initialized_) {
			throw std::logic_error("object is not completed");
		}
		return root_;
	}

	//DictContext
	DictContext::DictContext(Builder& builder)
		:builder_(builder) {}
	KeyContext DictContext::Key(std::string key) {
		builder_.Key(std::move(key));
		return KeyContext(builder_);
	}
	Builder& DictContext::EndDict() {
		builder_.EndDict();
		return builder_;
	}

	//KeyContext
	KeyContext::KeyContext(Builder& builder)
		:builder_(builder) {}
	DictContext KeyContext::Value(Node::Value val) {
		builder_.Value(val);
		return DictContext(builder_);
	}
	DictContext KeyContext::StartDict() {
		builder_.StartDict();
		return DictContext(builder_);
	}
	ArrayContext KeyContext::StartArray() {
		builder_.StartArray();
		return ArrayContext(builder_);
	}

	//ArrayContext
	ArrayContext::ArrayContext(Builder& builder)
		:builder_(builder) {}
	ArrayContext ArrayContext::Value(Node::Value val) {
		builder_.Value(val);
		return ArrayContext(builder_);
	}
	DictContext ArrayContext::StartDict() {
		builder_.StartDict();
		return DictContext(builder_);
	}
	ArrayContext ArrayContext::StartArray() {
		builder_.StartArray();
		return ArrayContext(builder_);
	}
	Builder& ArrayContext::EndArray() {
		builder_.EndArray();
		return builder_;
	}
}