/*
* TT_AMI_UpdaterDlg.cpp : main window dialog file
*
* Author: Francis Choquette
* Project: AMI
* Company: Orthogone Technologies inc.
*/

#include "stdafx.h"
#include <assert.h>
#include <vector>
#include <afxdialogex.h>
#include <stdio.h>
#include "TT_AMI_Updater.h"
#include "TT_AMI_UpdaterDlg.h"
#include "package.h"
#include "lang.h"
#include <locale>
#include <codecvt>
#ifdef __PRODUCTION__
#include <chrono>
#include <thread>
#endif


#define IDS_BATTERY_LEVEL       L"Battery Level : "
#define IDS_ERROR               L"Error"
#define IDS_CANT_COMM           L"A communication error occurred. Please try again. Error code: 099"	//Can't communicate with device
#define IDS_BATTERY_ABOVE_25    L"Battery Level must be more than 25%"
#define IDS_WARNING             L"Warning"
#define IDS_KEEP_CHGR_CONNECTED L"Keep charger connected until unit has been fully updated"
#define IDS_INFO                L"Information"
#define IDS_PREREQUISITES       L"Before running the update program, close all sessions and navigate to the Home screen."


#define _BAT_MON__TIMER_TICK_ 200
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/**
* @brief CTTAMIUpdaterDlg: main dialog constructor
*
* @param pParent: pointer to parent object
* @return None.
*/
CTTAMIUpdaterDlg::CTTAMIUpdaterDlg(LPTSTR pCmdLine, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_TT_AMI_UPDATER_DIALOG, pParent)
	, devUpdater(NULL)
	, devUpgrader(NULL)
	, devLister(NULL)
	, devInfoPoller(NULL)
	, cmdLine(pCmdLine)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/**
* @brief ~CTTAMIUpdaterDlg: main dialog destructor
*
* @param None.
* @return None.
*/
CTTAMIUpdaterDlg::~CTTAMIUpdaterDlg()
{
	assert(devUpdater == NULL);
	assert(devUpgrader == NULL);
	assert(devLister == NULL);
	assert(devInfoPoller == NULL);
}

/**
* @brief DoDataExchange: exchange and validate data for main dialog
*
* @param pDX: pointer to data exchange object
* @return None.
*/
void CTTAMIUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	AutoLock lock(uiDataCs);

	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVLIST_LIST, m_deviceListBox);
	DDX_Control(pDX, IDC_UPDATE_PROGRESS, updateProgressBar);
	DDX_Control(pDX, IDC_UPGRADE_PROGRESS, upgradeProgressBar);
	DDX_Control(pDX, IDC_UPDATE_MSG_TEXT, updateErrMsg);
	DDX_Control(pDX, IDC_UPGRADE_MSG_TEXT, upgradeErrMsg);
	DDX_Control(pDX, IDC_PRODUCT_TEXT, productId);
	DDX_Control(pDX, IDC_SERIAL_TEXT, serialNb);
	DDX_Control(pDX, IDC_FWVERSION_TEXT, currentFwVersion);
	DDX_Control(pDX, IDC_NEWVERSION_TEXT, nextFwVersion);
	DDX_Control(pDX, IDC_DEVLIST_MSG_TEXT, devListErrMsg);
	DDX_Control(pDX, IDC_UPGRADE_KEY_EDIT, upgradeKeyUI);
}

/**
* call to macro mapping dialog GUI objects to functions
*/
BEGIN_MESSAGE_MAP(CTTAMIUpdaterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DEVLIST_REFRESH_BUTTON, &CTTAMIUpdaterDlg::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDEXIT, &CTTAMIUpdaterDlg::OnBnClickedExit)
	ON_LBN_SELCHANGE(IDC_DEVLIST_LIST, &CTTAMIUpdaterDlg::OnLbnSelchangeDevList)
	ON_BN_CLICKED(IDC_UPDATE_BUTTON, &CTTAMIUpdaterDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_UPGRADE_BUTTON, &CTTAMIUpdaterDlg::OnBnClickedButtonUpgrade)
	ON_EN_CHANGE(IDC_UPGRADE_KEY_EDIT, &CTTAMIUpdaterDlg::OnEnChangeUpgradeKeyEdit)
	ON_BN_CLICKED(IDM_OSP, &CTTAMIUpdaterDlg::OnBnClickedOsp)
	//ON_BN_CLICKED(IDM_RDI, &CTTAMIUpdaterDlg::OnBnClickedRdi)
	ON_BN_CLICKED(IDM_CSP, &CTTAMIUpdaterDlg::OnBnClickedCsp)
	ON_WM_TIMER()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/**
* @brief OnInitDialog: handler for dialog box initialisation message
*
* @param None.
* @return TRUE: focus is at default location (1st control)
*/

////////////////////////////////////////////////////////
#ifdef __PRODUCTION__
void CTTAMIUpdaterDlg::ProductionNotify(ProductionHelper::Notifications id, void  * reserved, void  * parameter)
{
	if (id == ProductionHelper::DEVICE_POWERED_ON)
	{
		static_cast<CTTAMIUpdaterDlg*>(parameter)->SetWindowTextW(_T("Connecting..."));
	}

}

#endif
////////////////////////////////////////////////////////

void __stdcall HBCB(unsigned int handle, void * param)
{

}

BOOL CTTAMIUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

#ifdef __PRODUCTION__
	//GetDlgItem(IDC_DEVLIST_REFRESH_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDM_OSP)->ShowWindow(TRUE);
	GetDlgItem(IDM_CSP)->ShowWindow(TRUE);
	//GetDlgItem(IDM_RDI)->ShowWindow(TRUE);
	GetDlgItem(IDM_OSP)->EnableWindow(TRUE);
	GetDlgItem(IDM_CSP)->EnableWindow(FALSE);
	//GetDlgItem(IDM_RDI)->EnableWindow(FALSE);
	_productionHelper.SetUIItems(&m_deviceListBox);
	_productionHelper.SetNotify(ProductionNotify, this );
#endif


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// get language from region format
	WCHAR pszLanguage[LOCALE_NAME_MAX_LENGTH];  // arbritary string size
	GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SENGLISHLANGUAGENAME, pszLanguage, LOCALE_NAME_MAX_LENGTH);
	if (wcslen(cmdLine) != 0)	wcsncpy(pszLanguage, cmdLine, LOCALE_NAME_MAX_LENGTH);			// override with command line option for test purposes

    // Put text in proper language on static objects in UI
	langInit(pszLanguage);
	CWnd::SetDlgItemText(IDC_DEVICELIST_STATIC,			langGet(TXT_DEVLIST));
	CWnd::SetDlgItemText(IDC_PRODUCT_STATIC,			langGet(TXT_PRODUCT));
	CWnd::SetDlgItemText(IDC_SERIAL_STATIC,				langGet(TXT_SERNUM));
	CWnd::SetDlgItemText(IDC_FWVERSION_STATIC,			langGet(TXT_CURFWVER));
	CWnd::SetDlgItemText(IDC_UPDATE_STATIC,				langGet(TXT_NEWFWVER));
	CWnd::SetDlgItemText(IDC_UPGRADE_STATIC,			langGet(TXT_UPGRKEY));
	CWnd::SetDlgItemText(IDC_DEVLIST_REFRESH_BUTTON,	langGet(TXT_REFRESH_BTN));
	CWnd::SetDlgItemText(IDC_UPDATE_BUTTON,				langGet(TXT_UPDATE_BTN));
	CWnd::SetDlgItemText(IDC_UPGRADE_BUTTON,			langGet(TXT_UPGRADE_BTN));
	CWnd::SetDlgItemText(IDEXIT,						langGet(TXT_EXIT_BTN));

#ifdef __FW_UPGRADE_TOOL__
	nextFwVersion.SetWindowTextW(L"UpgradeTool");
#else
	nextFwVersion.SetWindowTextW(PACKAGEVERSION);
#endif
	// At start, disable the upgrade and update buttons. Will be re-enabled at proper time by ManageEnables
	GetDlgItem(IDC_UPDATE_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_UPGRADE_BUTTON)->EnableWindow(FALSE);
	OnBnClickedButtonRefresh();         // At start, do like if the use pressed the refresh button to obtain AMI device list

	upgradeKeyUI.SetLimitText(UPGKEY_LEN);
	this->SetTimer(1, _BAT_MON__TIMER_TICK_, (TIMERPROC)NULL);
	_productionHelper.SetCallbacks(HBCB);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**
* @brief OnPaint: handler for dialog box drawing message
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnPaint()
{
	// If you add a minimize button to your dialog, you will need the code below
	//  to draw the icon.  For MFC applications using the document/view model,
	//  this is automatically done for you by the framework.
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

/**
* @brief OnQueryDragIcon: handler that obtains the cursor to display while the
*                         user drags the minimized window
* @param None.
* @return handle to cursor icon
*/
HCURSOR CTTAMIUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**
* @brief OnBnClickedExit: handler for exit button
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnBnClickedExit()
{
	// destroy objects here because if they are called in the dialog destructor, some UI assets are destroyed before these
	// objects and if a notification is being done, then they will use destructed UI assets.
	if (devUpdater)     delete devUpdater;
	devUpdater = NULL;
	if (devUpgrader)     delete devUpgrader;
	devUpgrader = NULL;
	if (devLister)      delete devLister;
	devLister = NULL;
	if (devInfoPoller)  delete devInfoPoller;
	devInfoPoller = NULL;
	EndDialog(IDCANCEL);
}

///////////////////////////////////////////////////////////////////////////////
// Device List section
///////////////////////////////////////////////////////////////////////////////

/**
* @brief OnBnClickedDevicesListRefresh: handler for the AMI devices list refresh
*                                       button
* @param None
* @return None.
*/
void CTTAMIUpdaterDlg::OnBnClickedButtonRefresh()
{
	AutoLock lock(uiDataCs);

	deviceList.clear();                                 // Clear list

	// Clear list box
	while (m_deviceListBox.GetCount() > 0)    m_deviceListBox.DeleteString(0);

	// Clear info of former device
	productId.SetWindowTextW(L"");
	serialNb.SetWindowTextW(L"");
	currentFwVersion.SetWindowTextW(L"");
	devInfoFwVer = L"";

	// Call this function after having set devInfoFwVer to make sure buttons get updated
	ManageEnables(IDC_DEVLIST_REFRESH_BUTTON, true);    // begin refresh procedure

	lock.Unlock();      // release UI before launching procedure


///////////////////////////////////////////////////////////////  Production phase support for FTDI
#ifdef __PRODUCTION__
	_productionHelper.ListDevices();
	m_deviceListBox.SetCurSel(0);

#else
	if (devLister != NULL)  delete devLister;
	devLister = new DeviceList(deviceListNotifEntry, this);
	devLister->refresh();
#endif



}

/**
* @brief deviceListNotifEntry: Callback entry function used by the device lister
*       to notify new device found.
*
* @param ctx:     Opaque context meaningful for the notification function
* @param devName: Device name just discovered
* @param devAddr: Device MAC address
* @param paired:  Indicates if the device is paired or not
* @return         None
*/
void CTTAMIUpdaterDlg::deviceListNotifEntry(void *ctx, const wchar_t *devName, BTH_ADDR devAddr, bool isPaired)
{
	// NEVER DELETE THE devLister IN THIS THREAD (DEAD LOCK CONDITION GUARANTEED)

	CTTAMIUpdaterDlg *pDlg = static_cast<CTTAMIUpdaterDlg *>(ctx);

	pDlg->deviceListNotif(devName, devAddr, isPaired);
}

/**
* @brief deviceListNotif: Updates the UI based on device lister feedback
*
* @param devName: Device name just discovered
* @param devAddr: Device MAC address
* @param paired:  Indicates if the device is paired or not
* @return         None
*/
void CTTAMIUpdaterDlg::deviceListNotif(const wchar_t *devName, BTH_ADDR devAddr, bool isPaired)
{
	AutoLock lock(uiDataCs);

	if (devName == NULL)                // end of refresh procedure
	{
		ManageEnables(IDC_DEVLIST_REFRESH_BUTTON, false);     // end refresh procedure
	}
	else
	{
		deviceList.emplace_back(devName, isPaired, devAddr);
		int devListIx = (int)deviceList.size() - 1;
		int index=0;
		if (isPaired == true)
		{
			index = m_deviceListBox.AddString(devName);	
			m_deviceListBox.SetItemData(index, devListIx);  // associate this string to the index in the list
		}


	}
}

/**
* @brief OnLbnSelchangeDevList: handler for the AMI devices list selection
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnLbnSelchangeDevList()
{
	AutoLock lock(uiDataCs);


#ifdef __PRODUCTION__

#else

	int index = m_deviceListBox.GetCurSel();
	int devListIx = (int)m_deviceListBox.GetItemData(index);   // get index of this string in deviceList

	assert(devListIx <= (int) deviceList.size());
	auto it = deviceList.begin();
	std::advance(it, devListIx);
	BTH_ADDR devAddr = it->addr;                // address of device to poll


//    devListErrMsg.SetWindowTextW(L"");          // clear error code

	// Clear info of former device info
	productId.SetWindowTextW(L"");
	serialNb.SetWindowTextW(L"");
	currentFwVersion.SetWindowTextW(L"");
	devInfoFwVer = L"";

	// Call this function after having set devInfoFwVer to make sure buttons get updated
	ManageEnables(IDC_DEVLIST_LIST, true);      // begin device poll procedure

	lock.Unlock();

	if (devInfoPoller)  delete devInfoPoller;
	devInfoPoller = new DeviceInfo(devAddr, deviceInfoNotifEntry, this);

#endif

}





/**
* @brief deviceInfoNotifEntry: Callback entry function used by the device info poller
*
* @param ctx:           Opaque context meaningful for the notification function
* @param productId:     Product ID of that device
* @param serialNb:      Serial number of that device
* @param firmwareVer:   Current firmware version of that device
* @param errMsg:        Any error message encountered during the polling
* @return         None
*/
void CTTAMIUpdaterDlg::deviceInfoNotifEntry(void *ctx, const wchar_t *productId, const wchar_t *serialNb, const wchar_t *firmwareVer, const wchar_t *errMsg)
{
	// NEVER DELETE THE devPoller IN THIS THREAD (DEAD LOCK CONDITION GUARANTEED)

	CTTAMIUpdaterDlg *pDlg = static_cast<CTTAMIUpdaterDlg *>(ctx);

	pDlg->deviceInfoNotif(productId, serialNb, firmwareVer, errMsg);
}

/**
* @brief deviceInfoNotif: Updates the UI based on device poller feedback
*
* @param productId:     Product ID of that device
* @param serialNb:      Serial number of that device
* @param firmwareVer:   Current firmware version of that device
* @param errMsg:        Any error message encountered during the polling
* @return         None
*/
void CTTAMIUpdaterDlg::deviceInfoNotif(const wchar_t *_productId, const wchar_t *_serialNb, const wchar_t *_firmwareVer, const wchar_t *errMsg)
{
	AutoLock lock(uiDataCs);

	devListErrMsg.SetWindowTextW(errMsg);           // print error message
	if (wcscmp(errMsg, L"") == 0)                   // if no error detected, update device info
	{
		productId.SetWindowTextW(_productId);
		serialNb.SetWindowTextW(_serialNb);
		currentFwVersion.SetWindowTextW(_firmwareVer);
		devInfoFwVer = _firmwareVer;
	}

	// Call this function after having set devInfoFwVer to make sure update button get activated
	ManageEnables(IDC_DEVLIST_LIST, false);         // end of poller procedure
}


/**
* @brief BatteryStatusNotif: Updates the UI based on device poller feedback
*
* @param productId:     Product ID of that device
* @param serialNb:      Serial number of that device
* @param firmwareVer:   Current firmware version of that device
* @param errMsg:        Any error message encountered during the polling
* @return         None
*/
/*
void CTTAMIUpdaterDlg::BatteryStatusNotifEntry(void *ctx, const  unsigned char * soc, const unsigned short *  voltage, const bool* charging, const wchar_t *errMsg)
{
	CTTAMIUpdaterDlg *pDlg = static_cast<CTTAMIUpdaterDlg *>(ctx);

	pDlg->BatteryStatusNotif(soc, voltage, charging, errMsg);
}
//////////////////////////////////////////////////////////////////////////////
void CTTAMIUpdaterDlg::BatteryStatusNotif(const  unsigned char * soc, const unsigned short *  voltage, const bool* charging, const wchar_t *errMsg)
{

}*/
///////////////////////////////////////////////////////////////////////////////
// Device Update section
///////////////////////////////////////////////////////////////////////////////

/**
* @brief OnBnClickedButtonUpdate: Update button pressed
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnBnClickedButtonUpdate()
{	
#ifdef __FW_UPGRADE_TOOL__
	return;
#endif
	updateProgressBar.SetPos(0);
	updateErrMsg.SetWindowTextW(L"");
	ManageEnables(IDC_UPDATE_BUTTON, true);
	MessageBox(IDS_PREREQUISITES, IDS_INFO, MB_OK);
#ifdef __PRODUCTION__
	unsigned char soc = 0;
	unsigned short v;
	bool charging = false;

	bool result = _productionHelper.GetBatteryStatus( soc, charging,v);
	if (!result)
	{
		MessageBox(IDS_CANT_COMM, IDS_ERROR, MB_OK);
		return;
	}

	CString str;

	str.Format(_T("%s %d%% %d mV "), IDS_BATTERY_LEVEL, (int)soc, v);
	if(charging)
		str.Format(_T("%s %s"), str, _T("Charging") );

	devListErrMsg.SetWindowTextW(str);

	return;
	if ((soc < 25 ) )
	{
		if (charging)
		{
			MessageBox(IDS_KEEP_CHGR_CONNECTED, IDS_WARNING, MB_OK);
		}
		else
		{
			MessageBox(IDS_BATTERY_ABOVE_25,  IDS_ERROR, MB_OK);
			return;
		}
	}



	_productionHelper.UpdateDevice(deviceUpdateFeedbackEntry, this, package, sizeof(package));
#else
	AutoLock lock(uiDataCs);
	int index = m_deviceListBox.GetCurSel();
	int devListIx = (int)m_deviceListBox.GetItemData(index);   // get index of this string in deviceList

	assert(devListIx <= (int)deviceList.size());
	auto it = deviceList.begin();
	std::advance(it, devListIx);
	BTH_ADDR devAddr = it->addr;                // address of device to poll


	//////////////// Requesting battery status
	bool Skip = false;
	IBatteryStatus * BatteryStatusPoller = new IBatteryStatus(devAddr);
	if (!BatteryStatusPoller->getError())
	{
		unsigned char SOC = BatteryStatusPoller->getSOC();
		bool charging = BatteryStatusPoller->getCharging();
		if (SOC < 25 )
		{
			if (charging)
			{
				MessageBox(IDS_KEEP_CHGR_CONNECTED, IDS_WARNING, MB_OK);
			}
			else
			{
				MessageBox(IDS_BATTERY_ABOVE_25, IDS_ERROR, MB_OK);
				Skip = true;
			}
		}
	}
	else
	{
		MessageBox(BatteryStatusPoller->getErrorMessage().c_str(), IDS_ERROR, MB_OK);
		Skip = true;
	}

	delete BatteryStatusPoller;
	///////////////
#ifndef __FW_UPGRADE_TOOL__
	ManageEnables(IDC_UPDATE_BUTTON, true);     // begin update procedure
#endif
	lock.Unlock();

	if (devUpdater != NULL)    delete devUpdater;

	if (!Skip)
		devUpdater = new DeviceUpdate(devAddr, deviceUpdateFeedbackEntry, this);

#endif
}

/**
* @brief deviceUpdateFeedbackEntry: Callback entry function used by the device updater
*       to notify the progress of the transfer.
*
* @param ctx:       this object
* @param percent:   the percentage of progress (0-100)
* @param errMsg:    the error encountered or the final message. This this is not "", the transfer is completed
* @return None.
*/
void CTTAMIUpdaterDlg::deviceUpdateFeedbackEntry(void *ctx, int percent, const wchar_t *errMsg, bool endProcedure)
{
	// NEVER DELETE THE devUpdater IN THIS THREAD (DEAD LOCK CONDITION GUARANTEED)

	CTTAMIUpdaterDlg *pDlg = static_cast<CTTAMIUpdaterDlg *>(ctx);

	pDlg->deviceUpdateFeedback(percent, errMsg, endProcedure);
}

/**
* @brief deviceUpdateFeedback: Updates the UI based on device updater feedback
*
* @param percent:   the percentage of progress (0-100)
* @param errMsg:    the error encountered or the final message. This this is not "", the transfer is completed
* @return None.
*/
void CTTAMIUpdaterDlg::deviceUpdateFeedback(int percent, const wchar_t *errMsg, bool endProcedure)
{
	AutoLock lock(uiDataCs);

	updateProgressBar.SetPos(percent);
	updateErrMsg.SetWindowTextW(errMsg);

	if (endProcedure)
	{
		ManageEnables(IDC_UPDATE_BUTTON, false);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Device Upgrade section
///////////////////////////////////////////////////////////////////////////////

/**
* @brief OnBnClickedButtonUpdate: Upgrade button pressed
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnBnClickedButtonUpgrade()
{
#ifdef __PRODUCTION__
 

	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	string f = myconv.to_bytes(upgradeKeyValue);
	_productionHelper.UpgradeDevice(deviceUpgradeFeedbackEntry, this, f.c_str(), strlen(f.c_str()));
#else
	AutoLock lock(uiDataCs);

	int index = m_deviceListBox.GetCurSel();
	int devListIx = (int)m_deviceListBox.GetItemData(index);   // get index of this string in deviceList

	assert(devListIx <= (int)deviceList.size());
	auto it = deviceList.begin();
	for (; devListIx != 0; devListIx--, ++it);
	BTH_ADDR devAddr = it->addr;                // address of device to poll

												//////////////// Requesting battery status
	bool Skip = false;

	ManageEnables(IDC_UPGRADE_BUTTON, true);     // begin update procedure
	lock.Unlock();

	if (devUpgrader != NULL)    delete devUpgrader;
	devUpgrader = new DeviceUpgrade(devAddr, deviceUpgradeFeedbackEntry, this);
#endif
}

/**
* @brief OnEnChangeUpgradeKeyEdit: Upgrade text box.
*
* @param None.
* @return None.
*/
void CTTAMIUpdaterDlg::OnEnChangeUpgradeKeyEdit()
{
	wchar_t buf[100];
	int len = upgradeKeyUI.GetLine(0, buf, 99);
	buf[len] = L'\0';       // terminate string
	upgradeKeyValue = buf;

	ManageEnables(0, false);        // Call the function to potentially enable the upgrade button
}

/**
* @brief deviceUpgradeFeedbackEntry: Callback entry function used by the device updater
*       to notify the progress of the transfer.
*
* @param ctx:       this object
* @param percent:   the percentage of progress (0-100)
* @param errMsg:    the error encountered or the final message. This this is not "", the transfer is completed
* @return None.
*/
void CTTAMIUpdaterDlg::deviceUpgradeFeedbackEntry(void *ctx, const wchar_t *errMsg)
{
	// NEVER DELETE THE devUpdater IN THIS THREAD (DEAD LOCK CONDITION GUARANTEED)

	CTTAMIUpdaterDlg *pDlg = static_cast<CTTAMIUpdaterDlg *>(ctx);

	pDlg->deviceUpgradeFeedback(errMsg);
}

/**
* @brief deviceUpgradeFeedback: Updates the UI based on device updater feedback
*
* @param percent:   the percentage of progress (0-100)
* @param errMsg:    the error encountered or the final message. This this is not "", the transfer is completed
* @return None.
*/
void CTTAMIUpdaterDlg::deviceUpgradeFeedback(const wchar_t *errMsg)
{
	AutoLock lock(uiDataCs);

	if (*errMsg != L'\0')                           // Either an error or the no error message will terminate the transfer
	{
		upgradeErrMsg.SetWindowTextW(errMsg);
		ManageEnables(IDC_UPGRADE_BUTTON, false);     // end update procedure
													 // We leave the devUpdater zombie here. Will be deleted on next attempt and at end of program.
	}
}


///////////////////////////////////////////////////////////////////////////////
// Management of visual behavior
///////////////////////////////////////////////////////////////////////////////

/**
* @brief ManageEnables: Enable/disable the buttons on the UI based on what activity is taking place
*
* @param idcButton: UI asset being touched
* @param begin:     true: the button has just been pressed. false: the procedure associated to that button just completed
* @return None.
*/
void CTTAMIUpdaterDlg::ManageEnables(int idcButton, bool begin)
{
	enum ME_t {                 // Manage Enable enum
		ME_ENABLE = TRUE,       // Enable button/window
		ME_DISABLE = FALSE      // Disable button/window
	};
	static const struct
	{
		int buttonIn;                       // button having an event on it (or its procedure is completed)
		bool begin;                         // true: procedure begins, false: procedure completes
		int buttonOut;                      // button to modify
		ME_t enable;                        // new state of this button
	} actionList[] =
	{
		{ IDC_DEVLIST_REFRESH_BUTTON, true,     IDC_DEVLIST_REFRESH_BUTTON, ME_DISABLE },   // While refresh is being performed
		// refresh button enable is handled outside the table
		// refresh does not prevent anything else because it is a long process (several seconds)
		{ IDC_DEVLIST_LIST,           true,     IDC_DEVLIST_LIST,           ME_DISABLE },   // When a device is selected in the list
		{ IDC_DEVLIST_LIST,           true,     IDC_UPDATE_BUTTON,          ME_DISABLE },
		{ IDC_DEVLIST_LIST,           true,     IDC_UPGRADE_BUTTON,         ME_DISABLE },
		{ IDC_DEVLIST_LIST,           true,     IDC_DEVLIST_REFRESH_BUTTON, ME_DISABLE },
		{ IDC_DEVLIST_LIST,           false,    IDC_DEVLIST_LIST,           ME_ENABLE },    // When a device polling is completed
		// update is handled below depending if the polling was successful or not
		// upgrade is handled below depending if the polling was successful or not and a key has been typed or not
		// refresh is handled below depending if polling is done and refresh is done
		{ IDC_UPDATE_BUTTON,          true,     IDC_DEVLIST_REFRESH_BUTTON, ME_DISABLE },   // During update procedure
		{ IDC_UPDATE_BUTTON,          true,     IDC_DEVLIST_LIST,           ME_DISABLE },
		{ IDC_UPDATE_BUTTON,          true,     IDC_UPDATE_BUTTON,          ME_DISABLE },
		{ IDC_UPDATE_BUTTON,          true,     IDC_UPGRADE_BUTTON,         ME_DISABLE },
		// refresh button enable is handled outside the table
		{ IDC_UPDATE_BUTTON,          false,    IDC_DEVLIST_LIST,           ME_ENABLE },    // when update procedure is completed
		{ IDC_UPDATE_BUTTON,          false,    IDC_UPDATE_BUTTON,          ME_ENABLE },
		// upgrade is handled below depending if a key has been typed or not
		{ IDC_UPGRADE_BUTTON,         true,     IDC_DEVLIST_REFRESH_BUTTON, ME_DISABLE },   // During the key upgrde procedure
		{ IDC_UPGRADE_BUTTON,         true,     IDC_DEVLIST_LIST,           ME_DISABLE },
		{ IDC_UPGRADE_BUTTON,         true,     IDC_UPDATE_BUTTON,          ME_DISABLE },
		{ IDC_UPGRADE_BUTTON,         true,     IDC_UPGRADE_BUTTON,         ME_DISABLE },
		// refresh button enable is handled outside the table
		{ IDC_UPGRADE_BUTTON,         false,    IDC_DEVLIST_LIST,           ME_ENABLE },    // when key upgrde procedure is completed
		{ IDC_UPGRADE_BUTTON,         false,    IDC_UPDATE_BUTTON,          ME_ENABLE },
		{ IDC_UPGRADE_BUTTON,         false,    IDC_UPGRADE_BUTTON,         ME_ENABLE },
		{ 0, 0,     0, ME_ENABLE },             // buttonIn = 0 -> end of list
	};

	for (int i = 0; actionList[i].buttonIn != 0; i++)
	{
		if ((actionList[i].buttonIn == idcButton) && (actionList[i].begin == begin))
		{
#ifdef __FW_UPGRADE_TOOL__
			if ((actionList[i].buttonOut != IDC_UPDATE_BUTTON))
				GetDlgItem(actionList[i].buttonOut)->EnableWindow(actionList[i].enable);
#else
			GetDlgItem(actionList[i].buttonOut)->EnableWindow(actionList[i].enable); // enable/disable button/window
#endif
		}
	}

	static bool refreshBegin = false;           // Indicates if refresh is currently occurring
	static bool devListBegin = false;           // Indicates if a device is being polled for info
	static bool updateBegin = false;            // Indicates if an update is in progress
	static bool upgradeBegin = false;           // Indicates if an upgrade is in progress

	if (idcButton == IDC_DEVLIST_REFRESH_BUTTON)    refreshBegin = begin;
	if (idcButton == IDC_DEVLIST_LIST)              devListBegin = begin;
	if (idcButton == IDC_UPDATE_BUTTON)             updateBegin = begin;
	if (idcButton == IDC_UPGRADE_BUTTON)            upgradeBegin = begin;

	// Restore refresh button if no other action is being performed
	if ((refreshBegin == false) && (devListBegin == false) && (updateBegin == false) && (upgradeBegin == false))
	{
		//GetDlgItem(IDC_DEVLIST_REFRESH_BUTTON)->EnableWindow(TRUE);     // enable button
	}

	// Restore upgrade key button if device is polled, key is present and no update/upgrade is being performed
	if ((upgradeKeyValue == L"") || (devInfoFwVer == L""))  GetDlgItem(IDC_UPGRADE_BUTTON)->EnableWindow(FALSE);    // disable button
	else if ((upgradeKeyValue != L"") && (devInfoFwVer != L"") && (updateBegin == false) && (upgradeBegin == false))
	{
		GetDlgItem(IDC_UPGRADE_BUTTON)->EnableWindow(TRUE);             // enable button
	}

	// Restore update key button if device is polled, different than this fw version and no update/upgrade is being performed
	if (devInfoFwVer == L"")                                GetDlgItem(IDC_UPDATE_BUTTON)->EnableWindow(FALSE);     // disable button
	else if ((devInfoFwVer != L"") && ((devInfoFwVer != PACKAGEVERSION) || (PACKAGEVERSION == L"0.0.0.0")) && (updateBegin == false) && (upgradeBegin == false))
	{
#ifndef __FW_UPGRADE_TOOL__
		GetDlgItem(IDC_UPDATE_BUTTON)->EnableWindow(TRUE);              // enable button
#endif
	}

	if (begin == true)                          // Any button pressed? Any procedure beginning?
	{
		updateProgressBar.SetPos(0);            // clear update progress bar
		updateErrMsg.SetWindowTextW(L"");       // clear update error message
		upgradeErrMsg.SetWindowTextW(L"");      // clear upgrade error message
		devListErrMsg.SetWindowTextW(L"");      // clear device list error message
	}
}

///////////////////////////////////////////////////////////////////////////////////
void CTTAMIUpdaterDlg::OnBnClickedOsp()
{
#ifdef __PRODUCTION__
	if (_productionHelper.Open())
	{

		m_deviceListBox.EnableWindow(FALSE);
		GetDlgItem(IDM_CSP)->EnableWindow(TRUE);
		GetDlgItem(IDM_OSP)->EnableWindow(FALSE);
		GetDlgItem(IDM_RDI)->EnableWindow(TRUE);
		PortIsOpened = true;
		_productionHelper.EnableHeartBeat();
	}
	else
	{
		MessageBox(L"Can't open selected FTDI port",  MB_OK);
		return;
	}
#endif
}
///////////////////////////////////////////////////////////////////////////////////
void CTTAMIUpdaterDlg::ReadDeviceInfo()
{
	//OnBnClickedButtonRefresh();
	//_productionHelper.EnableHeartBeat();

//	devListErrMsg.SetWindowTextW(_productionHelper.EnableHeartBeat()?_T("Online"):_T("Offline"));
	//return;
#ifdef	__PRODUCTION__
	DeviceInfoC device_info;
	bool result = _productionHelper.GetDeviceInfo(device_info);
	if (result)
	{
		productId.SetWindowTextW(CString(device_info.ProductNumber));
		serialNb.SetWindowTextW(CString(device_info.SerialNumber));
		currentFwVersion.SetWindowTextW(CString(device_info.FirmwareVersion));
		devInfoFwVer = CString(device_info.FirmwareVersion);
	}
	else
		return;
	unsigned char soc = 0;
	bool charging = false;
	unsigned short v;

	result = _productionHelper.GetBatteryStatus(soc, charging,v);
	if (!result)
	{
		return;
	}

	
	CString str;

	str.Format(_T("%s %d%% %d mV "), IDS_BATTERY_LEVEL, (int)soc, v);
	if (charging)
		str.Format(_T("%s %s"), str, _T("Charging"));

	devListErrMsg.SetWindowTextW(str);

	ManageEnables(IDC_DEVLIST_LIST, false);
#endif
}
///////////////////////////////////////////////////////////////////////////////////
void CTTAMIUpdaterDlg::OnBnClickedCsp()
{
#ifdef __PRODUCTION__
	PortIsOpened = false;
	_productionHelper.Close();
	
	m_deviceListBox.EnableWindow(TRUE);
	GetDlgItem(IDM_CSP)->EnableWindow(FALSE);
	GetDlgItem(IDM_OSP)->EnableWindow(TRUE);
	GetDlgItem(IDM_RDI)->EnableWindow(FALSE);
#endif
}



bool ComIsBusy = false;
UINT32 TickTimerCounter = 0;
UINT32 LastTickTimerCounter = 0;
bool charging;

void CTTAMIUpdaterDlg::OnTimer(UINT_PTR nIDEvent)
{
	TickTimerCounter += _BAT_MON__TIMER_TICK_;
	UINT32 deltaT = TickTimerCounter - LastTickTimerCounter;
	if (ComIsBusy)return;
	ComIsBusy = true;
	if (PortIsOpened)
	{
		devListErrMsg.SetWindowTextW(_T("Connecting..."));
		UpdateData(FALSE);
		ReadDeviceInfo();
		UpdateData(FALSE);
	}
	else
	{
		//OnBnClickedButtonRefresh();
	}
	if (deltaT > 3000)
	{
		LastTickTimerCounter = TickTimerCounter;
		charging = !charging;
		if(_productionHelper.isBatStatValid())
			_productionHelper.Set5V_Relay(charging); //_productionHelper.LastBatStat.Charging);

	}
	CDialog::OnTimer(nIDEvent);
	ComIsBusy = false;
}
