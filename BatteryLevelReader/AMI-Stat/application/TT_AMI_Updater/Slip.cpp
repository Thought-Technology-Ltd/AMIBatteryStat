/*
* Slip.cpp : This file contains the class responsible to perform SLIP (serial line IP) framing
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
#include "stdafx.h"
#include <assert.h>
#include "Slip.h"
#include "ErrCodes.h"

//
// SLIP special character codes
//
#define		SLIP_END_BYTE             0xC0   // indicates end of packet
#define		SLIP_ESC_BYTE             0xDB   // indicates byte stuffing
#define		SLIP_ESC_END_BYTE         0xDC   // ESC ESC_END means END data byte
#define		SLIP_ESC_ESC_BYTE         0xDD   // ESC ESC_ESC means ESC data byte


/**
* @brief ctor: class constructor
*
* @param devAddr: device address
* @return None.
*/
Slip::Slip(BTH_ADDR devAddr)
: sppComm(NULL)
{
    deviceAddr = devAddr;
}

/**
* @brief dtor: class destructor.
*
* @return None.
*/
Slip::~Slip()
{
    if (sppComm)    delete sppComm;
}

/**
* @brief close: closes the connection.
*
* @return None.
*/
void Slip::close(void)
{
    if (sppComm)    sppComm->close();
}

/**
* @brief open: Try to open the channel with the device
*
* @return None.
*/
void Slip::open(void)
{
    sppComm = new SppComm(deviceAddr);
}

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
int Slip::send(const uint8_t *msg, int msgLen)
{
    int retCode = 0;
    const uint8_t *ptr = msg;       // ptr parses the buffer to find special code
    int nb = 0;                     // number of bytes parsed
    static const uint8_t delim[1] = { SLIP_END_BYTE };
    static const uint8_t escEnd[2] = { SLIP_ESC_BYTE, SLIP_ESC_END_BYTE };
    static const uint8_t escEsc[2] = { SLIP_ESC_BYTE, SLIP_ESC_ESC_BYTE };

    // send start of frame
    retCode = xmit(delim, sizeof(delim), retCode-sizeof(delim));         // -sizeof(..): Do not count this byte for the return code

    while ((nb < msgLen) && (retCode >= 0))     // still bytes and no error yet
    {
        switch (*ptr)
        {
            case SLIP_END_BYTE:                 // SLIP_END
                if ((int)(ptr-msg) != 0)    retCode = xmit(msg, (int)(ptr-msg), retCode);   // send data before special code
                if (retCode >= 0)           retCode = xmit(escEnd, sizeof(escEnd), retCode-sizeof(escEnd)); // send special escape code (do not count)
                if (retCode >= 0)           retCode++;                      // payload wise, we sent 1 byte in this switch case statement
                msg = ++ptr;                // skip special code and position for next segment to send
                break;

            case SLIP_ESC_BYTE:                 // SLIP_ESC
                if ((int)(ptr-msg) != 0)    retCode = xmit(msg, (int)(ptr-msg), retCode);   // send data before special code
                if (retCode >= 0)           retCode = xmit(escEsc, sizeof(escEsc) ,retCode-sizeof(escEsc)); // send special escape code (do not count)
                if (retCode >= 0)           retCode++;                      // payload wise, we sent 1 byte in this switch case statement
                msg = ++ptr;                // skip special code and position for next segment to send
                break;

            default:
                ptr++;                      // continue parsing, searching for special code
        }

        nb++;
    }

    if (ptr != msg) retCode = xmit(msg, (int)(ptr-msg), retCode);       // send remaining of data

    // send end of frame
    retCode = xmit(delim, sizeof(delim), retCode-sizeof(delim));        // -sizeof(..): Do not count this byte for the return code

    return retCode;
}

/**
* @brief xmit: send data to sppComm, update count or set an error if required
*
* @param msg:       data to send (bytes, not wchar).
* @param msgLen:    number of bytes to send
* @param nbSent:    number of bytes sent to far by the send() function
* @return > 0   number of bytes sent (including nbSent)
*         < 0   an error = -windows error. Windows error are positive numbers. Its negative version is returned
*/
int Slip::xmit(const uint8_t *msg, int msgLen, int nbSent)
{
    int retCode = sppComm->send(msg, msgLen);
    assert((retCode == msgLen) || (retCode < 0));

    if (retCode >= 0)   retCode += nbSent;      // if no error, cumulate number of bytes sent

    return retCode;
}

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
int Slip::read(uint8_t *retMsg, int maxLen, int waitMs)
{
    int retCode = 0;
    uint8_t *ptr = retMsg;

    DWORD entryTime = GetTickCount();       // Grab entry time

    // We could discard any data before receiving a C0, but in theory, there is no need to have
    // a C0 at the beginning and at the end; we only need 1 C0 to separate frames.
    // If the AMI always surround packets with 2 C0, then we could discard here any data
    // received before the first C0.

    do
    {
        uint8_t data;
        int nb = recv(&data, 1, entryTime, waitMs);
        
        if (nb == 1)
        {
            switch(data)
            {
                case SLIP_END_BYTE:     // start of frame and end of frame
                    if (ptr != retMsg)              retCode = (int)(ptr - retMsg);  // some data gathered, so this is a packet
                    break;

                case SLIP_ESC_BYTE:     // escape character
                    nb = recv(&data, 1, entryTime, waitMs);     // read next byte
                    if (nb == 1)
                    {
                        if (data == SLIP_ESC_END_BYTE)          *ptr++ = SLIP_END_BYTE;
                        else if (data == SLIP_ESC_ESC_BYTE)     *ptr++ = SLIP_ESC_BYTE;
                        else                                    retCode = ERR_SLIP_FRAMING;     // framing error
                    }
                    break;

                default:
                    *ptr++ = data;
            }
			if ((ptr - retMsg) >= maxLen)		    retCode = ERR_SLIP_BUFSHORT;    // buffer too short for response
        }

        if (nb == 0)                                retCode = ERR_SLIP_TIMEOUT;     // timeout
        else if (nb != 1)                           retCode = nb;                   // serial port error code
    } while (retCode == 0);

    return retCode;
}

/**
* @brief recv: read data from the sppComm channel and manage the timeout
*
* @param retMsg:    Pre-allocated buffer that is filled with the received data
* @param maxLen:    Max number of bytes that can be stored in retMsg
* @param entryTime: GetTickCount() value when we entered the read function for this frame
* @param waitMs:    Max time to wait in milliseconds for some data to arrive.
* @return   > 0     Number of bytes received.
*           <=0     Timeout
*/
int Slip::recv(uint8_t *retMsg, int maxLen, DWORD entryTime, int waitMs)
{
    int retCode = 0;

    DWORD curTime = GetTickCount();
    int waitTime = waitMs - (curTime-entryTime);
    
    // if time remain, read data
    if (waitTime >= 0)  retCode = sppComm->read(retMsg, maxLen, waitTime);
    
    return retCode;
}
