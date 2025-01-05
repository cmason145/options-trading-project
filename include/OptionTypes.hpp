#ifndef OPTION_TYPES_HPP
#define OPTION_TYPES_HPP

#include <string>
#include <cmath>
#include <stdexcept>

struct OptionOrder {
    enum class Type { BUY_TO_OPEN, SELL_TO_OPEN, BUY_TO_CLOSE, SELL_TO_CLOSE };
    enum class OrderType { MARKET, LIMIT, STOP, STOP_LIMIT };
    enum class Status { PENDING, FILLED, CANCELLED, REJECTED };
    
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
    Status status;
    
    // Order status tracking
    bool isActive;
    std::chrono::system_clock::time_point submitTime;
    std::chrono::system_clock::time_point fillTime;
    double fillPrice;
};

// New struct for position tracking
struct PositionKey {
    std::string underlying;
    std::string optionType;
    double strike;
    std::string expiry;

    bool operator==(const PositionKey& other) const {
        return underlying == other.underlying &&
               optionType == other.optionType &&
               strike == other.strike &&
               expiry == other.expiry;
    }
};

// Hash function for PositionKey
namespace std {
    template<>
    struct hash<PositionKey> {
        size_t operator()(const PositionKey& k) const {
            return hash<string>()(k.underlying) ^
                   hash<string>()(k.optionType) ^
                   hash<double>()(k.strike) ^
                   hash<string>()(k.expiry);
        }
    };
}

struct OptionData {
    std::string underlying;
    std::string optionType;  // "CALL" or "PUT"
    double strike;
    std::string expiry;
    double bid;
    double ask;
    double lastPrice;
    int volume;
    double impliedVol;
    
    // Greeks (calculated values)
    double delta;
    double gamma;
    double theta;
    double vega;
    double rho;

    // Constructor with default values
    OptionData() 
        : underlying("")
        , optionType("")
        , strike(0.0)
        , expiry("")
        , bid(0.0)
        , ask(0.0)
        , lastPrice(0.0)
        , volume(0)
        , impliedVol(0.0)
        , delta(0.0)
        , gamma(0.0)
        , theta(0.0)
        , vega(0.0)
        , rho(0.0) {}

    // Utility functions
    double getMidPrice() const {
        return (bid + ask) / 2.0;
    }

    double getSpread() const {
        return ask - bid;
    }

    bool isValid() const {
        return !underlying.empty() && 
               !optionType.empty() && 
               strike > 0.0 && 
               !expiry.empty() && 
               bid >= 0.0 && 
               ask >= bid && 
               lastPrice >= 0.0 && 
               volume >= 0 && 
               impliedVol >= 0.0;
    }

    bool isCall() const {
        return optionType == "CALL";
    }

    bool isPut() const {
        return optionType == "PUT";
    }
};

struct OptionPosition {
    std::string symbol;
    double strike;
    double quantity;
    bool isCall;
    double timeToExpiry;

    // Constructor
    OptionPosition(
        const std::string& sym = "",
        double strk = 0.0,
        double qty = 0.0,
        bool call = true,
        double tte = 0.0
    ) : symbol(sym)
      , strike(strk)
      , quantity(qty)
      , isCall(call)
      , timeToExpiry(tte) {}

    // Utility functions
    bool isLong() const {
        return quantity > 0;
    }

    bool isShort() const {
        return quantity < 0;
    }

    double getAbsQuantity() const {
        return std::abs(quantity);
    }

    bool isValid() const {
        return !symbol.empty() && 
               strike > 0.0 && 
               timeToExpiry >= 0.0;
    }

    // Calculate position value
    double calculateValue(double underlyingPrice, double optionPrice) const {
        if (!isValid()) {
            throw std::runtime_error("Invalid option position");
        }
        return quantity * optionPrice;
    }

    // Calculate position delta
    double calculatePositionDelta(double optionDelta) const {
        if (!isValid()) {
            throw std::runtime_error("Invalid option position");
        }
        return quantity * optionDelta;
    }
};

#endif