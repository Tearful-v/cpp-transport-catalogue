#include "json.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <exception>
#include <iostream>
#include <stdexcept>

int main() {
    try {
        transport_catalogue::TransportCatalogue catalogue;

        json::Document document = json::Load(std::cin);
        json_reader::JsonReader reader(document);

        reader.FillCatalogue(catalogue);

        json::Print(reader.ProcessRequests(catalogue), std::cout);
    } catch (const json::ParsingError& error) {
        std::cerr << "JSON parsing error: " << error.what() << '\n';
        return 1;
    } catch (const std::out_of_range& error) {
        std::cerr << "Missing or invalid JSON key: " << error.what() << '\n';
        return 1;
    } catch (const std::invalid_argument& error) {
        std::cerr << "Invalid argument: " << error.what() << '\n';
        return 1;
    } catch (const std::logic_error& error) {
        std::cerr << "Data logic error: " << error.what() << '\n';
        return 1;
    } catch (const std::runtime_error& error) {
        std::cerr << "Runtime error: " << error.what() << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << '\n';
        return 1;
    }
}
