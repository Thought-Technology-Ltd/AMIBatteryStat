/*
* DeviceList.h : This file contains the class responsible to retrieve the device list from
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
#ifndef _DEVICELIST_H
#define _DEVICELIST_H

#include <Windows.h>
#include <BluetoothAPIs.h>
#include <list>

/**
  * @brief Signature of function that will be called when a new device will be disovered
  *
  * @param ctx:     Opaque context meaningful for the notification function
  * @param devName: Device name just discovered
  * @param devAddr: Device MAC address
  * @param paired:  Indicates if the device is paired or not
  * @return         None
  *
  */
typedef void (*DeviceListAddNotif_t) (void *ctx, const wchar_t *devName, BTH_ADDR devAddr, bool isPaired);

/**
  * @brief Possible states for a thread. Used to make sure we don't delete a thread at wrong time.
  *
  */
enum ThreadState_t
{
    IDLE,               // Thread is idle
    STARTED,            // Thread has started the procedure
    BUSY,               // Thread is busy at this moment
    EXITED              // Thread has exited. Ok to clean.
};

class DeviceList
{
public:
    /**
    * @brief ctor: class constructor
    *
    * @param fnct:  function to execute to report new device discovery
    * @param ctx:   opaque context value for that function
    * @return None.
    */
    DeviceList(DeviceListAddNotif_t fnct, void *ctx);

    /**
    * @brief dtor: class destructor.
    *
    * @return None.
    */
    virtual ~DeviceList();

    /**
    * @brief refresh: Request to poll again the list of bluetooth devices
    *               The application should not call this function more than necessary.
    *               The end of the refresh procedure will be reported by a notification
    *               with no device name.
    *
    * @param None.
    * @return None.
    */
    void refresh(void);

private:

    /**
    * @brief pollDeviceEntry: thread entry point to poll for the devices
    *
    * @param arg: An abstract pointer to this instance.
    * @return 0.
    */
    static DWORD WINAPI pollDeviceEntry(void *arg);

    /**
    * @brief pollDevice: thread function waiting to start the bluetooth polling
    *
    * @param None
    * @return None.
    */
    void pollDevice(void);

    /**
    * @brief performPolling: Calls the windows bluetooth interface to perform a poll
    *
    * @param None
    * @return None.
    */
    void performPolling(void);

    /**
    * @brief notifyDeviceEntry: thread entry point to notify the application of new devices
    *
    * @param arg: An abstract pointer to this instance.
    * @return 0.
    */
    static DWORD WINAPI notifyDeviceEntry(void *arg);

    /**
    * @brief notifyDevice: thread function waiting to read bluetooth cache and notify application
    *
    * @param None
    * @return None.
    */
    void notifyDevice(void);

    /**
    * @brief updateList: Calls the windows bluetooth interface to read cache and notify application of new entries
    *
    * @param None
    * @return None.
    */
    void updateList(void);

    /**
    * @brief convertTime: Convert a SYSTEMTIME in a single value in ms
    *
    * @param sysTime    System time
    * @return a value in ms equivalent ot sysTime.
    */
    ULONGLONG convertTime(const SYSTEMTIME &sysTime);

    DeviceListAddNotif_t notifFnct;             // Function to execute when a new device is detected
    void *notifCtx;                             // Function context
    HANDLE startEvent;                          // Event to start polling a new list
    HANDLE quitEvent;                           // Event to quit the application
    HANDLE pollThreadHdl;                       // Thread performing the bluetooth discovery from the air to the cache
    volatile ThreadState_t pollThreadState;     // Indicate if poll thread is busy or not
    HANDLE notifyThreadHdl;                     // Thread reading the cache and notifying the application of new discoveries
    volatile ThreadState_t notifyThreadState;   // Indicates if notify thread is busy or not
    std::list<std::wstring> deviceList;         // List of device already notified to application since last refresh call

};

#endif // _DEVICELIST_H
