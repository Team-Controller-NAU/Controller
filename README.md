
# Weapon System Support Software

Weapon System Support Software is a downloadable exe program which is designed to receive data from Northrop Grumman weapon controllers and display the data in an easy to use GUI setup. Team Controller is a senior Computer Science capstone group for the Fall 2023 and Spring 2024 semesters from Northern Arizona University. For more information about the project and our team members, visit our [website](https://ceias.nau.edu/capstone/projects/CS/2024/TeamController_F23/)!

## Description

This application was designed to be easy to deploy and use. Simply download the installer for your platform (Linux or Windows) and follow the given installation guide to deploy the program. Launch your program, select the desired serial port and click the connect button to attempt a handshake with the connected controller. After a connection occurs, data will be sent from the controller and populated into the GUI pages for your reference. Additionally log files will be managed automatically and updated upon the arrival of any new information. You may open the directory containing these log files from the Events page to view the events and errors of past sessions and even load this data back into the GUI for easy filtering.

Overall, this project was designed to make data extraction and analysis during diagnostic testing easier for Northrop Grumman and its customers. ** No technical expertise is required to be able to use this application. ** A user release version is provided as well as a developer version for Northrop Grumman software engineers to easily update the app. Both of these versions come from the same source code files. To further aid Northrop Grumman software engineers, see our project docs folder for extensive documentation regarding the project and how to work with QT.

## Getting Started

### Dependencies (For Software engineers)

* This software was developed within the QT Framework. It is highly dependent on this environment for libraries and data types as well as testing infrastructure. The free community edition of QT can be installed [here.](https://www.qt.io/download-qt-installer "https://www.qt.io/download-qt-installer") Be sure to select QT serial port and serial bus as additional libraries to be included in your QT installation.
* At least 1 DB-9 serial port is required to properly interface with the weapon controller. For testing purposes our team used virtualized serial ports to simulate serial communication. You may install Com0Com and set up virtual serial ports from [here.](https://sourceforge.net/projects/com0com/files/com0com/2.2.2.0/com0com-2.2.2.0-x64-fre-signed.zip/download)
* A comprehensive developers guide will be provided which will quickly introduce new developers to the framework. Beyond the developers guide, the code has clear documentation and standards which will assist developers in navigating and altering the source code.

### Installing

Windows and Linux installers for both the developer version as well as the user version are provided directly in this repository. Be sure to reference the installation guide if you are in need of any assistance. 

## Help

Additional documentation can be found in the project-docs directory. 

## Authors

* [Zachary Parham](https://github.com/zjp292) 
* [Bradley Essegian](https://github.com/bradd07) 
* [Brandon Udall](https://github.com/bcu8) 
* [Dylan Motz](https://github.com/Dylan-Motz)

## Version History

* 0.2
    * Various bug fixes and optimizations
    * See [commit change]() or See [release history]()
* 0.1
    * Initial Release

## License

This project is licensed under the [NAME HERE] License - see the LICENSE.md file for details

## Acknowledgments

* [Northrop Grumman](https://www.northropgrumman.com/) - Project sponsor
* [Harlan Mitchell](https://www.linkedin.com/in/harlan-mitchell/) - Client & Mentor
* [Laurel Enstrom](https://www.linkedin.com/in/laurel-enstrom-6a7ab4107/) - Client & Mentor
