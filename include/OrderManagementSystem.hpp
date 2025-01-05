#ifndef ORDER_MANAGEMENT_SYSTEM_HPP
#define ORDER_MANAGEMENT_SYSTEM_HPP

#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include "OptionTypes.hpp"

class ExecutionEngine;

class OrderManagementSystem {
public:
    OrderManagementSystem();
    ~OrderManagementSystem();

    // Start and stop methods
    void start();
    void stop();

    // Set execution engine
    void setExecutionEngine(ExecutionEngine* engine) {
        executionEngine_ = engine;
    }

    // Order management methods
    void sendOrder(const std::string& symbol, double price, int quantity);
    std::string submitOptionOrder(const OptionOrder& order);
    void cancelOrder(const std::string& orderId);
    void modifyOrder(const std::string& orderId, const OptionOrder& newOrder);
    
    // Order tracking
    std::vector<OptionOrder> getActiveOrders() const;
    OptionOrder getOrderStatus(const std::string& orderId) const;

    // Position management
    std::vector<OptionPosition> getPositions() const;
    OptionPosition getPosition(const PositionKey& key) const;
    double getTotalPositionValue() const;

    // Order fill callbacks
    void onOrderFilled(const std::string& orderId, double fillPrice);
    void onOrderRejected(const std::string& orderId, const std::string& reason);

private:
    // Internal methods
    void updatePosition(const OptionOrder& order, double fillPrice);
    std::string generateOrderId();
    void validateOrder(const OptionOrder& order) const;

    // Execution engine
    ExecutionEngine* executionEngine_{nullptr};

    // Data storage
    std::unordered_map<std::string, OptionOrder> orders_;
    std::unordered_map<PositionKey, OptionPosition> positions_;
    
    // Thread safety
    mutable std::mutex ordersMutex_;
    mutable std::mutex positionsMutex_;
    
    // State tracking
    std::atomic<bool> isRunning_;
    std::atomic<uint64_t> orderCounter_;
};

#endif