#ifndef GUI_APP_HPP
#define GUI_APP_HPP

#include <QApplication>
#include <memory>
#include "PlotWindow.hpp"

class GuiApp {
public:
    GuiApp(int& argc, char** argv);
    ~GuiApp();
    int run();
    PlotWindow* getPlotWindow() { return mainWindow_; }

private:
    void cleanup();
    int& argc_;
    char** argv_;
    QApplication* app_;
    PlotWindow* mainWindow_;
    
    GuiApp(const GuiApp&) = delete;
    GuiApp& operator=(const GuiApp&) = delete;
};

#endif