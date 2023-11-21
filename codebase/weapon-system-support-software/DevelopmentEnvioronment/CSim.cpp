#include "ComUtility.h"

/*
CSim = basic command line utility
it is used by developers, so minimal input check is necessary

will need a supporting header file to define a library of events, errors and parameters to be 
chosen from during random generation

Upon startup...
    *launch connection and wait for handshake
    prompt: "\n\nWelcome to CSim\n"
            "-------------------\n"
            "This is a testing tool for the corresponding data display module, which must be run in parallel.\n"
            "For instructions on setting up a serial testing environment please visit *insert website link which has tutorial or maybe pdf included in installation?*"
            "Purpose: generate status information, and events/errors to send via RS-422 serial communication to\n"
            "         the data display module.\n\n"
            "This program will attempt to connect to the data display module via\n"
            *show user connection settings*

            "Program Start:\n--------------\n
            "Input options: 
                1 to start random generation
                2 to change random generation rate
                3 to input custom event
                4 to input custom error"
            
            "waiting for handshake..."
       then     
            "connection established"

user responds 1:
    prompt "press q at any point to stop data generation"
           "starting in 2 seconds..."

logs activity to screen, examples:

     -new event "xyz 1 2 3"
     -new status generated:
        *log generated data
     -Data display module disconnected!

other than option 1, simply prompt for input then return to main menu use do while loops for inputs
each time the user enters a new menu add spacing with a few \n then print the menu title before printing menu contents


*/
using namespace std;

void readHandler(const boost::system::error_code& error, std::size_t bytes_transferred, boost::asio::serial_port& serial) {
    if (!error) {
        std::istream is(&serial);
        std::string message;
        std::getline(is, message);
        std::cout << "Received: " << message << std::endl;

        // Start a new read operation
        boost::asio::async_read_until(serial, boost::asio::dynamic_buffer(message), '\n', 
            std::bind(readHandler, std::placeholders::_1, std::placeholders::_2, std::ref(serial)));
    } else {
        std::cerr << "Error reading from serial port: " << error.message() << std::endl;
    }
}

int main() {
    boost::asio::io_context io_context;
    boost::asio::serial_port serial(io_context);

    // Open the serial port (replace "COM1" with your specific port on Windows, or "/dev/ttyUSB0" on Linux)
    serial.open("/dev/ttyUSB0"); // Replace with your specific port
    serial.set_option(boost::asio::serial_port_base::baud_rate(9600));

    // Start an asynchronous read operation
    boost::asio::async_read_until(serial, boost::asio::dynamic_buffer(std::string()), '\n', 
        std::bind(readHandler, std::placeholders::_1, std::placeholders::_2, std::ref(serial)));

    io_context.run();

    return 0;
}