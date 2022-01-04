/*
* TT_AMI_Updater.h : main header file for the application
*
* Author: Francis Choquette
* Project: AMI
* Company: Orthogone Technologies inc.
*/

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CTTAMIUpdaterApp:
// See TT_AMI_Updater.cpp for the implementation of this class
//

class CTTAMIUpdaterApp : public CWinApp
{
public:
	CTTAMIUpdaterApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTTAMIUpdaterApp theApp;
