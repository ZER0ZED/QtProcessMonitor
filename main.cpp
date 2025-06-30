#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include "mainwindow.h"

/**
 * @brief Main entry point for the Qt Process Monitor application
 *
 * Initializes the Qt application, creates the main window,
 * and starts the event loop for GUI interaction and process monitoring.
 *
 * @param argc Argument count - command line argument count
 * @param argv Argument values - command line argument array
 * @return int Application exit code - 0 for success, non-zero for error
 */
int main(int argc, char *argv[])
{
    // Create Qt application instance
    QApplication _app(argc, argv);

    // Set application properties
    _app.setApplicationName("Qt Process Monitor");
    _app.setApplicationVersion("1.0");
    _app.setOrganizationName("Process Management Solutions");
    _app.setOrganizationDomain("example.com");

    qDebug() << "Starting Qt Process Monitor Application";
    qDebug() << "Application Name:" << _app.applicationName();
    qDebug() << "Version:" << _app.applicationVersion();

    // Check if config file exists in current directory
    QString _configPath = QDir::currentPath() + "/config.xml";
    QFileInfo _configInfo(_configPath);

    if (!_configInfo.exists()) {
        qDebug() << "Warning: Configuration file not found at:" << _configPath;
        qDebug() << "The application will attempt to create a default configuration.";
        qDebug() << "Please ensure you have a valid config.xml file in the application directory.";
    } else {
        qDebug() << "Configuration file found at:" << _configPath;
    }

    // Create and show main window
    MainWindow _mainWindow;
    _mainWindow.show();

    qDebug() << "Main window created and displayed";
    qDebug() << "Process monitoring system initialized";

    // Start the Qt event loop
    int _exitCode = _app.exec();

    qDebug() << "Application exiting with code:" << _exitCode;
    return _exitCode;
}
