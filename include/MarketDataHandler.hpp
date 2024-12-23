// MarketDataHandler.hpp
#ifndef MARKET_DATA_HANDLER_HPP
#define MARKET_DATA_HPP

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <curl/curl.h>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

struct OptionData {
    std::string underlying;
    std::string optionType;     
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
};

class MarketDataHandler {
public:
    using DataCallback = std::function<void(const OptionData&)>;
    
    MarketDataHandler(boost::asio::io_context& ioc, const std::string& apiKey);
    ~MarketDataHandler();

    void start();
    void stop();
    void setDataCallback(DataCallback cb);
    void subscribeToSymbol(const std::string& symbol);
    void unsubscribeFromSymbol(const std::string& symbol);
    OptionData getLatestData(const std::string& symbol) const;

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void fetchMarketData(const std::string& symbol);
    void processMarketData(const std::string& rawData);
    void calculateGreeks(OptionData& data);

    boost::asio::io_context& io_context_;
    std::thread data_thread_;
    std::atomic<bool> running_{false};
    DataCallback callback_;
    std::string api_key_;
    CURL* curl_;
    
    std::unordered_map<std::string, OptionData> latestData_;
    std::mutex dataMutex_;
    std::vector<std::string> subscribed_symbols_;
};

#endif