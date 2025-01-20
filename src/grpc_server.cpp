#include <grpcpp/grpcpp.h>
#include <boost/asio.hpp>
#include "MarketDataHandler.hpp"
#include "OrderManagementSystem.hpp"
#include "ExecutionEngine.hpp"
#include "RiskManagement.hpp"
#include "services/market_data_service.hpp"
#include "services/order_management_service.hpp"
#include "services/execution_service.hpp"

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    
    // Initialize core components
    boost::asio::io_context io_context;
    const std::string API_KEY = "YL2LLPOL7A39ORIO";
    
    MarketDataHandler mdHandler(io_context, API_KEY);
    OrderManagementSystem oms;
    ExecutionEngine execEngine;
    RiskManagement riskMgr;

    // Connect components
    execEngine.setOrderManagementSystem(&oms);
    oms.setExecutionEngine(&execEngine);

    // Initialize services
    MarketDataServiceImpl marketDataService(mdHandler);
    OrderManagementServiceImpl orderMgmtService(oms);
    ExecutionServiceImpl executionService(execEngine);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    
    // Register services
    builder.RegisterService(&marketDataService);
    builder.RegisterService(&orderMgmtService);
    builder.RegisterService(&executionService);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Start components
    mdHandler.start();
    oms.start();
    execEngine.start();

    // Wait for server to shutdown
    server->Wait();

    // Cleanup
    execEngine.stop();
    mdHandler.stop();
    oms.stop();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}