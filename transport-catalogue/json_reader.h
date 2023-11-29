#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"


namespace json_reader {

    json::Document LoadJSON(std::istream& in);
    json::Document LoadJSON(std::istream& in);
     
    namespace input {

        struct CommandDescription {
            // ����������, ������ �� ������� (���� command ��������)
            explicit operator bool() const {
                return !name.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string name;      // �������� �������
            std::string id;           // id �������� ��� ���������
            json::Dict description;  // ��������� �������
        };

        class Input {
        public:
            Input() = default;

            void FormTransportCatalogue(const json::Document& requests, transport_catalogue::TransportCatalogue& catalogue);

            //������ ������ � ��������� CommandDescription � ��������� ��������� � commands_
            void ParseRequest(const json::Node& request);

            //��������� ������� ������������ ����������, ��������� ������� �� commands_
            void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };

    }

    namespace output {
        json::Document FormOutput(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue);
    }

    namespace visual_settings {
        void SetVisual(const json::Document& requests, map_render::MapRender& map);
    }

}

std::istream& operator>> (std::istream& in, json::Document& document);
std::ostream& operator<< (std::ostream & out, const json::Document& document);
