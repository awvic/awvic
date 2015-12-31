/*
   Copyright Notice in awvic.cpp
*/
 
/* 
 * Class svTextView present the displayed text data on the screen.
 * It records every line (x,y) from text buffer(file). (for line wrap purpose)
 * It generates real display text(screen) from text buffer(file) with tab process, syntax highlight, etc.
*/

#ifndef _SVTEXTVIEW_H
#define _SVTEXTVIEW_H


#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "svLineText.h"
#include "svBufText.h"
#include "svListOfIntList.h"
#include "svAction.h"

#include <vector>
using namespace std;

enum         // move type.
{
    SVID_MOVE_UP=0,
    SVID_MOVE_DOWN,
    SVID_MOVE_LEFT,
    SVID_MOVE_RIGHT,
    SVID_MOVE_HEAD,
    SVID_MOVE_END,
    SVID_PAGE_UP,
    SVID_PAGE_DOWN,
    SVID_zt,
    SVID_zz,
    SVID_zb,
    SVID_H,
    SVID_M,
    SVID_L,

    SVID_CUR_PAGE,
    SVID_ALL_PAGE,

    SVID_FIND_UCHAR,
    SVID_FIND_WXSTR,
    SVID_FIND_REGEX
};

// A structure for which wrap_line ( coresponding to which unwrap line and which wrap line ).
// For example, if text is been read and wrap processed like below: 
// xxxxxxxxx xxxxxxxx xxxxxxxx
// xxxxxxxxx
// xxxxxxxxx AAAAAAAA xxxxxxxx
// xxxxxxxxx xxxxxxxx
// xxxxxxxxx
// Then the bufTextIndex of AAAAAAAA is (2, 1)
typedef struct
{
    size_t unwrap_idx;
    size_t wrap_idx;
    wrapLenDesc wrap_desc;   // 新增，縮簡計算存取時間
    int pixelWidth;   // 新增, 縮簡計算存取時間，該行的像素長度值
    inline
    size_t StartCol(void)
    {
        return wrap_desc.idx;
    }
    inline
    size_t Len(void)
    {
        return wrap_desc.len;
    }
} bufTextIndex;

typedef struct
{
    size_t row;
    size_t accumPixelWidth;
} caretPosOnView;           // 遊標在 TextView內的位置(行數、像素長度)

class svTextView
{
private:
    vector<bufTextIndex> m_textList;  //畫面中可見的資料
    /*
     * m_textList is a bufTextIndex list store text information on the screen. 
     * m_textList 是一個以 bufTextIndex 儲存畫面上的 text 資訊的資料結構。
     * 為 svTextView 的主要資料。
     * 例如，若文字檔案資料為如下：
     * This is a test line 1.\n
     * This is a test line 2.\n
     * This is a test line 3.\n
     * This is a test line 4.\n
     * This is a test line 5.\n
     * This is a test line 6.\n
     * This is a test line 7.\n
     * This is a test line 8.
     * 
     * 每行的寬度假設為12字元寬，畫面可容納下10行文字，
     * 則 m_textList 儲存的資料為：
     * (0,0),(0,1),(1,0),(1,1),(2,0),(2,1),(3,0),(3,1),(4,0),(4,1)共10筆資料
     */
  
    vector<bufTextIndex> m_pptextList;    // previous page textList. 畫面上不可見的前一頁的資料
    vector<bufTextIndex> m_nptextList;    // next page textList.     畫面上不可見的下一頁的資料

    // the previous and next page textList is for smooth screen scroll


    // svTextStyle* m_textStyle;       // text Style structure.

    // text buffer.
    svBufText* m_bufText;              // text buffer to be edit. 

    int m_pageLinesCnt;             // lines number in a text area. as m_linesPerTxtArea in SaluteVimTextCtrl.

    int m_memoryX;                  // keep last legal X for caret process. (when caret move up/down caret X could be changed because of the line length. Keeping X makes restoring X position possiblely.)

    const wxFont* m_font;

    bool m_wrap;                    // line wrap or not.  
    int m_maxPixelWidth;            // Max pixel width. Over and wrap. 包含不可見部份的像素值

    int m_visiblePixelWidth;        // Visible pixel width. svTextView 可見畫面像素值
    int m_HScrollPixel;             // horizontal scroll pixel length.  目前
    int m_old_HScrollPixel;         // horizontal scroll pixel length. keep previous value. 前一次

    bool m_needRefresh;             // If configure changed. Text Area Text need to be refreshed.
                                    // 以 m_needRefresh 判段是否重新 Refresh() 造成程式邏輯不明確
                                    // 先行取消，svTextView 相設定如有變更，請自行判段是否需呼叫 Refresh()

    bufTextIndex m_firstIndex;      // The first line in the text area index of wrapped index. 目前
    bufTextIndex m_old_firstIndex;  // The first line in the text area index of wrapped index. keep previous value.  前一次

    size_t m_tabSize;               // one tab equal to how many spaces.

    int m_charHeight;
    int m_spaceWidth;

    size_t m_lineNoIndicatorAreaMaxWidth;  // Line number indicator max width in pixel count.

    size_t m_firstRow, m_firstCol;       // The first row, col of svBufText on the TextView.
    size_t m_lastRow, m_lastCol;         // The last row, col of svBufText on the TextView.

    vector<svCaret> *m_inCaretList;     // Carets list in TextView
    vector<svCaret> *m_outCaretList;    // Carets list out of TextView

    int m_old_VScrollCnt;                // Last time scroll line(s) count. 最近一次的前一次
    int m_VScrollCnt;                    // Scroll line(s) count. 最近一次垂直捲動的行數

    svActionList m_actionRec;            // action list for user action recoding.

    bool m_showFindKeywordInd;           // display or no FindKeyIndicator

    int m_prevFind;                      // last success find type.
    svFindReplaceOption m_prevFROption;  // previous find / replace option.

public:
    svTextView();
    svTextView(svBufText* p_buftext, int p_linesPerTextArea, const wxFont& p_font, int p_maxPixelWidth, size_t p_tabSize, int p_charHeight, int p_spaceWidth);
    ~svTextView();
    void Clear(void);

    void Reset(svBufText* p_buftext, int p_linesPerTextArea, const wxFont& p_font, int p_maxPixelWidth, size_t p_tabSize, int p_charHeight, int p_spaceWidth);
    void Refresh();
    void SetVisiblePixelWidth(int p_visiblePixelWidth);
    inline
    void SetCharHeight(int p_charHeight)
    {
        m_charHeight = p_charHeight;
    }
    inline
    void SetSpaceWidth(int p_spaceWidth)
    {
        m_spaceWidth = p_spaceWidth;
    }
    inline
    void SetFont(const wxFont &p_font)
    {
        m_font = &p_font;
    }
    vector<styledWrapText>* GetStyledWrapText(size_t idx);
    vector<styledWrapText>* GetStyledWrapText_PP(size_t idx);
    vector<styledWrapText>* GetStyledWrapText_NP(size_t idx);
    size_t GetWrappedIndentPixelLen(size_t idx);
    size_t GetStyledWrapTextLineNoUW(size_t idx);
    size_t GetStyledWrapTextLineNoUW_PP(size_t idx);
    size_t GetStyledWrapTextLineNoUW_NP(size_t idx);
    size_t GetLineNoIndicatorAreaMaxWidth();


    inline
    int GetLinesPerPage(void)
    {
        return m_pageLinesCnt;
    }
    void SetPageLines(int p_pageLineCnt);
    // void SetWrap(bool p_isWrap);
    void SetMaxPixelWidth(int p_maxPixelWidth, bool p_isWrap);
    void SetFirstLineIndex(size_t p_firstLineIndex);

    inline
    int GetFirstUWLineNo(void)
    {
        return m_firstIndex.unwrap_idx;
    }
    inline
    int GetLastUWLineNo(void)
    {
        if (m_textList.size()>0)
            return m_textList.back().unwrap_idx;
        else
            return 0;
    }

    void MoveLineIndex(const size_t o_unwrap_idx, const size_t o_wrap_idx, const int offset, size_t &n_unwrap_idx, size_t &n_wrap_idx, int &m_cnt);
    int MoveFirstLineIndex(int offset);


    bool PixelXY2TextRowColUW(int p_pixelX, int p_pixelY, int p_lineHeight, bufTextIndex& p_textIdx);
    bool PixelXY2TextRowUW(int p_pixelX, int p_pixelY, int p_lineHeight, size_t& p_row);
    bool TextRowCol2PixelXYUW(int p_row, int p_col, int p_lineHeight, int &p_pixelX, int &p_pixelY);

    int GetMaxLinePixelWidth(void);
    inline
    int GetVisiblePixelWidth(void)
    {
        return m_visiblePixelWidth;
    }

    bool UpdateCaretsInfo(void);
    bool GetVisibleTextPosRange(size_t &p_srow, size_t &p_scol, size_t &p_erow, size_t &p_ecol);
    vector<caretPosOnView> *CalVisibleCaretsPos(void);
    bool GetNearestCaret(svCaret& p_caret, bool& p_forward);
    void KeepCaretOnView(void);
    int CalHScrollPixel(vector<caretPosOnView> *p_caretPosList);
    int CalWrappedLineDistance(size_t p_startRow, size_t p_startCol, size_t p_endRow, size_t p_endCol);

    /*
     *  Caret related functions.
     */
    void CaretsLeft(void);
    void CaretsLeftHead(void);
    void CaretsRight(void);
    void CaretsRightEnd(void);
    void CaretsUp(void);
    void CaretsDown(void);
    void PageUp(void);
    void PageDown(void);
    void CaretsLineHead(void);
    void CaretsLineEnd(void);
    void ClearCaretSelect(void);
    void SetCaretSelect(void);
    bool GetCaretSelectPixel(size_t idx, vector<int> &p_pixelList);
    bool GetCaretSelectPixel_PP(size_t idx, vector<int> &p_pixelList);
    bool GetCaretSelectPixel_NP(size_t idx, vector<int> &p_pixelList);
    bool GetTextRangePixel(size_t idx, vector<int> &p_pixelList, vector<textRange> p_range);
    void ResetCaretPosition(const size_t p_row, const size_t p_col);
    void AppendCaretPosition(const size_t p_row, const size_t p_col);
    void LastCaretMoveTo(const size_t p_row, const size_t p_col);
    inline
    bool CaretsMergeOverlap(void)
    {
        return m_bufText->CaretsMergeOverlap();
    }
    inline
    bool CaretsIsOverlaped(const svCaret p_target)
    {
        return m_bufText->CaretsIsOverlaped(p_target);
    }
    inline
    vector<svCaret> GetCaretsList(void)
    {
        return m_bufText->GetCaretsList();
    }

    bool GetLastCaretPixelXY(int &p_x, int &p_y, int p_lineHeight);
    bool GetLastCaretWordPixelXY(int &p_x, int &p_y, int p_lineHeight);
    
    bool GetTextIndentIndicatorCharLen(size_t idx, vector<int> &p_pixelList);
    bool GetTextIndentIndicatorCharLen_PP(size_t idx, vector<int> &p_pixelList);
    bool GetTextIndentIndicatorCharLen_NP(size_t idx, vector<int> &p_pixelList);
    
    inline
    int GetHScrollPixel(void)
    {
        return m_HScrollPixel;
    }

    inline
    int GetOldHScrollPixel(void)
    {
        return m_old_HScrollPixel;
    }

    inline
    void ResetOldHScrollPixel(void)
    {
        m_old_HScrollPixel = m_HScrollPixel;
    }

    inline
    void SetHScrollPixel(int p_hpixel)
    {
        // Doesn't check validation.
        m_old_HScrollPixel = m_HScrollPixel;
        m_HScrollPixel = p_hpixel;
    }

    inline
    bufTextIndex GetFirstLineIndex(void)
    {
        return m_firstIndex;
    }

    inline
    bufTextIndex GetOldFirstLineIndex(void)
    {
        return m_old_firstIndex;
    }

    inline
    int GetMaxPixelWidth(void)
    {
        return m_maxPixelWidth;
    }

    inline
    int GetPrevPageTextListCount(void)
    {
        return m_pptextList.size();
    }

    inline
    int GetCurPageTextListCount(void)
    {
        return m_textList.size();
    }

    inline
    int GetNextPageTextListCount(void)
    {
        return m_nptextList.size();
    }

    inline
    int GetAllPageTextListCount(void)
    {
        return m_pptextList.size() + m_textList.size() + m_nptextList.size();
    }

    inline
    int GetProcessedVScrollLineCount(void)
    {
        return m_old_VScrollCnt;
    }

    inline
    int GetVScrollLineCount(void)
    {
        return m_VScrollCnt;
    }

    inline
    void ResetVScrollLineCount(void)
    {
        m_old_VScrollCnt = m_VScrollCnt;
        m_VScrollCnt = 0;
    }

    inline
    bool HasBuffer(void)
    {
        if (m_bufText) 
            return true;
        else
            return false;
    }

    inline
    void AppendActionRecord(const svAction &p_action)
    {
        m_actionRec.Append(p_action);
    }

    inline
    bool DuplicatedAction(const int p_type, const int p_count)
    {
        return m_actionRec.DuplicatedAction(p_type, p_count);
    }

    /****************************************************************************
     *  Editing related functions.
     ****************************************************************************/
    void EditingInsertChar(const wxString &p_str);
    void EditingInsertHintChar(const wxString &p_str, int p_curWordLen, int p_curWordOffset);
    void EditingSplitLine(void);
    void EditingTextDelete(void);
    void EditingTextBackDelete(void);
    void EditingTextCopySelected(void);
    void EditingTextPaste(void);
    void EditingTextCut(void);
    void EditingTextLineComment(void);
    void EditingTextBlockComment(void);
    void EditingTextIndent(void);
    void EditingTextOutdent(void);
    void EditingTextResetCarets(void);
    void EditingTextDuplicateLine(void);

    void UndoEditing(void);


    /****************************************************************************
     *  Find related functions.
     ****************************************************************************/
    bool FindMatchLocations(const svFindReplaceOption &p_frOption);

    void FindNext(void);
    void FindPrev(void);
    void FindAll(void);

    bool GetFindKeywordPixel_PP(size_t idx, vector<int> &p_pixelList);
    bool GetFindKeywordPixel(size_t idx, vector<int> &p_pixelList);
    bool GetFindKeywordPixel_NP(size_t idx, vector<int> &p_pixelList);

    void ReplaceNext(const svFindReplaceOption &p_frOption);
    void ReplacePrev(const svFindReplaceOption &p_frOption);
    void ReplaceAll(const svFindReplaceOption &p_frOption);

    /*---------------------------------------------------------------------------
     *  Folding related functions.
     *---------------------------------------------------------------------------*/
    void DoFolding(const size_t p_unwrap_idx);
    // void Folding(const size_t p_unwrap_idx);
    // void Unfolding(const size_t p_unwrap_idx);

    bool HadFolding_PP(size_t idx);
    bool HadFolding(size_t idx);
    bool HadFolding_NP(size_t idx);

    bool HadFoldingTail_PP(size_t idx);
    bool HadFoldingTail(size_t idx);
    bool HadFoldingTail_NP(size_t idx);

    bool CanFolding_PP(size_t idx);
    bool CanFolding(size_t idx);
    bool CanFolding_NP(size_t idx);

    vector<textRange> *CalVisibleCaretsEmbraceSymbolPos(void);



    void GotoLine(const int p_lineNo);



};

#endif
