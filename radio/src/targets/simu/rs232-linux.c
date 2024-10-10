/*
    Cross-platform serial / RS232 library
    Version 0.21, 11/10/2015
    -> LINUX and MacOS implementation
    -> rs232-linux.c

    The MIT License (MIT)

    Copyright (c) 2013-2015 Frédéric Meslin, Florent Touchard
    Email: fredericmeslin@hotmail.com
    Website: www.fredslab.net
    Twitter: @marzacdev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
  
*/

#if defined(__unix__) || defined(__unix) || \
    defined(__APPLE__) && defined(__MACH__)

#define _DARWIN_C_SOURCE

#include "rs232.h"

#include <unistd.h>
#define __USE_MISC // For CRTSCTS
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>

#define __USE_SVID // For strdup
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/
/** Base name for COM devices */
#if defined(__APPLE__) && defined(__MACH__)
    static const char * devBases[] = {
        "tty."
    };
    static int noBases = 1;
#else
    static const char * devBases[] = {
        "ttyACM", "ttyUSB", "rfcomm", "ttyS"
    };
    static int noBases = 4;
#endif

/*****************************************************************************/
typedef struct {
    char * port;
    int handle;
} COMDevice;

#define COM_MAXDEVICES        64
static COMDevice comDevices[COM_MAXDEVICES];
static int noDevices = 0;

/*****************************************************************************/
/** Private functions */
void _AppendDevices(const char * base);
int _BaudFlag(int BaudRate);

/*****************************************************************************/
int comEnumerate()
{
    for (int i = 0; i < noDevices; i++) {
        if (comDevices[i].port) free(comDevices[i].port);
        comDevices[i].port = NULL;
    }
    noDevices = 0;
    for (int i = 0; i < noBases; i++)
        _AppendDevices(devBases[i]);
    return noDevices;
}

void comTerminate()
{
    comCloseAll();
    for (int i = 0; i < noDevices; i++) {
        if (comDevices[i].port) free(comDevices[i].port);
        comDevices[i].port = NULL;
    }
}

int comGetNoPorts()
{
    return noDevices;
}

/*****************************************************************************/
int comFindPort(const char * name)
{
    int p;
    for (p = 0; p < noDevices; p++)
        if (strcmp(name, comDevices[p].port) == 0)
            return p;
    return -1;
}

const char * comGetInternalName(int index)
{
    #define COM_MAXNAME    128
    static char name[COM_MAXNAME];
    if (index >= noDevices || index < 0)
        return NULL;
    sprintf(name, "/dev/%s", comDevices[index].port);
    return name;
}

const char * comGetPortName(int index) {
    if (index >= noDevices || index < 0)
        return NULL;
    return comDevices[index].port;
}

/*****************************************************************************/
int comOpen(int index, int baudrate)
{
    if (index >= noDevices || index < 0)
        return 0;
// Close if already open
    COMDevice * com = &comDevices[index];
    if (com->handle >= 0) comClose(index);
// Open port
    printf("Try %s \n", comGetInternalName(index));
    int handle = open(comGetInternalName(index), O_RDWR | O_NOCTTY | O_NDELAY);
    if (handle < 0)
        return 0;
    printf("Open %s \n", comGetInternalName(index));
// General configuration
    struct termios config;
    memset(&config, 0, sizeof(config));
    tcgetattr(handle, &config);
    config.c_iflag &= ~(INLCR | ICRNL);
    config.c_iflag |= IGNPAR | IGNBRK;
    config.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    config.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    config.c_cflag |= CLOCAL | CREAD | CS8;
    config.c_lflag &= ~(ICANON | ISIG | ECHO);
    int flag = _BaudFlag(baudrate);
    cfsetospeed(&config, flag);
    cfsetispeed(&config, flag);
// Timeouts configuration
    config.c_cc[VTIME] = 1;
    config.c_cc[VMIN]  = 0;
    //fcntl(handle, F_SETFL, FNDELAY);
// Validate configuration
    if (tcsetattr(handle, TCSANOW, &config) < 0) {
        close(handle);
        return 0;
    }
    com->handle = handle;
    return 1;
}

void comClose(int index)
{
    if (index >= noDevices || index < 0)
        return;
    COMDevice * com = &comDevices[index];
    if (com->handle < 0) 
        return;
    tcdrain(com->handle);
    close(com->handle);
    com->handle = -1;
}

void comCloseAll()
{
    for (int i = 0; i < noDevices; i++)
        comClose(i);
}

/*****************************************************************************/
int comWrite(int index, const char * buffer, size_t len)
{
    if (index >= noDevices || index < 0)
        return 0;
    if (comDevices[index].handle <= 0)
        return 0;
    int res = write(comDevices[index].handle, buffer, len);
    if (res < 0)
        res = 0;
    return res;
}

int comRead(int index, char * buffer, size_t len)
{
    if (index >= noDevices || index < 0)
        return 0;
    if (comDevices[index].handle <= 0)
        return 0;
    int res = read(comDevices[index].handle, buffer, len);
    if (res < 0)
        res = 0;
    return res;
}

/*****************************************************************************/
int _BaudFlag(int BaudRate)
{
    switch(BaudRate)
    {
        case 50:      return B50; break;
        case 110:     return B110; break;
        case 134:     return B134; break;
        case 150:     return B150; break;
        case 200:     return B200; break;
        case 300:     return B300; break;
        case 600:     return B600; break;
        case 1200:    return B1200; break;
        case 1800:    return B1800; break;
        case 2400:    return B2400; break;
        case 4800:    return B4800; break;
        case 9600:    return B9600; break;
        case 19200:   return B19200; break;
        case 38400:   return B38400; break;
        case 57600:   return B57600; break;
        case 115200:  return B115200; break;
        case 230400:  return B230400; break;
        default : return B0; break;
    }
}

void _AppendDevices(const char * base)
{
    int baseLen = strlen(base);
    struct dirent * dp;
// Enumerate devices
    DIR * dirp = opendir("/dev");
    while ((dp = readdir(dirp)) && noDevices < COM_MAXDEVICES) {
        if (strlen(dp->d_name) >= baseLen) {
            if (memcmp(base, dp->d_name, baseLen) == 0) {
                COMDevice * com = &comDevices[noDevices ++];
                com->port = (char *) strdup(dp->d_name);
                com->handle = -1;
            }
        }
    }
    closedir(dirp);
}

#endif // unix
