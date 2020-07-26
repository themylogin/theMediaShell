#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "Application.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    this->setEnglishLayout();
}

void Application::setEnglishLayout()
{
}
