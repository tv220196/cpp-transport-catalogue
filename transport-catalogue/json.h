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

    // ��� ������ ������ ������������� ��� ������� �������� JSON
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
        //��������� ������ Node ��������, �������� �� ������ �������� ���������� ���� :
        bool IsInt() const;
        bool IsDouble() const;//���������� true, ���� � Node �������� int ���� double
        bool IsPureDouble() const;//���������� true, ���� � Node �������� double
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        //���� ����������� ������, ������� ���������� ���������� ������ Node �������� ��������� ����. 
        //���� ������ ���������� �������� ������� ����, ������ ������������� ���������� std::logic_error
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;//���������� �������� ���� double, ���� ������ �������� double ���� int. � ��������� ������ ������������ ���������� � double ��������
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        Value value_;//��� ����� ��� ���� ���������� ��������, � � ������ ��������� �������� ������ �������������� ������� ������ 
                     //- ����������, ����������, ��� ����� ���������� �� ���� ����������? �� ���������, ��� ������� �������� Node?
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
