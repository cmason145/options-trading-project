#include "ExecutionEngine.hpp"
#include <iostream>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

ExecutionEngine::ExecutionEngine() {}
ExecutionEngine::~ExecutionEngine() {}

void ExecutionEngine::run() {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, orders_.size()),
    [this](const tbb::blocked_range<size_t>& r){
        for(size_t i=r.begin(); i<r.end(); ++i) {
            // Mock execution
            std::cout << "[ExecutionEngine] Executing: " << orders_[i] << std::endl;
        }
    });
}

void ExecutionEngine::addOrder(const std::string& order) {
    orders_.push_back(order);
}
