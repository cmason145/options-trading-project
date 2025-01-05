#ifndef BLACK_SCHOLES_MODEL_HPP
#define BLACK_SCHOLES_MODEL_HPP

#include <cmath>
#include <stdexcept>
#include <boost/math/distributions/normal.hpp>

class BlackScholesModel {
public:
    struct OptionParameters {
        double spot;          // Current price of underlying
        double strike;        // Strike price
        double riskFreeRate;  // Risk-free interest rate
        double volatility;    // Implied volatility
        double timeToExpiry;  // Time to expiration in years
        bool isCall;          // True for call, false for put

        // Add validation method
        bool isValid() const {
            return spot > 0 && 
                   strike > 0 && 
                   riskFreeRate >= 0 && 
                   volatility > 0 && 
                   timeToExpiry > 0;
        }
    };

    struct Greeks {
        double delta;
        double gamma;
        double theta;
        double vega;
        double rho;

        // Constructor with default initialization
        Greeks() : delta(0), gamma(0), theta(0), vega(0), rho(0) {}
    };

    // Core pricing functions with validation
    static double calculateOptionPrice(const OptionParameters& params);
    static Greeks calculateGreeks(const OptionParameters& params);

    // Added utility functions
    static double calculateImpliedVolatility(const OptionParameters& params, double targetPrice, 
                                           double tolerance = 1e-5, int maxIterations = 100);

private:
    // Helper functions for calculations
    static double calculateD1(const OptionParameters& params);
    static double calculateD2(const OptionParameters& params);
    static double normalCDF(double x);
    static double normalPDF(double x);
    static void validateParameters(const OptionParameters& params);

    // Constants
    static constexpr double EPSILON = 1e-10;
    static constexpr double MAX_VOL = 5.0;  // 500% volatility cap
    static constexpr double MIN_VOL = 0.0001; // Minimum volatility floor
};

#endif