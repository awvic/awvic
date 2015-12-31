/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVDICTIONARY_H
#define _SVDICTIONARY_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <map>
#include <vector>
#include <utility>

#include "svCommonLib.h"
#include "svBaseType.h"

#include "unicode/utypes.h"
#include "unicode/ustring.h"

using namespace std;

#define SVID_MAX_HINT 7

typedef struct svVocabulary
{
    wxString text;
    int32_t count;
    svVocabulary()
    {
        count = 0;
    }
    svVocabulary(const svVocabulary& obj)  // copy constructor
    {
        text = obj.text;
        count = obj.count;
    }
    svVocabulary& operator=(const svVocabulary& obj)
    {
        text = obj.text;
        count = obj.count;

        return *this;
    }
   ~svVocabulary()
    {
    }
} svVocabulary;

/*typedef struct UCharText
{
    UChar *uctext;
    int len;
    UCharText()
    {
        uctext = NULL;
        len = 0;
    }
    ~UCharText()
    {
        if (uctext) free(uctext);
    }
    UCharText(const UCharText& obj)  // copy constructor
    {
        if (obj.uctext)
        {
            uint32_t tlen = obj.len;
            uctext = (UChar *)malloc(sizeof(UChar) * tlen);
            u_memcpy(uctext, obj.uctext, tlen);
        }
        else
            uctext = NULL;

        len = obj.len;
    }
    UCharText& operator=(const UCharText& obj)
    {
        if (uctext) free(uctext);
        if (obj.uctext)
        {
            uint32_t tlen = obj.len;
            uctext = (UChar *)malloc(sizeof(UChar) * tlen);
            u_memcpy(uctext, obj.uctext, tlen);
        }
        else
            uctext = NULL;

        len = obj.len;

        return *this;
    }

} UCharText;

// sort 用，只須提供 < 即可
inline
bool operator< (const UCharText &obj1, const UCharText &obj2)
{
    UErrorCode status = U_ZERO_ERROR;

    // int32_t c01 = u_strCaseCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, 0, &status);
    // int32_t c02 = u_strCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, true);

    // if (c01<0 || c02<0)
    //     return true;
    // else
    //     return false;

    // 先不比較大小寫
    // if (u_strCaseCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, 0, &status)<0)
    // {
    //     return true;
    // }
    // else
    // {
    //     // if(!U_SUCCESS(status))
    //     //     wxLogMessage(wxString::Format("svDictionary::operator small than : u_strCaseCompare failed %i", status));

        // 第5個參數為 true 或 false 尚待確定
        if (u_strCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, true)<0)
            return true;
        else
            return false;
    // }
}

// inline
// bool operator== (const UCharText &obj1, const UCharText &obj2)
// {
//     // 第5個參數為 true 或 false 尚待確定
//     if (u_strCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, true)==0)
//         return true;
//     else
//         return false;
// }

// inline
// bool operator> (const UCharText &obj1, const UCharText &obj2)
// {
//     UErrorCode status = U_ZERO_ERROR;

//     // 先不比較大小寫
//     if (u_strCaseCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, 0, &status)>0)
//     {
//         return true;
//     }
//     else
//     {
//         if(!U_SUCCESS(status))
//             wxLogMessage(wxString::Format("svDictionary::operator greater than : u_strCaseCompare failed %i", status));

//         // 第5個參數為 true 或 false 尚待確定
//         if (u_strCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, true)>0)
//             return true;
//         else
//             return false;
//     }
// }*/

typedef map<UCharText, svVocabulary> innerMap;

// class svDictionary 是跟據 所有 svLineText 的 m_hintList 找到的 keyword
// 組成一個 map<UCharText, map<UCharText, svVocabulary>> 的資料結構 
// 供 user 在輸入字元時的提示可用字串之用
// 為了將 大小字元混合在一起，map 的第一層 key 是一律使用大寫
// 第二層的 map 則存放 有大小寫的 UChar wxString 以及其計數
// 如 文字檔內如有 xy xY Xy XY 這4個字串，
// 則資料存放在 m_dict (map<UCharText, map<UCharText, svVocabulary>>) 會如下圖：
// XY -> {(XY-> (wx("XY"), 1)), (Xy-> (wx("Xy"), 1)), (xY-> (wx("xY"), 1)), (xy-> (wx("xy"), 1))}
//
// Call GetHitTextList 傳入指定的 UChar * 會回傳可能的提示字串
class svDictionary
{
public:
    svDictionary();
    ~svDictionary();
    void Add(const UCharText &p_key);
    void Add(const UChar *p_ktext, const size_t p_klen);
    void Sub(const UCharText &p_key);
    void Sub(const UChar *p_ktext, const size_t p_klen);
    void Remove(const UCharText &p_key);
    int Find(const UCharText &p_key);
    vector<wxString> GetHitTextList(const UChar *p_ktext, const size_t p_klen);
    vector<wxString> GetHitTextList(const UCharText &p_key);

    static
    wxString UChar2WxStr(const UCharText &p_text)
    {
        svVocabulary voc;

        // Convert UChar to char* encoding in UTF-8

        char *result=NULL;

        result = (char *)malloc(sizeof(char)*p_text.len*3); // UTF-8 uses max 3 bytes per char

        UErrorCode status = U_ZERO_ERROR;
        int32_t len = 0;

        u_strToUTF8(
            result,
            p_text.len*3,
            &len,
            p_text.uctext,
            p_text.len,
            &status
        );

        if(!U_SUCCESS(status))
        {
            wxLogMessage(wxString::Format("svDictionary::UChar2WxStr : u_strToUTF8 failed %i", status));
            if (result) free(result);
            result = NULL;
            len = 0;
        }
        else
        {
            result = (char*) realloc (result, len * sizeof(char));
        }

        wxString wxtext = wxString::FromUTF8(result, len);

        if (result) free(result);

        return wxtext;
    }

    static
    UCharText UChar2Lower(const UCharText &p_text)
    {

        UErrorCode status = U_ZERO_ERROR;
        int32_t len = 0;

        UCharText dest;
        dest.uctext = NULL;
        dest.len = 0;

        dest.len = p_text.len;
        dest.uctext = (UChar *) malloc(sizeof(UChar) * p_text.len);

        u_strToLower(dest.uctext, dest.len, p_text.uctext, p_text.len, NULL, &status);

        if(!U_SUCCESS(status))
        {
            wxLogMessage(wxString::Format("svDictionary::UChar2UpperCase : u_strToLower failed %i", status));
            if (dest.uctext) free(dest.uctext);
            dest.uctext = NULL;
            dest.len = 0;
        }

        return dest;
    }

    static
    UCharText UChar2Upper(const UCharText &p_text)
    {

        UErrorCode status = U_ZERO_ERROR;
        int32_t len = 0;

        UCharText dest;
        dest.uctext = NULL;
        dest.len = 0;

        dest.len = p_text.len;
        dest.uctext = (UChar *) malloc(sizeof(UChar) * p_text.len);

        u_strToUpper(dest.uctext, dest.len, p_text.uctext, p_text.len, NULL, &status);

        if(!U_SUCCESS(status))
        {
            wxLogMessage(wxString::Format("svDictionary::UChar2UpperCase : u_strToLower failed %i", status));
            if (dest.uctext) free(dest.uctext);
            dest.uctext = NULL;
            dest.len = 0;
        }

        return dest;
    }

private:
    map<UCharText, innerMap > m_dict;
};

#endif
