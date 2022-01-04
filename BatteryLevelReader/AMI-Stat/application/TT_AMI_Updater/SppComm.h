/*
* SppComm.h : This file contains the class responsible to perform the Bluetooth Serial Port
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
#ifndef _SPPCOMM_H
#define _SPPCOMM_H

#include <Windows.h>
#include <stdint.h>
#include <BluetoothAPIs.h>

class SppComm
{
public:
    /**
    * @brief ctor: class constructor
    *
    * @param devAddr: device address
    * @return None.
    */
    SppComm(BTH_ADDR devAddr);

    /**
    * @brief dtor: class destructor.
    *
    * @return None.
    */
    virtual ~SppComm();

    /**
    * @brief close: terminate the connection
    *
    * @return None.
    */
    void close(void);

    /**
    * @brief send: Send data to the device on the SPP channel.
    *
    * @param msg:       data to send (bytes, not wchar).
    * @param msgLen:    number of bytes to send
    * @return > 0   number of bytes sent
    *         < 0   an error = -windows error. Windows error are positive numbers. Its negative version is returned
    */
    int send(const uint8_t *msg, int msgLen);

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
    int read(uint8_t *retMsg, int maxLen, DWORD maxWaitTimeMs);

private:
    SOCKET sock;            // Socket used for the communication
    int connError;          // Any error encountered during connection time
    volatile bool initBusy; // When true, someone is in the constructor waiting for connection establishment
    volatile bool sendBusy; // When true, someone is in the send function
    volatile bool readBusy; // When true, someone is in the read function
    volatile bool exiting;  // When true, we need to destroy the object
};

#endif // _SPPCOMM_H
