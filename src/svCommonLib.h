/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVCOMMONLIB_H
#define _SVCOMMONLIB_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#pragma warning(disable: 4996)
#endif

// #include <vector>
// using namespace std;

// #include "unicode/uchar.h"
// #include "unicode/ucnv.h"     /* C   Converter API    */

enum
{
    SVID_BIG_ENDIAN=0,
    SVID_LITTLE_ENDIAN=1
};

class svCommonLib
{
private:


public:

    svCommonLib();
    ~svCommonLib();

    inline static
    wxString CharFilename2wxStr(const char *p_filename)
    {
        /*
         * **Only for filename convertion.
         * The filename convertion rule from char* to wxString is defferent
         * from Linux to Windows.
         * So we add this funtion.
         */

        /*
         * In windows filename is encoding in CP950(In traditional chinese version, other language version needed to be test).
         * In Morden Unix(Linux, OSX) filename is encoding in UTF8.
         * So Wi have different filename converting from wxString.
         */

#ifdef __WXMSW__
        wxString filename = wxString::FromAscii(p_filename);
#else
        wxString filename = wxString(p_filename, wxConvUTF8);
#endif

        return filename;

    }

    inline static
    const char *wxStrFilename2Char(const wxString& p_filename)
    {
        /*
         * **Only for filename convertion.
         * The filename convertion rule from char* to wxString is defferent
         * from Linux to Windows.
         * So we add this funtion.
         */

        /*
         * In windows filename is encoding in CP950.
         * In Morden Unix(Linux, OSX) filename is encoding in UTF8.
         * So Wi have different filename converting from wxString.
         */
#ifdef __WXMSW__
        return p_filename.mb_str();
#else
        return p_filename.ToUTF8();
#endif

    }

    // inline static
    // UChar *ICUstrdup(UChar *in) {
    //     uint32_t len = u_strlen(in) + 1;
    //     UChar *result = (UChar *)malloc(sizeof(UChar) * len);
    //     u_memcpy(result, in, len);
    //     return result;
    // }


    // static
    // UErrorCode ConvertwxStrToUChar(const wxString &p_str, UChar *p_ubuf, int &p_ubufLen);


    inline static
    const bool wxStrMatchPattern(const wxString &p_str, const wxString &p_pattern)
    {
        /*
            If a wxString match a specified pattern in sequential.

            for example:
    
            wxString                   pattern          result 
            aabbccddeeff               abc              true 
            aabbccddeeff               abccce           false
            aabbccddeeff               aa               true
        */

        if (p_str.Length()==0) return false;
        if (p_pattern.Length()==0) return true;

        int strLen = p_str.Length();
        int patLen = p_pattern.Length();

        int strIdx = 0;
        int patIdx = 0;

        while (strIdx<strLen && patIdx<patLen)
        {
            if (p_pattern.Mid(patIdx, 1).Lower()==p_str.Mid(strIdx, 1).Lower())
            {
                ++patIdx;
                ++strIdx;
            }
            else
            {
                ++strIdx;
            }
        }

        if (patIdx==patLen)
            return true;
        else
            return false;
    }

};

#endif