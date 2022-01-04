/*
* lang.cpp : Module to convert the string to print on the UI in the proper language
*
*   All strings printable on the UI are declared here in English.
*	They will be used in the language files for their equivalent in the other languages.
*
* Author: Luc Tremblay
* Project: AMI
* Company: Orthogone Technologies inc.
*/
#include <stdafx.h>
#include "lang.h"

extern const LangMsg langFrench[];		// from langFrench.cpp
extern const LangMsg langKorean[];		// from langKorean.cpp

const LangMsg *curLang = NULL;

/**
* @brief langInit: Sets the language for this session
*
* @param reqLang:           English name of the language to use (as it is returned by GetLocaleInfoEx())
* @return None.
*/
void langInit(WCHAR *reqLang)
{
	if (wcscmp(reqLang, L"French") == 0)			curLang = langFrench;
	else if (wcscmp(reqLang, L"Korean") == 0)		curLang = langKorean;
}

/**
* @brief langGet: Returns the appropriate string in the language selected for the provided
*                 english string.
*
* @param engStr:           The string in English. See lang.h for valid English strings
* @return The string in the proper language or the English version if no string found.
*/
LPCTSTR langGet(LPCTSTR engStr)
{
	LPCTSTR retStr = NULL;
	if (curLang != NULL)
	{
		for (int i = 0; curLang[i].englishStr != NULL; i++)
		{
			if (wcscmp(engStr, curLang[i].englishStr) == 0)
			{
				retStr = curLang[i].langStr;
				break;
			}
		}
	}
	if (retStr == NULL)		retStr = engStr;		// if nothing found, return english string

	return retStr;
}
