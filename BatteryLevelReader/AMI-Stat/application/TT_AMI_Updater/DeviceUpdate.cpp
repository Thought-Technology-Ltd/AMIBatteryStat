/*
* DeviceUpdate.cpp : This file contains the class responsible to push the new firmware image
*               to a device via the Bluetooth interface.
*
*   In a nutshell, this class implements:
*       - 1 thread that:
*           - read the file and send it
*       - A progress event is generated at every 1% of transfer done, and at the end.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include "stdafx.h"
#include <assert.h>
#include "DeviceUpdate.h"
#include "TT_AMI_UpdaterDlg.h"
#include "package.h"
#include "lang.h"
#include "ErrCodes.h"
#include "time.h"

// Definitions for the "Q" SLIP channel messages
// Generic command offsets
#define CHAN_OFF            0       // Offset to reach the SLIP channel identifier
#define CMD_OFF             1       // Offset to reach the command number

// Valid channel and commands
#define CHAN_UPDATE         'Q'     // Channel used to update files
#define CMD_UPDREQ          0xB0    // Update request command
#define CMD_UPDRESP         0xB1    // Update response command

// Update request command fields
#define CMD_UPDREQ_OFFSET       2   // Offset to reach the offset field (4 bytes)
#define CMD_UPDREQ_DATA         6   // Offset to reach the beginning of the data
#define CMD_UPDREQ_MAXDATALEN   900 // Max number of data bytes that can be sent in a command
#define CMD_UPDREQ_MAXLEN   (CMD_UPDREQ_DATA + CMD_UPDREQ_MAXDATALEN)       // See UNSLIPPED_PKT_MAX_SIZE defined in SLIP.h of AMI project

#define CMD_UPDRESP_ERR     2       // Offset to reach the error code field (1 byte)
#define CMD_UPDRESP_OFFSET  3       // Offset to reach the offset field (4 bytes)
#define CMD_UPDRESP_LEN     7       // Total length of an update response command

// response error codes in protocol (must match those in FirmwareUpdate.c of MAIN app)
#define RESP_ERR_OK         0       // No error
#define RESP_ERR_TOOSHORT   1       // Message too short
#define RESP_ERR_INVCMD     2       // Invalid command number
#define RESP_ERR_ERASE      3       // Error during flash erase
#define RESP_ERR_WRITE      4       // Error during flash programming
#define RESP_ERR_READ       5       // Error during flash reading
#define RESP_ERR_CRCDWLD	6		// CRC error during download
#define RESP_ERR_CRCSAVE	7		// CRC error when reading back image
#define RESP_ERR_INCOMPATIBLE	8	// Package is incompatible with this device (not used yet but interpreted by PC updater application)

// Bluetooth timeout management
#define BLUETOOTH_TIMEOUT	20			// Number of seconds that bluetooth driver can't create socket after device crash
static time_t bluetoothCrashTime = 0;	// Time when the device crash occurs


/**
* @brief ctor: class constructor
*
* @param devAddr:   MAC address of device to update
* @param fnct:      function to execute to report new device discovery
* @param ctx:       opaque context value for that function
* @return None.
*/
DeviceUpdate::DeviceUpdate(BTH_ADDR devAddr, DeviceUpdateNotif_t fnct, void *ctx)
{
    notifFnct = fnct;
    notifCtx = ctx;

	// Send info about the update to the screen
	std::wstring infoStr;
	infoStr = langGet(TXT_UPDATE_INFO);
	notifFnct(notifCtx, 0, infoStr.c_str(), false);

	// If bluetooth connection crashed earlier, wait until windows drivers can connect to device
	int seconds = (int)(difftime(time(0), bluetoothCrashTime));
	if (seconds <= BLUETOOTH_TIMEOUT)
	{
		Sleep(1000*(BLUETOOTH_TIMEOUT - seconds));
	}

    slip = new Slip(devAddr);

    exiting = false;
    threadBusy = true;
    threadHdl = CreateThread(NULL, 1024 /*stackSize*/, updateDeviceEntry, this, 0 /*creationFlags*/, NULL /*threadId*/);
    assert(threadHdl != NULL);
}


/**
* @brief dtor: class destructor.
*
* @return None.
*/
DeviceUpdate::~DeviceUpdate()
{
    exiting = true;                                 // indicate we are exiting

    if (slip != NULL)   slip->close();              // close the connection (this will make the thread exit

    while (threadBusy == true)           Sleep(100); // wait for thread to exit
    
    if (slip != NULL)   delete slip;
    slip = NULL;

    CloseHandle(threadHdl);
}

/**
* @brief updateDeviceEntry: thread entry point to update a device
*
* @param arg: An abstract pointer to this instance.
* @return 0.
*/
DWORD DeviceUpdate::updateDeviceEntry(void *arg)
{
    DeviceUpdate *pThis = static_cast<DeviceUpdate *>(arg);

    pThis->updateDevice();
    return 0;
}

/**
* @brief updateDevice: thread function performing the firmware update
*
* @param None
* @return None.
*/
void DeviceUpdate::updateDevice(void)
{
    const uint8_t *filePtr = package;
    const uint8_t *fileEnd = package + sizeof(package);
    int timeoutMs = 30000;              // First packet may take long to respond if flash gets erased
    int lastPercentNotif = 0;           // Last percentage notified to application
	CString formattedErr;
    
    slip->open();           // Try to open comm channel. In case of error, it will be reported by the send function.
    if (exiting == false)   // The above function may be long to execute
    {
        std::wstring errMsg;                // Error message to return to application. "" as long as everything goes well
        int rxOffset;                       // Offset returned by the device for the next packet

        while ((filePtr < fileEnd) && (errMsg == L""))
        {
            // build request
            int offset = (int)(filePtr-package);         // offset in file
            int dataLen = ((int)(fileEnd-filePtr) < CMD_UPDREQ_MAXDATALEN) ? (int) (fileEnd-filePtr) : CMD_UPDREQ_MAXDATALEN;

            send(offset, filePtr, dataLen, &errMsg);
            if (errMsg == L"")                          // if no error yet, wait for response
            {
                read(&rxOffset, &errMsg, timeoutMs);
            }
            if (errMsg == L"")                          // if no error yet
            {
                filePtr = package + rxOffset;
				if (filePtr < fileEnd)		timeoutMs = 2000;           // All other packets should be answered very quickly
				else                        timeoutMs = 30000;			// Last packet takes long because CRC is recomputed

                int percent = (int)((filePtr - package) * 100 / sizeof(package));
                if (percent != lastPercentNotif)
                {
                    lastPercentNotif = percent;
					if (exiting == false)	notifFnct(notifCtx, lastPercentNotif, L"", false);  // Notify the application of the progress
                }
            }
        }

        if (errMsg == L"")      // If no error during update, send an extra transaction with no data and offset=total length
        {                       // to indicate the end of the transfer
            send(sizeof(package), NULL, 0, &errMsg);
            if (errMsg == L"")                          // if no error yet, wait for response
            {
                read(&rxOffset, &errMsg, timeoutMs);
            }
            if (errMsg == L"")      errMsg = langGet(TXT_ERR_DONE);     // if no error yet, use Done message
        }

        // send the last notification
		if (exiting == false)	notifFnct(notifCtx, lastPercentNotif, errMsg.c_str(), true);
    }

    if (exiting == false)       // Not absolutely safe (could have 2 delete for same object) but unlikely to happen
    {
        delete slip;            // Close the connection
        slip = NULL;
    }

    threadBusy = false;     // We have finished the job, we can be destroyed
}

/**
* @brief send: Format the message and send it
*
* @param offset:    offset in bytes from beginning of the package
* @param dataPtr:   pointer to the data to transmit
* @param dataLen:   number of bytes to include in the packet
* @param retErrMsg: will be filled with an error message if any encountered
* @return None.
*/
void DeviceUpdate::send(int offset, const uint8_t *dataPtr, int dataLen, std::wstring *retErrMsg)
{
    uint8_t tmpBuf[CMD_UPDREQ_MAXLEN];
	CString formattedErr;

    assert(dataLen <= CMD_UPDREQ_MAXDATALEN);
    assert(retErrMsg != NULL);

    tmpBuf[CHAN_OFF]            = CHAN_UPDATE;
    tmpBuf[CMD_OFF]             = CMD_UPDREQ;
    tmpBuf[CMD_UPDREQ_OFFSET+0] = (offset >> 24) & 0xFF;
    tmpBuf[CMD_UPDREQ_OFFSET+1] = (offset >> 16) & 0xFF;
    tmpBuf[CMD_UPDREQ_OFFSET+2] = (offset >>  8) & 0xFF;
    tmpBuf[CMD_UPDREQ_OFFSET+3] = (offset >>  0) & 0xFF;
    memcpy(&tmpBuf[CMD_UPDREQ_DATA], dataPtr, dataLen);

    // send message
    int err = slip->send(tmpBuf, CMD_UPDREQ_DATA+dataLen);
	if (err != CMD_UPDREQ_DATA + dataLen)   *retErrMsg = ErrTranslate(err, TXT_ERR_SENDFAIL);
}

/**
* @brief read: wait for and decode an update response message
*
*   it is possible that we receive a message from a wrong channel. We just discard it.
*
* @param retOffset:     to be filled with the offset returned by the device
* @param retErrMsg:     to be filled with any error message encountered during the processing
* @param timeoutMs:     max time to wait for an answer
* @return None.
*/
void DeviceUpdate::read(int *retOffset, std::wstring *retErrMsg, int timeoutMs)
{
    int errCode = ERR_OK;
    uint8_t tmpBuf[1024 * CMD_UPDRESP_LEN];            // Length is big to accept other channel data

    assert(retErrMsg != NULL);
    assert(*retErrMsg == L"");

    DWORD entryTime = GetTickCount();
    do
    {
        int waitTime = timeoutMs - (GetTickCount() - entryTime);        // Max remaining time to wait for an answer.
        if (waitTime < 0)       errCode = ERR_SLIP_TIMEOUT;
        else
        {
            // wait response (returns number of bytes received in frame
            int err = slip->read(tmpBuf, sizeof(tmpBuf), waitTime);
            if (err == 0)       errCode = ERR_SLIP_TIMEOUT;             // if no byte received, it is a timeout condition
            else if (err < 0)   errCode = err;
            else
            {
                CString formattedErr;
                CString formattedCmd;
                formattedCmd.Format(L": %d", tmpBuf[CMD_OFF]);
                formattedErr.Format(L": %d", err);
                
                if (tmpBuf[CHAN_OFF] != CHAN_UPDATE)    {}  // process only our channel, ignore others
                else if (tmpBuf[CMD_OFF] != CMD_UPDRESP)            *retErrMsg = langGet(TXT_ERR_INCOMPATIBLE) + formattedCmd;  // Append command number to ease field troubleshooting
                else if (err != CMD_UPDRESP_LEN)                    *retErrMsg = langGet(TXT_ERR_INV_RESPLEN) + formattedErr;   // Append number of bytes received to ease field troubleshooting
                else if (tmpBuf[CMD_UPDRESP_ERR] != RESP_ERR_OK)    errCode = tmpBuf[CMD_UPDRESP_ERR];
                else
                {
                    assert(retOffset != NULL);
                    *retOffset = 0;
                    *retOffset += ((int)tmpBuf[CMD_UPDRESP_OFFSET+0]) << 24;
                    *retOffset += ((int)tmpBuf[CMD_UPDRESP_OFFSET+1]) << 16;
                    *retOffset += ((int)tmpBuf[CMD_UPDRESP_OFFSET+2]) <<  8;
                    *retOffset += ((int)tmpBuf[CMD_UPDRESP_OFFSET+3]) <<  0;
                    break;                              // valid answer, exit loop
                }
            }
        }
    } while ((*retErrMsg == L"") && (errCode == ERR_OK));        // Break inside loop when the proper answer is received
    
    if (*retErrMsg == L"")      *retErrMsg = ErrTranslate(errCode, TXT_ERR_RXFAIL);

	if (errCode == ERR_SLIP_TIMEOUT)
	{
		bluetoothCrashTime = time(0);
	}
}

/**
* @brief ErrTranslate: convert an error code into a printable text
*
* @param errCode:       error code detected
* @param basicMsg:      Text to print if error code is not found
* @return The associated string to display to user
*/
std::wstring DeviceUpdate::ErrTranslate(int errCode, LPCTSTR basicMsg)
{
    std::wstring retStr;
    switch (errCode)
    {
        // Firmware Update specific protocol errors
        case RESP_ERR_TOOSHORT:     retStr = langGet(TXT_ERR_DEV_SHORTMSG);     break;  // Device received a message too short
        case RESP_ERR_INVCMD:       retStr = langGet(TXT_ERR_DEV_INVCMD);       break;  // Device received an invalid command
        case RESP_ERR_ERASE:        retStr = langGet(TXT_ERR_DEV_ERASING);      break;  // Device got error during eMMC erasing
        case RESP_ERR_WRITE:        retStr = langGet(TXT_ERR_DEV_PROGRAMMING);  break;  // Device got error during eMMC writing
        case RESP_ERR_READ:         retStr = langGet(TXT_ERR_DEV_READING);      break;  // Device got error during eMMC reading
        case RESP_ERR_CRCDWLD:      retStr = langGet(TXT_ERR_CRC_DWLD);         break;  // Device detected a CRC error during download
        case RESP_ERR_CRCSAVE:      retStr = langGet(TXT_ERR_CRC_SAVE);         break;  // Device detected a CRC error when reading back image from eMMC
        case RESP_ERR_INCOMPATIBLE: retStr = langGet(TXT_ERR_INCOMPATIBLE);     break;  // Package is incompatible with this device
        // Look into global error resolution
        default:                    retStr = ::ErrTranslate(errCode, basicMsg); break;
    }
    return retStr;
}
