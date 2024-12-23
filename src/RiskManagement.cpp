#include "RiskManagement.hpp"
#include <ql/quantlib.hpp>

RiskManagement::RiskManagement() {}

double RiskManagement::calculateRiskMetric(const std::vector<double>& positions) {
    // Simple average as a mock risk metric
    double sum = 0.0;
    for (auto& p : positions) sum += p;
    if (positions.empty()) return 0.0;
    return sum / positions.size();
}
