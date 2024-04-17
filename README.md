


# Weapon System Support Software

Weapon System Support Software is a downloadable application which is designed to receive data from Northrop Grumman weapon controllers and display the data in an easy to use GUI. Team Controller is a senior Computer Science capstone group for the Fall 2023 and Spring 2024 semesters from Northern Arizona University. For more information about the project and our team members, visit our [website](https://ceias.nau.edu/capstone/projects/CS/2024/TeamController_F23/)!

## Description

This application was designed to be easy to deploy and use. Simply download the installer for your platform **(Linux and Windows options are available)** and follow the given installation guide to deploy the program. Launch your program, select the desired serial port and click the connect button to attempt a handshake with the controller. After a connection occurs, data will be sent from the controller and populated into the GUI. Additionally log files will be managed automatically and updated upon the arrival of any new information. You may open the directory containing these log files from the Events page to view the events and errors of past sessions and even load this data back into the GUI for analysis (filtering and color coding will be available).

Overall, this project was designed to make data extraction and analysis during diagnostic testing easier for Northrop Grumman and its customers. **No technical expertise is required to be able to use this application.** A user release version is provided as well as a developer version for Northrop Grumman software engineers to easily update the app. Both of these versions come from the same source code files. To further aid Northrop Grumman software engineers, **check our project docs folder and Developer Guide for extensive documentation regarding the project and how to work with QT.**

## Getting Started

### Installing

Windows and Linux installers for both the developer version as well as the user version are provided directly in this repository. Be sure to reference the **Installation guide** if you are in need of any assistance. 

### Software Dependencies

* This project was developed within the QT Framework. It is highly dependent on this environment for libraries and data types as well as testing infrastructure. The free community edition of QT can be installed [here.](https://www.qt.io/download-qt-installer "https://www.qt.io/download-qt-installer") Be sure to select QT serial port and serial bus as additional libraries to be included in your QT installation.
* At least 1 DB-9 serial port is required to properly interface with the weapon controller. For testing purposes our team used virtualized serial ports to simulate serial communication. You may install Com0Com and set up virtual serial ports from [here.](https://sourceforge.net/projects/com0com/files/com0com/2.2.2.0/com0com-2.2.2.0-x64-fre-signed.zip/download)

## Help

Please reference the **Developer Guide** for comprehensive implementation information. Additional documentation can be found in the project-docs directory. The source code contains comments to document the purpose of each functional line/block of code.

## Authors

* [Zachary Parham](https://github.com/zjp292) - Team Lead
* [Bradley Essegian](https://github.com/bradd07)
* [Brandon Udall](https://github.com/bcu8) 
* [Dylan Motz](https://github.com/Dylan-Motz)

## Version History

### v6.0.0-alpha
	- Various bug fixes and optimizations
 	- GUI enhancements
	 	- Revamped Status page
	 	- Styling for buttons on mouse hover
	 	- Custom assets for drop down menus
	 	- Fixed zoom related display issues
	 	- Refresh serial ports button
	 	- Search button on events page
	 - Advanced log file optional setting
	 - Notify on error cleared optional setting
	 - Added clear error functionality for log files
	 - Enhanced unit testing
	 - Documentation
		 - Developer Guide
		 - Installation Guide
		 - Updated old diagrams
### v5.0.0-alpha
	- Various bug fixes and optimizations
	- Notifications page
	    - Notification pop up feature
	- Session Timeout feature
	- Polishing serial communication
	- GUI enhancements
	- Status page rework
	- Continued testing
### v4.0.0-alpha
	- Various bug fixes and optimizations
	- Added Settings page
	- GUI enhancements
	- Testing started
	- Physical wires testing
	- Added additional information to Status page
	- Added DEV_MODE compilation option to toggle between compiling for user and dev versions.
### v3.0.0-alpha
    - Various bug fixes and optimizations
    - Events page search feature
    - Tool tips
    - Dynamic electrical page
    - GUI enhancements
    - Controller session timer
    - Added feed position graphic
### v2.0.0-alpha
    - Various bug fixes and optimizations
    - Initial settings added to constants.h
    - GUI enhancements
    - Added autosave feature
    - Settings save cross session
### v1.0.0-alpha
	- Added dev page
    - Manual clear error
    - CSim
    - Custom message
    - Output CSim messages button
	- Added Connection page
        - Connection setting selection
        - Connect button
	- Added Events page
        - Log file folder selection
        - Manual log file download
        - Filtering options
	- Added Status page
	- Added Electrical page

## License

This project is licensed under the [NAME HERE] License - see the LICENSE.md file for details

## Acknowledgments

* [Northrop Grumman](https://www.northropgrumman.com/) - Project sponsor
* [Harlan Mitchell](https://www.linkedin.com/in/harlan-mitchell/) - Client
* [Laurel Enstrom](https://www.linkedin.com/in/laurel-enstrom-6a7ab4107/) - Client
* Italo Santos - Semester 1 Mentor
* Tayyaba Shaheen - Semester 2 Mentor
