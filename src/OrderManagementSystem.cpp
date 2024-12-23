#include "OrderManagementSystem.hpp"
#include <iostream>

OrderManagementSystem::OrderManagementSystem() {}
OrderManagementSystem::~OrderManagementSystem() {}

void OrderManagementSystem::start() {
    // Mock start logic
    std::cout << "OMS started." << std::endl;
}

void OrderManagementSystem::stop() {
    // Mock stop logic
    std::cout << "OMS stopped." << std::endl;
}

void OrderManagementSystem::sendOrder(const std::string& symbol, double price, int quantity) {
    // Mock order sending
    std::cout << "Sending Order -> Symbol: " << symbol
              << ", Price: " << price
              << ", Quantity: " << quantity << std::endl;
}
