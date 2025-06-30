#include "a_settingsclass.h"

A_settingsclass::A_settingsclass()
{
    // Initialize default values
    ServerId = "";
    ServerPort = "";
    ServerIp = "";
    XmlFilePath = "";
}

A_settingsclass::~A_settingsclass()
{
    // Destructor - cleanup if needed
}

bool A_settingsclass::LoadConfiguration(const QString& _filePath)
{
    XmlFilePath = _filePath;

    QFile _xmlFile(_filePath);
    if (!_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Cannot open XML file:" << _filePath;
        return false;
    }

    QString _errorMsg;
    int _errorLine, _errorColumn;

    if (!XmlDocument.setContent(&_xmlFile, &_errorMsg, &_errorLine, &_errorColumn)) {
        qDebug() << "Error parsing XML:" << _errorMsg
                 << "at line" << _errorLine << "column" << _errorColumn;
        _xmlFile.close();
        return false;
    }

    _xmlFile.close();

    // Parse the loaded XML
    QDomElement _rootElement = XmlDocument.documentElement();
    if (_rootElement.tagName() != "configuration") {
        qDebug() << "Error: Invalid XML structure. Expected 'configuration' root element.";
        return false;
    }

    // Parse settings
    QDomNodeList _settingsNodes = _rootElement.elementsByTagName("settings");
    if (_settingsNodes.size() > 0) {
        ParseSettings(_settingsNodes.at(0).toElement());
    }

    // Parse applications
    QDomNodeList _appsNodes = _rootElement.elementsByTagName("applications");
    if (_appsNodes.size() > 0) {
        ParseApplications(_appsNodes.at(0).toElement());
    }

    return true;
}

bool A_settingsclass::SaveConfiguration()
{
    if (XmlFilePath.isEmpty()) {
        qDebug() << "Error: No file path set for saving configuration.";
        return false;
    }

    // Build the XML document with current data
    BuildXmlDocument();
    
    // Save to the current working file first
    bool _savedToCurrent = SaveToFile(XmlFilePath);
    
    // Determine if we need to save to additional locations
    QString _currentPath = QFileInfo(XmlFilePath).absoluteFilePath();
    qDebug() << "Current XML file path:" << _currentPath;
    
    // Try to identify both the source and build locations
    QString _sourcePath;
    QString _buildPath;
    
    // Case 1: We're loading from the main project directory (our fix working)
    if (_currentPath.contains("/QtProcessMonitor/config.xml") && !_currentPath.contains("/build/")) {
        _sourcePath = _currentPath;  // Already the source
        _buildPath = QFileInfo(_currentPath).absolutePath() + "/build/bin/config.xml";
        qDebug() << "Detected source config, will also save to build:" << _buildPath;
    }
    // Case 2: We're loading from build directory
    else if (_currentPath.contains("/build/bin/config.xml")) {
        _buildPath = _currentPath;  // Already the build copy
        _sourcePath = QFileInfo(_currentPath).absolutePath() + "/../../config.xml";
        qDebug() << "Detected build config, will also save to source:" << _sourcePath;
    }
    // Case 3: Fallback - try both possibilities
    else {
        _sourcePath = QDir::currentPath() + "/config.xml";
        _buildPath = QDir::currentPath() + "/build/bin/config.xml";
        qDebug() << "Fallback mode - trying both paths";
    }
    
    // Save to the other location if it exists or is writable
    bool _savedToOther = false;
    QString _otherPath = (_currentPath == _sourcePath) ? _buildPath : _sourcePath;
    
    if (!_otherPath.isEmpty() && _otherPath != _currentPath) {
        QFileInfo _otherInfo(_otherPath);
        QFileInfo _otherDir(QFileInfo(_otherPath).absolutePath());
        
        if (_otherInfo.exists() || _otherDir.isWritable()) {
            _savedToOther = SaveToFile(_otherPath);
            if (_savedToOther) {
                qDebug() << "Configuration also saved to:" << _otherPath;
            } else {
                qDebug() << "Failed to save to:" << _otherPath;
            }
        } else {
            qDebug() << "Cannot save to (not writable or doesn't exist):" << _otherPath;
        }
    }
    
    // Return true if at least the main save succeeded
    return _savedToCurrent;
}

// Helper method to save XML to a specific file
bool A_settingsclass::SaveToFile(const QString& _filePath)
{
    QFile _xmlFile(_filePath);
    if (!_xmlFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "Error: Cannot open XML file for writing:" << _filePath;
        qDebug() << "File error:" << _xmlFile.errorString();
        return false;
    }

    QTextStream _stream(&_xmlFile);
    _stream.setEncoding(QStringConverter::Utf8);
    _stream << XmlDocument.toString(4); // 4 spaces indentation
    _stream.flush();
    _xmlFile.close();

    // Verify the file was written
    QFileInfo _fileInfo(_filePath);
    if (_fileInfo.exists() && _fileInfo.size() > 0) {
        qDebug() << "Configuration saved successfully to:" << _filePath;
        qDebug() << "File size:" << _fileInfo.size() << "bytes";
        return true;
    } else {
        qDebug() << "Error: Failed to write configuration file";
        return false;
    }
}

void A_settingsclass::UpdateSettings(const QString& _id, const QString& _port, const QString& _ip)
{
    ServerId = _id;
    ServerPort = _port;
    ServerIp = _ip;
    qDebug() << "Settings updated - ID:" << _id << "Port:" << _port << "IP:" << _ip;
}

bool A_settingsclass::UpdateAppStatus(const QString& _appName, const QString& _status)
{
    for (int _i = 0; _i < Applications.size(); ++_i) {
        if (Applications[_i].Name == _appName) {
            Applications[_i].Status = _status;
            qDebug() << "App status updated -" << _appName << ":" << _status;
            return true;
        }
    }

    qDebug() << "Warning: Application not found:" << _appName;
    return false;
}

void A_settingsclass::ParseSettings(const QDomElement& _settingsElement)
{
    QDomNodeList _children = _settingsElement.childNodes();

    for (int _i = 0; _i < _children.size(); ++_i) {
        QDomElement _child = _children.at(_i).toElement();

        if (_child.tagName() == "id") {
            ServerId = _child.text().trimmed();
        } else if (_child.tagName() == "port") {
            ServerPort = _child.text().trimmed();
        } else if (_child.tagName() == "ip") {
            ServerIp = _child.text().trimmed();
        }
    }

    qDebug() << "Settings parsed - ID:" << ServerId << "Port:" << ServerPort << "IP:" << ServerIp;
}

void A_settingsclass::ParseApplications(const QDomElement& _appsElement)
{
    Applications.clear();
    QDomNodeList _appNodes = _appsElement.elementsByTagName("app");

    for (int _i = 0; _i < _appNodes.size(); ++_i) {
        QDomElement _appElement = _appNodes.at(_i).toElement();
        AppInfo _appInfo;

        // Parse app children
        QDomNodeList _appChildren = _appElement.childNodes();
        for (int _j = 0; _j < _appChildren.size(); ++_j) {
            QDomElement _child = _appChildren.at(_j).toElement();

            if (_child.tagName() == "n") {
                _appInfo.Name = _child.text().trimmed();
            } else if (_child.tagName() == "executable") {
                _appInfo.Executable = _child.text().trimmed();
            } else if (_child.tagName() == "status") {
                _appInfo.Status = _child.text().trimmed();
            }
        }

        if (!_appInfo.Name.isEmpty() && !_appInfo.Executable.isEmpty()) {
            Applications.append(_appInfo);
            qDebug() << "App parsed:" << _appInfo.Name << "Status:" << _appInfo.Status;
        }
    }

    qDebug() << "Total applications loaded:" << Applications.size();
}

void A_settingsclass::BuildXmlDocument()
{
    XmlDocument.clear();

    // Create root element
    QDomElement _rootElement = XmlDocument.createElement("configuration");
    XmlDocument.appendChild(_rootElement);

    // Create settings section
    QDomElement _settingsElement = XmlDocument.createElement("settings");
    _rootElement.appendChild(_settingsElement);

    // Add settings children
    QDomElement _idElement = XmlDocument.createElement("id");
    _idElement.appendChild(XmlDocument.createTextNode(ServerId));
    _settingsElement.appendChild(_idElement);

    QDomElement _portElement = XmlDocument.createElement("port");
    _portElement.appendChild(XmlDocument.createTextNode(ServerPort));
    _settingsElement.appendChild(_portElement);

    QDomElement _ipElement = XmlDocument.createElement("ip");
    _ipElement.appendChild(XmlDocument.createTextNode(ServerIp));
    _settingsElement.appendChild(_ipElement);

    // Create applications section
    QDomElement _appsElement = XmlDocument.createElement("applications");
    _rootElement.appendChild(_appsElement);

    // Add each application
    for (const AppInfo& _app : Applications) {
        QDomElement _appElement = XmlDocument.createElement("app");
        _appsElement.appendChild(_appElement);

        QDomElement _nameElement = XmlDocument.createElement("n");
        _nameElement.appendChild(XmlDocument.createTextNode(_app.Name));
        _appElement.appendChild(_nameElement);

        QDomElement _execElement = XmlDocument.createElement("executable");
        _execElement.appendChild(XmlDocument.createTextNode(_app.Executable));
        _appElement.appendChild(_execElement);

        QDomElement _statusElement = XmlDocument.createElement("status");
        _statusElement.appendChild(XmlDocument.createTextNode(_app.Status));
        _appElement.appendChild(_statusElement);
    }
}
