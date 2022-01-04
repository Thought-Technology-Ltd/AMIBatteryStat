/*
* BatteryStatus.h : This file contains the class responsible to poll the device for the battery Status
*
*   In a nutshell, this class implements a thread that performs the transaction with the device
*   and then decode the response and push the info toward the application via a callback.
*   It is done that way to avoid blocking the UI during the transaction.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#ifndef _BATTERSTATUS_H
#define _BATTERSTATUS_H

#include <Windows.h>
#include <string>
#include "Slip.h"



class IBatteryStatus
{
public:
    /**
    * @brief ctor: class constructor
    *
    * @param devAddr: device address
    * @return None.
    */
	IBatteryStatus(BTH_ADDR _devAddr);
	unsigned char getSOC(void);
	bool getCharging(void);
	unsigned  short getVoltage(void);
	bool getError();
	std::wstring getErrorMessage();

    /**
    * @brief dtor: class destructor.
    *           MUST NOT BE CALLED BY THE NOTIF FNCT!!!
    *
    * @return None.
    */
    virtual ~IBatteryStatus();

private:


    /**
    * @brief queryDevice: Perform the query transaction and decode the answer
    *
    * @param None
    * @return None
    */
    void queryDevice(void);

    void jsonExtract(const uint8_t *resp, const char *key, std::wstring *retData,  float *  number, bool  * boolean);

    volatile bool exiting;							// When true, we want to destroy the object
    Slip *slip;										// Slip instance to use
	std::wstring _errMsg;
	unsigned char _SOC = 0;
	unsigned short _voltage = 0;
	bool  _charging = 0;
	bool _error;
	HANDLE threadHdl;								// Thread performing the transaction
	volatile bool threadBusy;						// While true, the thread is still running
};

#endif // _BATTERYSTATUS_H
