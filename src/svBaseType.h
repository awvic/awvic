/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVBASETYPE_H
#define _SVBASETYPE_H

#ifdef __WXMSW__
#pragma warning(disable: 4996)
#endif

#include "unicode/uchar.h"
#include "unicode/ustring.h"
// #include "unicode/ucnv.h"     /* C   Converter API    */

#include "svCaret.h"

#define MAX_LINE_NO 2147483647  // INT_MAX
#define SVID_NONE_BLOCKED -1     // not a blocked keyword
#define SVID_BLOCK_RE_RULE 0     // block regular expression rule
#define SVID_DEFAULT_PID 0       // default pid for non matched keyword of any regular expression rules

enum
{
    SVID_BLOCK_START=0,
    SVID_BLOCK_END=1
};


// re_rule structure is for regular expression in syntax processing.
// re_rule 是作語法hiligh處理時需要使用到的regular expression的基本單位
typedef struct re_rule
{
    char *name;
    unsigned int pid;                        // pattern id.
    char *re1;
    char *re2;
    char *cm1;
    char *cm2;
    int type;                                // different type for different processing. 
    int32_t regroup;                         // whick match grouop will be used.
    re_rule()
    {
        name = NULL;
        pid = 0;
        re1 = NULL;
        re2 = NULL;
        cm1 = NULL;
        cm2 = NULL;
        type = 0;
        regroup = 0;
    }
    re_rule(const re_rule& obj)  // copy constructor
    {
        if (obj.name)
            name = strdup(obj.name);
        else
            name = NULL;
        pid = obj.pid;
        if (obj.re1)
            re1 = strdup(obj.re1);
        else 
            re1 = NULL;
        if (obj.re2)
            re2 = strdup(obj.re2);
        else
            re2 = NULL;
        if (obj.cm1)
            cm1 = strdup(obj.cm1);
        else 
            cm1 = NULL;
        if (obj.cm2)
            cm2 = strdup(obj.cm2);
        else
            cm2 = NULL;
        type = obj.type;
        regroup = obj.regroup;
    }
    re_rule& operator=(const re_rule& obj)
    {
        if (name) free(name);
        if (obj.name)
            name = strdup(obj.name);
        else
            name = NULL;
        pid = obj.pid;
        if (re1) free(re1);
        if (obj.re1)
            re1 = strdup(obj.re1);
        else 
            re1 = NULL;
        if (re2) free(re2);
        if (obj.re2)
            re2 = strdup(obj.re2);
        else
            re2 = NULL;
        if (cm1) free(cm1);
        if (obj.cm1)
            cm1 = strdup(obj.cm1);
        else 
            cm1 = NULL;
        if (cm2) free(cm2);
        if (obj.cm2)
            cm2 = strdup(obj.cm2);
        else
            cm2 = NULL;
        type = obj.type;     
        regroup = obj.regroup;   
        return *this;
    }
    ~re_rule()
    {
        if (name) free(name);
        if (re1) free(re1);
        if (re2) free(re2);
        if (cm1) free(cm1);
        if (cm2) free(cm2);
    }

} re_rule;

enum
{
    SVID_C_FOLDING=0,
    SVID_PYTHON_FOLDING=1
};

// embrace_rule structure is for embrace and folding in syntax processing.
// embrace_rule 是作語法hiligh處理時需要使用到的regular expression的基本單位(for embrace symbal and folding)
typedef struct embrace_rule
{
    char *name;
    unsigned int pid;                        // pattern id.
    char *re1;
    char *re2;
    char *cm1;
    char *cm2;
    int type;                                // different type for different processing. 
    int folding_type;                        // The way we processing folding.

    embrace_rule()
    {
        name = NULL;
        pid = 0;
        re1 = NULL;
        re2 = NULL;
        cm1 = NULL;
        cm2 = NULL;
        type = 0;
        folding_type = 0;
    }
    embrace_rule(const embrace_rule& obj)  // copy constructor
    {
        if (obj.name)
            name = strdup(obj.name);
        else
            name = NULL;
        pid = obj.pid;
        if (obj.re1)
            re1 = strdup(obj.re1);
        else 
            re1 = NULL;
        if (obj.re2)
            re2 = strdup(obj.re2);
        else
            re2 = NULL;
        if (obj.cm1)
            cm1 = strdup(obj.cm1);
        else 
            cm1 = NULL;
        if (obj.cm2)
            cm2 = strdup(obj.cm2);
        else
            cm2 = NULL;
        type = obj.type;
        folding_type = obj.folding_type;
    }
    embrace_rule& operator=(const embrace_rule& obj)
    {
        if (name) free(name);
        if (obj.name)
            name = strdup(obj.name);
        else
            name = NULL;
        pid = obj.pid;
        if (re1) free(re1);
        if (obj.re1)
            re1 = strdup(obj.re1);
        else 
            re1 = NULL;
        if (re2) free(re2);
        if (obj.re2)
            re2 = strdup(obj.re2);
        else
            re2 = NULL;
        if (cm1) free(cm1);
        if (obj.cm1)
            cm1 = strdup(obj.cm1);
        else 
            cm1 = NULL;
        if (cm2) free(cm2);
        if (obj.cm2)
            cm2 = strdup(obj.cm2);
        else
            cm2 = NULL;
        type = obj.type;     
        folding_type = obj.folding_type;
        return *this;
    }
    ~embrace_rule()
    {
        if (name) free(name);
        if (re1) free(re1);
        if (re2) free(re2);
        if (cm1) free(cm1);
        if (cm2) free(cm2);
    }

} embrace_rule;



// keywordUnit structure is for syntax processing.
// keywordUnit 是作語法hiligh處理時用以儲存資訊的基本單位
typedef struct keywordUnit
{
    UChar* k_text;                    // Text.
    unsigned int k_pid;               // Pattern id
    size_t k_pos;                     // The start position of the line.
    size_t k_len;                     // The length of the keyword. 
    int k_blocked_pid;                // Block tag id (start end id) -1 for none
                                      // 當該 keywordUnit 被 block tag 包圍時，block_pid 將被設為與 block tag pid 相同

    keywordUnit()
    {
        k_text = NULL;
        k_pid = 0;
        k_pos = 0;
        k_len = 0;
        k_blocked_pid = -1;
    }
    keywordUnit(const keywordUnit& obj)  // copy constructor
    {
        if (obj.k_text)
        {
            // uint32_t len = u_strlen(obj.k_text) + 1;
            // k_text = (UChar *)malloc(sizeof(UChar) * len);
            // u_memcpy(k_text, obj.k_text, len);
            uint32_t len = obj.k_len;
            k_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(k_text, obj.k_text, len);
        }
        else
            k_text = NULL;

        k_pid = obj.k_pid;
        k_pos = obj.k_pos;
        k_len = obj.k_len;
        k_blocked_pid = obj.k_blocked_pid;
    }

    keywordUnit& operator=(const keywordUnit& obj)
    {
        if (k_text) free(k_text);
        if (obj.k_text)
        {
            // uint32_t len = u_strlen(obj.k_text) + 1;
            // k_text = (UChar *)malloc(sizeof(UChar) * len);
            // u_memcpy(k_text, obj.k_text, len);
            uint32_t len = obj.k_len;
            k_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(k_text, obj.k_text, len);
        }
        else
            k_text = NULL;

        k_pid = obj.k_pid;
        k_pos = obj.k_pos;
        k_len = obj.k_len;
        k_blocked_pid = obj.k_blocked_pid;
        return *this;
    }
    ~keywordUnit()
    {
        if (k_text) free(k_text);
    }

} keywordUnit;


// svFileDesc structure is for file information with it's full path and display name one awvic.
typedef struct svFileDesc
{
    wxString m_fullPathName;              // Full path and file name of the file.
    wxString m_displayName;               // Display file name. Usually m_fullPath without path.
    wxDateTime m_lastModificationTime;    // Record the files file system last modification date & time.
                                          // For check if a file is modified outside of AWVIC and need to be reload.
    bool m_needResetWhenModificated;      // default to true => means reload is need when last modification date and time is differ.
                                          // Set to false will ignore files last modification date and time check.

    svFileDesc()
    {
        m_needResetWhenModificated = true;
    }

    svFileDesc(const wxString &p_fullPathName, const wxString p_displayName)
    {
        m_fullPathName = p_fullPathName;
        m_displayName = p_displayName;
        m_needResetWhenModificated = true;
    }
    svFileDesc(const svFileDesc& obj)  // copy constructor
    {
        m_fullPathName = obj.m_fullPathName;
        m_displayName = obj.m_displayName;
        m_lastModificationTime = obj.m_lastModificationTime;
        m_needResetWhenModificated = obj.m_needResetWhenModificated;
    }

    svFileDesc& operator=(const svFileDesc& obj)
    {
        m_fullPathName = obj.m_fullPathName;
        m_displayName = obj.m_displayName;
        m_lastModificationTime = obj.m_lastModificationTime;
        m_needResetWhenModificated = obj.m_needResetWhenModificated;
        return *this;
    }

    void SetLastModificationTime(void)
    {
        wxStructStat strucStat;
        wxStat(m_fullPathName, &strucStat);
        //wxFileOffset filelen=strucStat.st_size;
        //wxDateTime last_modified_time(strucStat.st_mtime);

        //wxLogDebug(wxT("%d - %s"), (unsigned long)filelen, last_modified_time.Format(wxT("%d-%b-%Y at %H:%M:%S"))) ;
        m_lastModificationTime = wxDateTime(strucStat.st_mtime);
    }
        
    void SetLastModificationTime(const wxDateTime &p_time)
    {
        m_lastModificationTime = p_time;
    }
    
    bool IsModificationTimeChanged(void)
    {
        // If not need reset when last modification time changed, alway return false;
        if (!m_needResetWhenModificated)
            return false;
            
        wxStructStat strucStat;
        wxStat(m_fullPathName, &strucStat);
        wxDateTime lastModifiedTime(strucStat.st_mtime);

        return !(m_lastModificationTime==lastModifiedTime);
    }
    
    void SetNeedReset(bool p_needReset)
    {
        m_needResetWhenModificated = p_needReset;
    }
    
    ~svFileDesc()
    {
    }

} svFileDesc;

inline
bool operator==(const svFileDesc &fd1, const svFileDesc &fd2)
{
    if (fd1.m_fullPathName==fd2.m_fullPathName)
        return true;
    else
        return false;
}

inline
bool operator!=(const svFileDesc &fd1, const svFileDesc &fd2)
{
    if (fd1.m_fullPathName!=fd2.m_fullPathName)
        return true;
    else
        return false;
}

typedef struct svFileDescList
{
    vector<svFileDesc> m_fileDescList;

    svFileDescList()
    {
    }

    svFileDescList(const svFileDescList &obj)  // copy constructor
    {
        m_fileDescList = obj.m_fileDescList;
    }

    svFileDescList& operator=(const svFileDescList& obj)
    {
        m_fileDescList = obj.m_fileDescList;
        return *this;
    }
    ~svFileDescList()
    {
        m_fileDescList.clear();
    }

    bool CheckDuplication(const svFileDesc &p_fd)
    {
        for (std::vector<svFileDesc>::iterator it=m_fileDescList.begin();
             it!=m_fileDescList.end();
             ++it)
        {
            if (*it==p_fd)
                return true;
        }
        return false;
    }

    bool Append(const svFileDesc &p_fd)
    {
        if (CheckDuplication(p_fd))
            return false;

        m_fileDescList.push_back(p_fd);
    }

} svFileDescList;


// pairSymbol structure is for sotre paired symbol like {,} (,) begin,end etc.
typedef struct pairSymbol
{
    UChar* s_text;                    // Start symbol Text.
    UChar* e_text;                    // End symbol Text.
    size_t s_len;                     // The length of the start symbol. 
    size_t e_len;                     // The length of the end symbol. 

    pairSymbol()
    {
        s_text = NULL;
        e_text = NULL;
        s_len = 0;
        e_len = 0;
    }
    pairSymbol(char *s_char, char *e_char)
    {
        s_text = (UChar*) malloc (sizeof(UChar)*(strlen(s_char)+1));
        u_charsToUChars(s_char, s_text, strlen(s_char)+1);

        e_text = (UChar*) malloc (sizeof(UChar)*(strlen(e_char)+1));
        u_charsToUChars(e_char, e_text, strlen(e_char)+1);

        s_len = strlen(s_char);
        e_len = strlen(e_char);
    }
    pairSymbol(const pairSymbol& obj)  // copy constructor
    {
        if (obj.s_text)
        {
            uint32_t len = obj.s_len;
            s_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(s_text, obj.s_text, len);
        }
        else
            s_text = NULL;

        if (obj.e_text)
        {
            uint32_t len = obj.e_len;
            e_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(e_text, obj.e_text, len);
        }
        else
            e_text = NULL;

        s_len = obj.s_len;
        e_len = obj.e_len;
    }

    pairSymbol& operator=(const pairSymbol& obj)
    {
        if (s_text) free(s_text);
        if (obj.s_text)
        {
            uint32_t len = obj.s_len;
            s_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(s_text, obj.s_text, len);
        }
        else
            s_text = NULL;

        if (e_text) free(e_text);
        if (obj.e_text)
        {
            uint32_t len = obj.e_len;
            e_text = (UChar *)malloc(sizeof(UChar) * len);
            u_memcpy(e_text, obj.e_text, len);
        }
        else
            e_text = NULL;

        s_len = obj.s_len;
        e_len = obj.e_len;

        return *this;
    }
    ~pairSymbol()
    {
        if (s_text) free(s_text);
        if (e_text) free(e_text);
    }
    
    bool MatchStart(UChar *p_text, int32_t p_len)
    {
        if ( s_len==p_len &&
            (u_memcmp(s_text, p_text, s_len)==0) )
            return true;
        else
            return false;        
    }

    bool MatchEnd(UChar *p_text, int32_t p_len)
    {
        if ( e_len==p_len &&
            (u_memcmp(e_text, p_text, e_len)==0) )
            return true;
        else
            return false;        
    }

} pairSymbol;

inline
bool operator==(const pairSymbol &ps1, const pairSymbol &ps2)
{
    if ( ps1.s_len==ps2.s_len && ps1.e_len==ps2.e_len &&
        (u_memcmp(ps1.s_text, ps2.s_text, ps1.s_len)==0) &&
        (u_memcmp(ps1.e_text, ps2.e_text, ps1.e_len)==0) )
        return true;
    else
        return false;
}


// foldingInfo structure is for folding information.
// folding infomation : alway from the next line of the current line to +range line.
// 
// for example:
// 
// 10 if (foo)   
// 11 {  <= If line 11 folding, foldingInfo will be stored on line 11, and foldingInfo.range = 2.
// 12    a += 1;
// 13    b += 2;
// 14 }
typedef struct foldingInfo
{
    int range;                        // How many line to be folding. (Counting from the next line.) 

    foldingInfo()
    {
        range = 0;
    }
    foldingInfo(const int p_r)
    {
        range = p_r;
    }
    foldingInfo(const foldingInfo& obj)  // copy constructor
    {
        range = obj.range;
    }

    foldingInfo& operator=(const foldingInfo& obj)
    {
        range = obj.range;

        return *this;
    }
    ~foldingInfo()
    {
    }

} foldingInfo;

/* -------------------------------------------------------------------------------
 *      A data structure for recording text range position.
 *      start_row start_col is the start position of the text.
 *      end_row end_col is the end position of the text.
 * ------------------------------------------------------------------------------- */           
typedef struct textRange
{
    int start_row;
    int start_col;
    int end_row;
    int end_col;

    textRange()
    {
        start_row = 0;
        start_col = 0;
        end_row = 0;
        end_col = 0;
    }
    textRange(const int p_sr, const int p_sc, const int p_er, const int p_ec)
    {
        start_row = p_sr;
        start_col = p_sc;
        end_row = p_er;
        end_col = p_ec;
    }
    textRange(const textRange& obj)  // copy constructor
    {
        start_row = obj.start_row;
        start_col = obj.start_col;
        end_row = obj.end_row;
        end_col = obj.end_col;
    }

    textRange& operator=(const textRange& obj)
    {
        start_row = obj.start_row;
        start_col = obj.start_col;
        end_row = obj.end_row;
        end_col = obj.end_col;

        return *this;
    }
    ~textRange()
    {
    }
} textRange;

/* ------------------------------------------------------------------------------ *
 * A class for getting sequential code.
 * Very simple, not complicated as anything algorithm.
 * ------------------------------------------------------------------------------ */

typedef unsigned long sv_seq_code;

class svSeqCodeGenerator
{
public: 
    static const signed long NEW_FILE_INIT_SEQ_CODE = 0; 
    static const signed long OLD_FILE_INIT_SEQ_CODE = 1;

public:
    svSeqCodeGenerator()
    {
        /*
         * m_curNum 設為 1 
         * 參照 NEW_FILE_INIT_SEQ_CODE = 0 
         *           則當 bufText 是 new files 時 m_savedSeqCode 永遠不會回到 0 
         *           而必須存檔才能使 m_modified=false;
         *
         *
         * m_curNum 設為 1 
         * 參照 NEW_FILE_INIT_SEQ_CODE = 1 
         *           則當 bufText 是 open files 時 m_savedSeqCode 會回到 1 
         */
        m_curNum = 1;
    }

    ~svSeqCodeGenerator()
    {
    }

    // inline
    // void Initial(const unsigned long p_initValue)
    // {
    //     m_curNum = p_initValue;
    // }

    inline
    signed long GetCurCode(void)
    {
        return m_curNum++;   // Update m_curNum;
    }

    inline
    signed long CurCode(void)
    {
        return m_curNum;  // Does not update m_curNum;
    }

private:
    // m_curNum 是最近一次被取走的序號
    signed long m_curNum;

};


typedef struct svRect
{
    int X, Y;
    int Width, Height;

    svRect()
    {
        X=Y=0;
        Width=Height=0;
    }

    ~svRect()
    {
    }

} svRect;

typedef struct svFileDescOpened : svFileDesc
{
    int m_firstLineNo;
    vector<svCaret> m_carets;

    svFileDescOpened()
    {
        m_firstLineNo = 0;
    }

    ~svFileDescOpened()
    {
        m_carets.clear();
    }

    void ClearCaret(void)
    {
        m_carets.clear();
    }

    void AddCaret(const svCaret &p_c)
    {
        m_carets.push_back(p_c);
    }

    void AddCarets(const vector<svCaret> &p_cs)
    {
        m_carets.clear();
        m_carets = p_cs;
    }


} svFileDescOpened;

// appCloseStatus is for recording awvic status before closing.
typedef struct svAppCloseStatus
{
    vector<svFileDescOpened> m_buffers;
    int m_activeBuffer;
    svRect m_appPosition;

    bool m_openFiles;
    int m_openFilesWidth;
    bool m_findCtrl;

    svAppCloseStatus()
    {
        m_activeBuffer = 0;

        m_openFiles = false;
        m_openFilesWidth = 0;
        m_findCtrl = false;
    }

    void ClearBuffer(void)
    {
        m_buffers.clear();
    }

    void AddBuffer(const svFileDescOpened &p_fd)
    {
        m_buffers.push_back(p_fd);
    }

    void SetPosition(const svRect &p_r)
    {
        m_appPosition = p_r;
    }

    svRect GetPosition(void)
    {
        return m_appPosition;
    }

    ~svAppCloseStatus()
    {
        m_buffers.clear();
    }

} svAppCloseStatus;

typedef struct UCharText
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
    bool IsEmpty(void)
    {
        if (uctext==NULL)
            return true;
        else
            return false;
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

inline
bool operator==(const UCharText &obj1, const UCharText &obj2)
{
    // 第5個參數為 true 或 false 尚待確定
    if (u_strCompare(obj1.uctext, obj1.len, obj2.uctext, obj2.len, true)==0)
        return true;
    else
        return false;
}

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
// }

typedef struct svFindReplaceOption
{
    UCharText m_currentWord2Find;       // for Quick find (find current word)

    wxString m_wxStr2Find;              // wxString for find (from find window) 目前只單行，無換行符號
    wxString m_wxStr2Replace;           // wxString for replace (from replace window) 目前只單行，無換行符號

    bool m_case;                        // Case sensitive for m_wsStr2Find/m_wxStr2Replace
    bool m_regex;
    bool m_inSelect;
    bool m_wholeWord;

    char m_from;                        // where the svFindReplaceOption come from.

    static const char SVID_FIND_WXSTR = 0;
    static const char SVID_FIND_UCHAR = 1;
    static const char SVID_FIND_LAST_UCHAR = 2;
    static const char SVID_FIND_REGEX = 3;
    static const char SVID_FIND_UNKNOWN = 4;

    static const char SVID_FIND_NOT_SET= 5;                   // not set yet.
    static const char SVID_FIND_FROM_PANEL = 6;               // from the svFindReplaceCtrl
    static const char SVID_FIND_FROM_KEYSTROKE = 7;           // from svTextEditorCtrl key stroke (Ctrl+F3) etc.
    static const char SVID_FIND_FROM_LAST_KEYSTROKE = 8;      // from svTextEditorCtrl key stroke (F3/Alt+F3) etc.

    svFindReplaceOption()
    {
        m_case = false;
        m_regex = false;
        m_inSelect = false;
        m_wholeWord = false;
        // m_from = svFindReplaceOption::SVID_FIND_FROM_PANEL;  
        m_from = svFindReplaceOption::SVID_FIND_NOT_SET;  
    }

    svFindReplaceOption(const svFindReplaceOption& obj)
    {
        m_currentWord2Find = obj.m_currentWord2Find;
        m_wxStr2Find = obj.m_wxStr2Find;
        m_wxStr2Replace = obj.m_wxStr2Replace;
        m_case = obj.m_case;
        m_regex = obj.m_regex;
        m_inSelect = obj.m_inSelect;
        m_wholeWord = obj.m_wholeWord;
        m_from = obj.m_from;  
    }

    svFindReplaceOption& operator=(const svFindReplaceOption& obj)
    {
        m_currentWord2Find = obj.m_currentWord2Find;
        m_wxStr2Find = obj.m_wxStr2Find;
        m_wxStr2Replace = obj.m_wxStr2Replace;
        m_case = obj.m_case;
        m_regex = obj.m_regex;
        m_inSelect = obj.m_inSelect;
        m_wholeWord = obj.m_wholeWord;
        m_from = obj.m_from;

        return *this;
    }

    char GetType(void) const
    {
        if (m_from == svFindReplaceOption::SVID_FIND_FROM_PANEL)
        {
            if (m_regex)
            {
                return svFindReplaceOption::SVID_FIND_REGEX;
            }
            else
            {
                return svFindReplaceOption::SVID_FIND_WXSTR;
            }
        }
        else if (m_from == svFindReplaceOption::SVID_FIND_FROM_KEYSTROKE)
        {
            return svFindReplaceOption::SVID_FIND_UCHAR;
        }
        else if (m_from == svFindReplaceOption::SVID_FIND_FROM_LAST_KEYSTROKE)
        {
            return svFindReplaceOption::SVID_FIND_LAST_UCHAR;
        }        else // unknown
        {
            return svFindReplaceOption::SVID_FIND_UNKNOWN;
        }
    }

} svFindReplaceOption;

inline
bool operator==(const svFindReplaceOption &obj1, const svFindReplaceOption &obj2)
{
    if ( obj1.m_from != svFindReplaceOption::SVID_FIND_NOT_SET &&
         obj1.m_currentWord2Find == obj2.m_currentWord2Find &&
         obj1.m_wxStr2Find == obj2.m_wxStr2Find &&
         obj1.m_wxStr2Replace == obj2.m_wxStr2Replace &&
         obj1.m_case == obj2.m_case &&
         obj1.m_regex == obj2.m_regex &&
         obj1.m_inSelect == obj2.m_inSelect &&
         obj1.m_wholeWord == obj2.m_wholeWord &&
         obj1.m_from == obj2.m_from )
        return true;
    else
        return false;
}


typedef struct svIntPair
{
    int num1;
    int num2;
    svIntPair()
    {
        num1=0;
        num2=0;
    }
    svIntPair(int n1, int n2)
    {
        num1=n1;
        num2=n2;
    }

} svIntPair;

typedef struct svInt2Pair
{
    int num1;
    int num2;
    int num3;
    int num4;

    svInt2Pair()
    {
        num1=0;
        num2=0;
        num3=0;
        num4=0;
    }
    svInt2Pair(int n1, int n2, int n3, int n4)
    {
        num1=n1;
        num2=n2;
        num3=n3;
        num4=n4;
    }

} svInt2Pair;

/*
 * The structure is to store the status of undo.
 * It record it's undo count and undo text length.
 * It's been used to confirm if the file been updated between 2 status.
 *
 * svUndoStatus 用途
 * svUndoStatus usage
 * 當進行 search 時, 記錄其 svUndoStatus
 * When a search is fired, record the file's svUndoStatus
 * 每當 search next 或 search prev 時，比對 該檔案目前的 svUndoStatus 與 search 時的 svUndoStatus
 * On search next or search prev, compare the file's current svUndoStatus with the search svUndoStatus
 * 若不同，則重作搜尋
 * If different, ReSearch it.
 *
 */
typedef struct svUndoStatus
{
    int m_undoCount;                 // How many undo are there?
    int m_lastUndoActionCount;       // How many action are there in the last undo.
    int m_lastUndoTextLength;        // The length of modified text in the last undo.
    svUndoStatus()
    {
        m_undoCount = -1;
        m_lastUndoActionCount = -1;
        m_lastUndoTextLength = -1;
    }
} svUndoStatus;

inline
bool operator==(const svUndoStatus &us1, const svUndoStatus &us2)
{
    if ( us1.m_undoCount==us2.m_undoCount && 
         us1.m_lastUndoActionCount==us2.m_lastUndoActionCount &&
         us1.m_lastUndoTextLength==us2.m_lastUndoTextLength )
        return true;
    else
        return false;
}

inline
bool operator!=(const svUndoStatus &us1, const svUndoStatus &us2)
{
    if ( us1.m_undoCount!=us2.m_undoCount ||
         us1.m_lastUndoActionCount!=us2.m_lastUndoActionCount ||
         us1.m_lastUndoTextLength!=us2.m_lastUndoTextLength )
        return true;
    else
        return false;
}

// A structure of int and wxString combine for goto definition.
typedef struct svIntText
{
    int m_lineNo;
    wxString m_text;
    keywordUnit m_keyword;
} svIntText;

// for svIntText compare on vector of svIntText Sorting
// Comparing wxString first, then comparing it's lineNo.
inline
bool operator<(const svIntText &it1, const svIntText &it2)
{
    if (it1.m_text.Cmp(it2.m_text)<0)
        return true;
    else if (it1.m_text.Cmp(it2.m_text)==0)
        if (it1.m_lineNo<it2.m_lineNo)
            return true;

    return false;
}

#endif
