#ifndef A_PROCESS_H
#define A_PROCESS_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QMap>
#include <QString>
#include <QDebug>
#include "a_settingsclass.h"

/**
 * @brief Structure to hold process monitoring information
 */
struct ProcessInfo {
    QString ExecutablePath;     // Full path to executable - system path to binary
    QString Status;            // Desired status - "start" or "stop"
    bool IsRunning;           // Current running state - true if process is active
    qint64 ProcessId;         // System process ID - 0 if not running
};

/**
 * @brief Class responsible for monitoring and managing application processes
 *
 * This class monitors application processes every 2 seconds, automatically
 * restarting failed processes that should be running, and managing process
 * lifecycle based on configuration settings.
 */
class A_process : public QObject
{
    Q_OBJECT

public:
    explicit A_process(QObject *parent = nullptr);
    ~A_process();

    /**
     * @brief Sets reference to settings class for configuration access
     * @param _settings Pointer to settings class - must remain valid during lifetime
     */
    void SetSettingsReference(A_settingsclass* _settings);

    /**
     * @brief Starts the monitoring timer
     * Begins checking process status every 2 seconds
     */
    void StartMonitoring();

    /**
     * @brief Stops the monitoring timer
     * Halts all process monitoring activities
     */
    void StopMonitoring();

    /**
     * @brief Manually starts a specific application
     * @param _appName Application name - must match configuration
     * @return bool True if started successfully, false on error
     */
    bool StartApplication(const QString& _appName);

    /**
     * @brief Manually stops a specific application
     * @param _appName Application name - must match configuration
     * @return bool True if stopped successfully, false on error
     */
    bool StopApplication(const QString& _appName);

    /**
     * @brief Checks if specific application is currently running
     * @param _appName Application name - must match configuration
     * @return bool True if running, false if stopped or not found
     */
    bool IsApplicationRunning(const QString& _appName);

    /**
     * @brief Manually refresh the status of a specific application
     * @param _appName Application name - must match configuration
     * @return bool True if application exists in configuration
     */
    bool RefreshApplicationStatus(const QString& _appName);

signals:
    /**
     * @brief Emitted when an application status changes
     * @param appName Name of the application
     * @param isRunning Current running state
     */
    void ApplicationStatusChanged(const QString& appName, bool isRunning);

    /**
     * @brief Emitted when an application is automatically restarted
     * @param appName Name of the restarted application
     */
    void ApplicationRestarted(const QString& appName);

private slots:
    /**
     * @brief Timer callback for periodic process monitoring
     * Checks all processes every 2 seconds and restarts if needed
     */
    void CheckProcesses();

private:
    QTimer* MonitorTimer;               // Timer for periodic checks - 2 second interval
    A_settingsclass* SettingsRef;       // Reference to settings - configuration source
    QMap<QString, ProcessInfo> ProcessMap; // Map of managed processes - key: app name, value: process info
    QMap<QString, QProcess*> ActiveProcesses; // Map of active QProcess objects - for process management

    /**
     * @brief Updates internal process map from settings
     * Synchronizes process list with current configuration
     */
    void UpdateProcessMap();

    /**
     * @brief Checks if a process is running by executable path
     * @param _executablePath Full path to executable
     * @return qint64 Process ID if running, 0 if not found
     */
    qint64 FindProcessByExecutable(const QString& _executablePath);

    /**
     * @brief Kills a process by process ID
     * @param _processId System process ID - must be valid PID
     * @return bool True if killed successfully, false on error
     */
    bool KillProcess(qint64 _processId);

    /**
     * @brief Starts a new process
     * @param _appName Application name - for tracking purposes
     * @param _executablePath Full path to executable
     * @return bool True if started successfully, false on error
     */
    bool LaunchProcess(const QString& _appName, const QString& _executablePath);
};

#endif // A_PROCESS_H
