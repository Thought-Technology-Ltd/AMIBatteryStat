/*
* DeviceInfo.cpp : This file contains the class responsible to poll the device for its information
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
#include "stdafx.h"
#include <assert.h>
#include <BluetoothAPIs.h>
#include "DeviceInfo.h"
#include "TT_AMI_UpdaterDlg.h"
#include "lang.h"
#include "ErrCodes.h"

/**
* @brief ctor: class constructor
*
* @param devAddr: device address
* @return None.
*/
DeviceInfo::DeviceInfo(BTH_ADDR devAddr, DeviceInfoNotif_t fnct, void *ctx)
: exiting(false)
{
    notifFnct = fnct;
    notifCtx = ctx;

    slip = new Slip(devAddr);

    threadBusy = true;
    threadHdl = CreateThread(NULL, 1024 /*stackSize*/, queryDeviceEntry, this, 0 /*creationFlags*/, NULL /*threadId*/);
    assert(threadHdl != NULL);
}

/**
* @brief dtor: class destructor.
*           MUST NOT BE CALLED BY THE NOTIF FNCT!!!
*
* @return None.
*/
DeviceInfo::~DeviceInfo()
{
    exiting = true;             // Tell we want to destroy

    if (slip != NULL)   slip->close();              // close the connection (this will make the thread exit

    while (threadBusy == true)          Sleep(100); // wait for thread to exit
    
    if (slip != NULL)   delete slip;
    slip = NULL;

    CloseHandle(threadHdl);
}

/**
* @brief queryDeviceEntry: Thread entry function to perform the query
*
* @param arg:       This instance
* @return 0
*/
DWORD DeviceInfo::queryDeviceEntry(void *arg)
{
    DeviceInfo *pThis = static_cast<DeviceInfo *>(arg);
    pThis->queryDevice();
    return 0;
}

/**
* @brief queryDevice: Perform the query transaction and decode the answer
*
* @param None
* @return None
*/
void DeviceInfo::queryDevice(void)
{
    std::wstring errMsg;
    std::wstring prodId = L"???";
    std::wstring serialNb = L"???";
    std::wstring fwVer = L"???";

    slip->open();           // Try to open comm channel. In case of error, it will be reported by the send function.
    if (exiting == false)   // The above function may be long to execute
    {
        const char *req = "A" "{\n\"ID\": \"GetDeviceInfo\",\n\"Content\":{}\n}"; // Request JSON message
        int err = slip->send((const uint8_t *)req, (int) strlen(req));
        if (err < 0)                            errMsg = ErrTranslate(err, TXT_ERR_INFO_GATHER);
        else if (err != strlen(req))			errMsg = ErrTranslate(err, TXT_ERR_INFO_GATHER);
        else
        {
            uint8_t buf[1000];
            err = slip->read(buf, (int) sizeof(buf), 2000);           // 2 secs to get the answer
            if (err == 0)       err = ERR_SLIP_TIMEOUT;

            if (err < 0)                        errMsg = ErrTranslate(err, TXT_ERR_RXFAIL);
            else
            {
                buf[err] = L'\0';   // terminate string
                jsonExtract(buf, "\"ProductNumber\"",   &prodId);
                jsonExtract(buf, "\"SerialNumber\"",    &serialNb);
                jsonExtract(buf, "\"FwMainVersion\"", &fwVer);
            }
        }

        // Notify the application of the info obtained
        notifFnct(notifCtx, prodId.c_str(), serialNb.c_str(), fwVer.c_str(), errMsg.c_str());
    }

    if (exiting == false)       // Not absolutely safe (could have 2 delete for same object) but unlikely to happen
    {
        delete slip;            // Close the connection
        slip = NULL;
    }

    threadBusy = false;     // We have finished the job, we can be destroyed
}

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
void DeviceInfo::jsonExtract(const uint8_t *resp, const char *key, std::wstring *retData)
{
    const char *ptr = strstr((const char *) resp, key);
    const char *end;
    if (ptr != NULL)
    {
        // The key provided by the caller already includes the surrounding ""

        ptr = strstr(ptr, ":");                     // Reach the :
        assert(ptr != NULL);
        ptr = strstr(ptr, "\"");                    // Find the beginning "
        assert(ptr != NULL);
        ptr++;                                      // Skip the "
        end = strstr(ptr+1, "\"");                  // Find the ending "
        assert(end != NULL);
        std::string tmp(ptr, end - ptr);            // isolate the value
        retData->assign(tmp.begin(), tmp.end());    // convert to wchar
    }
}
