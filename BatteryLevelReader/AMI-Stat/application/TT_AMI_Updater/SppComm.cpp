/*
* SppComm.cpp : This file contains the class responsible to perform the Bluetooth Serial Port
*               Protocol profile connection.
*
*   In a nutshell, this class implements methods to:
*       - open the connection (via the constructor)
*       - send data on the connection
*       - receive data on the connection
*       - abort a connection (via the destructor). It is possible that a thread is stucked in
*           the receive data method. The destructor will make it exit.
*
*   The first packet sent to the device may trigger the flash erasure, which will be long to
*   perform. A long response time should be expected in that situation.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include "stdafx.h"
#include <assert.h>
#include <Ws2bth.h>
#include "SppComm.h"
#include "ErrCodes.h"

/**
* @brief ctor: class constructor
*
* @param devAddr: device address
* @return None.
*/
SppComm::SppComm(BTH_ADDR devAddr)
: sendBusy(false)
, readBusy(false)
, exiting(false)
, connError(0)
{
    initBusy = true;            // Connecting the socket may be long. Make sure we do not delete during that time.

    WSADATA WSAData;
    int err = WSAStartup(MAKEWORD(2, 2), &WSAData);
    assert(err == 0);

    SOCKADDR_BTH    sockAddr;
    sockAddr.addressFamily = AF_BTH;
    sockAddr.btAddr = devAddr;
    // 00001101-0000-1000-8000-00805F9B34FB GUID for SPP profile
    sockAddr.serviceClassId = { 0x00001101, 0x0000, 0x1000, 0x80,0x00,  0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
    sockAddr.port = 0;     // RFCOMM channel number.

    sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM); // Open a bluetooth socket using RFCOMM protocol
    assert (sock != INVALID_SOCKET);

    err = connect(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr));
    if (err != 0)
    {
        // In case of error opening the connection, it will be reported by the send function
        connError = 0 - WSAGetLastError();
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    initBusy = false;
}

/**
* @brief dtor: class destructor.
*
* @return None.
*/
SppComm::~SppComm()
{
    if (sock != INVALID_SOCKET)
    {
        while (initBusy == true)    Sleep(100);
        while (sendBusy == true)    Sleep(100);
        while (readBusy == true)    Sleep(100);

        closesocket(sock);
    }

    int err = WSACleanup();
    assert(err == 0);
}

/**
* @brief close: terminate the connection
*
* @return None.
*/
void SppComm::close(void)
{
    exiting = true;                 // signal send and read that object is about to be deleted
}

/**
* @brief send: Send data to the device on the SPP channel.
*
* @param msg:       data to send (bytes, not wchar).
* @param msgLen:    number of bytes to send
* @return > 0   number of bytes sent
*         < 0   an error = -windows error. Windows error are positive numbers. Its negative version is returned
*/
int SppComm::send(const uint8_t *msg, int msgLen)
{
    int retCode = 0;

    if (exiting == true)    retCode = ERR_SPP_CLOSING;  // return an error, object is being destroyed
    else
    {
        sendBusy = true;

        if (connError != 0)
        {
            retCode = connError;        // Return windows error code that happened at ctor time
        }
        else
        {
            assert(sock != INVALID_SOCKET);
            retCode = ::send(sock, (const char *) msg, msgLen, 0);
        }
        
        sendBusy = false;
    }
    return retCode;
}

/**
* @brief read: Read some data from the SPP channel
*
* @param retMsg:    Pre-allocated buffer that is filled with the received data
* @param maxLen:    Max number of bytes that can be stored in retMsg
* @param maxWaitTimeMs: Max time to wait in milliseconds for some data to arrive
*                   The function returns as soon as some data is received. We do not
*                   wait for the maxLen to be received.
* @return Number of bytes received.
*/
int SppComm::read(uint8_t *retMsg, int maxLen, DWORD maxWaitTimeMs)
{
    int retCode = 0;

    if (exiting == true)
    {
        retCode = ERR_SPP_CLOSING;
    }
    else
    {
        readBusy = true;

        DWORD entryTime = GetTickCount();       // Grab entry time

        // Wait less time to exit faster if dtor is called
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100 * 1000;     // 100 ms Converted into us

        do
        {
            fd_set fdRead;
            FD_ZERO(&fdRead);
            FD_SET(sock, &fdRead);
            int err = select(((int)sock) + 1, &fdRead, NULL, NULL, &timeout);
            switch (err)
            {
                case 0:             // timeout
                    if (exiting == true)    retCode = ERR_SPP_CLOSING;  // If system is exiting
                    break;

                case 1:             // Socket has received data
                    err = recv(sock, (char *)retMsg, maxLen, 0);
                    if (err != 0)   retCode = err;          // return now with number of bytes received
                    break;

                default:    assert(0);      // Other errors are not expected. Don't know what to do if such error occurs.
            }
        } while ((retCode == 0) && ((GetTickCount() - entryTime) < maxWaitTimeMs));
    
        readBusy = false;
    }
    return retCode;
}
