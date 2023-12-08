#pragma once
#include <boost/asio.hpp>
#include <iostream>

int index = 0;
std::string* output = new std::string[2];

class SerialCom
{
private:
	// member variables
	boost::asio::io_service ioservice;
	boost::asio::serial_port controllerPort;
	boost::asio::serial_port laptopPort;
	std::string portName1, portName2;

public:

	// initialization constructor opens both ports, and assigns port names
	SerialCom(std::string name1, std::string name2) : controllerPort(ioservice, name1), laptopPort(ioservice, name2), portName1(name1), portName2(name2)
	{
		controllerPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
		laptopPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
	}

	// closes both ports
	void closePorts()
	{
		controllerPort.close();
		laptopPort.close();
	}

	// writes to specified port with specified message
	void write(std::string& message)
	{
		boost::asio::write(controllerPort, boost::asio::buffer(message));
	}

	// reads from port with specified string to put received message into
	void read(std::string* receivedMessage)
	{
		boost::asio::streambuf receivedBuffer;
		boost::asio::read_until(laptopPort, receivedBuffer, '\n');

		std::istream inputStream(&receivedBuffer);
		std::getline(inputStream, *receivedMessage);
	}

};

void doClick()
{
	SerialCom serialCommunication("COM4", "COM5");
	std::string sentMessage = "8#$fGGFAFCVu2b!  _~349ru8\n";
	std::string received;

	serialCommunication.write(sentMessage);
	serialCommunication.read(&received);
	serialCommunication.closePorts();

	output[0] = sentMessage;
	output[1] = received;
}
