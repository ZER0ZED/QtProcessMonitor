#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QThread>

// AppControlButton Implementation
AppControlButton::AppControlButton(const QString& _appName, QWidget* parent)
    : QPushButton(parent), AppName(_appName), IsRunning(false)
{
    setText(_appName);
    setCheckable(false);
    setMinimumHeight(40);
    setMinimumWidth(200);

    connect(this, &QPushButton::clicked, this, &AppControlButton::OnButtonClicked);

    UpdateAppearance();
}

void AppControlButton::UpdateStatus(bool _isRunning)
{
    if (IsRunning != _isRunning) {
        IsRunning = _isRunning;
        UpdateAppearance();
    }
}

void AppControlButton::OnButtonClicked()
{
    if (IsRunning) {
        emit StopRequested(AppName);
    } else {
        emit StartRequested(AppName);
    }
}

void AppControlButton::UpdateAppearance()
{
    if (IsRunning) {
        setStyleSheet(
            "QPushButton {"
            "   background-color: #4CAF50;"  // Green
            "   color: white;"
            "   border: 2px solid #45a049;"
            "   border-radius: 8px;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #45a049;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #3d8b40;"
            "}"
            );
        setText(AppName + " - STOP");
    } else {
        setStyleSheet(
            "QPushButton {"
            "   background-color: #f44336;"  // Red
            "   color: white;"
            "   border: 2px solid #da190b;"
            "   border-radius: 8px;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #da190b;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #c1170a;"
            "}"
            );
        setText(AppName + " - START");
    }
}

// MainWindow Implementation
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Initialize core components
    Settings = new A_settingsclass();
    ProcessManager = new A_process(this);

    // Set up the process manager
    ProcessManager->SetSettingsReference(Settings);

    // Initialize UI
    InitializeUI();
    ApplyStyles();

    // Set up display update timer
    DisplayUpdateTimer = new QTimer(this);
    DisplayUpdateTimer->setInterval(1000); // 1 second updates
    connect(DisplayUpdateTimer, &QTimer::timeout, this, &MainWindow::UpdateDisplay);
    DisplayUpdateTimer->start();

    // Connect process manager signals
    connect(ProcessManager, &A_process::ApplicationStatusChanged,
            this, &MainWindow::OnApplicationStatusChanged);
    connect(ProcessManager, &A_process::ApplicationRestarted,
            this, &MainWindow::OnApplicationRestarted);

    // Always load directly from the main project config.xml file
    QString _configPath = "/home/zerozed/projects/QtProcessMonitor/config.xml";
    
    qDebug() << "Loading configuration from:" << _configPath;
    LoadConfiguration(_configPath);

    // Start process monitoring
    ProcessManager->StartMonitoring();

    setWindowTitle("Process Monitor - Qt Application Manager");
    resize(600, 800);

    ShowStatusMessage("Application initialized successfully");
}

MainWindow::~MainWindow()
{
    ProcessManager->StopMonitoring();
    delete Settings;
}

void MainWindow::InitializeUI()
{
    CentralWidget = new QWidget();
    setCentralWidget(CentralWidget);

    MainLayout = new QVBoxLayout(CentralWidget);
    MainLayout->setSpacing(15);
    MainLayout->setContentsMargins(20, 20, 20, 20);

    SetupSettingsSection();
    SetupApplicationsSection();
    SetupControlButtons();
}

void MainWindow::SetupSettingsSection()
{
    SettingsGroup = new QGroupBox("Server Configuration");
    SettingsGroup->setMinimumHeight(120);

    SettingsLayout = new QGridLayout(SettingsGroup);
    SettingsLayout->setSpacing(10);

    // ID field
    SettingsLayout->addWidget(new QLabel("Server ID:"), 0, 0);
    IdLineEdit = new QLineEdit();
    IdLineEdit->setPlaceholderText("Enter server identifier");
    SettingsLayout->addWidget(IdLineEdit, 0, 1);

    // Port field
    SettingsLayout->addWidget(new QLabel("Port:"), 1, 0);
    PortLineEdit = new QLineEdit();
    PortLineEdit->setPlaceholderText("Enter port number (1-65535)");
    SettingsLayout->addWidget(PortLineEdit, 1, 1);

    // IP field
    SettingsLayout->addWidget(new QLabel("IP Address:"), 2, 0);
    IpLineEdit = new QLineEdit();
    IpLineEdit->setPlaceholderText("Enter IP address (IPv4)");
    SettingsLayout->addWidget(IpLineEdit, 2, 1);

    MainLayout->addWidget(SettingsGroup);
}

void MainWindow::SetupApplicationsSection()
{
    AppsGroup = new QGroupBox("Application Control");
    QVBoxLayout* _appsGroupLayout = new QVBoxLayout(AppsGroup);

    // Create scroll area for applications
    ScrollArea = new QScrollArea();
    ScrollArea->setWidgetResizable(true);
    ScrollArea->setMinimumHeight(400);

    ScrollWidget = new QWidget();
    AppsLayout = new QVBoxLayout(ScrollWidget);
    AppsLayout->setSpacing(10);
    AppsLayout->setAlignment(Qt::AlignTop);

    ScrollArea->setWidget(ScrollWidget);
    _appsGroupLayout->addWidget(ScrollArea);

    MainLayout->addWidget(AppsGroup);
}

void MainWindow::SetupControlButtons()
{
    QHBoxLayout* _buttonLayout = new QHBoxLayout();

    SaveButton = new QPushButton("Save Configuration");
    SaveButton->setMinimumHeight(40);
    SaveButton->setMinimumWidth(150);
    connect(SaveButton, &QPushButton::clicked, this, &MainWindow::OnSaveClicked);

    _buttonLayout->addWidget(SaveButton);
    _buttonLayout->addStretch();

    MainLayout->addLayout(_buttonLayout);
}

bool MainWindow::LoadConfiguration(const QString& _filePath)
{
    if (!Settings->LoadConfiguration(_filePath)) {
        ShowStatusMessage("Failed to load configuration file: " + _filePath, true);
        return false;
    }

    UpdateSettingsDisplay();
    UpdateApplicationButtons();

    // Update process manager
    ProcessManager->SetSettingsReference(Settings);

    ShowStatusMessage("Configuration loaded successfully");
    return true;
}

void MainWindow::UpdateSettingsDisplay()
{
    IdLineEdit->setText(Settings->GetId());
    PortLineEdit->setText(Settings->GetPort());
    IpLineEdit->setText(Settings->GetIp());
}

void MainWindow::UpdateApplicationButtons()
{
    // Clear existing buttons
    for (auto _it = AppButtons.begin(); _it != AppButtons.end(); ++_it) {
        delete _it.value();
    }
    AppButtons.clear();

    // Create new buttons for each application
    QList<AppInfo> _apps = Settings->GetApplications();

    for (const AppInfo& _app : _apps) {
        AppControlButton* _button = new AppControlButton(_app.Name);

        // Connect button signals
        connect(_button, &AppControlButton::StartRequested,
                this, &MainWindow::OnStartApplication);
        connect(_button, &AppControlButton::StopRequested,
                this, &MainWindow::OnStopApplication);

        // Update initial status
        bool _isRunning = ProcessManager->IsApplicationRunning(_app.Name);
        _button->UpdateStatus(_isRunning);

        AppButtons[_app.Name] = _button;
        AppsLayout->addWidget(_button);
    }

    if (_apps.isEmpty()) {
        QLabel* _noAppsLabel = new QLabel("No applications configured");
        _noAppsLabel->setAlignment(Qt::AlignCenter);
        _noAppsLabel->setStyleSheet("color: gray; font-style: italic; padding: 20px;");
        AppsLayout->addWidget(_noAppsLabel);
    }
}

void MainWindow::ApplyStyles()
{
    setStyleSheet(
        "QMainWindow {"
        "   background-color: #1E293B;"
        "}"
        "QGroupBox {"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   border: 2px solid #334155;"
        "   border-radius: 8px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "   color: #E2E8F0;"
        "   background-color: #293548;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
        "QLineEdit {"
        "   border: 2px solid #475569;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "   background-color: #334155;"
        "   color: #F1F5F9;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #60A5FA;"
        "}"
        "QLabel {"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "   color: #E2E8F0;"
        "}"
        "QPushButton {"
        "   background-color: #3B82F6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2563EB;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1D4ED8;"
        "}"
        "QScrollArea {"
        "   border: 1px solid #475569;"
        "   border-radius: 5px;"
        "   background-color: #334155;"
        "}"
        "QWidget#ScrollWidget {"
        "   background-color: #334155;"
        "}"
        "QStatusBar {"
        "   background-color: #293548;"
        "   color: #E2E8F0;"
        "}"
        );
}

void MainWindow::OnSaveClicked()
{
    qDebug() << "Save button clicked - starting save process";

    // Update settings from input fields
    QString _id = IdLineEdit->text().trimmed();
    QString _port = PortLineEdit->text().trimmed();
    QString _ip = IpLineEdit->text().trimmed();

    qDebug() << "Current values - ID:" << _id << "Port:" << _port << "IP:" << _ip;

    // Basic validation
    if (_id.isEmpty() || _port.isEmpty() || _ip.isEmpty()) {
        ShowStatusMessage("Please fill in all fields", true);
        return;
    }

    bool _portOk;
    int _portNum = _port.toInt(&_portOk);
    if (!_portOk || _portNum < 1 || _portNum > 65535) {
        ShowStatusMessage("Port must be a number between 1 and 65535", true);
        return;
    }

    // Update settings in memory
    qDebug() << "Updating settings in memory";
    Settings->UpdateSettings(_id, _port, _ip);

    // Verify settings were updated
    qDebug() << "Verifying settings update - ID:" << Settings->GetId()
             << "Port:" << Settings->GetPort() << "IP:" << Settings->GetIp();

    // Save to file
    qDebug() << "Attempting to save configuration to file";
    if (Settings->SaveConfiguration()) {
        ShowStatusMessage("Configuration saved successfully");
        qDebug() << "Configuration save completed successfully";

        // Force refresh display to show saved values
        UpdateSettingsDisplay();
    } else {
        ShowStatusMessage("Failed to save configuration", true);
        qDebug() << "Configuration save failed";
    }
}

void MainWindow::OnStartApplication(const QString& _appName)
{
    qDebug() << "Attempting to start application:" << _appName;

    if (ProcessManager->StartApplication(_appName)) {
        ShowStatusMessage("Starting application: " + _appName);
        qDebug() << "Application start request successful for:" << _appName;

        // Save the status change to XML file
        if (Settings->SaveConfiguration()) {
            qDebug() << "Status change saved to XML for:" << _appName;
        } else {
            qDebug() << "Warning: Failed to save status change to XML for:" << _appName;
        }
    } else {
        ShowStatusMessage("Failed to start application: " + _appName, true);
        qDebug() << "Application start request failed for:" << _appName;
    }
}

void MainWindow::OnStopApplication(const QString& _appName)
{
    qDebug() << "Attempting to stop application:" << _appName;

    if (ProcessManager->StopApplication(_appName)) {
        ShowStatusMessage("Stopping application: " + _appName);
        qDebug() << "Application stop request successful for:" << _appName;

        // Save the status change to XML file
        if (Settings->SaveConfiguration()) {
            qDebug() << "Status change saved to XML for:" << _appName;
        } else {
            qDebug() << "Warning: Failed to save status change to XML for:" << _appName;
        }
    } else {
        ShowStatusMessage("Failed to stop application: " + _appName, true);
        qDebug() << "Application stop request failed for:" << _appName;
    }
}

void MainWindow::OnApplicationStatusChanged(const QString& _appName, bool _isRunning)
{
    if (AppButtons.contains(_appName)) {
        AppButtons[_appName]->UpdateStatus(_isRunning);
    }

    QString _status = _isRunning ? "running" : "stopped";
    qDebug() << "Application status changed:" << _appName << "-" << _status;
}

void MainWindow::OnApplicationRestarted(const QString& _appName)
{
    ShowStatusMessage("Application automatically restarted: " + _appName);
}

void MainWindow::UpdateDisplay()
{
    // Update button statuses based on current process states
    for (auto _it = AppButtons.begin(); _it != AppButtons.end(); ++_it) {
        QString _appName = _it.key();
        AppControlButton* _button = _it.value();
        bool _isRunning = ProcessManager->IsApplicationRunning(_appName);
        _button->UpdateStatus(_isRunning);
    }
}

void MainWindow::ShowStatusMessage(const QString& _message, bool _isError)
{
    if (_isError) {
        statusBar()->setStyleSheet("color: red; font-weight: bold;");
        qDebug() << "Error:" << _message;
    } else {
        statusBar()->setStyleSheet("color: green; font-weight: bold;");
        qDebug() << "Info:" << _message;
    }

    statusBar()->showMessage(_message, 5000); // Show for 5 seconds
}
