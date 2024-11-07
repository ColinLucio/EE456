#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <sstream>
#include <Radio.Lib.h>
#include "PiHal.h"
#include <chrono>

PiHal* hal = new PiHal(0);

SX1262 radio = new Module(hal, 29, 27, 1, 28);



void parseNMEA(const std::string &line) {
    if (line.rfind("$GNGGA", 0) == 0) {  // Check if line starts with $GNGGA
        std::istringstream ss(line);
        std::string token;
        int index = 0;
        std::string latitude, longitude, lat_dir, lon_dir;

        while (std::getline(ss, token, ',')) {
            if (index == 2) latitude = token;        // Latitude
            else if (index == 3) lat_dir = token;    // Latitude Direction
            else if (index == 4) longitude = token;  // Longitude
            else if (index == 5) lon_dir = token;    // Longitude Direction
            index++;
        }
        std::cout << "Latitude: " << latitude << " " << lat_dir << ", Longitude: " << longitude << " " << lon_dir << std::endl;
    }
}

int main() {
    int serial_port = open("/dev/serial0", O_RDWR | O_NOCTTY);

    if (serial_port < 0) {
        std::cerr << "Error opening serial port\n";
        return 1;
    }

    printf("[SX1262] Initializing... ");
    int state = radio.begin(915.0, 125.0, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0.0, false);
    if (state != RADIOLIB_ERR_NONE) {
        printf("Initialization failed, code %d\n", state);
        return 1;
    }

    printf("Initialization success!\n");
    int count = 0;

    struct termios tty;
    tcgetattr(serial_port, &tty);
    tty.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    tty.c_iflag = IGNPAR;
    tcsetattr(serial_port, TCSANOW, &tty);

    char buf[256];
    while (true) {
        int n = read(serial_port, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            std::string line(buf);
            
            parseNMEA(line);
            //std::cout << line << std::endl; // comment this out when you uncomment parseNMEA function above
        }
        char str[64];
        sprint(str, parseNMEA(line));

        state = radio.transmit(str);
        if (state == RADIOLIB_ERR_NONE) {
            printf("success!\n");
        }
        hal->dleay(1000);

    }

    close(serial_port);
    return 0;
}
