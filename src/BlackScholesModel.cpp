#include "BlackScholesModel.hpp"

void BlackScholesModel::validateParameters(const OptionParameters& params) {
    if (!params.isValid()) {
        throw std::invalid_argument("Invalid option parameters");
    }
    if (params.volatility > MAX_VOL || params.volatility < MIN_VOL) {
        throw std::invalid_argument("Volatility out of reasonable bounds");
    }
}

double BlackScholesModel::calculateOptionPrice(const OptionParameters& params) {
    validateParameters(params);

    double d1 = calculateD1(params);
    double d2 = calculateD2(params);
    
    if (std::isnan(d1) || std::isnan(d2)) {
        throw std::runtime_error("Invalid d1/d2 calculation result");
    }

    double spotTimesNd1;
    double strikeTimesNd2;
    
    if (params.isCall) {
        spotTimesNd1 = params.spot * normalCDF(d1);
        strikeTimesNd2 = params.strike * normalCDF(d2);
    } else {
        spotTimesNd1 = params.spot * normalCDF(-d1);
        strikeTimesNd2 = params.strike * normalCDF(-d2);
    }
    
    double discountFactor = exp(-params.riskFreeRate * params.timeToExpiry);
    
    if (params.isCall) {
        return std::max(0.0, spotTimesNd1 - strikeTimesNd2 * discountFactor);
    } else {
        return std::max(0.0, strikeTimesNd2 * discountFactor - spotTimesNd1);
    }
}

BlackScholesModel::Greeks BlackScholesModel::calculateGreeks(const OptionParameters& params) {
    validateParameters(params);
    Greeks greeks;

    double d1 = calculateD1(params);
    double d2 = calculateD2(params);
    
    if (std::isnan(d1) || std::isnan(d2)) {
        throw std::runtime_error("Invalid d1/d2 calculation result");
    }

    double discountFactor = exp(-params.riskFreeRate * params.timeToExpiry);
    
    // Calculate Delta
    greeks.delta = params.isCall ? normalCDF(d1) : normalCDF(d1) - 1;
    
    // Calculate Gamma (same for calls and puts)
    double sqrtTimeToExpiry = sqrt(params.timeToExpiry);
    greeks.gamma = normalPDF(d1) / (params.spot * params.volatility * sqrtTimeToExpiry);
    
    // Calculate Theta
    double spotGammaPart = -(params.spot * params.volatility * normalPDF(d1)) / (2 * sqrtTimeToExpiry);
    double ratesPart = params.isCall ?
        -params.strike * params.riskFreeRate * discountFactor * normalCDF(d2) :
        params.strike * params.riskFreeRate * discountFactor * normalCDF(-d2);
    greeks.theta = spotGammaPart + ratesPart;
    
    // Calculate Vega (same for calls and puts)
    greeks.vega = params.spot * sqrtTimeToExpiry * normalPDF(d1) * 0.01; // Scale to 1% move
    
    // Calculate Rho
    greeks.rho = params.isCall ?
        params.strike * params.timeToExpiry * discountFactor * normalCDF(d2) * 0.01 :
        -params.strike * params.timeToExpiry * discountFactor * normalCDF(-d2) * 0.01;
    
    return greeks;
}

double BlackScholesModel::calculateD1(const OptionParameters& params) {
    double sqrtTimeToExpiry = sqrt(params.timeToExpiry);
    if (sqrtTimeToExpiry < EPSILON) {
        throw std::runtime_error("Time to expiry too close to zero");
    }

    return (log(params.spot / params.strike) +
            (params.riskFreeRate + params.volatility * params.volatility / 2) * params.timeToExpiry) /
           (params.volatility * sqrtTimeToExpiry);
}

double BlackScholesModel::calculateD2(const OptionParameters& params) {
    return calculateD1(params) - params.volatility * sqrt(params.timeToExpiry);
}

double BlackScholesModel::normalCDF(double x) {
    static const boost::math::normal_distribution<double> norm;
    return boost::math::cdf(norm, x);
}

double BlackScholesModel::normalPDF(double x) {
    static const double sqrt2pi = sqrt(2.0 * M_PI);
    return exp(-x*x/2.0) / sqrt2pi;
}

double BlackScholesModel::calculateImpliedVolatility(const OptionParameters& params, 
                                                   double targetPrice, 
                                                   double tolerance, 
                                                   int maxIterations) {
    // Start with an initial guess using a modified Brenner-Subrahmanyam formula
    double initial_vol = sqrt(2 * M_PI / params.timeToExpiry) * 
                        (targetPrice / params.spot);
    
    // Ensure initial guess is within bounds
    initial_vol = std::max(MIN_VOL, std::min(MAX_VOL, initial_vol));
    
    OptionParameters iterParams = params;
    iterParams.volatility = initial_vol;
    
    // Newton-Raphson iteration
    for (int i = 0; i < maxIterations; ++i) {
        double price = calculateOptionPrice(iterParams);
        double diff = price - targetPrice;
        
        if (std::abs(diff) < tolerance)
            return iterParams.volatility;
            
        // Vega is the derivative of price with respect to volatility
        double vega = calculateGreeks(iterParams).vega;
        
        if (std::abs(vega) < EPSILON)
            throw std::runtime_error("Zero vega in implied vol calculation");
            
        // Update volatility estimate
        double newVol = iterParams.volatility - diff / (vega * 100.0); // Adjust for vega scaling
        newVol = std::max(MIN_VOL, std::min(MAX_VOL, newVol));
        
        if (std::abs(newVol - iterParams.volatility) < tolerance)
            return newVol;
            
        iterParams.volatility = newVol;
    }
    
    throw std::runtime_error("Implied volatility did not converge");
}