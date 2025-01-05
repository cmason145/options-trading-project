#ifndef RISK_METRICS_HPP
#define RISK_METRICS_HPP

#include <cmath>
#include <vector>

struct RiskMetrics {
    double totalDelta;
    double totalGamma;
    double totalTheta;
    double totalVega;
    double totalRho;
    double portfolioValue;
    double valueAtRisk;
    double marginRequirement;
    double expectedShortfall;
    double marginUtilization;

    // Constructor with default values
    RiskMetrics()
        : totalDelta(0.0)
        , totalGamma(0.0)
        , totalTheta(0.0)
        , totalVega(0.0)
        , totalRho(0.0)
        , portfolioValue(0.0)
        , valueAtRisk(0.0)
        , marginRequirement(0.0)
        , expectedShortfall(0.0)
        , marginUtilization(0.0) {}

    // Utility functions
    bool hasExcessiveRisk() const {
        // Example risk thresholds - adjust based on your risk tolerance
        constexpr double MAX_DELTA = 100.0;
        constexpr double MAX_GAMMA = 10.0;
        constexpr double MAX_MARGIN_UTIL = 0.8; // 80%

        return std::abs(totalDelta) > MAX_DELTA ||
               std::abs(totalGamma) > MAX_GAMMA ||
               marginUtilization > MAX_MARGIN_UTIL;
    }

    bool needsRebalancing() const {
        // Example rebalancing thresholds
        constexpr double DELTA_THRESHOLD = 50.0;
        constexpr double GAMMA_THRESHOLD = 5.0;

        return std::abs(totalDelta) > DELTA_THRESHOLD ||
               std::abs(totalGamma) > GAMMA_THRESHOLD;
    }

    double getPortfolioStress(double marketMove) const {
        // Calculate potential P&L for a given market move using Greeks
        return totalDelta * marketMove + 
               0.5 * totalGamma * marketMove * marketMove +
               totalTheta / 365.0;  // Daily theta
    }

    // Calculate maximum drawdown given current positions
    double calculateMaxDrawdown(const std::vector<double>& historicalPrices) const {
        if (historicalPrices.empty()) return 0.0;

        double maxPrice = historicalPrices[0];
        double maxDrawdown = 0.0;

        for (double price : historicalPrices) {
            if (price > maxPrice) {
                maxPrice = price;
            }
            double drawdown = (maxPrice - price) / maxPrice;
            maxDrawdown = std::max(maxDrawdown, drawdown);
        }

        return maxDrawdown;
    }

    // Check if margin call is imminent
    bool isMarginCallImminent() const {
        constexpr double MARGIN_CALL_THRESHOLD = 0.9; // 90%
        return marginUtilization > MARGIN_CALL_THRESHOLD;
    }
};

#endif // RISK_METRICS_HPP