#ifndef _COMM_UTILS_H
#define _COMM_UTILS_H
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <string>
#include <iostream>

double millis() {
  timespec timer;
  clock_gettime(CLOCK_MONOTONIC, &timer);
  return timer.tv_nsec / 1E6 + timer.tv_sec * 1000.0;
}

struct ArduinoValues {
  ArduinoValues(int fn = 0, int lm = 0, int rm = 0)
      : frameNumber(fn), leftMotor(lm), rightMotor(rm) {}
  int frameNumber;
  int leftMotor;
  int rightMotor;
};

class  SerialComm {
public:
  SerialComm(int baudRate = 57600, int dataBits = 8,
         int parity = 0, int stopBits = 0, int bufferSize = 1000)
      : dev(dev), baud(baudRate), dataBits(dataBits), parity(parity),
        stopBits(stopBits), bufferSize(bufferSize), bufferIndex(0),
        blocking(false) {}

  ~SerialComm() { closePort(); }

  bool openPort(const std::string &dev) {
    printf("SERIAL: Opening %s at %i baud...", dev.c_str(), baud);
    fd = open(dev.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd != -1) {
      printf("OK\n");
      if (blocking == 1)
        printf("SERIAL: Blocking enabled\n");
      else
        printf("SERIAL: Blocking disabled\n");
      fcntl(fd, F_SETFL, FNDELAY);
      struct termios options;
      tcgetattr(fd, &options);
      switch (baud) {
      case 4800:
        cfsetispeed(&options, B4800);
        cfsetospeed(&options, B4800);
        break;
      case 9600:
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        break;
      case 38400:
        cfsetispeed(&options, B38400);
        cfsetospeed(&options, B38400);
        break;
      case 57600:
        cfsetispeed(&options, B57600);
        cfsetospeed(&options, B57600);
        break;
      case 115200:
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        break;
      default:
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        break;
      }
      options.c_cflag |= (CLOCAL | CREAD);
      switch (parity) {
      case 0:
        options.c_cflag &= ~PARENB;
        printf("SERIAL: Parity NONE\n");
        break;
      case 1:
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        printf("SERIAL: Parity EVEN\n");
        break;
      case 2:
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
        printf("SERIAL: Parity ODD\n");
        break;
      }
      switch (dataBits) {
      case 7:
        options.c_cflag |= CS7;
        printf("SERIAL: Databits set to 7\n");
        break;
      case 8:
        options.c_cflag |= CS8;
        printf("SERIAL: Databits set to 8\n");
        break;
      default:
        options.c_cflag |= CS8;
        printf("SERIAL: Databits not set!\n");
        break;
      }
      if (!stopBits) {
        options.c_cflag &= ~CSTOPB;
        printf("SERIAL: STOP 1 bit\n");

      } else {
        options.c_cflag |= CSTOPB;
        printf("SERIAL: STOP 2 bits!\n");
      }
      options.c_cflag &= ~CRTSCTS;
      options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
      tcsetattr(fd, TCSANOW, &options);
    } else {
      printf("FAIL\n");
      perror("Error opening port:");
      closePort();
      return false;
    }
    return true;
  }

  void flushPort() {
    if (fd != -1)
      ioctl(fd, TCFLSH, 2);
  }

  int getData(char *data, int size) {
    if (fd != -1) {
      return read(fd, data, size);
    } else
      return -1;
  }
  
  int writeData(char *data, int size) {
    if (fd != -1) {
      return write(fd, data, size);
    } else
      return -1;
  }
  


  void closePort() {
    if (fd != -1) {
      close(fd);
      printf("SERIAL: Device is now closed.\n");
    }
  }

private:
  int fd;
  std::string dev;
  int baud;
  int dataBits;
  int parity;
  int stopBits;
  int bufferSize;
  int bufferIndex;
  bool blocking;
};

#endif