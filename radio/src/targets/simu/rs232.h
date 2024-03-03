/*
    Cross-platform serial / RS232 library
    Version 0.21, 11/10/2015
    -> All platforms header
    -> rs232.h

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

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef RS232_H
#define RS232_H

#ifdef __cplusplus
extern "C" {
#endif

    #include <stdint.h>
    #include <stdlib.h>

/*****************************************************************************/
    /* Doxywizard specific */
    /**
    * \mainpage RS232
    * \section intro_sec C / C++ RS232 cross-platform serial library
    * <b>Version 0.21 - 11/10/2015</b>
    *
    * Supported platforms:  
    * - Windows (XP / Win7, possibly 8 and 10)  
    * - Linux  
    * - MacOS X  
    *
    * Copyright (c) 2013-2015 Fr&eacute;d&eacute;ric Meslin, Florent Touchard <br>
    * Email: fredericmeslin@hotmail.com <br>
    * Website: www.fredslab.net <br>
    * Twitter: \@marzacdev <br>
    */
    
/*****************************************************************************/
    /**
     * \fn int comEnumerate()
     * \brief Enumerate available serial ports (Serial, USB serial, Bluetooth serial)
     * \return number of enumerated ports
     */
    int comEnumerate();
    
    /**
     * \fn int comGetNoPorts()
     * \brief Return the number of enumerated ports
     * \return number of enumerated ports
     */
    int comGetNoPorts();

    /**
     * \fn int comTerminate()
     * \brief Release ports and memory resources used by the library
     */    
    void comTerminate();

    /**
     * \fn const char * comGetPortName(int index)
     * \brief Get port user-friendly name
     * \param[in] index port index
     * \return null terminated port name
     */    
    const char * comGetPortName(int index);

    /**
     * \fn const char * comGetInternalName(int index)
     * \brief Get port operating-system name
     * \param[in] index port index
     * \return null terminated port name
     */        
    const char * comGetInternalName(int index);
    
    /**
     * \fn int comFindPort(const char * name)
     * \brief Try to find a port given its user-friendly name
     * \param[in] name port name (case sensitive)
     * \return index of found port or -1 if not enumerated
     */        
    int comFindPort(const char * name);

/*****************************************************************************/
    /**
     * \fn int comOpen(int index, int baudrate)
     * \brief Try to open a port at a specific baudrate
     * \brief (No parity, single stop bit, no hardware flow control)
     * \param[in] index port index
     * \param[in] baudrate port baudrate
     * \return 1 if opened, 0 if not available
     */        
    int comOpen(int index, int baudrate);
    
    /**
     * \fn void comClose(int index)
     * \brief Close an opened port
     * \param[in] index port index
     */            
    void comClose(int index);
    
    /**
     * \fn void comCloseAll()
     * \brief Close all opened ports
     */            
    void comCloseAll();

/*****************************************************************************/
    /**
     * \fn int comWrite(int index, const char * buffer, size_t len)
     * \brief Write data to the port (non-blocking)
     * \param[in] index port index
     * \param[in] buffer pointer to transmit buffer
     * \param[in] len length of transmit buffer in bytes
     * \return number of bytes transferred
     */            
    int comWrite(int index, const char * buffer, size_t len);
    
    /**
     * \fn int comRead(int index, const char * buffer, size_t len)
     * \brief Read data from the port (non-blocking)
     * \param[in] index port index
     * \param[in] buffer pointer to receive buffer
     * \param[in] len length of receive buffer in bytes
     * \return number of bytes transferred
     */                
    int comRead(int index, char * buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif // RS232_H
