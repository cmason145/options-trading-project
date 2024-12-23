#include "MarketDataHandler.hpp"
#include <iostream>
#include <chrono>

MarketDataHandler::MarketDataHandler(boost::asio::io_context& ioc)
: io_context_(ioc) {}

MarketDataHandler::~MarketDataHandler() {
    stop();
}

void MarketDataHandler::start() {
    running_ = true;
    data_thread_ = std::thread([this]() {
        while (running_) {
            // Mock market data reception
            std::string mockData = "TICK:SYMBOL=XYZ PRICE=" + std::to_string(rand() % 100) + "\n";
            if (callback_) {
                callback_(mockData);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void MarketDataHandler::stop() {
    running_ = false;
    if (data_thread_.joinable()) {
        data_thread_.join();
    }
}

void MarketDataHandler::setDataCallback(DataCallback cb) {
    callback_ = cb;
}
