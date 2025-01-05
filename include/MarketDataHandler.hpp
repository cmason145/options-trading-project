#ifndef MARKET_DATA_HANDLER_HPP
#define MARKET_DATA_HANDLER_HPP

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include "OptionTypes.hpp"
#include <curl/curl.h>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "BlackScholesModel.hpp"

class MarketDataHandler {
public:
    using DataCallback = std::function<void(const OptionData&)>;
    
    MarketDataHandler(boost::asio::io_context& ioc, const std::string& apiKey);
    ~MarketDataHandler();

    // Public interface
    void start();
    void stop();
    void setDataCallback(DataCallback cb);
    void subscribeToSymbol(const std::string& symbol);
    void unsubscribeFromSymbol(const std::string& symbol);
    OptionData getLatestData(const std::string& symbol) const;

private:
    // Network and data handling
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void fetchMarketData(const std::string& symbol);
    
    // Data processing methods
    double processStockQuote(const std::string& rawData);
    void processOptionsData(const std::string& rawData, double underlying_price);
    void processExpiryOptions(const nlohmann::json& expiry_data, double underlying_price);
    
    // Options calculations
    void calculateGreeks(OptionData& data, double underlying_price);
    std::chrono::system_clock::time_point parseExpiryDate(const std::string& date_str);

    // Member variables
    boost::asio::io_context& io_context_;
    std::thread data_thread_;
    std::atomic<bool> running_{false};
    DataCallback callback_;
    std::string api_key_;
    CURL* curl_;
    
    // Data storage
    std::unordered_map<std::string, OptionData> latestData_;
    mutable std::mutex dataMutex_;
    std::vector<std::string> subscribed_symbols_;
};

#endif