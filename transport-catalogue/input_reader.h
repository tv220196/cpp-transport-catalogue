#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace input {
    struct CommandDescription {
        // ����������, ������ �� ������� (���� command ��������)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // �������� �������
        std::string id;           // id �������� ��� ���������
        std::string description;  // ��������� �������
    };

    class InputReader {
    public:
        /**
         * ������ ������ � ��������� CommandDescription � ��������� ��������� � commands_
         */
        void ParseLine(std::string_view line);

        /**
         * ��������� ������� ������������ ����������, ��������� ������� �� commands_
         */
        void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

    private:
        std::vector<CommandDescription> commands_;
    };
}