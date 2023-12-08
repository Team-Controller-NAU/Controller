#pragma unmanaged
/*
Implementation file for SerialCom.h
> Team Controller 2023
*/
#include "SerialCom.h"

// initialization constructor opens both ports, and assigns port names
SerialCom::SerialCom(std::string name1, std::string name2) : controllerPort(ioservice, name1), laptopPort(ioservice, name2), portName1(name1), portName2(name2)
{
	// set baud rate and other settings
	controllerPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
	laptopPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
}

// closes both ports safely
void SerialCom::closePorts()
{
	controllerPort.close();
	laptopPort.close();
}

// writes to first port with specified message
void SerialCom::write(std::string &message)
{
	// write
	boost::asio::write(controllerPort, boost::asio::buffer(message));

	// set sent message
	sentMessage = message;
}

// reads from second port
void SerialCom::read()
{
	// initialize buffer type boost likes
	boost::asio::streambuf receivedBuffer;

	// read until endline character
	boost::asio::read_until(laptopPort, receivedBuffer, '\n');

	// convert buffer message to readable string, set received message
	std::istream inputStream(&receivedBuffer);
	std::getline(inputStream, receivedMessage);
}

// getter for sent message
std::string SerialCom::getSent()
{
	return sentMessage;
}

// getter for received message
std::string SerialCom::getReceived()
{
	return receivedMessage;
}

// function to be ran when click event handler is triggered
// creates serialcom object, sends random string, receives random string
std::string* doDemo()
{
	// initialize serial connection
	std::string* output = new std::string[2];
	SerialCom serialCommunication("COM4", "COM5");
	std::string message = "8#$fGGFAFCVu2b!  _~349ru8\n";

	// write to COM4
	serialCommunication.write(message);

	// read from COM5
	serialCommunication.read();

	// close ports
	serialCommunication.closePorts();

	// set output, return
	output[0] = serialCommunication.getSent();
	output[1] = serialCommunication.getReceived();
	return output;
}
