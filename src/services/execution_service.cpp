#include <grpcpp/grpcpp.h>
#include "execution.grpc.pb.h"
#include "ExecutionEngine.hpp"

class ExecutionServiceImpl final : public trading::ExecutionService::Service {
public:
    explicit ExecutionServiceImpl(ExecutionEngine& execEngine)
        : execEngine_(execEngine) {}

    grpc::Status ExecuteTrade(
        grpc::ServerContext* context,
        const trading::ExecuteTradeRequest* request,
        trading::ExecuteTradeResponse* response) override {
        
        try {
            OptionOrder order;
            order.orderId = request->order_id();
            order.underlying = request->symbol();
            order.quantity = request->quantity();
            order.limitPrice = request->price();
            order.type = request->side() == "BUY" ? 
                OptionOrder::Type::BUY_TO_OPEN : 
                OptionOrder::Type::SELL_TO_OPEN;
            order.orderType = OptionOrder::OrderType::MARKET;

            execEngine_.addOrder(order);
            
            response->set_execution_id("EXEC-" + request->order_id());
            response->set_success(true);
            response->set_message("Trade executed successfully");
            response->set_filled_quantity(request->quantity());
            response->set_fill_price(request->price());
            
            return grpc::Status::OK;
            
        } catch (const std::exception& e) {
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }

    grpc::Status GetExecutionReport(
        grpc::ServerContext* context,
        const trading::ExecutionReportRequest* request,
        trading::ExecutionReportResponse* response) override {
        
        // In a real system, you would look up the execution report from a database
        // For now, we'll create a simulated response
        response->set_execution_id(request->execution_id());
        response->set_order_id(request->execution_id().substr(5)); // Remove "EXEC-" prefix
        response->set_status("FILLED");
        
        return grpc::Status::OK;
    }

private:
    ExecutionEngine& execEngine_;
};