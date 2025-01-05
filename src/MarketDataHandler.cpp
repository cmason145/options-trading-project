// MarketDataHandler.cpp
#include "MarketDataHandler.hpp"
#include <iostream>
#include <chrono>
#include <sstream>

MarketDataHandler::MarketDataHandler(boost::asio::io_context& ioc, const std::string& apiKey)
    : io_context_(ioc), api_key_(apiKey) {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    // Set up CURL options that don't need to change between requests
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10L);  // 10 second timeout
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Setup headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
}

MarketDataHandler::~MarketDataHandler() {
    stop();
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

size_t MarketDataHandler::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void MarketDataHandler::start() {
    running_ = true;
    data_thread_ = std::thread([this]() {
        while (running_) {
            for (const auto& symbol : subscribed_symbols_) {
                try {
                    fetchMarketData(symbol);
                    // Alpha Vantage rate limit: 5 API calls per minute for standard API
                    std::this_thread::sleep_for(std::chrono::milliseconds(12000)); // 12 seconds between calls
                } catch (const std::exception& e) {
                    std::cerr << "Error fetching market data for " << symbol << ": " << e.what() << std::endl;
                }
            }
        }
    });
}

void MarketDataHandler::fetchMarketData(const std::string& symbol) {
    if (!curl_) return;

    std::string readBuffer;
    
    // First, fetch the underlying stock price
    std::string quote_url = "https://www.alphavantage.co/query?"
                           "function=GLOBAL_QUOTE&"
                           "symbol=" + symbol + "&"
                           "apikey=" + api_key_;

    curl_easy_setopt(curl_, CURLOPT_URL, quote_url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
    }

    // Process stock quote first
    double underlying_price = processStockQuote(readBuffer);
    
    // Now fetch options data
    readBuffer.clear();
    std::string options_url = "https://www.alphavantage.co/query?"
                             "function=HISTORICAL_OPTIONS&"
                             "symbol=" + symbol + "&"
                             "apikey=" + api_key_;

    curl_easy_setopt(curl_, CURLOPT_URL, options_url.c_str());
    
    res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
    }

    processOptionsData(readBuffer, underlying_price);
}

double MarketDataHandler::processStockQuote(const std::string& rawData) {
    try {
        nlohmann::json j = nlohmann::json::parse(rawData);
        
        if (j.contains("Error Message")) {
            throw std::runtime_error(j["Error Message"].get<std::string>());
        }

        auto quote = j["Global Quote"];
        if (quote.empty()) {
            throw std::runtime_error("No quote data received");
        }

        return std::stod(quote["05. price"].get<std::string>());
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    }
}

void MarketDataHandler::processOptionsData(const std::string& rawData, double underlying_price) {
    try {
        nlohmann::json j = nlohmann::json::parse(rawData);
        
        if (j.contains("Error Message")) {
            throw std::runtime_error(j["Error Message"].get<std::string>());
        }

        auto options_chain = j["options"];
        if (options_chain.empty()) {
            throw std::runtime_error("No options data received");
        }

        for (const auto& expiry_data : options_chain) {
            processExpiryOptions(expiry_data, underlying_price);
        }
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("JSON parsing error: ") + e.what());
    }
}

void MarketDataHandler::processExpiryOptions(const nlohmann::json& expiry_data, double underlying_price) {
    std::string expiry_date = expiry_data["expirationDate"].get<std::string>();
    
    // Process calls
    for (const auto& call : expiry_data["calls"]) {
        OptionData data;
        data.underlying = call["symbol"].get<std::string>();
        data.optionType = "CALL";
        data.strike = std::stod(call["strikePrice"].get<std::string>());
        data.expiry = expiry_date;
        data.bid = std::stod(call["bid"].get<std::string>());
        data.ask = std::stod(call["ask"].get<std::string>());
        data.lastPrice = std::stod(call["lastPrice"].get<std::string>());
        data.volume = std::stoi(call["volume"].get<std::string>());
        data.impliedVol = std::stod(call["impliedVolatility"].get<std::string>());

        calculateGreeks(data, underlying_price);
        
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            latestData_[data.underlying] = data;
        }

        if (callback_) {
            callback_(data);
        }
    }

    // Process puts (similar to calls)
    for (const auto& put : expiry_data["puts"]) {
        OptionData data;
        data.underlying = put["symbol"].get<std::string>();
        data.optionType = "PUT";
        data.strike = std::stod(put["strikePrice"].get<std::string>());
        data.expiry = expiry_date;
        data.bid = std::stod(put["bid"].get<std::string>());
        data.ask = std::stod(put["ask"].get<std::string>());
        data.lastPrice = std::stod(put["lastPrice"].get<std::string>());
        data.volume = std::stoi(put["volume"].get<std::string>());
        data.impliedVol = std::stod(put["impliedVolatility"].get<std::string>());

        calculateGreeks(data, underlying_price);
        
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            latestData_[data.underlying] = data;
        }

        if (callback_) {
            callback_(data);
        }
    }
}

void MarketDataHandler::calculateGreeks(OptionData& data, double underlying_price) {
    // Convert expiry string to time to expiry in years
    auto expiry_tp = parseExpiryDate(data.expiry);
    auto now = std::chrono::system_clock::now();
    double time_to_expiry = std::chrono::duration<double>(expiry_tp - now).count() / (365.25 * 24 * 3600);

    BlackScholesModel::OptionParameters params{
        underlying_price,
        data.strike,
        0.02,  // Risk-free rate (should be fetched from a proper source)
        data.impliedVol,
        time_to_expiry,
        data.optionType == "CALL"
    };
    
    auto greeks = BlackScholesModel::calculateGreeks(params);
    data.delta = greeks.delta;
    data.gamma = greeks.gamma;
    data.theta = greeks.theta;
    data.vega = greeks.vega;
    data.rho = greeks.rho;
}

std::chrono::system_clock::time_point MarketDataHandler::parseExpiryDate(const std::string& date_str) {
    std::tm tm = {};
    std::stringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse expiry date: " + date_str);
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void MarketDataHandler::subscribeToSymbol(const std::string& symbol) {
    subscribed_symbols_.push_back(symbol);
}

void MarketDataHandler::unsubscribeFromSymbol(const std::string& symbol) {
    auto it = std::find(subscribed_symbols_.begin(), subscribed_symbols_.end(), symbol);
    if (it != subscribed_symbols_.end()) {
        subscribed_symbols_.erase(it);
    }
}

OptionData MarketDataHandler::getLatestData(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto it = latestData_.find(symbol);
    if (it != latestData_.end()) {
        return it->second;
    }
    return OptionData{};
}

void MarketDataHandler::stop() {
    running_ = false;
    if (data_thread_.joinable()) {
        data_thread_.join();
    }
}

void MarketDataHandler::setDataCallback(DataCallback cb) {
    callback_ = std::move(cb);
}