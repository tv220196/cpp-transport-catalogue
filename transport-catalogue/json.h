#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Ёта ошибка должна выбрасыватьс€ при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

    class Node : public Value{
    public:
        Node(Value value);
        Node() = default;
        const Value& GetValue() const;
        //—ледующие методы Node сообщают, хранитс€ ли внутри значение некоторого типа :
        bool IsInt() const;
        bool IsDouble() const;//¬озвращает true, если в Node хранитс€ int либо double
        bool IsPureDouble() const;//¬озвращает true, если в Node хранитс€ double
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        //Ќиже перечислены методы, которые возвращают хран€щеес€ внутри Node значение заданного типа. 
        //≈сли внутри содержитс€ значение другого типа, должно выбрасыватьс€ исключение std::logic_error
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;//¬озвращает значение типа double, если внутри хранитс€ double либо int. ¬ последнем случае возвращаетс€ приведЄнное в double значение
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        Value value_;//еще можно без этой переменной обойтись, а в методе получени€ значени€ просто разыменовывать текущий объект 
                     //- подскажите, пожалуйста, как можно избавитьс€ от этой переменной? не пониманию, где хранить значение Node?
    };

    class Document {
    public:
        Document(Node root);
        Document() = default;
                
        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    bool operator== (const Node& left, const Node& right);
    bool operator!= (const Node& left, const Node& right);
    bool operator== (const Document& left, const Document& right);
    bool operator!= (const Document& left, const Document& right);
    std::ostream& operator<< (std::ostream& out, const Array& values);
    std::ostream& operator<< (std::ostream& out, const Dict& values);
}
