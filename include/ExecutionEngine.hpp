#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include <vector>
#include <queue>
#include <string>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include "OptionTypes.hpp"

class OrderManagementSystem;

class ExecutionEngine {
public:
    ExecutionEngine();
    ~ExecutionEngine();

    // Core execution methods
    void start();
    void stop();
    void addOrder(const OptionOrder& order);
    
    // OMS connection
    void setOrderManagementSystem(OrderManagementSystem* oms);
    
    // Market simulation settings
    void setSimulatedSlippage(double slippage);
    void setSimulatedFillRate(double fillRate);

private:
    // Internal processing methods
    void processOrders();
    void processOrder(const OptionOrder& order);
    bool tryExecuteOrder(const OptionOrder& order);
    double calculateFillPrice(const OptionOrder& order) const;
    bool shouldFillOrder(const OptionOrder& order) const;
    
    // Threading
    void executionLoop();
    
    // Member variables
    std::queue<OptionOrder> orderQueue_;
    std::mutex queueMutex_;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> executionThread_;
    
    // Configuration
    double simulatedSlippage_{0.01};  // 1% default slippage
    double simulatedFillRate_{0.95};  // 95% default fill rate
    
    // OMS reference
    OrderManagementSystem* oms_{nullptr};
    
    // Statistics
    std::atomic<uint64_t> totalOrdersProcessed_{0};
    std::atomic<uint64_t> totalOrdersFilled_{0};
    std::atomic<uint64_t> totalOrdersRejected_{0};
};

#endif