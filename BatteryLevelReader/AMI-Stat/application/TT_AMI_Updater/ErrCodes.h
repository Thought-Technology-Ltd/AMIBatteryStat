/*
* ErrCodes.h : Error codes used in the Updater application
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#ifndef _ERR_CODES_H
#define _ERR_CODES_H

#include <string>

// Internal errors
#define ERR_OK                  0   // No error
#define ERR_SPP_CLOSING         -1  // The serial port object is being destroyed
#define ERR_SLIP_FRAMING        -2  // SLIP framing error (ESC byte not followed by proper valid byte)
#define ERR_SLIP_TIMEOUT        -3  // No full frame received within allocated time
#define ERR_SLIP_BUFSHORT       -4  // Buffer provided is too short to hold a complete SLIP frame
#define ERR_INV_RESPLEN         -5  // Invalid response length

/**
* @brief ErrTranslate: convert an error code into a printable text
*
* @param errCode:       error code detected
* @param basicMsg:      Text to print if error code is not found
* @return The associated string to display to user
*/
std::wstring ErrTranslate(int errCode, LPCTSTR basicMsg);

#endif // _ERR_CODES_H
