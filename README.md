# Qt Process Monitor

<div align="center">

![Qt](https://img.shields.io/badge/Qt-6.0+-green.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20Ubuntu-orange.svg)
![License](https://img.shields.io/badge/License-Educational-lightgrey.svg)

**A professional Qt application for monitoring and managing system processes on Linux Ubuntu**

*Automatically restart failed applications • Real-time process monitoring • Dark theme interface*

</div>

---

## 🚀 Features

### ✨ **Core Functionality**
- **🔄 Automatic Process Monitoring** - Checks application status every 2 seconds
- **🔁 Auto-Restart Failed Processes** - Automatically restarts crashed applications
- **⚙️ XML Configuration Management** - Easy-to-edit configuration files
- **🎯 Dynamic Application Control** - Generate control buttons based on configuration
- **💾 Real-time Settings Persistence** - Changes saved immediately to XML

### 🎨 **User Interface**
- **🌙 Modern Dark Theme** - Professional dark interface with teal accents
- **🔘 Color-Coded Status Buttons** - Green (running) / Red (stopped) indicators
- **📊 Real-time Status Updates** - Live monitoring with visual feedback
- **📜 Scrollable Application List** - Handles multiple applications gracefully
- **💬 Status Messages** - Clear feedback for all operations

### 🛠️ **Technical Features**
- **🔧 Professional Code Architecture** - Well-documented, maintainable codebase
- **⚡ Efficient Process Detection** - Multiple fallback methods for process finding
- **🛡️ Robust Error Handling** - Comprehensive logging and error recovery
- **🔄 Cross-location XML Sync** - Automatically syncs between source and build directories

---

## 📸 Screenshots

### Main Interface
- **Server Configuration** - Configure ID, Port, and IP settings
- **Application Control** - Start/stop applications with color-coded buttons
- **Dark Theme** - Modern, professional interface design

### Process Management
- **Real-time Monitoring** - Live status updates every 2 seconds  
- **Automatic Restart** - Failed processes restarted automatically
- **Manual Control** - Click buttons to start/stop applications

---

## 🔧 Installation

### Prerequisites

**System Requirements:**
- Ubuntu 18.04 or later
- Qt 6.0+ (or Qt 5.15+)
- CMake 3.16+
- GCC 7.0+ or Clang 8.0+

**Install Dependencies:**
```bash
# For Qt 6 (recommended)
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Alternative: For Qt 5
sudo apt install qtbase5-dev qttools5-dev-tools cmake build-essential
```

### Build Instructions

#### Option 1: CMake (Recommended)
```bash
# Clone or extract the project
cd qtprocessmonitor

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the application
make -j$(nproc)

# Run the application
./bin/qtprocessmonitor
```

#### Option 2: qmake
```bash
# Generate Makefile
qmake qtprocessmonitor.pro

# Build the application
make

# Run the application
./qtprocessmonitor
```

---

## ⚙️ Configuration

### XML Configuration File

The application uses `config.xml` for all settings. Here's the structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<configuration>
    <settings>
        <id>SERVER_001</id>
        <port>8080</port>
        <ip>192.168.1.100</ip>
    </settings>
    <applications>
        <app>
            <n>Calculator</n>
            <executable>/usr/bin/gnome-calculator</executable>
            <status>stop</status>
        </app>
        <app>
            <n>TextEditor</n>
            <executable>/usr/bin/gedit</executable>
            <status>start</status>
        </app>
    </applications>
</configuration>
```

### Configuration Elements

#### Server Settings
- **`id`** - Server identifier (alphanumeric string)
- **`port`** - Network port (1-65535)
- **`ip`** - IP address (IPv4 format)

#### Applications
- **`n`** - Application display name
- **`executable`** - Full path to executable file
- **`status`** - Desired status (`start` or `stop`)

### Finding Application Paths

Use these commands to find application paths:
```bash
# Find common applications
which gnome-calculator  # Calculator
which gedit             # Text Editor  
which firefox           # Web Browser
which code              # VS Code

# Search for applications
find /usr/bin -name "*calc*" 2>/dev/null
ls /usr/share/applications/ | grep -i [app-name]
```

---

## 🎮 Usage

### Main Interface

1. **Server Configuration**
   - Edit ID, Port, and IP in the top section
   - Click "Save Configuration" to persist changes

2. **Application Control**
   - **Green Button** = Application running → Click to **STOP**
   - **Red Button** = Application stopped → Click to **START**

3. **Automatic Monitoring**
   - Applications with `status="start"` are monitored every 2 seconds
   - Failed processes are automatically restarted
   - Button colors update in real-time

### Process States

| Status | Behavior | Button Color | Action |
|--------|----------|--------------|---------|
| **start** | Monitored & auto-restarted if failed | 🟢 Green (running) / 🔴 Red (stopped) | Automatic |
| **stop** | Not monitored, stays stopped | 🔴 Red | Manual only |

### Example Workflow

1. **Add Application** - Edit `config.xml` to add new application
2. **Set Status** - Set `status="start"` for automatic monitoring  
3. **Launch App** - Run Qt Process Monitor
4. **Monitor** - Watch real-time status updates
5. **Test Recovery** - Close application manually, watch auto-restart

---

## 🏗️ Architecture

### Class Structure

```
📁 Qt Process Monitor
├── 🔧 a_settingsclass    # XML configuration management
├── ⚙️ a_process          # Process monitoring & control  
├── 🖥️ mainwindow         # GUI interface
└── 🚀 main               # Application entry point
```

#### `a_settingsclass`
- **Purpose**: XML reading/writing operations
- **Features**: Parse configuration, manage settings, save changes
- **Methods**: `LoadConfiguration()`, `SaveConfiguration()`, `UpdateSettings()`

#### `a_process`  
- **Purpose**: Process monitoring and lifecycle management
- **Features**: 2-second monitoring, auto-restart, process detection
- **Methods**: `StartApplication()`, `StopApplication()`, `CheckProcesses()`

#### `mainwindow`
- **Purpose**: GUI interface and user interaction
- **Features**: Dark theme, dynamic buttons, real-time updates
- **Methods**: `UpdateApplicationButtons()`, `OnStartApplication()`

### Key Features

- **Signal-Slot Architecture** - Clean separation of concerns
- **Professional Error Handling** - Comprehensive logging and recovery
- **Memory Management** - Qt parent-child system prevents leaks
- **Cross-Platform Ready** - Designed for easy Windows/macOS porting

---

## 🔍 Troubleshooting

### Common Issues

#### ❌ **Application Won't Start Processes**
```bash
# Check if executable exists
ls -la /usr/bin/gnome-calculator

# Test manually
gnome-calculator &
pgrep -f gnome-calculator
pkill gnome-calculator
```

**Solutions:**
- Verify executable path in `config.xml`
- Check file permissions (`chmod +x`)
- Try alternative applications (`gedit`, `firefox`)

#### ❌ **XML Changes Not Saving**
```bash
# Check file permissions
ls -la config.xml
chmod 664 config.xml

# Check directory permissions  
ls -la .
```

**Solutions:**
- Ensure write permissions on config file
- Run from correct directory
- Check debug output for error messages

#### ❌ **Dark Theme Not Applied**
- Restart the application
- Check terminal for Qt stylesheet errors
- Verify Qt version compatibility

### Debug Information

**Enable verbose output:**
```bash
./qtprocessmonitor
```

**Debug messages include:**
- Configuration loading/saving status
- Process launch attempts and results
- XML file operations and errors
- Application status changes

### Manual Testing

**Test process detection:**
```bash
# Start application manually
gnome-calculator &

# Find process ID
pgrep -f gnome-calculator

# Kill process
pkill -f gnome-calculator
```

**Validate XML structure:**
```bash
# Check file contents
cat config.xml

# Validate XML syntax
xmllint --format config.xml
```

---

## 🚀 Development

### Code Standards

- **C++17** - Modern C++ features and best practices
- **Qt Conventions** - Signal-slot architecture, parent-child memory management  
- **Professional Documentation** - Every class, method, and variable documented
- **Error Handling** - Comprehensive logging and graceful error recovery

### Variable Naming Convention

- **Class Variables**: `CamelCase` (e.g., `MyVariable`)
- **Function Variables**: `_underscore_prefix` (e.g., `_myVariable`)
- **File Names**: `lowercase` (e.g., `mainwindow.h`)

### Adding New Features

1. **New Applications** - Simply add entries to XML configuration
2. **Custom Process Logic** - Modify `a_process` class methods
3. **GUI Enhancements** - Update `mainwindow` class  
4. **Configuration Options** - Extend `a_settingsclass` functionality

### Building for Development

```bash
# Debug build with verbose output
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
./bin/qtprocessmonitor
```

### Contributing

1. Follow existing code style and naming conventions
2. Add comprehensive documentation for new features
3. Include error handling and logging
4. Test on Ubuntu 20.04+ with Qt 6.0+

---

## 📁 Project Structure

```
qtprocessmonitor/
├── 📄 README.md                 # This documentation
├── ⚙️ CMakeLists.txt            # CMake build configuration
├── 📋 qtprocessmonitor.pro      # qmake build configuration  
├── 🔧 config.xml               # Sample configuration file
├── 🚀 main.cpp                 # Application entry point
├── 🖥️ mainwindow.h/.cpp        # GUI interface implementation
├── ⚙️ a_settingsclass.h/.cpp   # XML configuration management
├── 🔧 a_process.h/.cpp         # Process monitoring and control
├── 📁 build/                   # Build output directory
└── 📝 .gitignore               # Git ignore rules
```

---

## 🔗 Dependencies

### Runtime Dependencies
- Qt 6.0+ (Core, Widgets, Xml)
- Linux system utilities (`pgrep`, `pkill`, `pidof`)
- X11 display server (for GUI applications)

### Build Dependencies  
- CMake 3.16+ or qmake
- GCC 7.0+ or Clang 8.0+
- Qt development packages
- Standard build tools (`make`, etc.)

### Optional Dependencies
- `xmllint` - for XML validation
- `valgrind` - for memory debugging
- `gdb` - for debugging

---

## 📜 License

This application is provided for **educational and development purposes**.

### Usage Rights
- ✅ Use for learning Qt development
- ✅ Modify for educational projects  
- ✅ Use as template for similar applications
- ✅ Share with attribution

### Restrictions
- ❌ Commercial use without permission
- ❌ Distribution without source code
- ❌ Removal of attribution

---

## 📞 Support

### Getting Help

1. **Check Documentation** - Review this README thoroughly
2. **Debug Output** - Run from terminal to see detailed logs
3. **Test Manually** - Verify applications work outside the monitor
4. **Check Permissions** - Ensure file and executable permissions

### Reporting Issues

When reporting issues, please include:
- Ubuntu version and Qt version
- Full debug output from terminal
- Contents of `config.xml` file
- Steps to reproduce the problem

### Resources

- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Ubuntu Package Search](https://packages.ubuntu.com/)

---

<div align="center">

**Made with ❤️ using Qt and C++**

*Professional process monitoring for Linux systems*

</div>