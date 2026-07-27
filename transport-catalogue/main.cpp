#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;

    input_reader::ReadAndApplyCommands(std::cin, catalogue);
    stat_reader::ReadAndPrintStats(std::cin, std::cout, catalogue);
}
