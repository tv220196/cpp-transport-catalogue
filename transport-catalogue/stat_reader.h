#pragma once

#include <iosfwd>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"

namespace request {

    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output);

}