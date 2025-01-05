#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>
#include <boost/asio.hpp>

#include "OptionTypes.hpp"
#include "RiskMetrics.hpp"
#include "MarketDataHandler.hpp"
#include "OrderManagementSystem.hpp"
#include "ExecutionEngine.hpp"
#include "RiskManagement.hpp"
#include "GuiApp.hpp"
#include "PlotWindow.hpp"
#include "BlackScholesModel.hpp"

void printRiskMetrics(const RiskMetrics& metrics) {
    std::cout << std::fixed << std::setprecision(2)
              << "\n=== Portfolio Risk Metrics ===\n"
              << "Total Delta: " << metrics.totalDelta << "\n"
              << "Total Gamma: " << metrics.totalGamma << "\n"
              << "Total Theta: " << metrics.totalTheta << "\n"
              << "Total Vega: " << metrics.totalVega << "\n"
              << "Total Rho: " << metrics.totalRho << "\n"
              << "Portfolio Value: $" << metrics.portfolioValue << "\n"
              << "Value at Risk (95%): $" << metrics.valueAtRisk << "\n"
              << "Margin Requirement: $" << metrics.marginRequirement << "\n"
              << "==========================\n" << std::endl;
}

int main(int argc, char** argv) {
    boost::asio::io_context io_context;

    // Initialize with your Alpha Vantage API key
    const std::string API_KEY = "YL2LLPOL7A39ORIO";
    
    // Initialize Components
    MarketDataHandler mdHandler(io_context, API_KEY);
    OrderManagementSystem oms;
    ExecutionEngine execEngine;
    RiskManagement riskMgr;

    // Connect OMS and ExecutionEngine
    execEngine.setOrderManagementSystem(&oms);
    oms.setExecutionEngine(&execEngine);

    // Create some initial sample option orders
    OptionOrder sampleOrder1;
    sampleOrder1.underlying = "AAPL";
    sampleOrder1.optionType = "CALL";
    sampleOrder1.strike = 150.0;
    sampleOrder1.expiry = "2024-06-21";
    sampleOrder1.type = OptionOrder::Type::BUY_TO_OPEN;
    sampleOrder1.orderType = OptionOrder::OrderType::LIMIT;
    sampleOrder1.limitPrice = 5.0;
    sampleOrder1.quantity = 10;

    OptionOrder sampleOrder2;
    sampleOrder2.underlying = "MSFT";
    sampleOrder2.optionType = "PUT";
    sampleOrder2.strike = 300.0;
    sampleOrder2.expiry = "2024-06-21";
    sampleOrder2.type = OptionOrder::Type::SELL_TO_OPEN;
    sampleOrder2.orderType = OptionOrder::OrderType::LIMIT;
    sampleOrder2.limitPrice = 4.0;
    sampleOrder2.quantity = -5;

    // Subscribe to market data
    mdHandler.subscribeToSymbol("AAPL");
    mdHandler.subscribeToSymbol("MSFT");

    // Start all components
    mdHandler.start();
    oms.start();
    execEngine.start();

    // Submit initial orders
    oms.submitOptionOrder(sampleOrder1);
    oms.submitOptionOrder(sampleOrder2);

    // Run GUI
    GuiApp guiApp(argc, argv);
    PlotWindow* plotWindow = guiApp.getPlotWindow();

    // Connect market data to GUI
    mdHandler.setDataCallback([&](const OptionData& data) {
        std::cout << "\nReceived Market Data Update:"
                << "\nSymbol: " << data.underlying
                << "\nLast Price: " << data.lastPrice
                << "\nBid: " << data.bid
                << "\nAsk: " << data.ask
                << "\nImplied Vol: " << data.impliedVol * 100 << "%"
                << "\nDelta: " << data.delta
                << "\nGamma: " << data.gamma
                << "\nTheta: " << data.theta
                << std::endl;

        // Update risk metrics based on new market data
        std::unordered_map<std::string, double> underlyingPrices;
        underlyingPrices[data.underlying] = data.lastPrice;
        
        // Get current positions from OMS instead of using static positions
        std::vector<OptionPosition> currentPositions = oms.getPositions();
        
        RiskMetrics metrics = riskMgr.calculatePortfolioRisk(currentPositions, underlyingPrices);
        plotWindow->emitOptionChainUpdate({data});
        plotWindow->emitRiskMetricsUpdate(metrics);
        printRiskMetrics(metrics);
    });

    // Connect GUI orders to OMS
    QObject::connect(plotWindow, &PlotWindow::orderSubmitted,
        [&](const std::string& symbol, double strike, int quantity) {
            // Create a proper option order from GUI input
            OptionOrder order;
            order.underlying = symbol;
            order.strike = strike;
            order.quantity = quantity;
            order.type = quantity > 0 ? OptionOrder::Type::BUY_TO_OPEN 
                                    : OptionOrder::Type::SELL_TO_OPEN;
            order.orderType = OptionOrder::OrderType::MARKET;
            
            // Submit the order through OMS
            oms.submitOptionOrder(order);
        });

    int ret = guiApp.run();

    // Cleanup
    execEngine.stop();
    mdHandler.stop();
    oms.stop();

    return ret;
}