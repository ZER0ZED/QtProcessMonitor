#include "a_process.h"
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDataStream>
#include <QThread>

A_process::A_process(QObject *parent) : QObject(parent)
{
    // Initialize timer with 2 second interval
    MonitorTimer = new QTimer(this);
    MonitorTimer->setInterval(2000); // 2 seconds
    connect(MonitorTimer, &QTimer::timeout, this, &A_process::CheckProcesses);

    SettingsRef = nullptr;

    qDebug() << "A_process initialized with 2-second monitoring interval";
}

A_process::~A_process()
{
    StopMonitoring();

    // Clean up active processes
    for (auto _it = ActiveProcesses.begin(); _it != ActiveProcesses.end(); ++_it) {
        if (_it.value()) {
            _it.value()->kill();
            _it.value()->waitForFinished(3000);
            delete _it.value();
        }
    }
    ActiveProcesses.clear();
}

void A_process::SetSettingsReference(A_settingsclass* _settings)
{
    SettingsRef = _settings;
    UpdateProcessMap();
    qDebug() << "Settings reference set and process map updated";
}

void A_process::StartMonitoring()
{
    if (SettingsRef == nullptr) {
        qDebug() << "Error: Cannot start monitoring without settings reference";
        return;
    }

    UpdateProcessMap();
    MonitorTimer->start();
    qDebug() << "Process monitoring started";
}

void A_process::StopMonitoring()
{
    MonitorTimer->stop();
    qDebug() << "Process monitoring stopped";
}

bool A_process::StartApplication(const QString& _appName)
{
    if (!ProcessMap.contains(_appName)) {
        qDebug() << "Error: Application not found in configuration:" << _appName;
        return false;
    }

    ProcessInfo& _processInfo = ProcessMap[_appName];

    // Check if already running
    if (_processInfo.IsRunning) {
        qDebug() << "Application already running:" << _appName;
        return true;
    }

    // Launch the process
    bool _success = LaunchProcess(_appName, _processInfo.ExecutablePath);
    if (_success) {
        _processInfo.IsRunning = true;
        _processInfo.Status = "start";

        // Update settings
        if (SettingsRef) {
            SettingsRef->UpdateAppStatus(_appName, "start");
        }

        emit ApplicationStatusChanged(_appName, true);
    }

    return _success;
}

bool A_process::StopApplication(const QString& _appName)
{
    if (!ProcessMap.contains(_appName)) {
        qDebug() << "Error: Application not found in configuration:" << _appName;
        return false;
    }

    ProcessInfo& _processInfo = ProcessMap[_appName];

    // Kill the process if running
    if (_processInfo.IsRunning && _processInfo.ProcessId > 0) {
        bool _killSuccess = KillProcess(_processInfo.ProcessId);
        if (_killSuccess) {
            _processInfo.IsRunning = false;
            _processInfo.ProcessId = 0;
            _processInfo.Status = "stop";

            // Clean up QProcess object if exists
            if (ActiveProcesses.contains(_appName)) {
                QProcess* _proc = ActiveProcesses[_appName];
                ActiveProcesses.remove(_appName);
                _proc->kill();
                _proc->waitForFinished(3000);
                delete _proc;
            }

            // Update settings
            if (SettingsRef) {
                SettingsRef->UpdateAppStatus(_appName, "stop");
            }

            emit ApplicationStatusChanged(_appName, false);
            return true;
        }
    }

    return false;
}

bool A_process::IsApplicationRunning(const QString& _appName)
{
    if (ProcessMap.contains(_appName)) {
        return ProcessMap[_appName].IsRunning;
    }
    return false;
}

void A_process::CheckProcesses()
{
    if (!SettingsRef) {
        return;
    }

    // Update process map from current settings
    UpdateProcessMap();

    for (auto _it = ProcessMap.begin(); _it != ProcessMap.end(); ++_it) {
        QString _appName = _it.key();
        ProcessInfo& _processInfo = _it.value();

        // Check if process should be running
        if (_processInfo.Status == "start") {
            // Check if process is actually running
            qint64 _foundPid = FindProcessByExecutable(_processInfo.ExecutablePath);

            if (_foundPid > 0) {
                // Process is running
                if (!_processInfo.IsRunning) {
                    _processInfo.IsRunning = true;
                    _processInfo.ProcessId = _foundPid;
                    emit ApplicationStatusChanged(_appName, true);
                    qDebug() << "Process detected as running:" << _appName;
                }
            } else {
                // Process is not running but should be
                if (_processInfo.IsRunning) {
                    _processInfo.IsRunning = false;
                    _processInfo.ProcessId = 0;
                    emit ApplicationStatusChanged(_appName, false);
                }

                // Restart the process
                qDebug() << "Restarting failed process:" << _appName;
                if (LaunchProcess(_appName, _processInfo.ExecutablePath)) {
                    _processInfo.IsRunning = true;
                    emit ApplicationRestarted(_appName);
                    emit ApplicationStatusChanged(_appName, true);
                }
            }
        } else if (_processInfo.Status == "stop") {
            // Process should not be running
            if (_processInfo.IsRunning) {
                _processInfo.IsRunning = false;
                _processInfo.ProcessId = 0;
                emit ApplicationStatusChanged(_appName, false);
            }
        }
    }
}

void A_process::UpdateProcessMap()
{
    if (!SettingsRef) {
        return;
    }

    ProcessMap.clear();
    QList<AppInfo> _apps = SettingsRef->GetApplications();

    for (const AppInfo& _app : _apps) {
        ProcessInfo _processInfo;
        _processInfo.ExecutablePath = _app.Executable;
        _processInfo.Status = _app.Status;
        _processInfo.IsRunning = false;
        _processInfo.ProcessId = 0;

        ProcessMap[_app.Name] = _processInfo;
    }

    qDebug() << "Process map updated with" << ProcessMap.size() << "applications";
}

qint64 A_process::FindProcessByExecutable(const QString& _executablePath)
{
    // Use system command to find process
    QProcess _findProcess;
    QString _command = QString("pgrep -f \"%1\"").arg(_executablePath);

    _findProcess.start("sh", QStringList() << "-c" << _command);
    _findProcess.waitForFinished(3000);

    if (_findProcess.exitCode() == 0) {
        QString _output = _findProcess.readAllStandardOutput().trimmed();
        QStringList _pids = _output.split('\n', Qt::SkipEmptyParts);

        if (!_pids.isEmpty()) {
            bool _ok;
            qint64 _pid = _pids.first().toLongLong(&_ok);
            if (_ok && _pid > 0) {
                return _pid;
            }
        }
    }

    return 0; // Not found
}

bool A_process::KillProcess(qint64 _processId)
{
    if (_processId <= 0) {
        return false;
    }

    QProcess _killProcess;
    QString _command = QString("kill -TERM %1").arg(_processId);

    _killProcess.start("sh", QStringList() << "-c" << _command);
    _killProcess.waitForFinished(3000);

    if (_killProcess.exitCode() == 0) {
        qDebug() << "Process killed successfully:" << _processId;
        return true;
    } else {
        // Try force kill
        _command = QString("kill -KILL %1").arg(_processId);
        _killProcess.start("sh", QStringList() << "-c" << _command);
        _killProcess.waitForFinished(3000);

        qDebug() << "Force kill attempt for process:" << _processId;
        return (_killProcess.exitCode() == 0);
    }
}

bool A_process::LaunchProcess(const QString& _appName, const QString& _executablePath)
{
    // Check if executable exists
    QFileInfo _fileInfo(_executablePath);
    if (!_fileInfo.exists() || !_fileInfo.isExecutable()) {
        qDebug() << "Error: Executable not found or not executable:" << _executablePath;
        return false;
    }

    // Clean up any existing QProcess for this app
    if (ActiveProcesses.contains(_appName)) {
        QProcess* _oldProc = ActiveProcesses[_appName];
        ActiveProcesses.remove(_appName);
        _oldProc->kill();
        _oldProc->waitForFinished(3000);
        delete _oldProc;
    }

    // Create new QProcess
    QProcess* _newProcess = new QProcess(this);

    // Set up process to start detached (independent of parent)
    QString _program = _executablePath;
    QStringList _arguments;

    // Start the process
    QProcess::startDetached(_program, _arguments);

    // Give it a moment to start
    QThread::msleep(500);

    // Check if it started successfully
    qint64 _pid = FindProcessByExecutable(_executablePath);
    if (_pid > 0) {
        ActiveProcesses[_appName] = _newProcess;
        qDebug() << "Process launched successfully:" << _appName << "PID:" << _pid;
        return true;
    } else {
        delete _newProcess;
        qDebug() << "Error: Failed to launch process:" << _appName;
        return false;
    }
}
