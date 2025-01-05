#include "OrderManagementSystem.hpp"
#include "OptionTypes.hpp"
#include "ExecutionEngine.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

OrderManagementSystem::OrderManagementSystem() 
    : isRunning_(false), orderCounter_(0) {}

OrderManagementSystem::~OrderManagementSystem() {
    stop();
}

void OrderManagementSystem::start() {
    isRunning_ = true;
    std::cout << "OMS started." << std::endl;
}

void OrderManagementSystem::stop() {
    isRunning_ = false;
    std::cout << "OMS stopped." << std::endl;
}

void OrderManagementSystem::sendOrder(const std::string& symbol, double price, int quantity) {
    OptionOrder order;
    order.underlying = symbol;
    order.limitPrice = price;
    order.quantity = quantity;
    order.type = quantity > 0 ? OptionOrder::Type::BUY_TO_OPEN : OptionOrder::Type::SELL_TO_OPEN;
    order.orderType = OptionOrder::OrderType::LIMIT;
    
    submitOptionOrder(order);
}

std::string OrderManagementSystem::submitOptionOrder(const OptionOrder& order) {
    if (!isRunning_) {
        throw std::runtime_error("OMS is not running");
    }

    validateOrder(order);
    
    auto orderId = generateOrderId();
    OptionOrder newOrder = order;
    newOrder.orderId = orderId;
    newOrder.status = OptionOrder::Status::PENDING;
    newOrder.isActive = true;
    newOrder.submitTime = std::chrono::system_clock::now();

    {
        std::lock_guard<std::mutex> lock(ordersMutex_);
        orders_[orderId] = newOrder;
    }

    // Forward order to execution engine
    if (executionEngine_) {
        executionEngine_->addOrder(newOrder);
    } else {
        std::cerr << "Warning: No execution engine connected to OMS" << std::endl;
    }

    std::cout << "Order submitted -> ID: " << orderId
              << ", Symbol: " << order.underlying
              << ", Type: " << (order.quantity > 0 ? "BUY" : "SELL")
              << ", Quantity: " << std::abs(order.quantity)
              << ", Price: " << order.limitPrice << std::endl;

    return orderId;
}

void OrderManagementSystem::cancelOrder(const std::string& orderId) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    auto it = orders_.find(orderId);
    if (it != orders_.end() && it->second.isActive) {
        it->second.isActive = false;
        it->second.status = OptionOrder::Status::CANCELLED;
    }
}

void OrderManagementSystem::modifyOrder(const std::string& orderId, const OptionOrder& newOrder) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    auto it = orders_.find(orderId);
    if (it != orders_.end() && it->second.isActive) {
        validateOrder(newOrder);
        it->second = newOrder;
        it->second.orderId = orderId; // Preserve original order ID
    }
}

std::vector<OptionOrder> OrderManagementSystem::getActiveOrders() const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    std::vector<OptionOrder> activeOrders;
    for (const auto& pair : orders_) {
        if (pair.second.isActive) {
            activeOrders.push_back(pair.second);
        }
    }
    return activeOrders;
}

OptionOrder OrderManagementSystem::getOrderStatus(const std::string& orderId) const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    auto it = orders_.find(orderId);
    if (it != orders_.end()) {
        return it->second;
    }
    return OptionOrder{};
}

std::vector<OptionPosition> OrderManagementSystem::getPositions() const {
    std::lock_guard<std::mutex> lock(positionsMutex_);
    std::vector<OptionPosition> result;
    for (const auto& pair : positions_) {
        result.push_back(pair.second);
    }
    return result;
}

OptionPosition OrderManagementSystem::getPosition(const PositionKey& key) const {
    std::lock_guard<std::mutex> lock(positionsMutex_);
    auto it = positions_.find(key);
    if (it != positions_.end()) {
        return it->second;
    }
    return OptionPosition{};
}

void OrderManagementSystem::onOrderFilled(const std::string& orderId, double fillPrice) {
    std::lock_guard<std::mutex> orderLock(ordersMutex_);
    auto orderIt = orders_.find(orderId);
    if (orderIt == orders_.end()) return;

    OptionOrder& order = orderIt->second;
    order.status = OptionOrder::Status::FILLED;
    order.isActive = false;
    order.fillPrice = fillPrice;
    order.fillTime = std::chrono::system_clock::now();

    updatePosition(order, fillPrice);
}

void OrderManagementSystem::onOrderRejected(const std::string& orderId, const std::string& reason) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    auto it = orders_.find(orderId);
    if (it != orders_.end()) {
        it->second.isActive = false;
        it->second.status = OptionOrder::Status::REJECTED;
    }
    std::cerr << "Order " << orderId << " rejected: " << reason << std::endl;
}

void OrderManagementSystem::updatePosition(const OptionOrder& order, double fillPrice) {
    PositionKey key{order.underlying, order.optionType, order.strike, order.expiry};
    
    std::lock_guard<std::mutex> lock(positionsMutex_);
    auto& position = positions_[key];
    
    // Initialize position if it doesn't exist
    if (position.symbol.empty()) {
        position.symbol = order.underlying;
        position.strike = order.strike;
        position.isCall = (order.optionType == "CALL");
        // Calculate time to expiry based on expiry date
        // This needs proper date calculation
        position.timeToExpiry = 1.0; // Placeholder
    }

    // Update quantity based on order type
    if (order.type == OptionOrder::Type::BUY_TO_OPEN ||
        order.type == OptionOrder::Type::BUY_TO_CLOSE) {
        position.quantity += order.quantity;
    } else {
        position.quantity -= order.quantity;
    }

    // Remove position if quantity becomes zero
    if (position.quantity == 0) {
        positions_.erase(key);
    }
}

std::string OrderManagementSystem::generateOrderId() {
    std::stringstream ss;
    ss << "ORD-" << std::setfill('0') << std::setw(8) 
       << ++orderCounter_;
    return ss.str();
}

void OrderManagementSystem::validateOrder(const OptionOrder& order) const {
    if (order.underlying.empty()) {
        throw std::invalid_argument("Missing underlying symbol");
    }
    if (order.quantity == 0) {
        throw std::invalid_argument("Invalid quantity");
    }
    if (order.orderType == OptionOrder::OrderType::LIMIT && order.limitPrice <= 0) {
        throw std::invalid_argument("Invalid limit price");
    }
    if (order.orderType == OptionOrder::OrderType::STOP && order.stopPrice <= 0) {
        throw std::invalid_argument("Invalid stop price");
    }
}

double OrderManagementSystem::getTotalPositionValue() const {
    std::lock_guard<std::mutex> lock(positionsMutex_);
    double total = 0.0;
    for (const auto& pair : positions_) {
        // This is a simplified calculation
        // In reality, you'd need current market prices
        total += pair.second.quantity * pair.second.strike;
    }
    return total;
}