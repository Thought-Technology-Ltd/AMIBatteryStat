/*
* Slip.h : This file contains the class responsible to perform SLIP (serial line IP) framing
*
*   In a nutshell, this class wraps the SppComm class and implements methods to:
*       - open the connection (via the constructor)
*       - send data on the connection by framing it in a slip format
*       - receive data on the connection by deframing the slip format
*       - abort a connection (via the destructor).
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#ifndef _SLIP_H
#define _SLIP_H

#include <Windows.h>
#include "SppComm.h"

class Slip
{
public:
    /**
    * @brief ctor: class constructor
    *
    * @param devAddr: device address
    * @return None.
    */
    Slip(BTH_ADDR devAddr);
    
    /**
    * @brief dtor: class destructor.
    *
    * @return None.
    */
    virtual ~Slip();

    /**
    * @brief close: closes the connection.
    *
    * @return None.
    */
    void close(void);

    /**
    * @brief open: Try to open the channel with the device
    *
    * @return None.
    */
    void open(void);
    
    /**
    * @brief send: SLIP encode and send packet
    *
    * We send the data in block as large as possible to improve the probability of have longer packets sent
    * to bluetooth. If we were sending the data byte per byte to the sppComm, it could be possible to have
    * short packets sent. Does it make sense?
    *
    * @param msg:       data to send (bytes, not wchar).
    * @param msgLen:    number of bytes to send
    * @return > 0   number of bytes sent (excluding SLIP framing)
    *         < 0   an error = -windows error. Windows error are positive numbers. Its negative version is returned
    */
    int send(const uint8_t *msg, int msgLen);

    /**
    * @brief read: extract the next slip packet
    *
    * We read from the sppComm byte per byte for simplicity
    *
    * @param retMsg:    Pre-allocated buffer that is filled with the received data
    * @param maxLen:    Max number of bytes that can be stored in retMsg
    * @param waitMs:    Max time to wait in milliseconds for some data to arrive.
    *                   We wait for a full SLIP frame to be received before returning.
    * @return   > 0     Number of bytes received.
    *           < 0     An error condition
    */
    int read(uint8_t *retMsg, int maxLen, int waitMs);

private:
    /**
    * @brief xmit: send data to sppComm, update count or set an error if required
    *
    * @param msg:       data to send (bytes, not wchar).
    * @param msgLen:    number of bytes to send
    * @param nbSent:    number of bytes sent to far by the send() function
    * @return > 0   number of bytes sent (including nbSent)
    *         < 0   an error = -windows error. Windows error are positive numbers. Its negative version is returned
    */
    int xmit(const uint8_t *msg, int msgLen, int nbSent);
    int xmit(const char *msg, int msgLen, int nbSent)                   { return xmit((const uint8_t *)msg, msgLen, nbSent); }

    /**
    * @brief recv: read data from the sppComm channel and manage the timeout
    *
    * @param retMsg:    Pre-allocated buffer that is filled with the received data
    * @param maxLen:    Max number of bytes that can be stored in retMsg
    * @param entryTime: GetTickCount() value when we entered the read function for this frame
    * @param waitMs:    Max time to wait in milliseconds for some data to arrive.
    * @return   > 0     Number of bytes received.
    *           0       Timeout
    */
    int recv(uint8_t *retMsg, int maxLen, DWORD entryTime, int waitMs);
    int recv(char *retMsg, int maxLen, DWORD entryTime, int waitMs)     { return recv((uint8_t *)retMsg, maxLen, entryTime, waitMs); }

    BTH_ADDR deviceAddr;        // Device MAC address
    SppComm *sppComm;           // Pointer to SppComm communication channel for this SLIP instance
};

#endif // _SLIP_H
