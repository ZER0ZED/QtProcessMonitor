#include "a_process.h"
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <unistd.h>

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

bool A_process::RefreshApplicationStatus(const QString& _appName)
{
    if (!ProcessMap.contains(_appName)) {
        qDebug() << "Application not found for refresh:" << _appName;
        return false;
    }

    ProcessInfo& _processInfo = ProcessMap[_appName];
    qint64 _foundPid = FindProcessByExecutable(_processInfo.ExecutablePath);
    bool _actuallyRunning = (_foundPid > 0);

    qDebug() << "Refreshing status for" << _appName
             << "- Internal:" << _processInfo.IsRunning
             << "Actual:" << _actuallyRunning;

    if (_processInfo.IsRunning != _actuallyRunning) {
        _processInfo.IsRunning = _actuallyRunning;
        _processInfo.ProcessId = _actuallyRunning ? _foundPid : 0;
        emit ApplicationStatusChanged(_appName, _actuallyRunning);
        qDebug() << "Status updated for" << _appName << "to" << _actuallyRunning;
    }

    return true;
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

        // Check current process status
        qint64 _foundPid = FindProcessByExecutable(_processInfo.ExecutablePath);
        bool _actuallyRunning = (_foundPid > 0);

        if (_processInfo.Status == "start") {
            // Application SHOULD be running
            
            if (_actuallyRunning) {
                // Process is running as expected
                if (!_processInfo.IsRunning || _processInfo.ProcessId != _foundPid) {
                    _processInfo.IsRunning = true;
                    _processInfo.ProcessId = _foundPid;
                    emit ApplicationStatusChanged(_appName, true);
                    qDebug() << "Process confirmed running:" << _appName << "PID:" << _foundPid;
                }
            } else {
                // Process should be running but isn't - need to start it
                if (_processInfo.IsRunning) {
                    // Update our state first
                    _processInfo.IsRunning = false;
                    _processInfo.ProcessId = 0;
                    emit ApplicationStatusChanged(_appName, false);
                    qDebug() << "Process stopped unexpectedly:" << _appName;
                }

                // Wait before attempting restart to avoid rapid spawning
                static QMap<QString, qint64> _lastRestartTime;
                qint64 _currentTime = QDateTime::currentMSecsSinceEpoch();
                
                if (!_lastRestartTime.contains(_appName) || 
                    (_currentTime - _lastRestartTime[_appName]) > 5000) { // 5 second minimum between restarts
                    
                    _lastRestartTime[_appName] = _currentTime;
                    qDebug() << "Attempting to start missing process:" << _appName;
                    
                    if (LaunchProcess(_appName, _processInfo.ExecutablePath)) {
                        // Give process time to fully start
                        QThread::msleep(2000);
                        
                        // Verify it actually started
                        _foundPid = FindProcessByExecutable(_processInfo.ExecutablePath);
                        if (_foundPid > 0) {
                            _processInfo.IsRunning = true;
                            _processInfo.ProcessId = _foundPid;
                            emit ApplicationRestarted(_appName);
                            emit ApplicationStatusChanged(_appName, true);
                            qDebug() << "Process started successfully:" << _appName << "PID:" << _foundPid;
                        } else {
                            qDebug() << "Process failed to start properly:" << _appName;
                        }
                    } else {
                        qDebug() << "Failed to launch process:" << _appName;
                    }
                }
            }
            
        } else if (_processInfo.Status == "stop") {
            // Application should NOT be running
            
            if (_actuallyRunning) {
                // Process is running but shouldn't be - stop it
                qDebug() << "Stopping unwanted process:" << _appName << "PID:" << _foundPid;
                
                if (KillProcess(_foundPid)) {
                    _processInfo.IsRunning = false;
                    _processInfo.ProcessId = 0;
                    emit ApplicationStatusChanged(_appName, false);
                    qDebug() << "Successfully stopped process:" << _appName;
                } else {
                    qDebug() << "Failed to stop process:" << _appName;
                }
            } else {
                // Process is correctly stopped
                if (_processInfo.IsRunning) {
                    _processInfo.IsRunning = false;
                    _processInfo.ProcessId = 0;
                    emit ApplicationStatusChanged(_appName, false);
                }
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
    QProcess _findProcess;
    
    // Use pidof command which is more reliable for finding exact executable matches
    QFileInfo _fileInfo(_executablePath);
    QString _execName = _fileInfo.baseName();
    
    // Try pidof first (most reliable)
    QString _command = QString("pidof %1").arg(_execName);
    _findProcess.start("sh", QStringList() << "-c" << _command);
    _findProcess.waitForFinished(3000);

    if (_findProcess.exitCode() == 0) {
        QString _output = _findProcess.readAllStandardOutput().trimmed();
        QStringList _pids = _output.split(' ', Qt::SkipEmptyParts);

        if (!_pids.isEmpty()) {
            bool _ok;
            qint64 _pid = _pids.first().toLongLong(&_ok);
            if (_ok && _pid > 0) {
                qDebug() << "Found process by pidof:" << _execName << "PID:" << _pid;
                return _pid;
            }
        }
    }

    // Fallback to pgrep with exact executable name
    _command = QString("pgrep -x %1").arg(_execName);
    _findProcess.start("sh", QStringList() << "-c" << _command);
    _findProcess.waitForFinished(3000);

    if (_findProcess.exitCode() == 0) {
        QString _output = _findProcess.readAllStandardOutput().trimmed();
        QStringList _pids = _output.split('\n', Qt::SkipEmptyParts);

        if (!_pids.isEmpty()) {
            bool _ok;
            qint64 _pid = _pids.first().toLongLong(&_ok);
            if (_ok && _pid > 0) {
                qDebug() << "Found process by pgrep -x:" << _execName << "PID:" << _pid;
                return _pid;
            }
        }
    }

    return 0; // Not found
}

bool A_process::KillProcess(qint64 _processId)
{
    if (_processId <= 0) {
        qDebug() << "Invalid process ID for kill operation:" << _processId;
        return false;
    }

    QProcess _killProcess;
    
    // Try SIGTERM first (gentler approach)
    QString _command = QString("kill -TERM %1").arg(_processId);
    _killProcess.start("sh", QStringList() << "-c" << _command);
    _killProcess.waitForFinished(3000);
    
    if (_killProcess.exitCode() == 0) {
        qDebug() << "Process killed successfully with SIGTERM:" << _processId;
        return true;
    }
    
    // Try SIGHUP (close terminal) approach
    _command = QString("kill -HUP %1").arg(_processId);
    _killProcess.start("sh", QStringList() << "-c" << _command);
    _killProcess.waitForFinished(3000);
    
    if (_killProcess.exitCode() == 0) {
        qDebug() << "Process killed successfully with SIGHUP:" << _processId;
        return true;
    }
    
    // Try SIGINT (Ctrl+C equivalent)
    _command = QString("kill -INT %1").arg(_processId);
    _killProcess.start("sh", QStringList() << "-c" << _command);
    _killProcess.waitForFinished(3000);
    
    if (_killProcess.exitCode() == 0) {
        qDebug() << "Process killed successfully with SIGINT:" << _processId;
        return true;
    }
    
    // As a last resort, try SIGKILL (force kill)
    _command = QString("kill -KILL %1").arg(_processId);
    qDebug() << "Force kill attempt for process:" << _processId;
    _killProcess.start("sh", QStringList() << "-c" << _command);
    _killProcess.waitForFinished(3000);
    
    if (_killProcess.exitCode() == 0) {
        qDebug() << "Process force-killed successfully:" << _processId;
        return true;
    }
    
    qDebug() << "All kill attempts failed for process:" << _processId;
    return false;
}

bool A_process::LaunchProcess(const QString& _applicationName, const QString& _path)
{
    qDebug() << "Attempting to launch:" << _applicationName << "at path:" << _path;

    // Check if executable exists and is accessible
    QFileInfo _fileInfo(_path);
    if (!_fileInfo.exists() || !_fileInfo.isExecutable()) {
        qDebug() << "Executable does not exist or is not executable:" << _path;
        return false;
    }

    // For GUI applications, use startDetached as the primary method
    QProcessEnvironment _env = QProcessEnvironment::systemEnvironment();
    
    // Ensure GUI environment variables are set properly
    if (!_env.contains("DISPLAY")) {
        _env.insert("DISPLAY", ":0");
    }
    
    if (!_env.contains("XAUTHORITY")) {
        QString _xauth = QDir::homePath() + "/.Xauthority";
        if (QFileInfo(_xauth).exists()) {
            _env.insert("XAUTHORITY", _xauth);
        }
    }
    
    // Add XDG variables to ensure proper desktop integration
    _env.insert("XDG_RUNTIME_DIR", "/run/user/" + QString::number(getuid()));
    _env.insert("XDG_SESSION_TYPE", "x11");
    
    // Set working directory
    QString _workingDir = QDir::homePath();
    
    qint64 _pid = 0;
    
    // First try: startDetached with environment (best for GUI apps)
    QProcess _process;
    _process.setProcessEnvironment(_env);
    _process.setWorkingDirectory(_workingDir);
    
    if (_process.startDetached(_path, QStringList(), _workingDir, &_pid)) {
        qDebug() << "Process started detached successfully:" << _applicationName << "PID:" << _pid;
        return true;
    }
    
    // Second try: Use system command with proper environment
    QString _command = QString("DISPLAY=:0 XDG_RUNTIME_DIR=/run/user/%1 nohup \"%2\" > /dev/null 2>&1 &")
                      .arg(getuid())
                      .arg(_path);
                      
    qDebug() << "Trying system command:" << _command;
    int _result = system(_command.toLocal8Bit().constData());
    
    if (_result == 0) {
        qDebug() << "Process started with system command:" << _applicationName;
        // Give it a moment to start
        QThread::msleep(1000);
        return true;
    }
    
    qDebug() << "All launch methods failed for:" << _applicationName;
    return false;
}
