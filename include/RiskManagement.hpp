#ifndef RISK_MANAGEMENT_HPP
#define RISK_MANAGEMENT_HPP

#include <vector>
#include <unordered_map>
#include "BlackScholesModel.hpp"
#include "RiskMetrics.hpp"
#include "OptionTypes.hpp"

struct RiskLimits {
    double maxDelta;
    double maxGamma;
    double maxVega;
    double maxTheta;
    double maxPositionSize;
    double maxLoss;
};

class RiskManagement {
public:
    RiskManagement();
    
    RiskMetrics calculatePortfolioRisk(
        const std::vector<OptionPosition>& positions,
        const std::unordered_map<std::string, double>& underlyingPrices
    );
    
    bool checkOrderRisk(const OptionPosition& newPosition, const RiskMetrics& currentRisk);
    void setRiskLimits(const RiskLimits& limits);
    double calculateValueAtRisk(const std::vector<OptionPosition>& positions, double confidenceLevel);

private:
    RiskLimits limits_;
    static constexpr double DEFAULT_RISK_FREE_RATE = 0.02;  // 2% risk-free rate
    static constexpr double DEFAULT_VOLATILITY = 0.20;      // 20% volatility
};

#endif