/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the SystemView and RTT protocol, and J-Link.       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: 3.60e                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_embOS_Win32.c
Purpose : Sample setup configuration of SystemView with embOS.
Revision: $Rev: 28344 $
*/
#include "RTOS.h"
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "SEGGER_SYSVIEW_embOS.h"
#include "SEGGER_SYSVIEW_Win32.h"
#include "SEGGER_RTT.h"

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <time.h>
#include <winsock2.h>

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#ifndef   SYSVIEW_APP_NAME
  #define SYSVIEW_APP_NAME        "embOS start project"
#endif

// The target device name
#ifndef   SYSVIEW_DEVICE_NAME
  #define SYSVIEW_DEVICE_NAME     "Simulation"
#endif

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_Conf.h
#ifndef   SYSVIEW_TIMESTAMP_FREQ
  #define SYSVIEW_TIMESTAMP_FREQ  (1000u)
#endif

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#ifndef   SYSVIEW_CPU_FREQ
  #define SYSVIEW_CPU_FREQ        (1000000u)
#endif

// Define as 1 to immediately start recording after initialization to catch system initialization.
#ifndef   SYSVIEW_START_ON_INIT
  #define SYSVIEW_START_ON_INIT   0
#endif

#ifndef   MAX_ISRNAMES_LENGTH
  #define MAX_ISRNAMES_LENGTH     400
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define SYSVIEW_COMM_APP_HELLO_SIZE     32
#define SYSVIEW_COMM_TARGET_HELLO_SIZE  32


#ifdef WIN32
  #define _SYS_THREAD_PROC_EX_TYPE U32 __stdcall
  #define _SYS_THREAD_PROC_EX_R_TYPE U32
#else
  #define _SYS_THREAD_PROC_EX_TYPE void*
  #define _SYS_THREAD_PROC_EX_R_TYPE void*
#endif

#define _SYS_THREAD_CREATE_SUSPENDED     (1)

#define _SYS_SOCKET_INVALID_HANDLE       (-1)
#define _SYS_SOCKET_IP_ADDR_ANY          0
#define _SYS_SOCKET_IP_ADDR_LOCALHOST    0x7F000001                  // 127.0.0.1 (localhost)

#define _SYS_SOCKET_PORT_ANY             0

#define _SYS_SOCKET_ERR_UNSPECIFIED      -1
#define _SYS_SOCKET_ERR_WOULDBLOCK       -2
#define _SYS_SOCKET_ERR_TIMEDOUT         -3
#define _SYS_SOCKET_ERR_CONNRESET        -4

#define _SYS_SOCKET_SHUT_RD              0
#define _SYS_SOCKET_SHUT_WR              1
#define _SYS_SOCKET_SHUT_RDWR            2

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/
typedef void* _SYS_HANDLE;
typedef int _SYS_SOCKET_HANDLE;
typedef _SYS_THREAD_PROC_EX_TYPE _SYS_THREAD_PROC_EX(void* pPara);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/// "Hello" message expected by SystemView App: SEGGER SystemView VM.mm.rr
static const U8          _abHelloMsg[SYSVIEW_COMM_TARGET_HELLO_SIZE] = { 'S', 'E', 'G', 'G', 'E', 'R', ' ', 'S', 'y', 's', 't', 'e', 'm', 'V', 'i', 'e', 'w', ' ', 'V', '0' + SEGGER_SYSVIEW_MAJOR, '.', '0' + (SEGGER_SYSVIEW_MINOR / 10), '0' + (SEGGER_SYSVIEW_MINOR % 10), '.', '0' + (SEGGER_SYSVIEW_REV / 10), '0' + (SEGGER_SYSVIEW_REV % 10), '\0', 0, 0, 0, 0, 0 };
static volatile int      _CloseRequested;            // Indicator for threads to terminate themselves
static volatile int      _SysViewCommThreadRunning;  // Indicator for status of the "SysView communication" thread

static int               _int1   = 1;

static U64               _TSFreq;
static U32               _TSDiv;

static CRITICAL_SECTION  _csLockString;
static char              _sISRNames[MAX_ISRNAMES_LENGTH];

/*********************************************************************
*
*       Local functions, SYS
*
**********************************************************************
*/

/*********************************************************************
*
*       SYS_Sleep
*/
static void _SYS_Sleep(int ms) {
  Sleep(ms);
}

/*********************************************************************
*
*       SYS_GetLastError
*/
static U32 _SYS_GetLastError(void) {
  return GetLastError();
}

/*********************************************************************
*
*       SYS_CreateThreadEx
*/
static _SYS_HANDLE _SYS_CreateThreadEx(_SYS_THREAD_PROC_EX* pfThreadProc, void* pPara, U64* pThreadId, const char* sName, U32 Flags) {
  _SYS_HANDLE hThread;
  U32 ThreadId;
  U32 CreateFlags;

  CreateFlags = 0;
  if (Flags & _SYS_THREAD_CREATE_SUSPENDED) {
    CreateFlags = CREATE_SUSPENDED;
  }
  hThread = (_SYS_HANDLE)CreateThread(NULL, 0, pfThreadProc, pPara, CreateFlags, &ThreadId);
  if (sName != NULL) {
    OS_SIM_SetThreadName(ThreadId, sName);
  }
  if (hThread) {
    if (pThreadId) {
      *pThreadId = ThreadId;
    }
  }
  return hThread;
}

/*********************************************************************
*
*       _WSAStartup
*
*  Function description
*    Initializes Winsock API. Needs to be called once before using any socket API.
*    May be called multiple times.
*/
static void _WSAStartup(void) {
  WORD    wVersionRequested;
  WSADATA wsaData;
  //
  // Init Winsock API
  //
  wVersionRequested = MAKEWORD(2, 2);
  WSAStartup(wVersionRequested, &wsaData);
}

/*********************************************************************
*
*       _WSACleanup
*
*  Function description
*    Cleans up Winsock API. WSACleanup() needs to be called as many times
*    as WSAStartup() if socket API is no longer needed.
*/
static void _WSACleanup(void) {
  WSACleanup();
}

/*********************************************************************
*
*       _SYS_SOCKET_OpenTCP
*
*  Function description
*    Creates an IPv4 TCP socket.
*
*  Return value
*    Handle to socket
*/
static _SYS_SOCKET_HANDLE _SYS_SOCKET_OpenTCP(void) {
  SOCKET sock;
  //
  // Init Winsock API as this is the first socket-related function being called
  // WSACleanup is called on SocketClose()
  //
  _WSAStartup();
  //
  // Create socket
  //
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock != INVALID_SOCKET) {
    //
    // Disable Nagle's algorithm to speed things up
    // Nagle's algorithm prevents small packets from being transmitted and collects some time until data is actually sent out
    //
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&_int1, sizeof(int));
  } else {
    return _SYS_SOCKET_INVALID_HANDLE;
  }
  return (_SYS_SOCKET_HANDLE)sock;
}

/*********************************************************************
*
*       _SYS_SOCKET_Close
*
*  Function description
*    Closes a socket. Resources allocated by this socket are freed.
*
*  Parameters
*    hSocket  Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*/
static void _SYS_SOCKET_Close(_SYS_SOCKET_HANDLE hSocket) {
  SOCKET Sock;
  int OptVal;
  int OptLen;
  //
  // MSDN: A socket that is using the SO_EXCLUSIVEADDRUSE option must be shut down properly prior to closing it. Failure to do so can cause a denial of service attack if the associated service needs to restart.
  //
  Sock = (SOCKET)hSocket;
  OptLen = 4;
  OptVal = 0;  // Make sure it is zero initialized in case getsockopt does not fill it completely
  getsockopt(Sock, SOL_SOCKET, -5, (char*)&OptVal, &OptLen);  // SO_EXCLUSIVEADDRUSE (Define not known in the VC6 headers, we use...)
  if (OptVal) {
    shutdown(Sock, SD_BOTH);
  }
  //
  // Close socket
  //
  closesocket(Sock);
  //
  // De-init Winsock API. Needs to be called as often as WSAStartup() has been called
  // Has an internal reference counter
  // If the counter reaches 0, all sockets opened by the process, are forced closed
  //
  _WSACleanup();
}

/*********************************************************************
*
*       _SYS_SOCKET_ListenAtTCPAddr
*
*  Function description
*    Puts IPv4 socket into listening state.
*
*  Parameters
*    hSocket  Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*    IPAddr   IPv4 address expected in little endian form, meaning 127.0.0.1 is expected as 0x7F000001
*             To accept connections from any IP address, pass _SYS_SOCKET_IP_ADDR_ANY
*    Port     Port to listen at
*
*  Return value
*    >= 0: O.K.
*     < 0: Error
*/
static int _SYS_SOCKET_ListenAtTCPAddr(_SYS_SOCKET_HANDLE hSocket, U32 IPAddr, U32 Port, unsigned NumConnectionsQueued) {
  struct sockaddr_in addr;
  SOCKET Sock;
  int r;
  //
  // Option SO_REUSEADDR:
  //
  // <IPAddr>: IP addresses of network adapters
  //
  // ==================================================
  // Original idea from BSD sockets
  // ==================================================
  // bind() without SO_REUSEADDR set (default):
  // bind(SockA, 0.0.0.0:21)
  // bind(SockB, 192.168.0.1:21)
  // SockB will fail because <IPAddr> of SockA is a wildcard that means "Any local address",
  // so it is not possible to bind to any other local address with the same port.
  // bind(SockA, 127.0.0.1:21)
  // bind(SockB, 192.168.0.1:21)
  // Both calls will succeed, as different <IPAddr>:<Port> combinations are used.
  //
  // bind() with SO_REUSEADDR set:
  // bind(SockA, 0.0.0.0:21)
  // bind(SockB, 192.168.0.1:21)
  // SockA and SockB will succeed.
  // The original idea includes that *each* of the sockets must have SO_REUSEADDR set before bind().
  // If only the second one calls it, bind() will fail as the first socket did not allow sharing at all.
  //
  // This was the original idea of SO_REUSEADDR.
  // NOTE: Not sure who really ever needed this, but that's the way it is...
  //
  // ==================================================
  // Second effect of SO_REUSEADDR (TIME_WAIT)
  // ==================================================
  // There is another case where this option has an effect on:
  // Calls to send() do not guarantee that data is sent when the function returns. It may be sent delayed.
  // Therefore, it is possible that when calling close() to close a socket, send data is still pending.
  // What the OS does is: preparing everything for closing the connection and return from close().
  // Now the socket changed it's state from ACTIVE to TIME_WAIT but still exists inside the OS (not accessible for the user anymore)
  // If now a new socket is opened and a bind() is performed on exactly the <IPAddr>:<Port> combination of the TIME_WAIT socket, the behavior depends on if the original socket had SO_REUSEADDR set.
  //
  // SO_REUSEADDR not set:
  // bind() will fail as TIME_WAIT is handled as if it is ACTIVE
  //
  // SO_REUSEADDR set:
  // bind() will succeed as TIME_WAIT is handled as if socket was not existing anymore.
  // NOTE: Under rare circumstances, it now can happen that if there is any receive data arriving late at the system, the new socket that did the bind(), will receive it.
  //
  // ==================================================
  // OS specifics
  // ==================================================
  //
  // Windows:
  // When specifying SO_REUSEADDR before bind(), Windows will report SUCCESS on bind(),
  // even if there is another ACTIVE socket that is bound to the same <IPAddr>:<Port> combination.
  // It does not matter if the process that did the first bind() did specify SO_REUSEADDR for its socket or not
  // This allows processes to steal data from other ones...pretty awful bug in Windows... (See MSDN: Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE)
  // Microsoft introduced SO_EXCLUSIVEADDRUSE for this.
  // This makes sure hijacking the socket data is not possible.
  // But it still allows the special behavior in case a bind() to a closed socket in TIME_WAIT state is possible. (See "Second effect of SO_REUSEADDR (TIME_WAIT)" above)
  //
  // MSDN: A socket that is using the SO_EXCLUSIVEADDRUSE option must be shut down properly prior to closing it. Failure to do so can cause a denial of service attack if the associated service needs to restart.
  //
  // Linux:
  // Listening socket:
  // SO_REUSEADDR does not have any effect for the "original idea" (see above). Linux is more restrictive than BSD sockets here.
  // But it has the desired effect on closed sockets in TIME_WAIT state (see above).
  //
  // Client socket:
  // Behaves like the original BSD idea and has the TIME_WAIT effect
  //
  // Kernel >= 3.9: To have "original idea" effect for listening sockets, since kernel 3.9 SO_REUSEPORT has been introduced.
  //
  // Normal TCP connection close:
  // Client1 (C1), Client2 (C2)
  // C1 -> C2  FIN
  // C1 <- C2  ACK
  // C1 <- C2  FIN
  // C1 -> C2  ACK
  // Socket of C1 (as the initiator of the close request) now is in TIME_WAIT state and can stay there several seconds/minutes
  // so the <IPAddr>:<Port> combination is blocked for some time, after the socket has been closed
  // As this is not acceptable for us as the DLL and other J-Link utilities must be able to be started / terminated multiple times in a row,
  // we make use of SO_REUSEADDR for all of our listener sockets which need to bind() to a specific port
  //
  Sock = (SOCKET)hSocket;
  r = setsockopt(Sock, SOL_SOCKET, -5, (char*)&_int1, sizeof(int));  // SO_EXCLUSIVEADDRUSE (Define not known in the VC6 headers, we use...)
  if (r == 0) {
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((U16)Port);
    addr.sin_addr.s_addr = htonl(IPAddr);
    r = bind(Sock, (struct sockaddr*)&addr, sizeof(addr));
    if (r == 0) {
      r = listen(Sock, NumConnectionsQueued);
    }
    if (r) {
      r = -1;
    }
  } else {
    r = -1;
  }
  return r;
}

/*********************************************************************
*
*       _SYS_SOCKET_IsReady
*
*  Function description
*    Checks if a socket that has been connected with _SYS_SOCKET_Connect() is ready.
*    Mainly used on non-blocking sockets to check if they are ready to operate on.
*    The procedure (non-blocking connect, then trying FIONREAD) is recommended by MS (MSDN).
*
*  Parameters
*    hSocket  Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*
*  Return value
*    == 1  O.K., socket ready
*    == 0  O.K., socket not ready yet
*     < 0  Error
*/
static unsigned _SYS_SOCKET_IsReady(_SYS_SOCKET_HANDLE hSocket) {
  SOCKET Sock;
  int IsReady;
  unsigned long v;

  Sock = (SOCKET)hSocket;
  ioctlsocket(Sock, FIONREAD, &v);     // Check if socket is ready to read from
  IsReady = v ? 1 : 0;
  return IsReady;
}

/*********************************************************************
*
*       _SYS_SOCKET_IsReadable
*
*  Function description
*    Checks if a socket that has been connected with _SYS_SOCKET_Connect() is readable.
*
*  Parameters
*    hSocket  Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*
*  Return value
*    == 1  O.K., socket readable
*    == 0  O.K., socket not readable yet
*     < 0  Error
*/
static int _SYS_SOCKET_IsReadable(_SYS_SOCKET_HANDLE hSocket, int TimeoutMs) {
  SOCKET Sock;
  struct timeval tv;
  fd_set rfds;
  fd_set efds;
  int v;
  //
  //
  // Two possible cases when opening a TCP connection:
  // 1) On the other side, there is a server listening on the destination port
  // 2) On the other side, there is NO server listening on the destination port
  //
  // Reg 1)
  //   In such cases, the socket is reported non-writable for a few [us] up to a few [s] depending on if this is a localhost, LAN or internet connection
  //   After that period, the socket is reported as writable
  //
  // Reg 2)
  //   In such cases, the following happens (S = Server):
  //   C -> S: SYN
  //   S -> C: RST ACK
  //   This means, the server has rejected the connection and closed it
  //   In such a case, we need to close the socket and try connecting again
  //
  // However, Windows is difficult regarding 2)...
  // Usually an RST means "it's over, you can close the socket. There is nobody listening"
  // Windows keeps the socket in connecting state, so IsReadable() IsWriteable() simply returns 0 instead of 1 (which would allow a following send/receive to return with error)
  // In the background, Windows performs the SYN sending another 3 times in intervals of 500ms before giving up and reporting an error state
  // Usually, the retransmissions are there in case we do not get an ACK from the other side and therefore must assume that the packet got lost
  // Then also the interval time is doubled for each retransmission
  // However, Microsoft decided to also retry it in case of RST ACK but without increasing the interval time
  // This is explained here: https://support.microsoft.com/en-in/help/175523/info-winsock-tcp-connection-performance-to-unused-ports
  //
  // Unfortunately, even after this 1.5 seconds, Windows not necessarily reports the socket as readable/writable but instead returns "exceptional state" on select()
  // Therefore, we also pass the "exceptional state" structure to select() to catch this case
  //
  Sock = (SOCKET)hSocket;
  FD_ZERO(&rfds);       // Zero init file descriptor list
  FD_SET(Sock, &rfds);  // Add socket to file descriptor list to be monitored by select()
  FD_ZERO(&efds);
  FD_SET(Sock, &efds);
  tv.tv_sec = (long)(TimeoutMs / 1000);
  tv.tv_usec = (TimeoutMs % 1000) * 1000;
  v = select(0, &rfds, NULL, &efds, &tv);   // > 0: in case of success, == 0: Timeout, < 0: Error
  return v;
}

/*********************************************************************
*
*       _SYS_SOCKET_AcceptEx
*
*  Function description
*    Waits for a connection (with timeout) on the given socket.
*
*  Parameters
*    hSocket   Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*    TimeoutMs Timeout in ms for waiting
*
*  Return value
*    >= 0  Handle to socket of new connection that has been established
*     < 0  Error   (_SYS_SOCKET_INVALID_HANDLE)
*      -2  Timeout
*/
static _SYS_SOCKET_HANDLE _SYS_SOCKET_AcceptEx(_SYS_SOCKET_HANDLE hSocket, int TimeoutMs) {
  SOCKET SockChild;
  int r;
  //
  // accept() itself does not allow using timeouts
  // Therefore we check readability first and then call accept() which should not block then
  //
  r = _SYS_SOCKET_IsReadable(hSocket, TimeoutMs);
  if (r < 0) {
    return _SYS_SOCKET_INVALID_HANDLE; // error
  } else if (r == 0) {
    return -2;                        // timeout
  } else {
    SockChild = accept((SOCKET)hSocket, NULL, NULL);
    if (SockChild != INVALID_SOCKET) {
      //
      // If connection is successfully established, handle it as an implicit open(), so also init Winsock API as WSACleanup() is called on SocketClose()
      // No matter if the socket has been opened via open() or accept()
      //
      _WSAStartup();
      //
      // Disable Nagle's algorithm to speed things up
      // Nagle's algorithm prevents small packets from being transmitted and collects some time until data is actually sent out
      //
      setsockopt(SockChild, IPPROTO_TCP, TCP_NODELAY, (char*)&_int1, sizeof(int));
    } else {
      return _SYS_SOCKET_INVALID_HANDLE;
    }
  }
  return (_SYS_SOCKET_HANDLE)SockChild;
}

/*********************************************************************
*
*       _SYS_SOCKET_Receive
*
*  Function description
*    Receives data on the given socket.
*
*  Parameters
*    hSocket          Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*
*  Return value
*    >= 0:  O.K., number of bytes received
*     < 0:  Error, see _SYS_SOCKET_ERR_*
*
*  Notes
*    (1) Returns as soon as something has been received (may be less than MaxNumBytes) or error happened
*/
static int _SYS_SOCKET_Receive(_SYS_SOCKET_HANDLE hSocket, void* pData, U32 MaxNumBytes) {
  int r;
  int Err;
  SOCKET Sock;

  Sock = (SOCKET)hSocket;
  r = recv(Sock, (char*)pData, MaxNumBytes, 0);
  if (r < 0) {
    Err = WSAGetLastError();
    switch (Err) {
    case WSAEWOULDBLOCK:
      r = _SYS_SOCKET_ERR_WOULDBLOCK;
      break;
    case WSAECONNRESET:
      r = _SYS_SOCKET_ERR_CONNRESET;
      break;
    case WSAETIMEDOUT:
      r = _SYS_SOCKET_ERR_TIMEDOUT;
      break;
    default:
      r = _SYS_SOCKET_ERR_UNSPECIFIED;
    }
  }
  return r;
}

/*********************************************************************
*
*       _SYS_SOCKET_IsWriteable
*
*  Function description
*    Checks if a socket that has been connected with _SYS_SOCKET_Connect() is writeable.
*
*  Parameters
*    hSocket  Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*
*  Return value
*    == 1  O.K., socket writable
*    == 0  O.K., socket not writable yet
*/
static int _SYS_SOCKET_IsWriteable(_SYS_SOCKET_HANDLE hSocket, int TimeoutMs) {
  SOCKET Sock;
  struct timeval tv;
  fd_set wfds;
  fd_set efds;
  int v;
  //
  //
  // Two possible cases when opening a TCP connection:
  // 1) On the other side, there is a server listening on the destination port
  // 2) On the other side, there is NO server listening on the destination port
  //
  // Reg 1)
  //   In such cases, the socket is reported non-writeable for a few [us] up to a few [s] depending on if this is a localhost, LAN or internet connection
  //   After that period, the socket is reported as writeable
  //
  // Reg 2)
  //   In such cases, the following happens (S = Server):
  //   C -> S: SYN
  //   S -> C: RST ACK
  //   This means, the server has rejected the connection and closed it
  //   In such a case, we need to close the socket and try connecting again
  //
  // However, Windows is difficult regarding 2)...
  // Usually an RST means "it's over, you can close the socket. There is nobody listening"
  // Windows keeps the socket in connecting state, so IsReadable() IsWriteable() simply returns 0 instead of 1 (which would allow a following send/receive to return with error)
  // In the background, Windows performs the SYN sending another 3 times in intervals of 500ms before giving up and reporting an error state
  // Usually, the retransmissions are there in case we do not get an ACK from the other side and therefore must assume that the packet got lost
  // Then also the interval time is doubled for each retransmission
  // However, Microsoft decided to also retry it in case of RST ACK but without increasing the interval time
  // This is explained here: https://support.microsoft.com/en-in/help/175523/info-winsock-tcp-connection-performance-to-unused-ports
  //
  // Unfortunately, even after this 1.5 seconds, Windows not necessarily reports the socket as readable/writable but instead returns "exceptional state" on select()
  // Therefore, we also pass the "exceptional state" structure to select() to catch this case
  //
  Sock = (SOCKET)hSocket;
  FD_ZERO(&wfds);       // Zero init file descriptor list
  FD_SET(Sock, &wfds);  // Add socket to file descriptor list to be monitored by select()
  FD_ZERO(&efds);
  FD_SET(Sock, &efds);
  tv.tv_sec = (long)(TimeoutMs / 1000);
  tv.tv_usec = (TimeoutMs % 1000) * 1000;
  v = select(0, NULL, &wfds, &efds, &tv);   // > 0: in case of success, == 0: Timeout, < 0: Error
  return v;
}

/*********************************************************************
*
*       _SYS_SOCKET_Send
*
*  Function description
*    Sends data on the specified socket
*
*  Parameters
*    hSocket          Handle to socket that has been returned by _SYS_SOCKET_OpenTCP() / _SYS_SOCKET_OpenUDP()
*
*  Return value
*    >= 0:  O.K., number of bytes sent
*     < 0:  Error, see _SYS_SOCKET_ERR_*
*/
static int _SYS_SOCKET_Send(_SYS_SOCKET_HANDLE hSocket, const void* pData, U32 NumBytes) {
  int r;
  int Err;
  SOCKET Sock;

  Sock = (SOCKET)hSocket;
  r = send(Sock, pData, NumBytes, 0);
  if (r == SOCKET_ERROR) {
    Err = WSAGetLastError();
    r = (Err == WSAEWOULDBLOCK) ? _SYS_SOCKET_ERR_WOULDBLOCK : _SYS_SOCKET_ERR_UNSPECIFIED;
  }
  return r;
}

/*********************************************************************
*
*       Local functions, SystemView
*
**********************************************************************
*/
/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N=" SYSVIEW_APP_NAME ",O=embOS,D=" SYSVIEW_DEVICE_NAME );
  if (strlen(_sISRNames) > 0) {
    SEGGER_SYSVIEW_SendSysDesc(_sISRNames);
  }
}

/*********************************************************************
*
*       Local functions, SystemView Communication Channel
*
**********************************************************************
*/
/*********************************************************************
*
*       _SysViewCommThread
*
*  Function description
*    Function that handles TCP/IP connection and communication with SysView.
*
*  Parameters
*    pPara    Expected to be the ID of the RTT <Up> channel used by SysView
*/
_SYS_THREAD_PROC_EX_TYPE _SysViewCommThread(void* pPara) {
  _SYS_SOCKET_HANDLE hSockListen;
  _SYS_SOCKET_HANDLE hSockSV;
  int Result;
  int ChannelID;
  int v;
  int r;
  int NumBytes;
  char acBuf[2048];

  _SysViewCommThreadRunning = 1;
  Result = 0;
  ChannelID = (int)pPara;
  hSockSV = _SYS_SOCKET_INVALID_HANDLE;
  v = 0;
  //
  // Try and connect to SystemView instance
  //
  hSockListen = _SYS_SOCKET_OpenTCP();
  if (hSockListen == _SYS_SOCKET_INVALID_HANDLE) {  // Failed to open socket? => Done
    Result = -1;
    goto Done;
  }
  r = _SYS_SOCKET_ListenAtTCPAddr(hSockListen, _SYS_SOCKET_IP_ADDR_ANY, 19111, 1);
  if (r < 0) {                                     // Failed to set socket to listening? => Done
    Result = -1;
    goto Done;
  }
  //
  // After a succesful connection, poll RTT buffer for data
  //
  do {
    if (_CloseRequested) {        // Close requested? => Stop systemview session
      if (r == 1) {               // Systemview session running?
        r = _SYS_SOCKET_IsWriteable(hSockSV, 10);
        if (r == 1 && v == 1) {   // TCP/IP connection still established? => Stop systemview session
          r = SEGGER_RTT_ReadUpBufferNoLock(ChannelID, acBuf, sizeof(acBuf));
          if (r > 0) {            // Read RTT data? => Send it via TCP/IP
            NumBytes = _SYS_SOCKET_Send(hSockSV, acBuf, r);  // We do not care if the send succeeded or not as we are closing anyway.
            _SYS_Sleep(10);        // Give system view some time to receive <Stop> info before closing socket.
          }
        }
      }
      goto Done;
    }
    if (hSockSV > _SYS_SOCKET_INVALID_HANDLE) {
      r = _SYS_SOCKET_IsWriteable(hSockSV, 10);
      if (r == 0) {   // Timeout
        continue;
      } else if (r < 0) { // Error
        _SYS_SOCKET_Close(hSockSV);
        hSockSV = _SYS_SOCKET_INVALID_HANDLE;
        continue;
      }
    } else {
      hSockSV = _SYS_SOCKET_AcceptEx(hSockListen, 100);
      if (hSockSV < 0) {
        continue;
      }
      r = _SYS_SOCKET_IsReady(hSockSV);
      if (r != 1) {               // Failed to connect? => Try again later
        continue;
      }
      //
      // Successful connection? => Start systemview session
      // First, receive <Hello> message from SysView and send back own <Hello> message
      //
      r = _SYS_SOCKET_Receive(hSockSV, acBuf, SYSVIEW_COMM_APP_HELLO_SIZE);
      if (r != SYSVIEW_COMM_APP_HELLO_SIZE) {
        printf(" --- Failed to receive \"Hello\" message from SysView...\n");
      }
      r = _SYS_SOCKET_Send(hSockSV, _abHelloMsg, SYSVIEW_COMM_TARGET_HELLO_SIZE);
      if (r != SYSVIEW_COMM_TARGET_HELLO_SIZE) {
        printf(" --- Failed to send \"Hello\" message to SysView...\n");
      }
    }
    //
    // Connection established? => Handle communication
    // Check for data sent by SysView
    //
    r = _SYS_SOCKET_IsReadable(hSockSV, 0);
    if (r == 1) {                                 // Data to read from SysView available?
      r = _SYS_SOCKET_Receive(hSockSV, acBuf, 1);  // Receive <NumBytes> to read
      if (r != 1) {                               // Failed to receive data? => Connection lost
        _SYS_SOCKET_Close(hSockSV);
        hSockSV = _SYS_SOCKET_INVALID_HANDLE;
        continue;
      }
      v = acBuf[0];
      r = _SYS_SOCKET_Receive(hSockSV, acBuf, v);  // Receive all data
      if (r != v) {                               // Failed to receive data? => Connection lost
        _SYS_SOCKET_Close(hSockSV);
        hSockSV = _SYS_SOCKET_INVALID_HANDLE;
        continue;
      }
      NumBytes = SEGGER_RTT_WriteDownBufferNoLock(ChannelID, &acBuf[0], r);  // Write data into corresponding RTT buffer for application to read and handle accordingly
    }
    //
    // Check for data to send to SysView
    //
    NumBytes = SEGGER_RTT_ReadUpBufferNoLock(ChannelID, &acBuf[0], sizeof(acBuf));
    if (NumBytes > 0) {                               // Data to send available?
      r = _SYS_SOCKET_Send(hSockSV, acBuf, NumBytes);  // Send data to SysView
      if (NumBytes != r) {                            // Failed to send data? => Connection lost
        v = _SYS_GetLastError();
        _SYS_SOCKET_Close(hSockSV);
        hSockSV = _SYS_SOCKET_INVALID_HANDLE;
      }
    }
    _SYS_Sleep(1);                                     // Sleep for some time before polling again
  } while (1);
Done:
  //
  // Clean up
  //
  if (hSockSV >= 0) {
    _SYS_SOCKET_Close(hSockSV);
  }
  if (hSockListen >= 0) {
    _SYS_SOCKET_Close(hSockListen);
  }
  _SysViewCommThreadRunning = 0;
  return Result;
}

/*********************************************************************
*
*       _SetupComm()
*
*  Function description
*    Setup communication channel.
*/
static void _SetupComm(void) {
  int r;
  U64 ThreadID;
  //
  // Initialize SysView communication
  //
  r = SEGGER_SYSVIEW_GetChannelID();                                                               // Retrieve the ID of the RTT <Up> / <Down> channel used by SysView
  _SYS_CreateThreadEx(_SysViewCommThread, (void*)r, &ThreadID, "SysView Communication Thread", 0);  // Start thread handling TCP/IP connection and communication with SysView instance
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SEGGER_SYSVIEW_Conf()
*
*  Function description
*    Configure SystemView and embOS for use of SystemView.
*/
void SEGGER_SYSVIEW_Conf(void) {
  LARGE_INTEGER TSFreq;

  //
  // Get the performance counter frequency and scale it down to be < 1 GHz.
  // (We can only handle cycles >= 1ns)
  //
  QueryPerformanceFrequency(&TSFreq);
  _TSDiv = 1;
  if (TSFreq.QuadPart > 1000000000LL) {
    _TSDiv = (U32)(TSFreq.QuadPart / 1000000000LL);
    if (TSFreq.QuadPart % 1000000000LL) {
      _TSDiv++;
    }
    _TSFreq = TSFreq.QuadPart;
    TSFreq.QuadPart /= _TSDiv;
  }
  SEGGER_SYSVIEW_Init(TSFreq.LowPart, TSFreq.LowPart,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  OS_SetTraceAPI(&embOS_TraceAPI_SYSVIEW);    // Configure embOS to use SYSVIEW.
#if SYSVIEW_START_ON_INIT
  SEGGER_SYSVIEW_Start();                     // Start recording to catch system initialization.
#endif

  _SetupComm();
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetTimestamp()
*
*  Function description
*    Get the timestamp for SystemView.
*    On Windows use the performance counter.
*/
U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  LARGE_INTEGER TS;

  QueryPerformanceCounter(&TS);
  if (_TSDiv > 1) {
    TS.QuadPart /= _TSDiv;
  }

  return TS.LowPart;
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetInterruptId()
*
*  Function description
*    Get the "dummy" interrupt ID.
*/
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  return GetCurrentThreadId();
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_SetISRName()
*
*  Function description
*    Informs SystemView about an ISR name.
*
*  Parameters
*    sName: ISR Name
*
*  Additional information
*    Must be called from an ISR after SEGGER_SYSVIEW_Conf() only.
*    It uses the thread ID as an unique ID for SystemView.
*/
void SEGGER_SYSVIEW_X_SetISRName(const char* sName) {
  static int CriticalSectionInitialized = 0;
  char s[100];

  //
  // Make sure the used critical section is initialized.
  //
  if (CriticalSectionInitialized == 0) {
    InitializeCriticalSection(&_csLockString);
    CriticalSectionInitialized = 1;
  }
  //
  // Check whether the string fits in the string buffer.
  //
  if (strlen(sName) < (sizeof(s) - 10)) {
    //
    // If this is the first entry we don't need the comma.
    //
    EnterCriticalSection(&_csLockString);
    if (strlen(_sISRNames) == 0) {
      sprintf(s, "I#%u=%s", (unsigned int)GetCurrentThreadId(), sName);
    } else {
      sprintf(s, ",I#%u=%s", (unsigned int)GetCurrentThreadId(), sName);
    }
    //
    // Add new ISR name to the ISR name string and inform SystemView (if enough space is left in the string buffer).
    //
    if ((strlen(_sISRNames) + strlen(s) + 1) < MAX_ISRNAMES_LENGTH) {
      strcat(_sISRNames, s);
      //
      // Send new description if SystemView is started.
      //
      if (SEGGER_SYSVIEW_IsStarted() > 0) {
        SEGGER_SYSVIEW_SendSysDesc(_sISRNames);
      }
    }
    LeaveCriticalSection(&_csLockString);
  }
}

/*************************** End of file ****************************/
