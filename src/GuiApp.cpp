#include "GuiApp.hpp"

GuiApp::GuiApp(int& argc, char** argv)
    : app_(argc, argv), window_(nullptr)  // Initialize pointer to nullptr
{
    // Create window after QApplication is fully constructed
    window_ = new PlotWindow();
    window_->show();
}

int GuiApp::run() {
    int result = app_.exec();
    delete window_;    // Clean up the window
    return result;
}