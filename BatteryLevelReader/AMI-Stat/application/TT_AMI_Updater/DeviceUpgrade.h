/*
* DeviceUpgrade.h : This file contains the class responsible to push the new firmware image
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
#ifndef _DEVICEUPGRADE_H
#define _DEVICEUPGRADE_H

#include <Windows.h>
#include <BluetoothAPIs.h>
#include <string>
#include "Slip.h"

#define UPGKEY_LEN 20

/**
  * @brief Signature of function that will be called to indicate upgrade progress
  *         The upgrade process is terminated when an error message is provided.
  *
  * @param ctx:         Opaque context meaningful for the notification function
  * @param percent:     Percentage done so far
  * @param errMsg:      Error message (can be NULL, can be "")
  * @return         None
  *
  */
typedef void(*DeviceUpgradeNotif_t) (void *ctx, const wchar_t *errMsg);


class DeviceUpgrade
{
public:
	/**
	* @brief ctor: class constructor
	*
	* @param devAddr:   MAC address of device to upgrade
	* @param fnct:      function to execute to report new device discovery
	* @param ctx:       opaque context value for that function
	* @return None.
	*/
	DeviceUpgrade(BTH_ADDR devAddr, DeviceUpgradeNotif_t fnct, void *ctx);

	/**
	* @brief dtor: class destructor.
	*
	* @return None.
	*/
	virtual ~DeviceUpgrade();

private:




	/**
	* @brief upgradeDeviceEntry: thread entry point to upgrade a device
	*
	* @param arg: An abstract pointer to this instance.
	* @return 0.
	*/
	static DWORD WINAPI upgradeDeviceEntry(void *arg);

	/**
	* @brief upgradeDevice: thread function performing the firmware upgrade
	*
	* @param None
	* @return None.
	*/
	void upgradeDevice(void);

	/**
	* @brief send: Format the message and send it
	*
	* @param offset:    offset in bytes from beginning of the package
	* @param dataPtr:   pointer to the data to transmit
	* @param dataLen:   number of bytes to include in the packet
	* @param retErrMsg: will be filled with an error message if any encountered
	* @return None.
	*/
	void DeviceUpgrade::send(int offset, const uint8_t *dataPtr, int dataLen, std::wstring *retErrMsg);

	/**
	* @brief read: wait for and decode an upgrade response message
	*
	*   it is possible that we receive a message from a wrong channel. We just discard it.
	*
	* @param retErrMsg:     to be filled with any error message encountered during the processing
	* @param timeoutMs:     max time to wait for an answer
	* @return None.
	*/
	void read(std::wstring *retErrMsg, int timeoutMs);

    /**
    * @brief ErrTranslate: convert an error code into a printable text
    *
    * @param errCode:       error code detected
    * @param basicMsg:      Text to print if error code is not found
    * @return The associated string to display to user
    */
    std::wstring ErrTranslate(int errCode, LPCTSTR basicMsg);

	char upgradeKey[UPGKEY_LEN];					// Upgrade key to send to AMI device
	DeviceUpgradeNotif_t notifFnct;					// Function to execute to report progress
	void *notifCtx;									// Function context
	HANDLE threadHdl;								// Thread performing the transaction
	volatile bool threadBusy;						// While true, the thread is still running
	volatile bool exiting;							// When true, the object is destroying
	Slip *slip;										// Slip instance to use
};

#endif // _DEVICEUPGRADE_H
