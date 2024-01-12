#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"


namespace json_reader {

    json::Document LoadJSON(std::istream& in);
     
    namespace input {

        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !name.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string name;      // Название команды
            std::string id;           // id маршрута или остановки
            json::Dict description;  // Параметры команды
        };

        class Input {
        public:
            Input() = default;
            void FormTransportCatalogue(const json::Document& requests, transport_catalogue::TransportCatalogue& catalogue);
            void FormMap(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue, map_render::MapRender& map);
            transport_router::RoutingSettings FormRoutingSettings(const json::Document& requests);

            //Парсит строку в структуру CommandDescription и сохраняет результат в commands_
            void ParseRequest(const json::Node& request);

            //Наполняет данными транспортный справочник, используя команды из commands_
            void ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const;

        private:
            std::vector<CommandDescription> commands_;
        };

    }

    namespace output {
        json::Document FormOutput(const json::Document& requests, const transport_catalogue::TransportCatalogue& catalogue, map_render::MapRender& map, 
                                  const transport_router::BusGraph& bus_graph);
    }

    namespace visual_settings {
        void SetVisual(const json::Document& requests, map_render::MapRender& map);
    }
}

std::istream& operator>> (std::istream& in, json::Document& document);
std::ostream& operator<< (std::ostream & out, const json::Document& document);
