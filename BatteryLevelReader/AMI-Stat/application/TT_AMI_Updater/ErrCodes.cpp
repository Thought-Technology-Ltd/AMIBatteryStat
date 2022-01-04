/*
* ErrCodes.cpp : This file contains the function to translate an error condition into
*           a text understandable by the user (or at the least the tech support)
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include "stdafx.h"
#include "lang.h"
#include "ErrCodes.h"

/**
* @brief ErrTranslate: convert an error code into a printable text
*
* @param errCode:       error code detected
* @param basicMsg:      Text to print if error code is not found
* @return The associated string to display to user
*/
std::wstring ErrTranslate(int errCode, LPCTSTR basicMsg)
{
    std::wstring retStr;
    switch (errCode)
    {
        case ERR_OK:                break;
        case ERR_SPP_CLOSING:       retStr = langGet(TXT_ERR_ABORTED);          break;
        case ERR_SLIP_FRAMING:      retStr = langGet(TXT_ERR_RXFAIL);           break;
        case ERR_SLIP_TIMEOUT:      retStr = langGet(TXT_ERR_NOANSWER);         break;
        case ERR_SLIP_BUFSHORT:     retStr = langGet(TXT_ERR_INV_RESPLEN);      break;  // Slip frame too big for command sent
        case ERR_INV_RESPLEN:       retStr = langGet(TXT_ERR_INV_RESPLEN);      break;  // frame received does not match expected length

        // Windows errors are negated to have negative values for error conditions
        case -WSAETIMEDOUT:         retStr = langGet(TXT_ERR_CONNECTFAIL);      break;  // Error obtained while trying to connect
        case -WSAENETUNREACH:       retStr = langGet(TXT_ERR_UNREACHABLE);      break;  // Error obtained while trying to connect
        default:
        {
            CString formattedErr;
            formattedErr.Format(L": %d", errCode);
            retStr = langGet(basicMsg) + formattedErr;
        }
    }
    return retStr;
}
