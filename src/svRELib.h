/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVRELIB_H
#define _SVRELIB_H

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

// #include "unicode/uchar.h"
// #include "unicode/ustring.h"
#include "unicode/utypes.h"
// #include "unicode/ucsdet.h"
// #include "unicode/ucnv.h"     /* C ICU Converter API    */

#include <vector>
using namespace std;

#include "svBaseType.h"

class svRELib
{
private:


public:

    svRELib();
    ~svRELib();

    //
    // Regular Expression matching specified pattern.
    // for /* and */
    //
    static
    UErrorCode svRELib::ICU_RE_Pattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end, vector<keywordUnit> *p_keywordList, vector<keywordUnit> *p_bkeywordList);

    /*
     * Regular Expression matching specified pattern.
     */
    static
    UErrorCode svRELib::ICU_RE_Pattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList);

    /*
     * Regular Expression matching specified keyword.
     */
    static
    UErrorCode svRELib::ICU_RE_Pattern_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList);

    /*
     * Regular Expression matching specified keyword.
     */
    static
    UErrorCode svRELib::ICU_RE_Pattern_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_hintList);

    // m_keywordList shoud be order by k_pos
    inline static
    bool svRELib::InsertKeywordUnit(vector<keywordUnit>* p_keywordList, const keywordUnit *p_ku)
    {
        p_keywordList->push_back(*p_ku);
        return true;
    }

    // m_keywordList shoud be order by k_pos
    // bool svRELib::InsertKeywordUnit2(vector<keywordUnit>* p_keywordList, const keywordUnit& p_ku)
    // {

    //     p_keywordList->push_back(p_ku);

    //     return true;
    // }

    // p_hintList shoud be order by k_pos
    inline static
    bool svRELib::InsertHintUnit(vector<keywordUnit>* p_hintList, const keywordUnit *p_ku)
    {
        p_hintList->push_back(*p_ku);
        return true;
    }

    // p_hintList shoud be order by k_pos
    // bool svRELib::InsertHintUnit2(vector<keywordUnit>* p_hintList, const keywordUnit& p_ku)
    // {
    //     p_hintList->push_back(p_ku);

    //     return true;
    // }


    // m_keywordList shoud be order by k_pos
    inline static
    bool svRELib::InsertBlockTagKeywordUnit(vector<keywordUnit>* p_blockTagList, const keywordUnit *p_ku)
    {
        p_blockTagList->push_back(*p_ku);
        return true;
    }


};

#endif
