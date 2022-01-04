/*
* DeviceUpgrade.cpp : This file contains the class responsible to push the new firmware image
*               to a device via the Bluetooth interface.
*
*   In a nutshell, this class implements:
*       - 1 thread that:
*           - read the file and send it
*       - A progress event is generated at every 1% of transfer done, and at the end.
*
* Author: Thomas Roy
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include "stdafx.h"
#include <assert.h>
#include "DeviceUpgrade.h"
#include "TT_AMI_UpdaterDlg.h"
#include "lang.h"
#include "ErrCodes.h"

// Definitions for the "P" SLIP channel messages
// Generic command offsets
#define CHAN_OFF            0       // Offset to reach the SLIP channel identifier
#define CMD_OFF             1       // Offset to reach the command number

// Valid channel and commands
#define CHAN_UPGRADE         'P'    // Channel used to upgrade files
#define CMD_UPDREQ          0xB0    // Upgrade request command
#define CMD_UPDRESP         0xB1    // Upgrade response command

// Upgrade request command fields
#define CMD_UPDREQ_DATA         2   // Offset to reach the beginning of the data
#define CMD_UPDREQ_MAXDATALEN   20 // Max number of data bytes that can be sent in a command
#define CMD_UPDREQ_MAXLEN   (CMD_UPDREQ_DATA + CMD_UPDREQ_MAXDATALEN)       // See UNSLIPPED_PKT_MAX_SIZE defined in SLIP.h of AMI project

#define CMD_UPDRESP_ERR     2       // Offset to reach the error code field (1 byte)
#define CMD_UPDRESP_LEN     3       // Total length of an upgrade response command

// response error codes in protocol (must match those in FirmwareUpgrade.c of MAIN app)
#define RESP_ERR_OK         0       // No error
#define RESP_ERR_TOOSHORT   1       // Message too short
#define RESP_ERR_INVCMD     2       // Invalid command number
#define RESP_ERR_ERASE      3       // Error during flash erase
#define RESP_ERR_WRITE      4       // Error during flash programming
#define RESP_ERR_READ       5       // Error during flash reading
#define RESP_ERR_INV_KEY    6       // Key received is invalid
#define RESP_DEVICE_ALREADY_EXTENDED    7 // device is extended


/**
* @brief ctor: class constructor
*
* @param devAddr:   MAC address of device to upgrade
* @param fnct:      function to execute to report new device discovery
* @param ctx:       opaque context value for that function
* @return None.
*/
DeviceUpgrade::DeviceUpgrade(BTH_ADDR devAddr, DeviceUpgradeNotif_t fnct, void *ctx)
{
	notifFnct = fnct;
	notifCtx = ctx;
	slip = new Slip(devAddr);

	// Fetch key from UI
	std::wcstombs(upgradeKey, static_cast<CTTAMIUpdaterDlg *>(ctx)->upgradeKeyValue.c_str(), sizeof(upgradeKey));

	exiting = false;
	threadBusy = true;
	threadHdl = CreateThread(NULL, 1024 /*stackSize*/, upgradeDeviceEntry, this, 0 /*creationFlags*/, NULL /*threadId*/);
	assert(threadHdl != NULL);
}

/**
* @brief dtor: class destructor.
*
* @return None.
*/
DeviceUpgrade::~DeviceUpgrade()
{
	exiting = true;                                 // indicate we are exiting

	if (slip != NULL)   slip->close();              // close the connection (this will make the thread exit

	while (threadBusy == true)           Sleep(100); // wait for thread to exit

	if (slip != NULL)   delete slip;
	slip = NULL;

	CloseHandle(threadHdl);
}

/**
* @brief upgradeDeviceEntry: thread entry point to upgrade a device
*
* @param arg: An abstract pointer to this instance.
* @return 0.
*/
DWORD DeviceUpgrade::upgradeDeviceEntry(void *arg)
{
	DeviceUpgrade *pThis = static_cast<DeviceUpgrade *>(arg);

	pThis->upgradeDevice();
	return 0;
}

/**
* @brief upgradeDevice: thread function performing the firmware upgrade
*
* @param None
* @return None.
*/
void DeviceUpgrade::upgradeDevice(void)
{
	const uint8_t *filePtr = (uint8_t *)upgradeKey;
	CString formattedErr;
	int timeoutMs = 1000;              // First packet may take long to respond if flash gets erased

	slip->open();           // Try to open comm channel. In case of error, it will be reported by the send function.
	if (exiting == false)   // The above function may be long to execute
	{
		std::wstring errMsg;                // Error message to return to application. "" as long as everything goes well


		send(0, filePtr, sizeof(upgradeKey), &errMsg);
		if (errMsg == L"")                          // if no error yet, wait for response
		{
			read(&errMsg, timeoutMs);
		}
        if (errMsg == L"")              errMsg = langGet(TXT_ERR_DONE);     // if no error, use Done message

		// send the last notification
		if (exiting == false)	notifFnct(notifCtx, errMsg.c_str());
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
* @param offset:    offset in bytes from beginning of the upgradeKeyValue
* @param dataPtr:   pointer to the data to transmit
* @param dataLen:   number of bytes to include in the packet
* @param retErrMsg: will be filled with an error message if any encountered
* @return None.
*/
void DeviceUpgrade::send(int offset, const uint8_t *dataPtr, int dataLen, std::wstring *retErrMsg)
{
	uint8_t tmpBuf[CMD_UPDREQ_MAXLEN];

	assert(dataLen <= CMD_UPDREQ_MAXDATALEN);
	assert(retErrMsg != NULL);

	tmpBuf[CHAN_OFF] = CHAN_UPGRADE;
	tmpBuf[CMD_OFF] = CMD_UPDREQ;
	memcpy(&tmpBuf[CMD_UPDREQ_DATA], dataPtr, dataLen);

	// send message
	int err = slip->send(tmpBuf, CMD_UPDREQ_DATA + dataLen);
	if (err != CMD_UPDREQ_DATA + dataLen)   *retErrMsg = ErrTranslate(err, TXT_ERR_SENDFAIL);
}

/**
* @brief read: wait for and decode an upgrade response message
*
*   it is possible that we receive a message from a wrong channel. We just discard it.
*
* @param retErrMsg:     to be filled with any error message encountered during the processing
* @param timeoutMs:     max time to wait for an answer
* @return None.
*/
void DeviceUpgrade::read(std::wstring *retErrMsg, int timeoutMs)
{
    int errCode = ERR_OK;
	uint8_t tmpBuf[2 * CMD_UPDRESP_LEN];            // twice the required size for safety
	assert(retErrMsg != NULL);
	assert(*retErrMsg == L"");

	DWORD entryTime = GetTickCount();
	do
	{
		int waitTime = timeoutMs - (GetTickCount() - entryTime);        // Max remaining time to wait for an answer.
		if (waitTime < 0)       errCode = ERR_SLIP_TIMEOUT;
		else
		{
			// wait response
			int err = slip->read(tmpBuf, sizeof(tmpBuf), waitTime);
            if (err == 0)       errCode = ERR_SLIP_TIMEOUT;             // if no byte received, it is a timeout condition

            if (err < 0)        errCode = err;
            else
            {
                CString formattedErr;
                CString formattedCmd;
                formattedErr.Format(L": %d", err);
                formattedCmd.Format(L": %d", tmpBuf[CMD_OFF]);

                if (tmpBuf[CHAN_OFF] != CHAN_UPGRADE) {}   // process only our channel, ignore others
                else if (tmpBuf[CMD_OFF] != CMD_UPDRESP)    *retErrMsg = langGet(TXT_ERR_INV_MSGTYPE) + formattedCmd;
                else if (err != CMD_UPDRESP_LEN)            *retErrMsg = langGet(TXT_ERR_INV_RESPLEN) + formattedErr;
                else
                {
                    errCode = tmpBuf[CMD_UPDRESP_ERR];
                    break;                              // valid answer, exit loop
                }
			}
		}
	} while ((*retErrMsg == L"") && (errCode == ERR_OK));        // Break inside loop when the proper answer is received
    
    if (*retErrMsg == L"")      *retErrMsg = ErrTranslate(errCode, TXT_ERR_RXFAIL);
}

/**
* @brief ErrTranslate: convert an error code into a printable text
*
* @param errCode:       error code detected
* @param basicMsg:      Text to print if error code is not found
* @return The associated string to display to user
*/
std::wstring DeviceUpgrade::ErrTranslate(int errCode, LPCTSTR basicMsg)
{
    std::wstring retStr;
    switch (errCode)
    {
        // Firmware Upgrade specific protocol errors
        case RESP_ERR_TOOSHORT:     retStr = langGet(TXT_ERR_DEV_SHORTMSG);     break;  // Device received a message too short
        case RESP_ERR_INVCMD:       retStr = langGet(TXT_ERR_DEV_INVCMD);       break;  // Device received an invalid command
        case RESP_ERR_ERASE:        retStr = langGet(TXT_ERR_DEV_ERASING);      break;  // Device got error during eMMC erasing
        case RESP_ERR_WRITE:        retStr = langGet(TXT_ERR_DEV_PROGRAMMING);  break;  // Device got error during eMMC writing
        case RESP_ERR_READ:         retStr = langGet(TXT_ERR_DEV_READING);      break;  // Device got error during eMMC reading
        case RESP_ERR_INV_KEY:      retStr = langGet(TXT_ERR_INV_KEY);          break;  // Device detected the key does not match the serial number
		case RESP_DEVICE_ALREADY_EXTENDED:      retStr = langGet(TXT_ERR_ALREADY_EXTENDED);          break;  // Device detected the key does not match the serial number
																						// Look into global error resolution
        default:                    retStr = ::ErrTranslate(errCode, basicMsg); break;
    }
    return retStr;
}
