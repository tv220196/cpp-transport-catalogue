#include "json.h"

using namespace std;

namespace json {

    namespace {

        using Number = variant<int, double>;

        Number LoadNumber(istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        string LoadString(istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            char c;
            input >> c;
            if (c == 'u') {
                input >> c;
                if (c == 'l' && istreambuf_iterator<char>(input) != istreambuf_iterator<char>()) {
                    input >> c;
                    if (c == 'l') {
                        if ((input.peek() >= 'A' && input.peek() <= 'Z') || (input.peek() >= 'a' && input.peek() <= 'z')) {
                            throw ParsingError("Null parsing error");
                        }
                        else {
                            return Node{ nullptr };
                        }
                        //return Node{ nullptr };
                    }
                    else {
                        throw ParsingError("Null parsing error");
                    }
                }
                else {
                    throw ParsingError("Null parsing error");
                }
            }
            else {
                throw ParsingError("Null parsing error");
            }
        }

        Node LoadNum(istream& input) {
            auto result = LoadNumber(input);
            if (holds_alternative<int>(result)) {
                return Node{ get<int>(result) };
            }
            else {
                return Node{ get<double>(result) };
            }
        }

        Node LoadStr(istream& input) {
            string line = LoadString(input);
            return Node(move(line));
        }

        Node LoadBool(istream& input) {
            char c;
            input >> c;
            if (c == 't') {
                input >> c;
                if (c == 'r') {
                    input >> c;
                    if (c == 'u') {
                        input >> c;
                        if (c == 'e') {
                            if ((input.peek() >= 'A' && input.peek() <= 'Z') || (input.peek() >= 'a' && input.peek() <= 'z')) {
                                throw ParsingError("Bool parsing error");
                            }
                            else {
                                return Node{ true };
                            }
                            //return Node{ true };
                        }
                        else {
                            throw ParsingError("Bool parsing error");
                        }
                    }
                    else {
                        throw ParsingError("Bool parsing error");
                    }
                }
                else {
                    throw ParsingError("Bool parsing error");
                }
            }
            else {
                input >> c;
                if (c == 'a') {
                    input >> c;
                    if (c == 'l') {
                        input >> c;
                        if (c == 's') {
                            input >> c;
                            if (c == 'e') {
                                if ((input.peek() >= 'A' && input.peek() <= 'Z') || (input.peek() >= 'a' && input.peek() <= 'z')) {
                                    throw ParsingError("Bool parsing error");
                                }
                                else {
                                    return Node{ false };
                                }
                                //return Node{ false };
                            }
                            else {
                                throw ParsingError("Bool parsing error");
                            }
                        }
                        else {
                            throw ParsingError("Bool parsing error");
                        }
                    }
                    else {
                        throw ParsingError("Bool parsing error");
                    }
                }
                else {
                    throw ParsingError("Bool parsing error");
                }
            }
        }

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == EOF) {
                throw ParsingError("Array parsing error");
            }
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == EOF) {
                throw ParsingError("Dict parsing error");
            }
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadStr(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;
            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadStr(input);
            }
            else if (c == 'n') {
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '0' || c == '1' || c == '2' || c == '3' ||
                c == '4' || c == '5' || c == '6' || c == '7' ||
                c == '8' || c == '9' || c == '-') {
                input.putback(c);
                return LoadNum(input);
            }
            else {
                throw ParsingError("Parsing error");
            }
        }
    }

    Node::Node(nullptr_t) {
        value_ = nullptr;
    }
    Node::Node(int value)
        :value_(move(value)) {}
    Node::Node(double value)
        :value_(move(value)) {}
    Node::Node(string value)
        :value_(move(value)) {}
    Node::Node(bool value)
        :value_(move(value)) {}
    Node::Node(Array value)
        :value_(move(value)) {}
    Node::Node(Dict value)
        :value_(move(value)) {}
    const Node::Value& Node::GetValue() const {
        return value_;
    }
    bool Node::IsInt() const {
        return holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const {
        return holds_alternative<int>(value_) || holds_alternative<double>(value_);
    }
    bool Node::IsPureDouble() const {
        return holds_alternative<double>(value_);
    }
    bool Node::IsBool() const {
        return holds_alternative<bool>(value_);
    }
    bool Node::IsString() const {
        return holds_alternative<string>(value_);
    }
    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(value_);
    }
    bool Node::IsArray() const {
        return holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const {
        return holds_alternative<Dict>(value_);
    }
    int Node::AsInt() const {
        if (holds_alternative<int>(value_)) {
            return get<int>(value_);
        }
        else {
            throw logic_error("incorrect data type");
        }
    }
    bool Node::AsBool() const {
        if (holds_alternative<bool>(value_)) {
            return get<bool>(value_);
        }
        else {
            throw logic_error("incorrect data type");
        }
    }
    double Node::AsDouble() const {//Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение
        if (holds_alternative<double>(value_)) {
            return get<double>(value_);
        }
        else if (holds_alternative<int>(value_)) {
            return static_cast<double>(get<int>(value_));
        }
        else {
            throw logic_error("incorrect data type");
        }
    }
    const string& Node::AsString() const {
        if (holds_alternative<string>(value_)) {
            return get<string>(value_);
        }
        else {
            throw logic_error("incorrect data type");
        }
    }
    const Array& Node::AsArray() const {
        if (holds_alternative<Array>(value_)) {
            return get<Array>(value_);
        }
        else {
            throw logic_error("incorrect data type");
        }
    }
    const Dict& Node::AsMap() const {
        if (holds_alternative<Dict>(value_)) {
            return get<Dict>(value_);
        }
        else {
            throw logic_error("incorrect data type");
        }
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    string PrintString(string value) {
        string s;
        s += "\"";
        for (size_t i = 0; i < value.size(); ++i) {
            switch (value[i]) {
            case '"':
                s += "\\\"";
                break;
            case '\r':
                s += "\\r";
                break;
            case '\n':
                s += "\\n";
                break;
            case '\t':
                s += "\\t";
                break;
            case '\\':
                s += "\\\\";
                break;
            default:
                s.push_back(value[i]);
            }
        }
        s += "\"";
        return s;
    }

    struct NodePrinter {
        ostream& out;
        void operator() (nullptr_t) const {
            out << "null";
        }
        void operator() (int value) const {
            out << value;
        }
        void operator() (double value) const {
            out << value;
        }
        void operator() (string value) const {
            out << PrintString(value);
        }
        void operator() (bool value) const {
            out << boolalpha << value;
        }
        void operator() (Array value) const {
            out << value;
        }
        void operator() (Dict value) const {
            out << value;
        }
    };

    void Print(const Document& doc, ostream& output) {
        visit(NodePrinter{ output }, doc.GetRoot().GetValue());
    }

    bool operator== (const Node& left, const Node& right) {
        return left.GetValue() == right.GetValue();
    }
    bool operator!= (const Node& left, const Node& right) {
        return left.GetValue() != right.GetValue();
    }
    bool operator== (const Document& left, const Document& right) {
        return left.GetRoot() == right.GetRoot();
    }
    bool operator!= (const Document& left, const Document& right) {
        return left.GetRoot() != right.GetRoot();
    }

    ostream& operator<< (ostream& out, const Array& values) {
        out << "[";
        if (values.size() != 0) {
            for (size_t i = 0; i < values.size() - 1; ++i) {
                visit(NodePrinter{ out }, values[i].GetValue());
                out << ", ";
            }
            visit(NodePrinter{ out }, values[values.size() - 1].GetValue());
        }
        out << "]";
        return out;
    }

    ostream& operator<< (ostream& out, const Dict& values) {
        out << "{";
        if (values.size() != 0) {
            size_t count = 0;
            for (const auto& val : values) {
                if (count != values.size() - 1) {
                    out << "\"" << val.first << "\": ";
                    visit(NodePrinter{ out }, val.second.GetValue());
                    out << ", ";
                }
                else {
                    out << "\"" << val.first << "\": ";
                    visit(NodePrinter{ out }, val.second.GetValue());
                }
                ++count;
            }
        }
        out << "}";
        return out;
    }

}