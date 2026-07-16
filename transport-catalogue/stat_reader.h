#pragma once

#include <iostream>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

void ReadAndPrintStats(std::istream& input, std::ostream& output,
                       const transport_catalogue::TransportCatalogue& transport_catalogue);
