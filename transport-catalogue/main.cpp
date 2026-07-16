#include <iostream>

#include "input_reader.h"
#include "stat_reader.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;

    ReadAndApplyCommands(std::cin, catalogue);
    ReadAndPrintStats(std::cin, std::cout, catalogue);
}
