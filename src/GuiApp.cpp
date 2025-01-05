#include "GuiApp.hpp"
#include <QScreen>
#include <stdexcept>

GuiApp::GuiApp(int& argc, char** argv)
    : argc_(argc)
    , argv_(argv)
    , app_(nullptr)
    , mainWindow_(nullptr)
{
    try {
        // Set Qt platform to xcb explicitly
        qputenv("QT_QPA_PLATFORM", "xcb");
        
        // Create QApplication
        app_ = new QApplication(argc_, argv_);
        
        if (!app_) {
            throw std::runtime_error("Failed to create QApplication");
        }

        // Set application name and organization
        app_->setApplicationName("Trading Application");
        app_->setOrganizationName("TradingOrg");
        
        // Create main window
        mainWindow_ = new PlotWindow();
        if (!mainWindow_) {
            throw std::runtime_error("Failed to create PlotWindow");
        }

        // Center window on screen if there's a screen available
        if (QGuiApplication::screens().size() > 0) {
            QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
            mainWindow_->resize(800, 600);
            mainWindow_->move(
                screenGeometry.x() + (screenGeometry.width() - mainWindow_->width()) / 2,
                screenGeometry.y() + (screenGeometry.height() - mainWindow_->height()) / 2
            );
        }

        // Show the window
        mainWindow_->show();
        mainWindow_->raise();
        mainWindow_->activateWindow();

    } catch (const std::exception& e) {
        cleanup();
        throw;
    }
}

GuiApp::~GuiApp() {
    cleanup();
}

void GuiApp::cleanup() {
    if (mainWindow_) {
        delete mainWindow_;
        mainWindow_ = nullptr;
    }
    if (app_) {
        delete app_;
        app_ = nullptr;
    }
}

int GuiApp::run() {
    if (!app_ || !mainWindow_) {
        return -1;
    }
    return app_->exec();
}