/*
* BatteryStatus.cpp : This file contains the class responsible to poll the device for its information
*               about battery status
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
#include "BatteryStatus.h"
#include "TT_AMI_UpdaterDlg.h"
#include "lang.h"
#include "ErrCodes.h"
#include <string.h>
#include <stdio.h>
/**
* @brief ctor: class constructor
*
* @param devAddr: device address
* @return None.
*/
IBatteryStatus::IBatteryStatus(BTH_ADDR devAddr)
: exiting(false)
{
    slip = new Slip(devAddr);
	queryDevice();
}

/**
* @brief dtor: class destructor.
*           MUST NOT BE CALLED BY THE NOTIF FNCT!!!
*
* @return None.
*/
IBatteryStatus::~IBatteryStatus()
{
  
	exiting = true;             // Tell we want to destroy

	if (slip != NULL)   slip->close();              // close the connection (this will make the thread exit


	if (slip != NULL)   delete slip;
	slip = NULL;


}

//////////////////////////////////////////////////////////////////////
unsigned char  IBatteryStatus::getSOC(void)
{
	return _SOC;
}
//////////////////////////////////////////////////////////////////////
bool IBatteryStatus::getCharging(void)
{
	return _charging;
}
//////////////////////////////////////////////////////////////////////
unsigned  short IBatteryStatus::getVoltage(void)
{
	return _voltage;
}
//////////////////////////////////////////////////////////////////////
std::wstring IBatteryStatus::getErrorMessage()
{
	return _errMsg;
}
//////////////////////////////////////////////////////////////////////
bool IBatteryStatus::getError()
{
	return _error;
}
//////////////////////////////////////////////////////////////////////

/**
* @brief queryDevice: Perform the query transaction and decode the answer
*
* @param None
* @return None
*/
void IBatteryStatus::queryDevice(void)
{

	_error = true;
    slip->open();           // Try to open comm channel. In case of error, it will be reported by the send function.
    if (exiting == false)   // The above function may be long to execute
    {
        const char *req = "A" "{\n\"ID\": \"GetBatteryStatus\",\n\"Content\":{}\n}"; // Request JSON message
        int err = slip->send((const uint8_t *)req, (int) strlen(req));
        if (err < 0)                            _errMsg = ErrTranslate(err, TXT_ERR_INFO_GATHER);
        else if (err != strlen(req))			_errMsg = ErrTranslate(err, TXT_ERR_INFO_GATHER);
        else
        {
            uint8_t buf[1000];
			time_t t1;
			time_t t2;
			time(&t1);
			while (1)
			{				
				err = slip->read(buf, (int) sizeof(buf), 2000);           // 2 secs to get the answer
				if (strstr( (char * )buf, "GetBatteryStatus") != NULL) break;
				time(&t2);
				if ((t2 - t1) > 10) 
				{
					err = 0;
					break;
				}
			}


            if (err == 0)       err = ERR_SLIP_TIMEOUT;

            if (err < 0)                        _errMsg = ErrTranslate(err, TXT_ERR_RXFAIL);
            else
            {
                buf[err] = L'\0';   // terminate string
				std::wstring response; 
				float val = 0;
				bool bool_val = false;;

                jsonExtract(buf, "\"SOC\"",   NULL, &val, NULL);
				_SOC = static_cast<unsigned char> (val);
                jsonExtract(buf, "\"Voltage\"", NULL, &val, NULL);
				_voltage =val;
                jsonExtract(buf, "\"Charging\"", NULL, NULL, &bool_val);
				_charging = bool_val;
				_error = false;
			}
        }

    }


}



void IBatteryStatus::jsonExtract(const uint8_t *resp, const char *key, std::wstring *retData, float * number, bool  *  boolean)
{
    const char *ptr = strstr((const char *) resp, key);
    const char *end;
	assert(ptr != NULL);
	if (retData != NULL)
	{
		// The key provided by the caller already includes the surrounding ""
		ptr = strstr(ptr, ":");                     // Reach the :
		ptr = strstr(ptr, "\"");                    // Find the beginning "
		assert(ptr != NULL);
		ptr++;                                      // Skip the "
		end = strstr(ptr + 1, "\"");                  // Find the ending "
		assert(end != NULL);
		std::string tmp(ptr, end - ptr);            // isolate the value
		retData->assign(tmp.begin(), tmp.end());    // convert to wchar
		return;
	}

	if (number != NULL)
	{
		ptr = strstr(ptr, ":");                     // Reach the :
		assert(ptr != NULL);
		ptr++;                                      // Skip the "
		sscanf(ptr, "%f", number);
		return;
    }
	if (boolean != NULL)
	{
		ptr = strstr(ptr, ":");                     // Reach the :
		assert(ptr != NULL);
		ptr++;                                      // Skip the "
		*boolean = false;
		if (strstr(ptr, "true") != NULL)
			*boolean = true;
		if (strstr(ptr, "True") != NULL)
			*boolean = true;
		return;
	}

}
