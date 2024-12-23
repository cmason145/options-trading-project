#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "MarketDataHandler.hpp"
#include "OrderManagementSystem.hpp"
#include "ExecutionEngine.hpp"
#include "RiskManagement.hpp"
#include "GuiApp.hpp"

int main(int argc, char** argv) {
    boost::asio::io_context io_context;

    // Initialize Components
    MarketDataHandler mdHandler(io_context);
    OrderManagementSystem oms;
    ExecutionEngine execEngine;
    RiskManagement riskMgr;

    // Start Market Data Handler
    mdHandler.setDataCallback([&](const std::string& data) {
        std::cout << "Received Market Data: " << data;
    });
    mdHandler.start();

    // Start OMS and send a mock order
    oms.start();
    oms.sendOrder("AAPL", 150.0, 100);

    // Add some dummy orders to ExecutionEngine
    execEngine.addOrder("BUY 100 AAPL @150.00");
    execEngine.addOrder("SELL 50 MSFT @300.00");
    execEngine.run();

    // Compute Risk
    std::vector<double> positions = {1000000.0, 2000000.0, 1500000.0};
    double risk = riskMgr.calculateRiskMetric(positions);
    std::cout << "Current Risk Metric: " << risk << std::endl;

    // Run GUI
    GuiApp guiApp(argc, argv);
    int ret = guiApp.run();

    // Cleanup
    mdHandler.stop();
    oms.stop();

    return ret;
}
