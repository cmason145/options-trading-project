#include "ExecutionEngine.hpp"
#include "OptionTypes.hpp"
#include "OrderManagementSystem.hpp"
#include <iostream>
#include <random>
#include <chrono>

ExecutionEngine::ExecutionEngine() {}

ExecutionEngine::~ExecutionEngine() {
    stop();
}

void ExecutionEngine::start() {
    if (running_) return;
    
    running_ = true;
    executionThread_ = std::make_unique<std::thread>(&ExecutionEngine::executionLoop, this);
    
    std::cout << "Execution Engine started." << std::endl;
}

void ExecutionEngine::stop() {
    running_ = false;
    
    if (executionThread_ && executionThread_->joinable()) {
        executionThread_->join();
    }
    
    std::cout << "Execution Engine stopped. Statistics:\n"
              << "Total orders processed: " << totalOrdersProcessed_ << "\n"
              << "Total orders filled: " << totalOrdersFilled_ << "\n"
              << "Total orders rejected: " << totalOrdersRejected_ << std::endl;
}

void ExecutionEngine::setOrderManagementSystem(OrderManagementSystem* oms) {
    oms_ = oms;
}

void ExecutionEngine::addOrder(const OptionOrder& order) {
    if (!running_) {
        throw std::runtime_error("Execution Engine is not running");
    }
    
    std::lock_guard<std::mutex> lock(queueMutex_);
    orderQueue_.push(order);
}

void ExecutionEngine::executionLoop() {
    while (running_) {
        processOrders();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Simulate processing time
    }
}

void ExecutionEngine::processOrders() {
    // First process queued orders
    std::unique_lock<std::mutex> lock(queueMutex_);
    
    while (!orderQueue_.empty() && running_) {
        OptionOrder order = orderQueue_.front();
        orderQueue_.pop();
        lock.unlock();  // Unlock while processing order
        
        processOrder(order);
        
        lock.lock();  // Relock for next iteration
    }

    // Then check OMS for any pending orders that might have been missed
    if (oms_) {
        auto activeOrders = oms_->getActiveOrders();
        for (const auto& order : activeOrders) {
            if (order.status == OptionOrder::Status::PENDING) {
                processOrder(order);
            }
        }
    }
}

void ExecutionEngine::processOrder(const OptionOrder& order) {
    ++totalOrdersProcessed_;
    
    if (tryExecuteOrder(order)) {
        ++totalOrdersFilled_;
        
        // Calculate and report fill price
        double fillPrice = calculateFillPrice(order);
        
        if (oms_) {
            oms_->onOrderFilled(order.orderId, fillPrice);
        }
        
        std::cout << "Order filled -> ID: " << order.orderId
                  << ", Fill Price: " << fillPrice << std::endl;
    } else {
        ++totalOrdersRejected_;
        
        if (oms_) {
            oms_->onOrderRejected(order.orderId, "Order execution failed");
        }
        
        std::cout << "Order rejected -> ID: " << order.orderId << std::endl;
    }
}

bool ExecutionEngine::tryExecuteOrder(const OptionOrder& order) {
    // Implement basic order validation
    if (order.quantity == 0) return false;
    
    // Check if order should be filled based on simulated fill rate
    if (!shouldFillOrder(order)) return false;
    
    // Additional validation based on order type
    switch (order.orderType) {
        case OptionOrder::OrderType::MARKET:
            return true;  // Market orders always fill (in this simulation)
            
        case OptionOrder::OrderType::LIMIT: {
            double fillPrice = calculateFillPrice(order);
            if (order.quantity > 0) {  // Buy order
                return fillPrice <= order.limitPrice;
            } else {  // Sell order
                return fillPrice >= order.limitPrice;
            }
        }
        
        case OptionOrder::OrderType::STOP: {
            double fillPrice = calculateFillPrice(order);
            if (order.quantity > 0) {  // Buy stop
                return fillPrice >= order.stopPrice;
            } else {  // Sell stop
                return fillPrice <= order.stopPrice;
            }
        }
        
        case OptionOrder::OrderType::STOP_LIMIT: {
            double fillPrice = calculateFillPrice(order);
            if (order.quantity > 0) {  // Buy stop limit
                return fillPrice >= order.stopPrice && fillPrice <= order.limitPrice;
            } else {  // Sell stop limit
                return fillPrice <= order.stopPrice && fillPrice >= order.limitPrice;
            }
        }
    }
    
    return false;
}

double ExecutionEngine::calculateFillPrice(const OptionOrder& order) const {
    // In a real system, this would use actual market data
    // For simulation, we'll use the limit price with some random slippage
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> slippage(-simulatedSlippage_, simulatedSlippage_);
    
    double basePrice = order.orderType == OptionOrder::OrderType::MARKET ? 
                      order.limitPrice : order.limitPrice;
    
    // Apply random slippage
    double slippageAmount = basePrice * slippage(gen);
    
    // For market orders, slippage is always negative for buys and positive for sells
    if (order.orderType == OptionOrder::OrderType::MARKET) {
        slippageAmount = std::abs(slippageAmount) * (order.quantity > 0 ? 1 : -1);
    }
    
    return basePrice + slippageAmount;
}

bool ExecutionEngine::shouldFillOrder(const OptionOrder& order) const {
    // Simulate random fill probability based on configured fill rate
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 1);
    
    return dis(gen) < simulatedFillRate_;
}

void ExecutionEngine::setSimulatedSlippage(double slippage) {
    if (slippage < 0 || slippage > 1) {
        throw std::invalid_argument("Slippage must be between 0 and 1");
    }
    simulatedSlippage_ = slippage;
}

void ExecutionEngine::setSimulatedFillRate(double fillRate) {
    if (fillRate < 0 || fillRate > 1) {
        throw std::invalid_argument("Fill rate must be between 0 and 1");
    }
    simulatedFillRate_ = fillRate;
}