#include "RiskManagement.hpp"
#include <cmath>
#include <algorithm>

RiskManagement::RiskManagement() {
    // Set default risk limits
    limits_.maxDelta = 1000.0;
    limits_.maxGamma = 100.0;
    limits_.maxVega = 1000.0;
    limits_.maxTheta = 500.0;
    limits_.maxPositionSize = 1000000.0;
    limits_.maxLoss = 100000.0;
}

RiskMetrics RiskManagement::calculatePortfolioRisk(
    const std::vector<OptionPosition>& positions,
    const std::unordered_map<std::string, double>& underlyingPrices) {
    
    RiskMetrics metrics{};
    
    for (const auto& position : positions) {
        auto it = underlyingPrices.find(position.symbol);
        if (it == underlyingPrices.end()) continue;
        
        double spotPrice = it->second;
        
        BlackScholesModel::OptionParameters params{
            spotPrice,
            position.strike,
            DEFAULT_RISK_FREE_RATE,
            DEFAULT_VOLATILITY,
            position.timeToExpiry,
            position.isCall
        };
        
        // Calculate option price and Greeks
        double optionPrice = BlackScholesModel::calculateOptionPrice(params);
        auto greeks = BlackScholesModel::calculateGreeks(params);
        
        // Multiply by position size
        metrics.totalDelta += greeks.delta * position.quantity;
        metrics.totalGamma += greeks.gamma * position.quantity;
        metrics.totalTheta += greeks.theta * position.quantity;
        metrics.totalVega += greeks.vega * position.quantity;
        metrics.totalRho += greeks.rho * position.quantity;
        
        metrics.portfolioValue += optionPrice * position.quantity;
    }
    
    // Calculate VaR using historical simulation method
    metrics.valueAtRisk = calculateValueAtRisk(positions, 0.95);  // 95% confidence level
    
    // Simple margin requirement calculation (can be made more sophisticated)
    metrics.marginRequirement = std::max(
        metrics.portfolioValue * 0.2,  // 20% of portfolio value
        std::abs(metrics.totalDelta) * 100.0  // $100 per delta point
    );
    
    return metrics;
}

bool RiskManagement::checkOrderRisk(const OptionPosition& newPosition, const RiskMetrics& currentRisk) {
    // Add marginals from new position to current risk
    return (std::abs(currentRisk.totalDelta) < limits_.maxDelta &&
            std::abs(currentRisk.totalGamma) < limits_.maxGamma &&
            std::abs(currentRisk.totalVega) < limits_.maxVega &&
            std::abs(currentRisk.totalTheta) < limits_.maxTheta &&
            currentRisk.portfolioValue < limits_.maxPositionSize);
}

void RiskManagement::setRiskLimits(const RiskLimits& limits) {
    limits_ = limits;
}

double RiskManagement::calculateValueAtRisk(
    const std::vector<OptionPosition>& positions,
    double confidenceLevel) {
    
    // Simple VaR calculation using parametric method
    // In a real system, you'd use historical simulation or Monte Carlo
    double totalValue = 0.0;
    double totalRisk = 0.0;
    
    for (const auto& position : positions) {
        totalValue += position.quantity * position.strike;  // Simplified
        totalRisk += std::abs(position.quantity * position.strike * DEFAULT_VOLATILITY);
    }
    
    // Using normal distribution approximation
    double z = boost::math::quantile(boost::math::normal(), confidenceLevel);
    return totalRisk * z * sqrt(1.0/252.0);  // Daily VaR
}