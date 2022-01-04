/*
* lang.h : String to print on the UI
*
*   All strings printable on the UI are declared here in English.
*	They will be used in the language files for their equivalent in the other languages.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/

#ifndef _LANG_H
#define _LANG_H

// These defines are the real text in english (and the defines are used to translate in other languages)
#define TXT_DEVLIST				_T("FTDI port list")
#define TXT_PRODUCT				_T("Product")
#define TXT_SERNUM				_T("Serial Number")
#define TXT_CURFWVER			_T("Current firmware version")
#define TXT_NEWFWVER			_T("Firmware version to update")
#define TXT_UPGRKEY				_T("Upgrade key")
#define TXT_REFRESH_BTN			_T("Refresh")
#define TXT_UPDATE_BTN			_T("Update")
#define TXT_UPGRADE_BTN			_T("Upgrade")
#define TXT_UPDATE_INFO			_T("Starting update. It may take several seconds to start.")
#define TXT_EXIT_BTN			_T("Exit")
#define TXT_ERR_DEV_SHORTMSG    _T("A communication error occurred. Please try again. Error code: 001")	//Device detected a message too short
#define TXT_ERR_DEV_INVCMD      _T("A communication error occurred. Please try again. Error code: 002")	//Device detected an invalid command
#define TXT_ERR_DEV_ERASING		_T("An error occurred. Please try again. Error code: 003")	//Device error erasing memory
#define TXT_ERR_DEV_PROGRAMMING	_T("An error occurred. Please try again. Error code: 004")	//Device error programming memory
#define TXT_ERR_DEV_READING		_T("An error occurred. Please try again. Error code: 005")	//Device error reading memory
#define TXT_ERR_CRC_DWLD		_T("An error occurred. Please try again. Error code: 006")	//Integrity failed during transfer
#define TXT_ERR_CRC_SAVE		_T("An error occurred. Please try again. Error code: 007")	//Integrity failed during programming
#define TXT_ERR_INCOMPATIBLE	_T("An error occurred. Please try again. Error code: 008")	//Incompatible firmware
#define TXT_ERR_DEVICE			_T("An error occurred. Please try again. Error code: 009")	//Device error
#define TXT_ERR_DONE			_T("The update was completed successfully.")
#define TXT_ERR_CONNECTFAIL		_T("Connection failure. Please try again.")
#define TXT_ERR_UNREACHABLE		_T("Connection failure. Please try again or pair the device again.")
#define TXT_ERR_SENDFAIL		_T("A communication error occurred. Please try again. Error code: 010")	//Send fail
#define TXT_ERR_NOANSWER		_T("No response was detected from the device. Please try again.")
#define TXT_ERR_ABORTED			_T("Transfer stopped.")
#define TXT_ERR_RXFAIL			_T("A communication error occurred. Please try again. Error code: 011")	//Receive fail
#define TXT_ERR_INV_MSGTYPE		_T("A communication error occurred. Please try again. Error code: 012")	//Received invalid message type
#define TXT_ERR_INV_RESPLEN		_T("A communication error occurred. Please try again. Error code: 013")	//Received invalid length response
#define TXT_ERR_INV_KEY			_T("The upgrade key was invalid. Please enter a valid key.")
#define TXT_ERR_INFO_GATHER		_T("A communication error occurred. Please try again. Error code: 014")	//Info gathering send error
#define TXT_ERR_ALREADY_EXTENDED		_T("Your device is already Extended")	//Device has  already extended functionality

typedef struct
{
	LPCTSTR englishStr;			// English string
	LPCTSTR langStr;			// Same string in proper language
} LangMsg;

void langInit(WCHAR *curLang);
LPCTSTR langGet(LPCTSTR engStr);

#endif // _LANG_H
