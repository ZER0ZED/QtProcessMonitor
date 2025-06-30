#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QWidget>
#include <QMap>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QStatusBar>

#include "a_settingsclass.h"
#include "a_process.h"

/**
 * @brief Custom button class for application control
 *
 * Extends QPushButton to provide start/stop functionality with
 * visual indicators and status tracking for managed applications.
 */
class AppControlButton : public QPushButton
{
    Q_OBJECT

public:
    explicit AppControlButton(const QString& _appName, QWidget* parent = nullptr);

    /**
     * @brief Updates button state based on application status
     * @param _isRunning Current running state - true for running, false for stopped
     */
    void UpdateStatus(bool _isRunning);

    /**
     * @brief Gets the application name associated with this button
     * @return QString Application name - identifier for the managed app
     */
    QString GetAppName() const { return AppName; }

signals:
    /**
     * @brief Emitted when user requests to start the application
     * @param appName Name of the application to start
     */
    void StartRequested(const QString& appName);

    /**
     * @brief Emitted when user requests to stop the application
     * @param appName Name of the application to stop
     */
    void StopRequested(const QString& appName);

private slots:
    /**
     * @brief Handles button click events
     * Toggles between start and stop commands based on current state
     */
    void OnButtonClicked();

private:
    QString AppName;        // Application name - identifier for managed application
    bool IsRunning;         // Current status - true if app is running, false if stopped

    /**
     * @brief Updates button appearance based on current state
     * Sets colors, text, and visual indicators
     */
    void UpdateAppearance();
};

/**
 * @brief Main application window class
 *
 * Provides GUI interface for configuration management and process monitoring.
 * Displays server settings, dynamic application control buttons, and handles
 * user interactions for the process monitoring system.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief Handles save button clicks
     * Saves current settings and application states to XML file
     */
    void OnSaveClicked();

    /**
     * @brief Handles application start requests from control buttons
     * @param _appName Name of application to start
     */
    void OnStartApplication(const QString& _appName);

    /**
     * @brief Handles application stop requests from control buttons
     * @param _appName Name of application to stop
     */
    void OnStopApplication(const QString& _appName);

    /**
     * @brief Updates GUI when application status changes
     * @param _appName Name of application that changed
     * @param _isRunning New running state
     */
    void OnApplicationStatusChanged(const QString& _appName, bool _isRunning);

    /**
     * @brief Handles application restart notifications
     * @param _appName Name of application that was restarted
     */
    void OnApplicationRestarted(const QString& _appName);

    /**
     * @brief Periodic update of GUI elements
     * Refreshes status displays and button states
     */
    void UpdateDisplay();

private:
    // Core components
    A_settingsclass* Settings;      // Settings manager - handles XML configuration
    A_process* ProcessManager;      // Process manager - monitors and controls applications

    // Main layout widgets
    QWidget* CentralWidget;         // Central widget - main container for all GUI elements
    QVBoxLayout* MainLayout;        // Main layout - vertical arrangement of components

    // Settings section
    QGroupBox* SettingsGroup;       // Settings group box - container for server configuration
    QGridLayout* SettingsLayout;    // Settings layout - grid arrangement for labels and inputs
    QLineEdit* IdLineEdit;          // ID input field - server identifier entry
    QLineEdit* PortLineEdit;        // Port input field - network port entry (1-65535)
    QLineEdit* IpLineEdit;          // IP input field - IP address entry (IPv4 format)

    // Applications section
    QGroupBox* AppsGroup;           // Applications group box - container for app control buttons
    QScrollArea* ScrollArea;        // Scroll area - scrollable container for many apps
    QWidget* ScrollWidget;          // Scroll widget - content widget inside scroll area
    QVBoxLayout* AppsLayout;        // Apps layout - vertical arrangement of app buttons

    // Control buttons
    QPushButton* SaveButton;        // Save button - persists changes to XML file


    // Button tracking
    QMap<QString, AppControlButton*> AppButtons; // Map of app buttons - key: app name, value: button widget

    // Status updates
    QTimer* DisplayUpdateTimer;     // Display timer - periodic GUI refresh timer

    /**
     * @brief Initializes the user interface components
     * Creates and arranges all GUI elements
     */
    void InitializeUI();

    /**
     * @brief Sets up the settings section of the interface
     * Creates input fields for ID, Port, and IP configuration
     */
    void SetupSettingsSection();

    /**
     * @brief Sets up the applications section of the interface
     * Creates scrollable area for application control buttons
     */
    void SetupApplicationsSection();

    /**
     * @brief Sets up control buttons section
     * Creates save and refresh buttons with proper styling
     */
    void SetupControlButtons();

    /**
     * @brief Loads configuration from XML file
     * @param _filePath Path to XML configuration file - must be valid and readable
     * @return bool True if loaded successfully, false on error
     */
    bool LoadConfiguration(const QString& _filePath);

    /**
     * @brief Updates settings display with current values
     * Refreshes ID, Port, and IP fields from configuration
     */
    void UpdateSettingsDisplay();

    /**
     * @brief Recreates application control buttons
     * Generates buttons dynamically based on current configuration
     */
    void UpdateApplicationButtons();

    /**
     * @brief Applies consistent styling to the interface
     * Sets colors, fonts, and visual appearance
     */
    void ApplyStyles();

    /**
     * @brief Shows status message to user
     * @param _message Message text - user-friendly status information
     * @param _isError Error flag - true for error messages, false for info
     */
    void ShowStatusMessage(const QString& _message, bool _isError = false);
};

#endif // MAINWINDOW_H
