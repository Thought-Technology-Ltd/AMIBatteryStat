/*
* TT_AMI_UpdaterDlg.h : header file for main windoww dialog
*
* Author: Francis Choquette
* Project: AMI
* Company: Orthogone Technologies inc.
*/

#pragma once
#include <afxwin.h>
#include <afxcmn.h>
#include <list>
#include <string>
#include "DeviceUpdate.h"
#include "DeviceUpgrade.h"
#include "DeviceList.h"
#include "DeviceInfo.h"
#include "BatteryStatus.h"

#ifdef __PRODUCTION__
	#include "ProductionHelper.h"
#endif





// DeviceElem:  Structure to hold the name of a device along with its pairing status and its MAC address
struct DeviceElem
{
    DeviceElem(const wchar_t *_name, bool _isPaired, BTH_ADDR _addr)
        : name(_name), isPaired(isPaired), addr(_addr) {}

    std::wstring name;      // Name of the device
    bool isPaired;          // is device paired?
    BTH_ADDR addr;          // Device MAC address
};

// AutoLock:    class used to lock the critical section with an implicit unlock when the function exits.
class AutoLock
{
public:
    AutoLock(CCriticalSection &lock)
    {
        aLock = &lock;
        aLock->Lock();
        aIsLocked = true;
    }

    virtual ~AutoLock()
    {
        if (aIsLocked == true)  aLock->Unlock();
    }

    void Unlock(void)
    {
        aLock->Unlock();
        aIsLocked = false;
    }
private:
    CCriticalSection *aLock;
    bool aIsLocked;             // when true, lock is taken
};

// CTTAMIUpdaterDlg dialog
class CTTAMIUpdaterDlg : public CDialog
{
// Construction
public:
	CTTAMIUpdaterDlg(LPTSTR cmdLine, CWnd* pParent = nullptr);	// standard constructor
	bool PortIsOpened = false;
    virtual ~CTTAMIUpdaterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TT_AMI_UPDATER_DIALOG	};
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	#ifdef __PRODUCTION__
	ProductionHelper _productionHelper;
	#endif

	// Generated message map functions
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    static void deviceListNotifEntry(void *ctx, const wchar_t *devName, BTH_ADDR devAddr, bool isPaired);
    void deviceListNotif(const wchar_t *devName, BTH_ADDR devAddr, bool isPaired);
    static void deviceInfoNotifEntry(void *ctx, const wchar_t *productId, const wchar_t *serialNb, const wchar_t *firmwareVer, const wchar_t *errMsg);
    void deviceInfoNotif(const wchar_t *_productId, const wchar_t *_serialNb, const wchar_t *_firmwareVer, const wchar_t *errMsg);
	static void BatteryStatusNotifEntry(void *ctx, const  unsigned char * soc, const unsigned short *  voltage, const bool* charging, const wchar_t *errMsg);
	void BatteryStatusNotif(const  unsigned char * soc, const unsigned short *  voltage, const bool* charging, const wchar_t *errMsg);

    static void deviceUpdateFeedbackEntry(void *ctx, int percent, const wchar_t *errMsg, bool endProcedure);
	static void deviceUpgradeFeedbackEntry(void *ctx, const wchar_t *errMsg);
    void deviceUpdateFeedback(int percent, const wchar_t *errMsg, bool endProcedure);
	void deviceUpgradeFeedback(const wchar_t *errMsg);

    void ManageEnables(int idcButton, bool begin);

public:
#ifdef __PRODUCTION__
	static void ProductionNotify(ProductionHelper::Notifications id, void  * reserved, void  * parameter);
#endif

	WCHAR wcLocale[LOCALE_NAME_MAX_LENGTH];
	const CString csLocales[2] = { L"en-US", L"fr-CA" };
	LPTSTR cmdLine;						// Command line arguments

    CCriticalSection uiDataCs;          // Critical section to protect UI data from simultaneous changes and display

    std::list<DeviceElem> deviceList;   // List of device detected on bluetooth with its associated data
    CListBox m_deviceListBox;           // List shown on screen
    DeviceList *devLister;              // Device listing procedure instance
    DeviceInfo *devInfoPoller;          // Device information poll procedure instance
    std::wstring devInfoFwVer;          // Firmware version received from API (from device information poller)

    DeviceUpdate *devUpdater;           // Device update procedure instance
	DeviceUpgrade *devUpgrader;			// Device upgrade procedure instance

    CEdit upgradeKeyUI;                 // Upgrade key UI object
    std::wstring upgradeKeyValue;       // Upgrade key obtained from UI

    afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedExit();
	afx_msg void OnLbnSelchangeDevList();
    afx_msg void OnBnClickedButtonUpdate();
    afx_msg void OnBnClickedButtonUpgrade();
    CProgressCtrl updateProgressBar;
    CProgressCtrl upgradeProgressBar;
    CStatic updateErrMsg;
	CStatic upgradeErrMsg;
    CStatic productId;
    CStatic serialNb;
    CStatic currentFwVersion;
    CStatic nextFwVersion;
    CStatic devListErrMsg;
    afx_msg void OnEnChangeUpgradeKeyEdit();
	afx_msg void OnBnClickedOsp();
	afx_msg void ReadDeviceInfo();
	afx_msg void OnBnClickedCsp();


	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
