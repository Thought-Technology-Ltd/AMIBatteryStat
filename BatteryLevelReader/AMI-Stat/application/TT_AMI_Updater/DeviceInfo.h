/*
* DeviceInfo.h : This file contains the class responsible to poll the device for its information
*               such as the name, serial number, ...
*
*   In a nutshell, this class implements a thread that performs the transaction with the device
*   and then decode the response and push the info toward the application via a callback.
*   It is done that way to avoid blocking the UI during the transaction.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#ifndef _DEVICEINFO_H
#define _DEVICEINFO_H

#include <Windows.h>
#include <string>
#include "Slip.h"

/**
  * @brief Signature of function that will be called when a new device will be disovered
  *
  * @param ctx:         Opaque context meaningful for the notification function
  * @param productId:   Device product ID
  * @param serialNb:    Device serial number
  * @param firmwareVer: Device current firmware version
  * @param errMsg:      Error message (can be "")
  * @return         None
  *
  */
typedef void (*DeviceInfoNotif_t) (void *ctx, const wchar_t *productId, const wchar_t *serialNb, const wchar_t *firmwareVer, const wchar_t *errMsg);


class DeviceInfo
{
public:
    /**
    * @brief ctor: class constructor
    *
    * @param devAddr: device address
    * @return None.
    */
    DeviceInfo(BTH_ADDR _devAddr, DeviceInfoNotif_t fnct, void *ctx);

    /**
    * @brief dtor: class destructor.
    *           MUST NOT BE CALLED BY THE NOTIF FNCT!!!
    *
    * @return None.
    */
    virtual ~DeviceInfo();

private:
    /**
    * @brief queryDeviceEntry: Thread entry function to perform the query
    *
    * @param arg:       This instance
    * @return 0
    */
    static DWORD WINAPI queryDeviceEntry(void *arg);

    /**
    * @brief queryDevice: Perform the query transaction and decode the answer
    *
    * @param None
    * @return None
    */
    void queryDevice(void);

    /**
    * @brief jsonExtract: extract a field from the json response buffer and convert to wchar
    *       This function assumes that the response is well formed and looks like that:
    *
    *   {"ID":"GetDeviceInfo",
    *    "Content":
    *       {"ProductNumber":"SA9000","SerialNumber":"GA000028","ProductType":1,
    *        "HardwareConfig":4,"FirmwarePNumber":"TT9000","FirmwareVersion":"0.13.0.0",
    *        "HardwareVersion":"2.0.0","ProtocolVersion":"1.0.0"
    *       },
    *    "Status":0
    *   }
    *
    *   In particular, a key should not be a value of another key.
    *
    * @param resp:      Response packet received from device
    * @param key:       Key field to find in the response
    * @param retData:   Where to store the value of this field
    * @return None
    */
    void jsonExtract(const uint8_t *resp, const char *key, std::wstring *retData);

    DeviceInfoNotif_t notifFnct;					// Notification function for when data gets available
    void *notifCtx;									// Notification function context (opaque value)
    HANDLE threadHdl;								// Thread performing the transaction
    volatile bool exiting;							// When true, we want to destroy the object
    volatile bool threadBusy;						// While true, the thread is still running
    Slip *slip;										// Slip instance to use

};

#endif // _DEVICEINFO_H
