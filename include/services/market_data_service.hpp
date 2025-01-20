#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <grpcpp/grpcpp.h>
#include "market_data.grpc.pb.h"
#include "MarketDataHandler.hpp"

class MarketDataServiceImpl final : public trading::MarketDataService::Service {
public:
    explicit MarketDataServiceImpl(MarketDataHandler& marketDataHandler);
    
    grpc::Status StreamMarketData(
        grpc::ServerContext* context,
        const trading::MarketDataRequest* request,
        grpc::ServerWriter<trading::MarketDataResponse>* writer) override;

    grpc::Status GetOptionChain(
        grpc::ServerContext* context,
        const trading::OptionChainRequest* request,
        trading::OptionChainResponse* response) override;

private:
    MarketDataHandler& marketDataHandler_;
};

#endif