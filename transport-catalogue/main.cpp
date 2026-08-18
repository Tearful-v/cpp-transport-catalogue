#include "json.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

int main() {
    transport_catalogue::TransportCatalogue catalogue;

    json::Document document = json::Load(std::cin);
    json_reader::JsonReader reader(document);

    reader.FillCatalogue(catalogue);

    json::Print(reader.ProcessRequests(catalogue), std::cout);
}
