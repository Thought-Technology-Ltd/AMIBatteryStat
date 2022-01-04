/*
* DeviceList.cpp : This file contains the class responsible to retrieve the device list from
*               the Bluetooth interface.
*
*   In a nutshell, this class implements:
*       - 1 thread that:
*           - flushes the device cache (internal to windows bluetooth stuff)
*           - wait for device advertisement for a long time
*       - 1 thread that:
*           - reads over and over from the cache the device list
*           - populates an internal list to detect new arriving entries
*           - notify the UI of new devices being detected
*       - The threads are launched in the ctor and deleted in the dtor.
*       - The refresh() api triggers the execution of both threads
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include "stdafx.h"
#include <assert.h>
#include <Ws2bth.h>
#include "DeviceList.h"

#pragma comment(lib, "Bthprops.lib")

/**
* @brief ctor: class constructor
*
* @param fnct:  function to execute to report new device discovery
* @param ctx:   opaque context value for that function
* @return None.
*/
DeviceList::DeviceList(DeviceListAddNotif_t fnct, void *ctx)
{
    notifFnct = fnct;
    notifCtx = ctx;

    startEvent = CreateEvent(NULL, true /*manual reset*/, false /*initial state*/, NULL);
    assert(startEvent != NULL);

    quitEvent = CreateEvent(NULL, true /*manual reset*/, false /*initial state*/, NULL);
    assert(quitEvent != NULL);

    pollThreadState = IDLE;
    pollThreadHdl = CreateThread(NULL, 1024 /*stackSize*/, pollDeviceEntry, this, 0 /*creationFlags*/, NULL /*threadId*/);
    assert(pollThreadHdl != NULL);

    notifyThreadState = IDLE;
    notifyThreadHdl = CreateThread(NULL, 1024 /*stackSize*/, notifyDeviceEntry, this, 0 /*creationFlags*/, NULL /*threadId*/);
    assert(notifyThreadHdl != NULL);
}

/**
* @brief dtor: class destructor.
*
* @return None.
*/
DeviceList::~DeviceList()
{
    BOOL errBool;
    DWORD errDword;

    // Ask the threads to exit
    errBool = SetEvent(quitEvent);
    assert(errBool != 0);

    // The poll thread might be busy in the discovery process and if it is the case,
    // we might be stuck there for several seconds. In that situation, just stop the thread
    // to avoid problems and wait for exit() to clean it.
    if (pollThreadState == BUSY)
    {
        errDword = SuspendThread(pollThreadHdl);
        assert(errDword != (DWORD) -1);
    }
    else
    {   // Wait for thread to exit
        while (pollThreadState != EXITED)   Sleep(100);
    }

    // The notify thread might be busy reading the cache or notifying the application.
    // This period of time should be short, so we wait for the thread to terminate.
    while (notifyThreadState != EXITED)     Sleep(100);

    CloseHandle(startEvent);
    CloseHandle(quitEvent);
    CloseHandle(pollThreadHdl);
    CloseHandle(notifyThreadHdl);
}

/**
* @brief refresh: Request to poll again the list of bluetooth devices
*               The application should not call this function more than necessary.
*               The end of the refresh procedure will be reported by a notification
*               with no device name.
*
* @param None.
* @return None.
*/
void DeviceList::refresh(void)
{
    BOOL errBool;

    assert(pollThreadState == IDLE);        // Should not be called more than necessary
    assert(notifyThreadState == IDLE);      // ... else timing will be unpredictible

    // Launch the process (the process lasts several seconds)
    errBool = SetEvent(startEvent);
    assert(errBool != 0);
    
    // Wait for threads to be really launched before resetting the event
    while (pollThreadState == IDLE)     Sleep(10);
    while (notifyThreadState == IDLE)   Sleep(10);
    errBool = ResetEvent(startEvent);
    assert(errBool != 0);
}

/**
* @brief pollDeviceEntry: thread entry point to poll for the devices
*
* @param arg: An abstract pointer to this instance.
* @return 0.
*/
DWORD DeviceList::pollDeviceEntry(void *arg)
{
    DeviceList *pThis = static_cast<DeviceList *>(arg);

    pThis->pollDevice();
    return 0;
}

/**
* @brief pollDevice: thread function waiting to start the bluetooth polling
*
* @param None
* @return None.
*/
void DeviceList::pollDevice(void)
{
    DWORD errDword;
    bool exitLoop = false;
    do
    {
        HANDLE hdl[2] = { quitEvent, startEvent };
        errDword = WaitForMultipleObjects(2, hdl, FALSE /*waitAll*/, INFINITE);
        switch (errDword)
        {
            case WAIT_OBJECT_0:                     // end of application
                exitLoop = true;
                break;

            case WAIT_OBJECT_0+1:                   // start event
                // no need to transit to STARTED, as we switch immediately to BUSY
                pollThreadState = BUSY;
                performPolling();
                pollThreadState = IDLE;
                break;
            default:
                assert(0);
        }
    } while (exitLoop == false);
    
    pollThreadState = EXITED;
}

/**
* @brief performPolling: Calls the windows bluetooth interface to perform a poll
*
* @param None
* @return None.
*/
void DeviceList::performPolling(void)
{
    BOOL errBool;
    DWORD entryTime = GetTickCount();
    DWORD curTime;

    do
    {
        curTime = GetTickCount();
        int waitTime = 20000 - (curTime - entryTime);
        if (waitTime > 0)                                           // still time to poll
        {
            BLUETOOTH_DEVICE_SEARCH_PARAMS searchParam;
            ZeroMemory(&searchParam, sizeof(searchParam));
            searchParam.dwSize = sizeof(searchParam);
            searchParam.fReturnAuthenticated = TRUE;
            searchParam.fReturnRemembered = TRUE;
            searchParam.fReturnUnknown = TRUE;
            searchParam.fReturnConnected = TRUE;
            searchParam.fIssueInquiry = TRUE;                       // Flush cache and poll again
            searchParam.cTimeoutMultiplier = (UCHAR) ((waitTime/1000)/1.28);     // discovery time in proper unit for this structure
            searchParam.hRadio = NULL;

            BLUETOOTH_DEVICE_INFO info;
            ZeroMemory(&info, sizeof(info));
            info.dwSize = sizeof(info);
            HBLUETOOTH_DEVICE_FIND hdl = BluetoothFindFirstDevice(&searchParam, &info);

            if (hdl != NULL)
            {
                errBool = BluetoothFindDeviceClose(hdl);
                assert(errBool == TRUE);
            }
        }

        curTime = GetTickCount();
    } while (curTime - entryTime < 20000);              // 20 secs discovery time
}

/**
* @brief notifyDeviceEntry: thread entry point to notify the application of new devices
*
* @param arg: An abstract pointer to this instance.
* @return 0.
*/
DWORD DeviceList::notifyDeviceEntry(void *arg)
{
    DeviceList *pThis = static_cast<DeviceList *>(arg);

    pThis->notifyDevice();
    return 0;
}

/**
* @brief notifyDevice: thread function waiting to read bluetooth cache and notify application
*
* @param None
* @return None.
*/
void DeviceList::notifyDevice(void)
{
    DWORD errDword;
    DWORD waitTime = INFINITE;          // Normally, wait for trigger before activating
    bool exitLoop = false;
    do
    {
        HANDLE hdl[2] = { quitEvent, startEvent };
        errDword = WaitForMultipleObjects(2, hdl, FALSE /*waitAll*/, waitTime);
        switch (errDword)
        {
            case WAIT_OBJECT_0:                     // end of application
                exitLoop = true;
                break;

            case WAIT_OBJECT_0+1:                   // start event
                notifyThreadState = STARTED;        // procedure is started
                waitTime = 1000;                    // update application every second
                break;                              // wait 1 sec before reading cache because we could read old polling

            case WAIT_TIMEOUT:                      // repetitive event
                notifyThreadState = BUSY;
                updateList();
                if (pollThreadState != BUSY)        // The other thread has completed, we will no find new entries
                {
                    waitTime = INFINITE;            // wait for next refresh() execution
                    deviceList.clear();             // Flush the device list
                    notifFnct(notifCtx, NULL, 0, FALSE);   // Tell application that refresh is completed
                    notifyThreadState = IDLE;       // Procedure completed
                }
                else
                {
                    notifyThreadState = STARTED;    // We are idle but procedure is started
                }
                break;
                
            default:
                assert(0);
        }
    } while (exitLoop == false);
    
    notifyThreadState = EXITED;
}

/**
* @brief updateList: Calls the windows bluetooth interface to read cache and notify application of new entries
*
* @param None
* @return None.
*/
void DeviceList::updateList(void)
{
    BOOL errBool;

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParam;
    ZeroMemory(&searchParam, sizeof(searchParam));
    searchParam.dwSize = sizeof(searchParam);
    searchParam.fReturnAuthenticated = TRUE;
    searchParam.fReturnRemembered = TRUE;
    searchParam.fReturnUnknown = TRUE;
    searchParam.fReturnConnected = TRUE;
    searchParam.fIssueInquiry = FALSE;                      // Use cache content
    searchParam.cTimeoutMultiplier = 0;                     // No wait time
    searchParam.hRadio = NULL;

    BLUETOOTH_DEVICE_INFO info;
    ZeroMemory(&info, sizeof(info));
    info.dwSize = sizeof(info);
    HBLUETOOTH_DEVICE_FIND hdl = BluetoothFindFirstDevice(&searchParam, &info);

    if (hdl != NULL)                        // if NULL, there is no device found yet
    {
        do      // Loop in all devices found by bluetooth
        {
            // On windows 10, the time returned in stLastSeen field here is not according to UTC but according to current timezone.
            // There is another BluetoothFindFirstDevice above in this file and, at this particular place, the time
            // is according to UTC.
            // On windows 8.1, both calls return UTC timestamp.
            // For now, this check is just disabled because anyway, if a device is absent but has been paired previously,
            // then it is returned and the stLastSeen field is always a recent value (less than 15 seconds ago). We don't
            // know why.

            if ((wcsncmp(info.szName, L"AMI", wcslen(L"AMI")) == 0) || (wcsncmp(info.szName, L"MyOnyx", wcslen(L"MyOnyx")) == 0))
            {
                // check if already in the list
                auto it = deviceList.begin();
                for (; it != deviceList.end(); ++it)
                {
                    if (*it == info.szName) break;      // This device has already been notified
                }

                if (it == deviceList.end())             // New device found?
                {
                    deviceList.push_back(info.szName);
                    notifFnct(notifCtx, info.szName, info.Address.ullLong, (info.fAuthenticated != 0));   // Notify application
                }
            }

            errBool = BluetoothFindNextDevice(hdl, &info);
        } while (errBool == TRUE);

        int err = GetLastError();
        assert(err == ERROR_NO_MORE_ITEMS);     // there should be no error in BluetoothFindNextDevice

		errBool = BluetoothFindDeviceClose(hdl);
		assert(errBool == TRUE);
	}

}

