#include "PlotWindow.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QHeaderView>
#include <QString>

PlotWindow::PlotWindow(QWidget* parent)
    : QWidget(parent)
    , mainLayout_(new QVBoxLayout(this))
    , infoLabel_(new QLabel("Market Data Feed", this))
    , t_(0.0)
    , optionChainTable_(new QTableWidget(this))
    , positionsTable_(new QTableWidget(this))
    , ordersTable_(new QTableWidget(this))
    , symbolEdit_(new QLineEdit(this))
    , strikeEdit_(new QLineEdit(this))
    , quantityEdit_(new QLineEdit(this))
    , buyButton_(new QPushButton("Buy", this))
    , sellButton_(new QPushButton("Sell", this))
{
    setObjectName("PlotWindow");

    // Setup UI components
    setupCharts();
    setupOptionChainTable();
    setupOrderEntry();
    setupRiskDisplay();
    
    // Setup UI
    mainLayout_->addWidget(infoLabel_);
    
    connect(&timer_, &QTimer::timeout, this, &PlotWindow::updatePlot);
    connect(buyButton_, &QPushButton::clicked, this, &PlotWindow::handleBuyOrder);
    connect(sellButton_, &QPushButton::clicked, this, &PlotWindow::handleSellOrder);

    connect(this, &PlotWindow::optionChainUpdated,
            this, &PlotWindow::updateOptionChain);
    connect(this, &PlotWindow::riskMetricsUpdated,
            this, &PlotWindow::updateRiskMetrics);

    timer_.start(1000);
    setLayout(mainLayout_);
}

PlotWindow::~PlotWindow() = default;

void PlotWindow::updatePlot()
{
    t_ += 1.0;
    infoLabel_->setText(QString("Market Data Feed - Time: %1s").arg(t_));
}

void PlotWindow::updateOptionChain(const std::vector<OptionData>& data)
{
    optionChainTable_->setRowCount(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        const auto& option = data[i];
        
        // Convert to QString and use QString::number for numeric values
        optionChainTable_->setItem(i, 0, new QTableWidgetItem(QString::number(option.strike)));
        optionChainTable_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(option.optionType)));
        optionChainTable_->setItem(i, 2, new QTableWidgetItem(QString::number(option.bid, 'f', 2)));
        optionChainTable_->setItem(i, 3, new QTableWidgetItem(QString::number(option.ask, 'f', 2)));
        optionChainTable_->setItem(i, 4, new QTableWidgetItem(QString::number(option.lastPrice, 'f', 2)));
        optionChainTable_->setItem(i, 5, new QTableWidgetItem(QString::number(option.volume)));
        optionChainTable_->setItem(i, 6, new QTableWidgetItem(QString::number(option.delta, 'f', 4)));
        optionChainTable_->setItem(i, 7, new QTableWidgetItem(QString::number(option.gamma, 'f', 4)));
    }
}

void PlotWindow::handleOrderSubmission()
{
    // Validate inputs
    QString symbol = symbolEdit_->text().toUpper();
    QString strikeText = strikeEdit_->text();
    QString quantityText = quantityEdit_->text();
    
    if (symbol.isEmpty() || strikeText.isEmpty() || quantityText.isEmpty()) {
        // Show error message
        return;
    }
    
    double strike = strikeText.toDouble();
    int quantity = quantityText.toInt();
    
    // Add to orders table
    int row = ordersTable_->rowCount();
    ordersTable_->insertRow(row);
    ordersTable_->setItem(row, 0, new QTableWidgetItem(QString("ORD-%1").arg(row + 1)));
    ordersTable_->setItem(row, 1, new QTableWidgetItem(symbol));
    ordersTable_->setItem(row, 2, new QTableWidgetItem(quantity > 0 ? "BUY" : "SELL"));
    ordersTable_->setItem(row, 3, new QTableWidgetItem(QString::number(std::abs(quantity))));
    ordersTable_->setItem(row, 4, new QTableWidgetItem("PENDING"));
    
    // Emit order signal
    emit orderSubmitted(symbol.toStdString(), strike, quantity);
    
    // Clear inputs
    symbolEdit_->clear();
    strikeEdit_->clear();
    quantityEdit_->clear();
}

void PlotWindow::updateRiskMetrics(const RiskMetrics& metrics)
{
    auto* deltaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("deltaSeries").value<QLineSeries*>());
    auto* gammaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("gammaSeries").value<QLineSeries*>());
    auto* thetaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("thetaSeries").value<QLineSeries*>());
    auto* vegaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("vegaSeries").value<QLineSeries*>());

    if (deltaSeries && gammaSeries && thetaSeries && vegaSeries) {
        deltaSeries->append(t_, metrics.totalDelta);
        gammaSeries->append(t_, metrics.totalGamma);
        thetaSeries->append(t_, metrics.totalTheta);
        vegaSeries->append(t_, metrics.totalVega);
    }

    auto* plSeries = qobject_cast<QLineSeries*>(profitLossChart_->property("plSeries").value<QLineSeries*>());
    if (plSeries) {
        plSeries->append(t_, metrics.portfolioValue);
    }

    // Maintain 60-second window
    if (t_ > 60) {
        clearChartHistory();
    }
}

void PlotWindow::clearChartHistory()
{
    auto* deltaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("deltaSeries").value<QLineSeries*>());
    auto* gammaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("gammaSeries").value<QLineSeries*>());
    auto* thetaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("thetaSeries").value<QLineSeries*>());
    auto* vegaSeries = qobject_cast<QLineSeries*>(greeksChart_->property("vegaSeries").value<QLineSeries*>());
    auto* plSeries = qobject_cast<QLineSeries*>(profitLossChart_->property("plSeries").value<QLineSeries*>());

    for (auto* series : {deltaSeries, gammaSeries, thetaSeries, vegaSeries, plSeries}) {
        if (series) {
            while (!series->points().isEmpty() && series->points().first().x() < t_ - 60) {
                series->remove(0);
            }
        }
    }
}

void PlotWindow::setupOptionChainTable()
{
    optionChainTable_->setColumnCount(8);
    optionChainTable_->setHorizontalHeaderLabels({
        "Strike", "Type", "Bid", "Ask", "Last", "Volume", "Delta", "Gamma"
    });
    optionChainTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    optionChainTable_->setAlternatingRowColors(true);
    mainLayout_->addWidget(optionChainTable_);
}

void PlotWindow::setupOrderEntry()
{
    auto* orderGroup = new QGroupBox("Order Entry", this);
    auto* orderLayout = new QGridLayout;
    
    auto* symbolLabel = new QLabel("Symbol:", orderGroup);
    auto* strikeLabel = new QLabel("Strike:", orderGroup);
    auto* quantityLabel = new QLabel("Quantity:", orderGroup);
    
    // Set placeholders
    symbolEdit_->setPlaceholderText("e.g., AAPL");
    strikeEdit_->setPlaceholderText("e.g., 150.00");
    quantityEdit_->setPlaceholderText("e.g., 10");
    
    // Set validators
    symbolEdit_->setMaxLength(5);
    strikeEdit_->setValidator(new QDoubleValidator(0, 10000, 2, this));
    quantityEdit_->setValidator(new QIntValidator(1, 10000, this));
    
    orderLayout->addWidget(symbolLabel, 0, 0);
    orderLayout->addWidget(symbolEdit_, 0, 1);
    orderLayout->addWidget(strikeLabel, 1, 0);
    orderLayout->addWidget(strikeEdit_, 1, 1);
    orderLayout->addWidget(quantityLabel, 2, 0);
    orderLayout->addWidget(quantityEdit_, 2, 1);
    orderLayout->addWidget(buyButton_, 3, 0);
    orderLayout->addWidget(sellButton_, 3, 1);
    
    orderGroup->setLayout(orderLayout);
    mainLayout_->addWidget(orderGroup);
}

void PlotWindow::handleBuyOrder()
{
    quantityEdit_->setText(QString::number(std::abs(quantityEdit_->text().toInt())));
    handleOrderSubmission();
}

void PlotWindow::handleSellOrder()
{
    quantityEdit_->setText(QString::number(-std::abs(quantityEdit_->text().toInt())));
    handleOrderSubmission();
}

void PlotWindow::setupRiskDisplay()
{
    // Create positions table
    positionsTable_->setColumnCount(6);
    positionsTable_->setHorizontalHeaderLabels({
        "Symbol", "Type", "Strike", "Quantity", "P/L", "Delta"
    });
    positionsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    positionsTable_->setAlternatingRowColors(true);
    
    // Create orders table
    ordersTable_->setColumnCount(5);
    ordersTable_->setHorizontalHeaderLabels({
        "Order ID", "Symbol", "Type", "Quantity", "Status"
    });
    ordersTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ordersTable_->setAlternatingRowColors(true);
    
    // Add tables to layout
    auto* tabWidget = new QTabWidget(this);
    tabWidget->addTab(positionsTable_, "Positions");
    tabWidget->addTab(ordersTable_, "Orders");
    mainLayout_->addWidget(tabWidget);
}

void PlotWindow::setupCharts()
{
    // Create chart widget containers
    auto* chartsContainer = new QWidget(this);
    auto* chartsLayout = new QHBoxLayout(chartsContainer);

    // Set up P/L chart
    profitLossChart_ = new QChart();
    profitLossChart_->setTitle("Profit/Loss Over Time");
    profitLossChart_->legend()->setVisible(true);
    profitLossChart_->legend()->setAlignment(Qt::AlignBottom);

    // Create P/L series and axes
    auto* plSeries = new QLineSeries(profitLossChart_);
    plSeries->setName("Portfolio P/L");

    auto* timeAxis = new QValueAxis(profitLossChart_);
    timeAxis->setTitleText("Time (s)");
    timeAxis->setRange(0, 60);  // Show last 60 seconds by default
    timeAxis->setTickCount(7);  // Show 7 ticks on time axis

    auto* plAxis = new QValueAxis(profitLossChart_);
    plAxis->setTitleText("P/L ($)");
    plAxis->setRange(-10000, 10000);  // Adjust range based on your typical P/L values
    plAxis->setTickCount(5);

    profitLossChart_->addSeries(plSeries);
    profitLossChart_->addAxis(timeAxis, Qt::AlignBottom);
    profitLossChart_->addAxis(plAxis, Qt::AlignLeft);
    plSeries->attachAxis(timeAxis);
    plSeries->attachAxis(plAxis);

    profitLossChartView_ = new QChartView(profitLossChart_);
    profitLossChartView_->setRenderHint(QPainter::Antialiasing);
    profitLossChartView_->setMinimumSize(400, 300);

    // Set up Greeks chart
    greeksChart_ = new QChart();
    greeksChart_->setTitle("Option Greeks");
    greeksChart_->legend()->setVisible(true);
    greeksChart_->legend()->setAlignment(Qt::AlignBottom);

    // Create series for each Greek
    auto* deltaSeries = new QLineSeries(greeksChart_);
    auto* gammaSeries = new QLineSeries(greeksChart_);
    auto* thetaSeries = new QLineSeries(greeksChart_);
    auto* vegaSeries = new QLineSeries(greeksChart_);

    deltaSeries->setName("Delta");
    gammaSeries->setName("Gamma");
    thetaSeries->setName("Theta");
    vegaSeries->setName("Vega");

    // Set up axes for Greeks
    auto* greeksTimeAxis = new QValueAxis(greeksChart_);
    greeksTimeAxis->setTitleText("Time (s)");
    greeksTimeAxis->setRange(0, 60);
    greeksTimeAxis->setTickCount(7);

    auto* greeksValueAxis = new QValueAxis(greeksChart_);
    greeksValueAxis->setTitleText("Greek Value");
    greeksValueAxis->setRange(-100, 100);  // Adjust based on your typical Greek values
    greeksValueAxis->setTickCount(5);

    greeksChart_->addSeries(deltaSeries);
    greeksChart_->addSeries(gammaSeries);
    greeksChart_->addSeries(thetaSeries);
    greeksChart_->addSeries(vegaSeries);

    greeksChart_->addAxis(greeksTimeAxis, Qt::AlignBottom);
    greeksChart_->addAxis(greeksValueAxis, Qt::AlignLeft);

    for (auto* series : {deltaSeries, gammaSeries, thetaSeries, vegaSeries}) {
        series->attachAxis(greeksTimeAxis);
        series->attachAxis(greeksValueAxis);
    }

    greeksChartView_ = new QChartView(greeksChart_);
    greeksChartView_->setRenderHint(QPainter::Antialiasing);
    greeksChartView_->setMinimumSize(400, 300);

    // Add charts to layout
    chartsLayout->addWidget(profitLossChartView_);
    chartsLayout->addWidget(greeksChartView_);
    
    // Add charts container to main layout
    mainLayout_->addWidget(chartsContainer);

    // Store series pointers for later updates if needed
    profitLossChart_->setProperty("plSeries", QVariant::fromValue(plSeries));
    greeksChart_->setProperty("deltaSeries", QVariant::fromValue(deltaSeries));
    greeksChart_->setProperty("gammaSeries", QVariant::fromValue(gammaSeries));
    greeksChart_->setProperty("thetaSeries", QVariant::fromValue(thetaSeries));
    greeksChart_->setProperty("vegaSeries", QVariant::fromValue(vegaSeries));
}

void PlotWindow::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}