#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
///    Intermediate class to isolate the original code to the one used for production
///
///
///
////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <functional>
#include <thread>

#include "DeviceUpdate.h"
#include "DeviceUpgrade.h"
using namespace std;
#include  "ami.h"
#include "lang.h"
#include "DeviceUpdate.h"


#define PROD_UPGREQ_MAXDATALEN 20
#define PROD_CMD_UPDREQ_MAXDATALEN   900 // Max number of data bytes that can be sent in a command


class ProductionHelper
{
private:
	AMI_DEVICE_HANDLE handle;
public:
	ProductionHelper()
	{
		AMI_EnableServiceMode();
		AMI_SetScanFilter("");
		AMI_Init();
		_notifyCallback = nullptr;
		_notifyParameter = nullptr;
		_updating = false;
		_exiting = true;
	}
	virtual ~ProductionHelper()
	{
		AMI_End();

	}
	typedef enum 
	{
		DEVICE_POWERED_ON,
		DEVICE_POWERED_OFF,
		DEVICE_HEART_BEAT,
	}Notifications;
	typedef function<void(Notifications, void *, void * )> ProductionNotifier;
	void SetNotify(ProductionNotifier notifyCallback, void  * parameter)
	{
		_notifyCallback = notifyCallback;
		_notifyParameter = parameter;
	}
private:
	/////////////////////////////////////////vector<AccessData> ftdiDevices;
	vector<std::string> ftdiDevices;
	CListBox  *  _deviceList;
	////////////////////////////////////////CommHandler  _commHandler;
	ProductionNotifier _notifyCallback;
	void * _notifyParameter;	

	void  * _updateNotifyParameter;
	DeviceUpdateNotif_t _updateNotifCallback;
	void  * _upgradeNotifyParameter;
	DeviceUpgradeNotif_t _upgradeNotifCallback;

	char _key[PROD_UPGREQ_MAXDATALEN];
	size_t _keyLen;
	const uint8_t * _package;
	size_t _packageLen;
	TTL_UINT32 _offset;
	bool _exiting;
	bool _updating;
public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetUIItems(CListBox * deviceList)
	{
		_deviceList = deviceList;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	void ListDevices()
	{
		AMI_SetScanMode(SCAN_FAST);

		AMI_SetScanFilter("");
		AMI_ResumeScan();
		AMI_PauseScan();
		TTL_UINT32 count = 0;
		AMI_GetAvailableDevicesCount(count);
		ftdiDevices.clear();

		for (TTL_UINT32 i = 0; i < count; ++i)
		{
			ConnectionInfo ci;
			lastResult = AMI_GetDeviceConnectionInfo(i, ci);
			int insert_index = _deviceList->AddString(CString(CStringA(ci.ID)));
			ftdiDevices.push_back(string(ci.ID));
			_deviceList->SetItemData(insert_index, _deviceList->GetCount() - 1);
		}
		
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Open()
	{
		int curSel = _deviceList->GetCurSel();
		if (curSel == -1) return false;

		handle = _deviceList->GetItemData(curSel);
		return (AMI_DeviceOpen(handle)==0);

	}
	bool Close()
	{
		return (AMI_DeviceClose(handle)==0);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetDeviceInfo(DeviceInfoC & device_info_)
	{
		return (AMI_DeviceGetInfo(handle, device_info_, 1)==0);
	

	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	BatteryStatus LastBatStat = {0,0xff,false};
	CALL_RESULT lastResult = TTL_ERROR_NONE;
	bool isBatStatValid()
	{
		return LastBatStat.SOC != 0xff;
	}
	BatteryStatus GetBatteryStatus()
	{
		

		lastResult = AMI_DeviceGetBatteryStatus(handle, LastBatStat);

		if (lastResult != TTL_ERROR_NONE)
		{
			
			LastBatStat.SOC=0xff;
		}
		return LastBatStat;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool GetBatteryStatus(unsigned char & soc, bool & charging, unsigned short &Voltage)
	{
		BatteryStatus battery_status_= GetBatteryStatus();
		if (battery_status_.SOC!=0xff)
		{
			charging = battery_status_.Charging != 0;
			soc = battery_status_.SOC;
			Voltage = battery_status_.Voltage;
			return true;
		}
		return false;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Set5V_Relay(bool TurnOn)
	{
		BatteryStatus battery_status_;
		lastResult = AMI_DeviceSetVariable(handle, "AC_POWER_EN", (TTL_INT32)TurnOn);

		if (lastResult == TTL_ERROR_NONE)
		{
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool ChargingState(bool &ChargingRes)
	{
		GetBatteryStatus();
		if (lastResult == TTL_ERROR_NONE)
		{
			return true;
		}
		return false;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetCallbacks(HB_CALLBACK hb_cb)
	{
		//AMI_DeviceSetHBCallback(handle,hb_cb,this);

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool EnableHeartBeat()
	{
		return (lastResult = AMI_DeviceEnableHeartBeat(handle, 1)) == TTL_ERROR_NONE;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool IsDeviceOnline()
	{
		TTL_BOOL ido;
		AMI_DeviceIsOnline(handle, ido);
		return ido;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Update()
	{
		uint8_t *filePtr = (uint8_t*)_package;
		uint8_t *fileEnd = (uint8_t*)_package + _packageLen;
		int timeoutMs = 30000;              // First packet may take long to respond if flash gets erased
		int lastPercentNotif = 0;           // Last percentage notified to application
		CString formattedErr;
		int sleep = 1000;
		UpdateResponse response;
		if (_exiting == false)   // The above function may be long to execute
		{
			std::wstring errMsg;                // Error message to return to application. "" as long as everything goes well
			lastResult = TTL_ERROR_NONE;
			while ((filePtr < fileEnd) && (errMsg == L""))
			{
				// build request
				int offset = (int)(filePtr - _package);         // offset in file
				int dataLen = ((int)(fileEnd - filePtr) < PROD_CMD_UPDREQ_MAXDATALEN) ? (int)(fileEnd - filePtr) : PROD_CMD_UPDREQ_MAXDATALEN;

				UpdateResponse response;
				lastResult = AMI_WriteUpdateBatch(handle, offset, filePtr, dataLen, response, timeoutMs);

				errMsg = DeviceUpdate::ErrTranslate(response.error_code, TXT_ERR_RXFAIL);



				Sleep(sleep);
				sleep = 0;
				if (lastResult == TTL_ERROR_NONE && response.error_code == 0)
				{
					filePtr =(uint8_t*)_package + response.offset;
					if (filePtr < fileEnd)		
						timeoutMs = 2000;           // All other packets should be answered very quickly
					else
						timeoutMs = 30000;			// Last packet takes long because CRC is recomputed

					int percent = (int)((filePtr - _package) * 100 / _packageLen);
					if (percent != lastPercentNotif)
					{
						lastPercentNotif = percent;
						if (_exiting == false)	_updateNotifCallback(_updateNotifyParameter, lastPercentNotif, L"", false);  // Notify the application of the progress
					}

				}
			}
			if (errMsg == L"")      // If no error during update, send an extra transaction with no data and offset=total length
			{                       // to indicate the end of the transfer

				lastResult = AMI_WriteUpdateBatch(handle, _packageLen,NULL, 0, response,  timeoutMs);

				if (lastResult == TTL_ERROR_NONE)
					errMsg = L"Update done";			
			}

			// send the last notification
			if (_exiting == false)	_updateNotifCallback(_updateNotifyParameter, lastPercentNotif, errMsg.c_str(), true);
			CancelUpdateDevice();
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Upgrade()
	{
		UpdateResponse response;
		lastResult = AMI_WriteUpgradeKey(handle, _key, _keyLen, response, 1000);

		if (response.error_code == 0  && lastResult == 0)
			_upgradeNotifCallback(_upgradeNotifyParameter, L"");
		else
			_upgradeNotifCallback(_upgradeNotifyParameter, L"Upgrade error");

	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static void UpdateEntry( ProductionHelper * productionInstance_ )
	{
		productionInstance_->Update();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static void UpgradeEntry(ProductionHelper * productionInstance_)
	{
		productionInstance_->Upgrade();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	static void __stdcall UpdateCallBack(TTL_BYTE command, TTL_BYTE error_code, TTL_UINT32 rxoffset, void *param)
	{
	
	
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool UpdateDevice(DeviceUpdateNotif_t  notifFunction, void * notifyParameter, const uint8_t * package, size_t len)
	{
		if (_updating == true) return false;
		///////////////////////_commHandler.SetUpdateCallback(UpdateCallBack, this);
		_offset = 0;
		_package = package;
		_packageLen = len;
		_exiting = false;
		_updating = true;
		_updateNotifCallback = notifFunction;
		_updateNotifyParameter = notifyParameter;
		std::thread(UpdateEntry, this).detach();
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool CancelUpdateDevice()
	{
		_exiting = true;
		_updating = false;
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool UpgradeDevice(DeviceUpgradeNotif_t  notifFunction, void * notifyParameter, const char * key, size_t len)
	{
		if (len > PROD_UPGREQ_MAXDATALEN) return false;
		if (_updating) return false;
		_offset = 0;
		memset(_key,0, PROD_UPGREQ_MAXDATALEN);
		memcpy(_key , key, len);
		_keyLen = len;
		_exiting = false;
		_updating = true;
		_upgradeNotifCallback = notifFunction;
		_upgradeNotifyParameter = notifyParameter;
		std::thread(UpgradeEntry, this).detach();
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
};

