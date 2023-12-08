#pragma once
#include "json.h"

namespace json {
	class DictContext;
	class KeyContext;
	class ArrayContext;

	class Builder {
	public:
		Builder() = default;
		KeyContext Key(std::string key);
		Builder& Value(Node::Value val);
		DictContext StartDict();
		ArrayContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
		bool root_initialized_ = false;
		enum NOW_BUILDING {
			DICT,
			ARRAY
		};
		std::vector<NOW_BUILDING> now_building_;
	};

	class DictContext {
	public:
		DictContext(Builder& builder);
		KeyContext Key(std::string key);
		Builder& EndDict();
		Builder& Value(Node::Value val) = delete;
		DictContext StartDict() = delete;
		ArrayContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	private:
		Builder& builder_;
	};

	class KeyContext {
	public:
		KeyContext(Builder& builder);
		DictContext Value(Node::Value val);
		DictContext StartDict();
		ArrayContext StartArray();
		KeyContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	private:
		Builder& builder_;
	};

	class ArrayContext {
	public:
		ArrayContext(Builder& builder);
		ArrayContext Value(Node::Value val);
		DictContext StartDict();
		ArrayContext StartArray();
		Builder& EndArray();
		KeyContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Node Build() = delete;
	private:
		Builder& builder_;
	};
}