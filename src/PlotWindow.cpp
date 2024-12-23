#include "PlotWindow.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <QString>

PlotWindow::PlotWindow(QWidget* parent)
    : QWidget(parent)
    , mainLayout_(new QVBoxLayout(this))
    , infoLabel_(new QLabel("Market Data Feed", this))
    , t_(0.0)
{
    setObjectName("PlotWindow");  // Add this for better Qt integration
    mainLayout_->addWidget(infoLabel_);
    
    // Use new-style connect syntax
    connect(&timer_, &QTimer::timeout, this, &PlotWindow::updatePlot);
    timer_.start(1000);
    
    setLayout(mainLayout_);
}

PlotWindow::~PlotWindow()
{
    // Qt's parent-child system handles cleanup
}

void PlotWindow::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);  // Always call base class implementation
}

void PlotWindow::updatePlot()
{
    t_ += 1.0;
    infoLabel_->setText(QString("Market Data Feed - Time: %1s").arg(t_));
}