/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVLINETEXT_H
#define _SVLINETEXT_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "svTextStyleList.h"
#include "svListOfIntList.h"
#include "svBaseType.h"
#include "svDictionary.h"

#include <vector>

#include "unicode/uchar.h"
#include "unicode/ucnv.h"     /* C   Converter API    */
#include "unicode/regex.h"

using namespace std;

class svBufText; // forward declaretion.

#define SVID_END_OF_LINE -1    // symbol for end of line

// A struct stored information about the line wrap.
// The unit of idx and len is chacacter count.
typedef struct
{
    size_t idx;                  // The wraped line beggin in which position on m_text.
    size_t len;                  // the wraped line text length.
} wrapLenDesc;

// A struct stored information about a wraped and sytled text token.
// The unit of w_puxelWidth and w_accumPixelWidth is pixel count.
// 儲存 文字風格的訊息 
// 包含 文字、風格名稱(對應到theme)
typedef struct
{
    wxString w_text;                  // Text.
    wxString w_styleName;             // It's style name.
    size_t w_pixelWidth;              // The toal pixel width of the text.
    size_t w_accumPixelWidth;         // The summary pixel width of every text before the text on the same wrapped line. 
} styledWrapText;

enum
{
    SVID_LINE_WRAP=0,
    SVID_LINE_END=1
};

enum
{
    SVID_NEWLINE_CRLF=0,
    SVID_NEWLINE_CR=1,
    SVID_NEWLINE_LF=2,
    SVID_NEWLINE_NONE=3
};

enum
{
    SVID_NO_CRLF=0,
    SVID_WITH_CRLF=1
};

enum
{
    SVID_CONV_FROM_BUFFER=0,
    SVID_CONV_FROM_TEXT=1
};

enum
{
    SVID_HEAD_OF_KEYWORD=0,
    SVID_TAIL_OF_KEYWORD
};

/*
 * svLineText is a class representing a line text in a file.
 * A line text is a sequence of characters withing ending cr lf (or cr or lf).
 * The last line text in a file could have no ending cr lf.
 * 
 * We stored 3 kind of characters in this class.
 * 1. The original binary data reded from file. (unsigned char *) 
 * 2. The UTF-16 encoding Unicode String buffer converted from 1. (ICU UChar *)
 * 3. The wxWidgets wxString instance converted from 1. It's a unicode string too.
 * Why we have to use both ICU & wxString but not either? 
 * Because we need a great unicode regular expression but wxWidget's Regular expression is lacking many features. 
 * And also we need wxString to communicate with other wxWidgets components.
 * I know that make it complicated. I will try to figure out a better way.
 */
class svLineText
{
friend class svBufText;             // We need to rugular expression in an overall view.
       								// But I don't think using frind class is a good practice for Object Orient Programming.

private:
    
    /* svLineText 使用三種 buffer 紀錄文字
     *        m_buffer : unsigned char* 用來存放讀取自檔案的原始資料，檔案存什麼 m_buffer 就存什麼。
     *        m_ICUbuffer : 將 m_buffer 跟據可能的文字編碼轉換為 ICU UChar 字串，用以 regular expression 處理。
     *        m_text : 將 m_buffer 跟據可能的文字編碼轉換為 wxString 字串，用以顯示在 GUI 及與 wxWidgets library 交互作用。
     *       
     * 讀入檔案時，1.由檔案轉為 m_buffer
     *            2.由 m_buffer 轉為 m_ICUbuffer
     *            3.由 m_buffer 轉為 m_text
     *
     * 當使用者改變 GUI(文字編輯器)上的文字時：
     *            1.計算出新的 m_text
     *            2.由 m_text 轉為 m_buffer
     *            3.由 m_buffer 轉為 m_ICUbuffer
     *
     * 為了執行效能的考量，讀入檔案時，只有準備顯示在畫面上的資料行才會進行動作3(將m_buffer轉為m_text)
     * 從未顯示在畫面且未被準備顯示在畫面上的文字行的 m_text 是 null 
     *
     * 當其中任一資料異動時，要記得其餘兩者的資料，並呼叫以下的函式
     *  1.CreateKeywordTable 以 RegEx 重新取得 keyword 的位置
     *  2.ProcWrapLine 以畫面寬度重新計算每個字元的pixel寬度以便得知該於何處折行
     *  3.ProcStyledText 跟據 keyword 的位置重算字元該何重樣式呈現
     *  4.ProcStyledWrapLineText 彙整 2+3的資料(3的資料加上折行)
     *
     *
     *
     *
     * 語法高亮度處理：
     *           1.文字行(svLineText)應該要有m_ICUbuffer
     *           2.呼叫 svLineText::CreateKeyWordTable() 產生 keyword 資料 (以 ICU RegEx 處理而得)
     *           3.以在 svBufText 內呼叫 svBufText::CheckBlockTag() 以處理 區域型的 語法高亮度 (如跨行的註解等)
     * 為了提高程式的效率，svBufText 在讀入檔案時會進行一次處始的 語法高亮度處理，他的方式如下：
     *           1.讀入整個檔案
     *           2.以ICU RegEx處理整個檔案keyword位置
     *           3.找出換行符號(end of line)的位置
     *           4.計算2+3將各keyword的位置寫入對應的行號
     *           5.呼叫呼叫 svBufText::CheckBlockTag() 以處理 區域型的 語法高亮度 (如跨行的註解等)
     *         
     */

    svBufText *m_parent;            // pointer to parent svBufText class;

    unsigned char* m_buffer;        // Original data from file in bytes format.
    size_t m_bufferLen;             // The length of m_buffer. (Including CR LF)
    int m_bufferCRLFType;           // CRLF CR LF or NONE(the last line of a file)
    size_t m_bufferCRLFLen;         // The byte length of CRLF|CR|LF. 
    bool m_converted;               // 是否已將 m_buffer 轉為 m_text

    UChar* m_ICUbuffer;             // ICU Unicode data (UChar*) from m_buffer. UTF-16 encoding.  
    int32_t m_ICUbufferLen;         // The length of m_ICUbuffer. (Including CR LF)
    bool m_ICUconverted;            // 是否已將 m_buffer 轉為 m_ICUbuffer

    // wxString m_text;
    wxString* m_text;
    // vector<size_t> m_charWidList;
    // vector<size_t> m_charPixelWidList;
    svIntList m_charWidList;        // spaces number per char (for tab processing, a tab could be equal to 1,2,3 or 4 spaces.)
    svIntList m_charPixelWidList;   // pixels number per char
    svTextStyleList m_styleList;         // a list stored style information, one element for a word with it's start posion, length and style(color).
    vector<wrapLenDesc> m_wrapLenList;   // wrap text information, one element for a wrapped line record its start position and length.
    // vector<svTextStyleList> m_listOfStyledWrapTextList;
    vector< vector<styledWrapText> > m_listOfStyledWrapTextList; 
                                    // m_listOfStyledWrapTextList is a combination of m_styleList and m_wrapLenList.
    size_t m_indentPixelLen;        // for wrapped line processing. The first character position of wrapped line should be the same as it indent position.
                                    // for 折行處理，如果該行有前置tab、space，則折行的起始位置應該內縮
                                    // for example: unwrapped line=> bbbbThis is a unwrapped line. And is very long.
                                    // after wrapped              => bbbbThis is a unwrapped line.
                                    //                            =>     And is very long.
                                    //                                   ^ 
                                    // m_indentPixelLen is the space pixel length before "A"
    size_t m_indentCharLen;         // the char cound version of m_indentPixelLen


    bool m_updated;     // If text been updated.
    bool m_wrapped;     // If been wrap processed.
    bool m_styled;      // If been styled text processed.
    bool m_wrapStyled;  // If been wrap styled text processed.
    bool m_visible;     // if visible;
    
    vector<keywordUnit> m_keywordList;   // A vector stored keyword information.
    vector<keywordUnit> m_blockTagList;  // A vector stored block tag information.
    vector<keywordUnit> m_hintList;      // A vector stored hint word on typing.

    vector<foldingInfo> m_folding;       // A vector stored folding information.
                                         // It's a vector, but actually only 1 item will be sotred.

public:
    svLineText();
    svLineText(const svLineText& obj);
    svLineText(svBufText *p_parent, unsigned char* p_buf, size_t p_bufLen, int p_crlfType, size_t p_crlfLen);
    svLineText& operator=(const svLineText& obj);

    void CopyAttribute(const svLineText& obj);
    void SetBuffer(unsigned char* p_buf, size_t p_bufLen, int p_crlfType, size_t p_crlfLen);
    inline
    unsigned char* GetBuffer(void)
    {
        return m_buffer;
    }
    inline
    int GetBufferLen(void)
    {
        return m_bufferLen;
    }
    ~svLineText();

    size_t TextLen(short int p_type);
    inline
    bool TextIsEmpty(void)
    {
        return GetTextUW().IsEmpty();
    }
    inline
    bool TextIsEmptyOrBlank(void)
    {
        if (GetTextUW().IsEmpty() ||
            TextLen(SVID_NO_CRLF)==FindFirstNonSpaceColUW())
            return true;
        else
            return false;
    }

    bool ConvertBuffer2String(const wxString& p_charSet);
    UErrorCode ConvertBufferICU(const char *p_encoding);
    void SetNewLineType(const int p_CRLFType, const size_t p_unitSize);
    bool ConvertString2Buffer(const wxString& p_charSet);
    wxString ConvertString2Buffer(const wxString &p_text, const wxString& p_charSet);
    void UnifyBuffers(const wxString& p_charSet, const char *p_encoding, char p_convert_type);

    // member function name end with UW means for Unwrapped version.
    // member function name end with W means for Wrapped version.
    
    // bool CheckTextUW(const wxString &p_charSet);
    wxString GetTextUW(void);
    void SetTextUW(const wxString &p_txt);

    wxString GetTextBeforeUW(int pos);
    wxString GetTextAfterUW(int pos);
    wxString GetTextUW(int p_scol, int p_len);

    wxString DeleteTextAfterUW(int x);
    wxString DeleteTextBeforeUW(int x);
    wxString DeleteTextUW(int sx, int ex);

    wxString InsertTextUW(int sx, const wxString& txt);


    void ProcWrapLine(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    size_t GetWrapLineCount();
    bool GetWrapLenOnIdx(size_t p_idx, wrapLenDesc &p_wrapLenDesc);
    int InWhichWrappedLine(size_t p_idx);
    bool InWhichWrappedLineCol(size_t p_idx, int &p_wrapline, int &p_wrapcol);
    int GetWrappedLineStartColOnIdx(size_t p_idx);
    int GetWrappedLineStartColOnPosition(size_t p_idx);

    inline
    bool IsLastWrappedLine(size_t p_idx)
    {
        if (p_idx==m_wrapLenList.size()-1)
            return true;
        else
            return false;
    }

    int GetTextPixelLen(size_t p_idx, size_t p_len);

    void ProcStyledText(vector<re_rule> *p_synReRules);

    void ProcStyledWrapLineText();
    vector<vector<styledWrapText> >* GetStyledWrapLineText();
    vector<styledWrapText>* GetStyledWrapLineText(size_t idx);
    unsigned char GetStyledWrapLineType(size_t idx);

    bool IsStyledWrapProcessed();

    bool PixelX2TextColUW(int p_pixelX, size_t& p_textCol);
    bool TextCol2PixelUW(size_t p_textCol, int& p_pixelX);
    bool TextCol2PixelUW(size_t p_sCol, size_t p_eCol, int& p_pixelX);
    bool TextCol2PixelW(size_t p_sCol, size_t p_eCol, int &p_pixelStart, int &p_pixelEnd);

    /*
     * Keyword processing (syntax hilight) functions.
     * syntax hilight
     */
    void remove_overlaped_keyword(void); 
    bool CreateKeywordTable(vector<re_rule> *p_synReRules);
    // bool CreateKeywordTablePhrase02(const re_rule *p_re_rule);   
    bool OverlapedKeyword(const keywordUnit *p_ku); 
    bool InsertKeywordUnit(const keywordUnit* p_ku);
    bool remove_from_block_list(const keywordUnit *p_ku);
    bool InsertHintUnit(const keywordUnit* p_ku);
    bool InsertBlockTagKeywordUnit(const keywordUnit* p_ku);
    // bool UpdateBlockKeyword(const size_t p_start_col, const size_t p_end_col, const int p_pid);
    void ClearBlockKeywordInRange(const size_t p_start_col, const size_t p_end_col, const int p_pid);
    bool UpdateBlockKeywordInRange(const size_t p_start_col, const size_t p_end_col, const int p_pid);
    UErrorCode REMatchPattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end);
    UErrorCode REMatchPattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup);
    UErrorCode REMatchKeyword_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup);
    UErrorCode REMatchKeyword_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup);

    void CreateHintDictionary(svDictionary &p_dict);
    void SubHintDictionary(svDictionary &p_dict);
    
    bool GetPositionHint(const int p_pos, UChar **p_text, int &p_size, int &p_offset);
    bool GetKeywordPosition(const int p_cpos, int &p_wpos, int &p_wlen);
    
    bool GetKeywordPosition(const int p_cpos, char p_direction, char p_type, int &p_wpos);

    inline
    bool Visible(void)
    {
        return m_visible;
    }
    inline
    void Visible(const bool p_visible)
    {
        m_visible = p_visible;
    }

    inline
    // 內縮的像素值
    size_t GetIndentPixelLen(void)
    {
        return m_indentPixelLen;
    }

    inline
    // 內縮的字元數
    size_t GetIndentCharLen(void)
    {
        return m_indentCharLen;
    }


    int Col2SpaceW(const int p_wrapline, const int p_col);
    int Space2ColW(const int p_wrapline, const int p_space);

    // Col2SpaceUW 是 Col2SpaceW 的 unwrapped 版本
    // 傳入指定的未折行的 col 位置，回傳對應的 keepSpace 值
    inline
    int Col2SpaceUW(const int p_unwrapcol)
    {
        int wrapped_line, wrapped_col;
        wrapped_line = wrapped_col = 0;
        InWhichWrappedLineCol(p_unwrapcol, wrapped_line, wrapped_col);
        return Col2SpaceW(wrapped_line, wrapped_col);
    }

    inline
    void TextUpdated(void)
    {
        m_updated = true;
        m_wrapped = false;
        m_styled = false;
        m_wrapStyled = false;
    }
    inline
    bool IsUpdated(void)
    {
        return m_updated;
    }

    // ----------------------------------------------------------------------
    // Find related functions
    // ----------------------------------------------------------------------
    bool FindNextKeywordUCharFrom(const UCharText &p_keyword, const int p_scol, int &p_fcol);
    bool FindPrevKeywordUCharFrom(const UCharText &p_keyword, const int p_scol, int &p_fcol);

    bool FindNextKeywordwxStrFrom(const wxString &p_keyword, const wxString& p_charSet, const int p_scol, int &p_fcol, bool p_case);
    bool FindPrevKeywordwxStrFrom(const wxString &p_keyword, const wxString& p_charSet, const int p_scol, int &p_fcol, bool p_case);

    // ----------------------------------------------------------------------
    // Folding related functions
    // ----------------------------------------------------------------------
    bool AddFolding(const foldingInfo &p_fi);
    bool RemoveFolding(foldingInfo &p_fi);
    bool HadFolding(foldingInfo &p_fi);
    bool KeywordContainSymbolStart(vector<pairSymbol> &p_ps, const int p_start_col, int &p_pos, int &p_e_cnt, pairSymbol &p_pair);
    bool KeywordContainSymbolEnd(vector<pairSymbol> &p_ps, const int p_start_col, int &p_pos, int &p_s_cnt, pairSymbol &p_pair);

    bool KeywordContainPID(vector<int> &p_pid, keywordUnit &p_keyword);
    
    inline
    vector<foldingInfo> GetFoldingInfo(void)
    {
        return m_folding;
    }

    inline
    void SetFoldingInfo(const vector<foldingInfo> &fi)
    {
        if (m_folding.size()) m_folding.clear();
        m_folding = fi;
    }

    inline 
    void RemoveFoldingInfo(void)
    {
        m_folding.clear();
    }

    int FindFirstNonSpaceColUW(void);
    int FindFirstNonSpaceColUW(int p_spaceCnt, int p_tabSpace);

};  

#endif