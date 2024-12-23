#ifndef PLOT_WINDOW_HPP
#define PLOT_WINDOW_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

class PlotWindow : public QWidget {
    Q_OBJECT
public:
    explicit PlotWindow(QWidget* parent = nullptr);
    
private slots:
    void updateOptionChain(const std::vector<OptionData>& data);
    void handleOrderSubmission();
    void updateRiskMetrics(const RiskMetrics& metrics);
    
private:
    // GUI components
    QTableWidget* optionChainTable_;
    QTableWidget* positionsTable_;
    QTableWidget* ordersTable_;
    
    // Charts for visualization
    QChart* profitLossChart_;
    QChart* greeksChart_;
    
    void setupOptionChainTable();
    void setupOrderEntry();
    void setupRiskDisplay();
};

#endif // PLOT_WINDOW_HPP