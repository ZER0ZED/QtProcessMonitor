#ifndef A_SETTINGSCLASS_H
#define A_SETTINGSCLASS_H

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringConverter>
#include <QFileInfo>
#include <QDir>

/**
 * @brief Structure to hold application information
 */
struct AppInfo {
    QString Name;           // Application name - identifier for the application
    QString Executable;     // Full path to executable - must be valid system path
    QString Status;         // Current status - "start" or "stop"
};

/**
 * @brief Class responsible for reading and writing XML configuration data
 *
 * This class handles all XML operations including reading settings,
 * managing application configurations, and persisting changes to file.
 */
class A_settingsclass
{
public:
    A_settingsclass();
    ~A_settingsclass();

    /**
     * @brief Loads configuration from XML file
     * @param _filePath Path to XML configuration file - must exist and be readable
     * @return bool True if successful, false on error
     */
    bool LoadConfiguration(const QString& _filePath);

    /**
     * @brief Saves current configuration to XML file
     * @return bool True if successful, false on error
     */
    bool SaveConfiguration();

    /**
     * @brief Updates main settings (ID, Port, IP)
     * @param _id Server identifier - alphanumeric string
     * @param _port Network port - range 1-65535
     * @param _ip IP address - valid IPv4 format
     */
    void UpdateSettings(const QString& _id, const QString& _port, const QString& _ip);

    /**
     * @brief Updates status of specific application
     * @param _appName Application name - must match existing app
     * @param _status New status - "start" or "stop"
     * @return bool True if app found and updated, false otherwise
     */
    bool UpdateAppStatus(const QString& _appName, const QString& _status);

    // Getters for main settings
    QString GetId() const { return ServerId; }           // Server ID - current identifier
    QString GetPort() const { return ServerPort; }       // Server port - current network port
    QString GetIp() const { return ServerIp; }           // Server IP - current IP address

    /**
     * @brief Gets list of all applications
     * @return QList<AppInfo> List of application configurations
     */
    QList<AppInfo> GetApplications() const { return Applications; }

private:
    QString XmlFilePath;        // Path to XML file - stores current file location
    QString ServerId;           // Server identifier - main configuration ID
    QString ServerPort;         // Server port - main configuration port
    QString ServerIp;           // Server IP - main configuration IP address
    QList<AppInfo> Applications; // List of applications - all managed applications

    QDomDocument XmlDocument;   // XML document object - handles XML parsing and writing

    /**
     * @brief Parses settings section from XML
     * @param _settingsElement DOM element containing settings
     */
    void ParseSettings(const QDomElement& _settingsElement);

    /**
     * @brief Parses applications section from XML
     * @param _appsElement DOM element containing applications
     */
    void ParseApplications(const QDomElement& _appsElement);

    /**
     * @brief Creates XML structure for saving
     */
    void BuildXmlDocument();
    
    /**
     * @brief Helper method to save XML to a specific file
     * @param _filePath Path to save the XML file
     * @return bool True if successful, false on error
     */
    bool SaveToFile(const QString& _filePath);
};

#endif // A_SETTINGSCLASS_H
