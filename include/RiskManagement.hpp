#ifndef RISK_MANAGEMENT_HPP
#define RISK_MANAGEMENT_HPP

#include <vector>

// In RiskManagement.hpp
struct RiskMetrics {
    double totalDelta;
    double totalGamma;
    double totalTheta;
    double totalVega;
    double totalRho;
    double portfolioValue;
    double valueAtRisk;
    double marginRequirement;
};

class RiskManagement {
public:
    // Enhanced risk calculation methods
    RiskMetrics calculatePortfolioRisk(
        const std::vector<OptionPosition>& positions,
        const std::unordered_map<std::string, OptionData>& marketData
    );
    
    // Risk limits and checks
    bool checkOrderRisk(const OptionOrder& order);
    void setRiskLimits(const RiskLimits& limits);
    
private:
    double calculateImpliedVolatility(const OptionData& data);
    double calculateOptionPrice(const OptionData& data);
    std::vector<double> calculateGreeks(const OptionData& data);
};

#endif
