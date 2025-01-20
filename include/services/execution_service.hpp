#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <grpcpp/grpcpp.h>
#include "execution.grpc.pb.h"
#include "ExecutionEngine.hpp"

class ExecutionServiceImpl final : public trading::ExecutionService::Service {
public:
    explicit ExecutionServiceImpl(ExecutionEngine& execEngine);

    grpc::Status ExecuteTrade(
        grpc::ServerContext* context,
        const trading::ExecuteTradeRequest* request,
        trading::ExecuteTradeResponse* response) override;

    grpc::Status GetExecutionReport(
        grpc::ServerContext* context,
        const trading::ExecutionReportRequest* request,
        trading::ExecutionReportResponse* response) override;

private:
    ExecutionEngine& execEngine_;
};

#endif