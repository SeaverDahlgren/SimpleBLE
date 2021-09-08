#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "simpleble/SimpleBLE.h"

std::vector<SimpleBLE::Peripheral> peripherals;

void print_byte_array(SimpleBLE::ByteArray& bytes) {
    for (auto byte : bytes) {
        std::cout << std::hex << std::setfill('0') << (uint32_t)((uint8_t)byte) << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    auto adapter_list = SimpleBLE::Adapter::get_adapters();

    if (adapter_list.size() == 0) {
        std::cout << "No adapter was found." << std::endl;
    }

    // Pick the first detected adapter as the default.
    // TODO: Allow the user to pick an adapter.
    SimpleBLE::Adapter adapter = adapter_list[0];

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });

    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) {
        std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        peripherals.push_back(peripheral);
    });

    // Scan for 5 seconds and return.
    adapter.scan_for(5000);

    std::cout << "The following devices were found:" << std::endl;
    for (int i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i].identifier() << " [" << peripherals[i].address() << "]"
                  << std::endl;
    }

    int selection = -1;
    std::cout << "Please select a device to connect to: ";
    std::cin >> selection;

    if (selection >= 0 && selection < peripherals.size()) {
        auto peripheral = peripherals[selection];
        std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
        peripheral.connect();

        std::cout << "Successfully connected, printing services and characteristics.." << std::endl;

        // Store all service and characteristic uuids in a vector.
        std::vector<std::pair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;
        for (auto service : peripheral.services()) {
            for (auto characteristic : service.characteristics) {
                uuids.push_back(std::make_pair(service.uuid, characteristic));
            }
        }

        std::cout << "The following services and characteristics were found:" << std::endl;
        for (int i = 0; i < uuids.size(); i++) {
            std::cout << "[" << i << "] " << uuids[i].first << " " << uuids[i].second << std::endl;
        }

        std::cout << "Please select a characteristic to read: ";
        std::cin >> selection;

        if (selection >= 0 && selection < uuids.size()) {
            // Attempt to read the characteristic.
            SimpleBLE::ByteArray rx_data = peripheral.read(uuids[selection].first, uuids[selection].second);
            std::cout << "Characteristic contents were: ";
            print_byte_array(rx_data);
        }
        peripheral.disconnect();
    }

    return 0;
}
