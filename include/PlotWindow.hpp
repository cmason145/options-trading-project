#ifndef PLOT_WINDOW_HPP
#define PLOT_WINDOW_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "OptionTypes.hpp"
#include "RiskMetrics.hpp"

class PlotWindow : public QWidget {
    Q_OBJECT
public:
    explicit PlotWindow(QWidget* parent = nullptr);
    ~PlotWindow();
    void emitOptionChainUpdate(const std::vector<OptionData>& data) {
        emit optionChainUpdated(data);
    }
    void emitRiskMetricsUpdate(const RiskMetrics& metrics) {
        emit riskMetricsUpdated(metrics);
    }

signals:
    void orderSubmitted(const std::string& symbol, double strike, int quantity);
    void optionChainUpdated(const std::vector<OptionData>& data);
    void riskMetricsUpdated(const RiskMetrics& metrics);

private slots:
    void updateOptionChain(const std::vector<OptionData>& data);
    void handleOrderSubmission();
    void handleBuyOrder();
    void handleSellOrder();
    void updateRiskMetrics(const RiskMetrics& metrics);
    void updatePlot();

private:
    // Layout components
    QVBoxLayout* mainLayout_;
    QLabel* infoLabel_;
    QTimer timer_;
    double t_;

    // GUI components
    QTableWidget* optionChainTable_;
    QTableWidget* positionsTable_;
    QTableWidget* ordersTable_;

    // Order entry components
    QLineEdit* symbolEdit_;
    QLineEdit* strikeEdit_;
    QLineEdit* quantityEdit_;
    QPushButton* buyButton_;
    QPushButton* sellButton_;

    // Charts
    QChart* profitLossChart_;
    QChartView* profitLossChartView_;
    QChart* greeksChart_;
    QChartView* greeksChartView_;

    // Setup functions
    void setupOptionChainTable();
    void setupOrderEntry();
    void setupRiskDisplay();
    void setupCharts();
    void clearChartHistory();

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif