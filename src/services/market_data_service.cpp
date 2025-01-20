#include "services/market_data_service.hpp"
#include "MarketDataHandler.hpp"
#include <chrono>

MarketDataServiceImpl::MarketDataServiceImpl(MarketDataHandler& marketDataHandler)
    : marketDataHandler_(marketDataHandler) {}

grpc::Status MarketDataServiceImpl::StreamMarketData(
    grpc::ServerContext* context,
    const trading::MarketDataRequest* request,
    grpc::ServerWriter<trading::MarketDataResponse>* writer) {
    
    // Subscribe to the requested symbol
    marketDataHandler_.subscribeToSymbol(request->symbol());
    
    // Set up callback for market data updates
    marketDataHandler_.setDataCallback([writer](const OptionData& data) {
        trading::MarketDataResponse response;
        response.set_symbol(data.underlying);
        response.set_option_type(data.optionType);
        response.set_strike(data.strike);
        response.set_bid(data.bid);
        response.set_ask(data.ask);
        response.set_price(data.lastPrice);
        response.set_volume(data.volume);
        response.set_implied_volatility(data.impliedVol);
        response.set_delta(data.delta);
        response.set_gamma(data.gamma);
        response.set_theta(data.theta);
        response.set_vega(data.vega);
        response.set_rho(data.rho);
        response.set_timestamp(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        writer->Write(response);
    });

    // Keep the stream open until the client disconnects
    while (!context->IsCancelled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return grpc::Status::OK;
}

grpc::Status MarketDataServiceImpl::GetOptionChain(
    grpc::ServerContext* context,
    const trading::OptionChainRequest* request,
    trading::OptionChainResponse* response) {
    
    // Get current market data for the symbol
    auto data = marketDataHandler_.getLatestData(request->underlying_symbol());
    
    if (data.underlying.empty()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "No data available for symbol");
    }

    // Add data to calls or puts based on option type
    auto* target_list = (data.optionType == "CALL") ? 
        response->mutable_calls() : response->mutable_puts();
    
    auto* option_data = target_list->Add();
    option_data->set_symbol(data.underlying);
    option_data->set_option_type(data.optionType);
    option_data->set_strike(data.strike);
    option_data->set_price(data.lastPrice);
    option_data->set_bid(data.bid);
    option_data->set_ask(data.ask);
    option_data->set_volume(data.volume);
    option_data->set_implied_volatility(data.impliedVol);
    option_data->set_delta(data.delta);
    option_data->set_gamma(data.gamma);
    option_data->set_theta(data.theta);
    option_data->set_vega(data.vega);
    option_data->set_rho(data.rho);

    response->set_underlying_price(data.getMidPrice());
    response->set_timestamp(
        std::chrono::system_clock::now().time_since_epoch().count()
    );

    return grpc::Status::OK;
}