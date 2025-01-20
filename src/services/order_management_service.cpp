#include <grpcpp/grpcpp.h>
#include "order_management.grpc.pb.h"
#include "OrderManagementSystem.hpp"
#include "services/order_management_service.hpp"
#include <chrono>

OrderManagementServiceImpl::OrderManagementServiceImpl(OrderManagementSystem& oms)
    : oms_(oms) {}

grpc::Status OrderManagementServiceImpl::PlaceOrder(
    grpc::ServerContext* context,
    const trading::OrderRequest* request,
    trading::OrderResponse* response) {
    
    try {
        OptionOrder order;
        order.underlying = request->symbol();
        order.optionType = request->option_type();
        order.strike = request->strike();  // Using strike instead of strike_price
        order.quantity = request->quantity();
        order.expiry = request->expiration_date();
        
        // Set order type
        if (request->order_type() == "MARKET") {
            order.orderType = OptionOrder::OrderType::MARKET;
        } else if (request->order_type() == "LIMIT") {
            order.orderType = OptionOrder::OrderType::LIMIT;
            order.limitPrice = request->price();
        }
        
        // Set side
        if (request->side() == "BUY") {
            order.type = OptionOrder::Type::BUY_TO_OPEN;
        } else {
            order.type = OptionOrder::Type::SELL_TO_OPEN;
        }

        std::string orderId = oms_.submitOptionOrder(order);
        
        response->set_order_id(orderId);
        response->set_status("PENDING");
        response->set_message("Order submitted successfully");
        
        return grpc::Status::OK;
        
    } catch (const std::exception& e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status OrderManagementServiceImpl::CancelOrder(
    grpc::ServerContext* context,
    const trading::CancelOrderRequest* request,
    trading::CancelOrderResponse* response) {
    
    try {
        oms_.cancelOrder(request->order_id());
        response->set_success(true);
        response->set_message("Order cancelled successfully");
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status OrderManagementServiceImpl::GetOrderStatus(
    grpc::ServerContext* context,
    const trading::OrderStatusRequest* request,
    trading::OrderStatusResponse* response) {
    
    auto order = oms_.getOrderStatus(request->order_id());
    
    if (order.orderId.empty()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Order not found");
    }

    response->set_order_id(order.orderId);
    
    switch (order.status) {
        case OptionOrder::Status::PENDING:
            response->set_status("PENDING");
            break;
        case OptionOrder::Status::FILLED:
            response->set_status("FILLED");
            break;
        case OptionOrder::Status::CANCELLED:
            response->set_status("CANCELLED");
            break;
        case OptionOrder::Status::REJECTED:
            response->set_status("REJECTED");
            break;
    }

    if (order.status == OptionOrder::Status::FILLED) {
        response->set_filled_quantity(order.quantity);
        response->set_remaining_quantity(0);
        response->set_average_price(order.fillPrice);
    } else {
        response->set_filled_quantity(0);
        response->set_remaining_quantity(order.quantity);
        response->set_average_price(0);
    }

    response->set_last_update_time(
        std::chrono::duration_cast<std::chrono::seconds>(
            order.submitTime.time_since_epoch()
        ).count()
    );

    return grpc::Status::OK;
}

grpc::Status OrderManagementServiceImpl::GetOrderHistory(
    grpc::ServerContext* context,
    const trading::OrderHistoryRequest* request,
    trading::OrderHistoryResponse* response) {
    
    auto activeOrders = oms_.getActiveOrders();
    
    for (const auto& order : activeOrders) {
        auto* orderStatus = response->add_orders();
        orderStatus->set_order_id(order.orderId);
        orderStatus->set_symbol(order.underlying);
        orderStatus->set_option_type(order.optionType);
        orderStatus->set_strike(order.strike);
        orderStatus->set_expiration_date(order.expiry);
        orderStatus->set_side(order.type == OptionOrder::Type::BUY_TO_OPEN ? "BUY" : "SELL");
        orderStatus->set_order_type(order.orderType == OptionOrder::OrderType::MARKET ? "MARKET" : "LIMIT");
        orderStatus->set_quantity(order.quantity);
        orderStatus->set_price(order.orderType == OptionOrder::OrderType::LIMIT ? 
                             order.limitPrice : order.fillPrice);
        
        switch (order.status) {
            case OptionOrder::Status::PENDING:
                orderStatus->set_status("PENDING");
                break;
            case OptionOrder::Status::FILLED:
                orderStatus->set_status("FILLED");
                break;
            case OptionOrder::Status::CANCELLED:
                orderStatus->set_status("CANCELLED");
                break;
            case OptionOrder::Status::REJECTED:
                orderStatus->set_status("REJECTED");
                break;
        }
        
        // Convert times to Unix timestamps
        orderStatus->set_submission_time(
            std::chrono::duration_cast<std::chrono::seconds>(
                order.submitTime.time_since_epoch()
            ).count()
        );
        
        orderStatus->set_last_update_time(
            std::chrono::duration_cast<std::chrono::seconds>(
                order.fillTime.time_since_epoch()
            ).count()
        );
    }

    return grpc::Status::OK;
}