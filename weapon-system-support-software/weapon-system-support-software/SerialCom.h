/*
SerialCom class that handles all serial communication between two
devices. Must be initialized with two port names. Both ports will be
opened upon initialization.
> Team Controller 2023
*/
#pragma once
#include <boost/asio.hpp>
#include <iostream>

// function declarations
std::string* doDemo();

class SerialCom
{
private:
	// member variables
	boost::asio::io_service ioservice;
	boost::asio::serial_port controllerPort;
	boost::asio::serial_port laptopPort;
	std::string portName1, portName2, receivedMessage, sentMessage;

public:
	// initialization constructor
	SerialCom(std::string name1, std::string name2);

	// function declarations
	void closePorts();
	void write(std::string &message);
	void read();
	std::string getSent();
	std::string getReceived();
};
