#ifndef GUI_APP_HPP
#define GUI_APP_HPP

#include <QApplication>
#include "PlotWindow.hpp"

class GuiApp {
public:
    GuiApp(int& argc, char** argv);
    int run();

private:
    QApplication app_;      // Declare QApplication first
    PlotWindow* window_;    // Make this a pointer instead of an object
};

#endif