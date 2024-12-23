#ifndef ORDER_MANAGEMENT_SYSTEM_HPP
#define ORDER_MANAGEMENT_SYSTEM_HPP

#include <string>

struct OptionOrder {
    enum class Type { BUY_TO_OPEN, SELL_TO_OPEN, BUY_TO_CLOSE, SELL_TO_CLOSE };
    enum class OrderType { MARKET, LIMIT, STOP, STOP_LIMIT };
    
    std::string underlying;
    std::string optionType;     // "CALL" or "PUT"
    double strike;
    std::string expiry;
    Type type;
    OrderType orderType;
    double limitPrice;          // Used for LIMIT and STOP_LIMIT orders
    double stopPrice;           // Used for STOP and STOP_LIMIT orders
    int quantity;
    std::string orderId;
    
    // Order status tracking
    std::atomic<bool> isActive;
    std::chrono::system_clock::time_point submitTime;
};

class OrderManagementSystem {
public:
    // Enhanced order submission methods
    std::string submitOptionOrder(const OptionOrder& order);
    void cancelOrder(const std::string& orderId);
    void modifyOrder(const std::string& orderId, const OptionOrder& newOrder);
    
    // Order tracking
    std::vector<OptionOrder> getActiveOrders() const;
    OptionOrder getOrderStatus(const std::string& orderId) const;
};

#endif
