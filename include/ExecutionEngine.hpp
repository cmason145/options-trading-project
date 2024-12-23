#ifndef EXECUTION_ENGINE_HPP
#define EXECUTION_ENGINE_HPP

#include <vector>
#include <string>

class ExecutionEngine {
public:
    ExecutionEngine();
    ~ExecutionEngine();

    void run();
    void addOrder(const std::string& order);
private:
    std::vector<std::string> orders_;
};

#endif
