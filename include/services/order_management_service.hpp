#ifndef ORDER_MANAGEMENT_SERVICE_HPP
#define ORDER_MANAGEMENT_SERVICE_HPP

#include <grpcpp/grpcpp.h>
#include "order_management.grpc.pb.h"
#include "OrderManagementSystem.hpp"

class OrderManagementServiceImpl final : public trading::OrderManagementService::Service {
public:
    explicit OrderManagementServiceImpl(OrderManagementSystem& oms);

    grpc::Status PlaceOrder(
        grpc::ServerContext* context,
        const trading::OrderRequest* request,
        trading::OrderResponse* response) override;

    grpc::Status CancelOrder(
        grpc::ServerContext* context,
        const trading::CancelOrderRequest* request,
        trading::CancelOrderResponse* response) override;

    grpc::Status GetOrderStatus(
        grpc::ServerContext* context,
        const trading::OrderStatusRequest* request,
        trading::OrderStatusResponse* response) override;

    grpc::Status GetOrderHistory(
        grpc::ServerContext* context,
        const trading::OrderHistoryRequest* request,
        trading::OrderHistoryResponse* response) override;

private:
    OrderManagementSystem& oms_;
};

#endif