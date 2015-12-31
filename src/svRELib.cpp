/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include "svRELib.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode/utypes.h"
#include "unicode/ucsdet.h"
#include "unicode/ucnv.h"     /* C ICU Converter API    */
#include "unicode/uregex.h"

#include "stdwx.h"
// #include "svBaseType.h"

svRELib::svRELib()
{
}

svRELib::~svRELib()
{
}

//
// Regular Expression matching specified pattern.
// for /* and */
//
// static
UErrorCode svRELib::ICU_RE_Pattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end, vector<keywordUnit> *p_keywordList, vector<keywordUnit> *p_bkeywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_0 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    UChar *ustart = NULL;
    UChar *uend   = NULL;
    int32_t startLen = 0;
    int32_t endLen = 0;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    ustart = (UChar*) malloc (sizeof(UChar)*(strlen(start)+1));
    u_charsToUChars(start, ustart, strlen(start)+1);
    startLen = strlen(start);

    uend = (UChar*) malloc (sizeof(UChar)*(strlen(end)+1));
    u_charsToUChars(end, uend, strlen(end)+1);
    endLen = strlen(end);

    URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize); coold
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE0 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);        
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)
        if (p_regroup<=gCnt)
        { 
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);        
                }   
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);        
                }   
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg);        
                }   
                // fprintf(stdout, "Regex Match(g=%d):", i);
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                bool inserted = svRELib::InsertKeywordUnit(p_keywordList, &ku);
                // ku.k_text = NULL;

                // Insert into start/end tag into block tag vector.
                if (inserted)
                {
                    keywordUnit tku;
                    // tku.k_text = dest;
                    tku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                    u_memcpy(tku.k_text, dest, (sPos-s));
                    if (u_strncmp(dest, ustart, startLen)==0)
                    {
                        // start tag.
                        tku.k_pid = SVID_BLOCK_START;
                    }
                    else // if not /* then */
                    {
                        // end tag.
                        tku.k_pid = SVID_BLOCK_END;
                    }
                    tku.k_pos = s;
                    tku.k_len = sPos - s;
                    svRELib::InsertBlockTagKeywordUnit(p_bkeywordList, &tku);
                    // tku.k_text = NULL;
                }
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE0 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE0 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (ustart) free(ustart);
    if (uend) free(uend);
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_0 end");
// #endif
    return U_ZERO_ERROR;
}


/*
 * Regular Expression matching specified pattern.
 */
// static
UErrorCode svRELib::ICU_RE_Pattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_1 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    char const* tab = "\t";
    UChar *utab = NULL;

    char const* space = " ";
    UChar *uspace = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    utab = (UChar*) malloc (sizeof(UChar)*(strlen(tab)+1));
    u_charsToUChars(tab, utab, strlen(tab)+1);
    int tablen = strlen(tab);

    uspace = (UChar*) malloc (sizeof(UChar)*(strlen(space)+1));
    u_charsToUChars(space, uspace, strlen(space)+1);
    int spacelen = strlen(space);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize); coold
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE1 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);        
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        //for (int32_t i=0; i<1; i++)
        // we only process the regroup specified.
        if (p_regroup<=gCnt)
        { 
            for(int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg);
                }
                // fprintf(stdout, "Regex Match(g=%d):", i);
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);

// #ifndef NDEBUGs
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 i=%ld", i));
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 u_strlen(dest)=%ld", u_strlen(dest)));
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 sPos-s=%ld", sPos-s));
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 sPos=%ld", sPos));
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_1 s=%ld", s));
// #endif

                // pattern 1 have to consider \t

                int32_t old_x = 0;
                // int32_t textLen = u_countChar32(dest,  u_strlen(dest));
                int32_t textLen = sPos - s;
                for(int32_t i=0; i<textLen; i++)
                {
                    // break /t and space for symbol(tab space) display.
                    if (u_strncmp(dest+i, utab, 1)==0)
                    {
                        // break data
                        if (i!=old_x)
                        {
                            UChar* dst = NULL;
                            dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                            u_strncpy(dst, dest+old_x, i-old_x);
                            keywordUnit ku;
                            // ku.k_text = dst;
                            ku.k_text = (UChar *)malloc(sizeof(UChar) * (i-old_x));
                            u_memcpy(ku.k_text, dst, (i-old_x));
                            ku.k_pid = p_pid;      
                            ku.k_pos = s+old_x;
                            ku.k_len = i-old_x;
                            svRELib::InsertKeywordUnit(p_keywordList, &ku);
                            // ku.k_text = NULL;
                            free(dst);
                        }

                        // break /t
                        {
                        keywordUnit xku;
                        // xku.k_text = utab;
                        xku.k_text = (UChar *)malloc(sizeof(UChar) * tablen);
                        u_memcpy(xku.k_text, utab, tablen);
                        xku.k_pid = p_pid;      
                        xku.k_pos = s+i;
                        xku.k_len = 1;
                        svRELib::InsertKeywordUnit(p_keywordList, &xku);
                        // xku.k_text = NULL;
                        }

                        old_x = i+1;
                    }
                    else if (u_strncmp(dest+i, uspace, 1)==0)
                    {
                        // break data
                        if (i!=old_x)
                        {
                            UChar* dst = NULL;
                            dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                            u_strncpy(dst, dest+old_x, i-old_x);
                            keywordUnit ku;
                            // ku.k_text = dst;
                            ku.k_text = (UChar *)malloc(sizeof(UChar) * (i-old_x));
                            u_memcpy(ku.k_text, dst, (i-old_x));
                            ku.k_pid = p_pid;      
                            ku.k_pos = s+old_x;
                            ku.k_len = i-old_x;
                            svRELib::InsertKeywordUnit(p_keywordList, &ku);
                            // ku.k_text = NULL;
                            free(dst);
                        }

                        // break space
                        {
                        keywordUnit xku;
                        // xku.k_text = uspace;
                        xku.k_text = (UChar *)malloc(sizeof(UChar) * spacelen);
                        u_memcpy(xku.k_text, uspace, spacelen);
                        xku.k_pid = p_pid;      
                        xku.k_pos = s+i;
                        xku.k_len = 1;
                        svRELib::InsertKeywordUnit(p_keywordList, &xku);
                        // xku.k_text = NULL;
                        }

                        old_x = i+1;
                    }                
                }

                // 句尾的處理
                // 要考慮是否包含 /r/n 的情況
                // regular expression rule 要有相對應的設定
                if (old_x!=textLen)
                {
                    int32_t i = textLen;
                    UChar* dst = NULL;
                    dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                    u_strncpy(dst, dest+old_x, i-old_x);
                    keywordUnit ku;
                    // ku.k_text = dst;
                    ku.k_text = (UChar *)malloc(sizeof(UChar)*(i-old_x));
                    u_memcpy(ku.k_text, dst, (i-old_x));
                    ku.k_pid = p_pid;
                    ku.k_pos = s+old_x;
                    ku.k_len = i-old_x;
                    svRELib::InsertKeywordUnit(p_keywordList, &ku);
                    // ku.k_text = NULL;
                    free(dst);
                }

                free(dest);
            }
        }
        else  // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE1 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE1 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (utab) free(utab);
    if (uspace) free(uspace);
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_1 end");
// #endif
    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// static
UErrorCode svRELib::ICU_RE_Pattern_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_2 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize);
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);  
    }
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_2 01");
// #endif
    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE2 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_2 02");
// #endif        
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)   // We only care group 0 which is the complete match.
        if (p_regroup<=gCnt)
        { 
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                int32_t debug = uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
    // #ifndef NDEBUGs
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 u_strlen(dest)=%ld", u_strlen(dest)));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 sPos-s=%ld", sPos-s));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 sPos=%ld", sPos));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 s=%ld", s));
    // #endif
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_2 02-1 uregex_group=%i"), (int)debug);
    // #endif
                svRELib::InsertKeywordUnit(p_keywordList, &ku);
                // ku.k_text = NULL;
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE2 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE2 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_2 end");
// #endif
    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// static
UErrorCode svRELib::ICU_RE_Pattern_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_hintList)
{
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_3 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize);
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);  
    }
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_3 01");
// #endif
    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE3 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_3 02");
// #endif
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)   // We only care group 0 which is the complete match.
        if (p_regroup<=gCnt)
        {
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                int32_t debug = uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);

    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 u_strlen(dest)=%ld", u_strlen(dest)));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 sPos-s=%ld", sPos-s));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 sPos=%ld", sPos));
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 s=%ld", s));
    // #endif            
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svRELib::ICU_RE_Pattern_3 02-1 uregex_group=%i"), (int)debug);
    // #endif 
                InsertHintUnit(p_hintList, &ku);
                // ku.k_text = NULL;
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE3 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE3 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);
// #ifndef NDEBUG
//     wxLogMessage("svRELib::ICU_RE_Pattern_3 end");
// #endif
    return U_ZERO_ERROR;
}

