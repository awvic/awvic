/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVBUFTEXT_H
#define _SVBUFTEXT_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <vector>
#include <list>
#include <iostream>
#include <fstream>

#include <json/json.h>

#include "svPozList.h"
#include "svLineText.h"
#include "svBaseType.h"
#include "svCaret.h"
#include "svCommonLib.h"
#include "svUndoAction.h"
// #include "svTextEditorCtrl.h"
// #include "svDictionary.h"

using namespace std;

// forward declaration. avoid cross reference problem.
// Also have to include svTextEditorCtrl.h in svBufText.cpp to solving the cross reference problem.
class svTextEditorCtrl;

/*
/* If #define SINGLE_BUFFER, all svLineText will be stored in a single vector of svLineText
 * If not #define SINGLE_BUFFER, all svLineText will be stored in a vector of (vector of svLineText)
 * If all svLineText stored in a single vector, performance On editing large files(a file contain lots of lines) is slow.
 * So, we break it into pieces of vectors to improving performance. 
 * BUF_SECTION_LEN is the max number of svLineText a vector can stored (in initial) when SINGLE_BUFFER is not defined.
 
 Using vector as the foundamental data structure is slow when paste lines.
 2015/11/28 Trying to Use list as the foundamental data structure, but the performance is still slow on pasting and every text operation. I guess that's because the frequently iterator advance operation is time consumming.
 
 Is there any data structure performance well in every situation.

 */ 
 
//#define SINGLE_BUFFER         // using a single vector as the foundamental data structure.
#define BUF_SECTION_LEN 10000 // the initial lenght of a buftext Section

enum
{
    SVID_FORWARD=0,
    SVID_BACKWARD,

    SVID_LINE_COMMENTED,
    SVID_LINE_UNCOMMENTED,
    SVID_LINE_NOT_CHANGED,

    SVID_BLOCK_COMMENTED_MULTI_LINE,
    SVID_BLOCK_UNCOMMENTED_MULTI_LINE,
    SVID_BLOCK_COMMENTED_SINGLE_LINE,
    SVID_BLOCK_UNCOMMENTED_SINGLE_LINE,

    SVID_NEXT_MATCH,
    SVID_PREV_MATCH
};


class svBufText
{
private:
#ifdef SINGLE_BUFFER
    vector<svLineText> m_buftext; 
    //list<svLineText> m_buftext; 
#else
    vector<vector<svLineText> > m_buftextSection;
#endif
    UChar* m_bufUCharAll;      // The file in UChar format.
    // bool m_bufUCharAllDirty;   // true if it's dirty(the m_bufUCharAll is not consist with the current buffer, or the file is been edited and the m_bufUCharAll is not updated.)
    // bool m_dirtyAfterSearch;   // true if the file is been edited after searched. Research is needed.
    int m_bufUCharAllLen;
    vector<svIntPair> m_newlineLocations;  // location of newline for m_bufUCharAll.

    wxString m_charSet;
    char *m_CScharSet;     // CS for C String
    wxString m_EOL_wxStr;  // end of line type.
    char m_EOL_type;       // end of line type.
    size_t m_unitSize;     // 一個cr或lf字元所佔的 bytes 數
    wxString m_filename;

    string m_synName;
    vector<re_rule> m_synReRules;
    // int m_synDefPid;           // definition pid of syntax regular expression rule.
    vector<int> m_synDefPid;           // definition pid of syntax regular expression rule.

    vector<pairSymbol> m_foldingSymbol;
    vector<pairSymbol> m_embraceSymbol;

    svCaretList m_carets;               // List of carets.    

    svUndoActionList m_undoActions;     // svUndoAction list for undo.

    svDictionary m_hintDict;            // all keyword for hint dictionay. 提示字串資料庫
    vector<wxString> m_availableHint;   // hint for to be select by user. caret所在位置的提示字元

    // UCharText m_currentWord2Find;       // for Quick find (find current word)

    // wxString m_wxStr2Find;              // wxString for find (from find window) 目前只單行，無換行符號
    // wxString m_wxStr2Replace;           // wxString for replace (from replace window) 目前只單行，無換行符號
    svFindReplaceOption m_frOption;     // Find & Replace Options.  
    vector<svInt2Pair> m_matchLocations;        // a vector stored all found match string locations.

    int m_folding_type;                 // The way we processing folding. 

    svSeqCodeGenerator m_seqCodeGen;    // sequential code generator.
    sv_seq_code m_savedSeqCode;         // The last saved sequential code. for buffer modified status checking.
    bool m_modified;                     // Status for buffer modified or not.
    svUndoStatus m_searchUndoStatus;    // the svUndoStatus of the moment of searching. 

private:
    void break_file_by_crlf(const char *filename, int unit_size, int endian);
    void append_new_line(svLineText const *p_line);
    void insert_new_line(int uw_idx, svLineText const *p_line);
    void insert_new_lines(int uw_idx, vector<svLineText> &p_newlines);
    void delete_line(int uw_idx);
    void delete_line_range(int uw_idx, int p_cnt);
    inline
    void clear_buffer_sections(void)
    {
#ifdef SINGLE_BUFFER
        m_buftext.clear();
#else
        for(int i=0; i<(int)m_buftextSection.size(); ++i)
        {
            m_buftextSection.at(i).clear();
        }
        m_buftextSection.clear();
#endif
    }

    bool get_only_the_line_position(const int p_baseRow, int &p_scol, int &p_ecol, bool &p_eol, const int p_startRow, const int p_startCol, const int p_endRow, const int p_endCol);
    bool get_only_the_line_position(const int p_baseRow, const int p_basescol, const int p_baseecol, int &p_scol, int &p_ecol, bool &p_eol, const int p_startRow, const int p_startCol, const int p_endRow, const int p_endCol);
    bool get_position_and(const int p_scol1, const int p_ecol1, const int p_scol2, const int p_ecol2, int &p_xscol, int &p_xecol);

    // bool get_line_comment(wxString &p_lineComment);
    // bool get_block_comment(wxString &p_startComment, wxString &p_endComment);

public:

    /*
     * Functions name end with '_W' means it's for wrapped line, 
     * else is for unwrapped line(index of m_bufText).
     * parameter uw_idx stands for unwrapped line number index.
     */
    svBufText();
    ~svBufText();

    bool InitNewFile(const wxString& filename, svTextEditorCtrl *editor);
    bool ReadTextFile(const wxString& filename, svTextEditorCtrl *editor);
    // bool ReadTextFile(const wxString& filename);
    // bool ReadTextFile(const wxString& filename, void *ed, void (svTextEditorCtrl::*func)(const wxString &p_msg));
    // bool ReadTextFileICU(const wxString& filename);

    wxString GetFileName(void);
    // wxString* GetTextUW(void);

    svLineText *GetLine(int uw_idx);

    wxString GetTextAt(int uw_idx);
    int GetTextLengthAt(int uw_idx);
    wxString GetTextBeforeAt(int uw_idx, int pos);
    wxString GetTextAfterAt(int uw_idx, int pos);
    wxString GetTextAt(int uw_idx, int p_scol, int p_len);
    wxString GetTextRange(int p_s_row, int p_s_col, int p_e_row, int p_e_col);

    size_t TextLenAt(size_t uw_idx, short int p_type);

    bool SetLineTextAt(int uw_idx, const wxString& txt);

    bool InsertAfterLineAt(int uw_idx, const wxString& txt);
    bool InsertBeforeLineAt(int uw_idx, const wxString& txt);
    void InsertTextAt(int uw_idx, int sx, const wxString& txt, int &p_insertLineCnt, int &p_lastLineLen);

    wxString DeleteLineAt(int uw_idx);
    wxString DeleteLineRangeAt(int uw_idx, int p_len);
    wxString DeleteTextAfterAt(int uw_idx, int x);
    wxString DeleteTextBeforeAt(int uw_idx, int x);
    wxString DeleteTextAt(int uw_idx, int sx, int ex);
    wxString DeleteTextRange(int p_s_row, int p_s_col, int p_e_row, int p_e_col);

    void SplitLineAt(int uw_idx, int sx);
    bool JoinNextLineAt(int uw_idx);
    wxString DuplicateLineAt(int uw_idx);

    char LineCommentLineAt(int uw_idx, const wxString &p_lineCom);
    char BlockCommentAt(int p_sr, int p_sc, int p_er, int p_ec, const wxString &p_startCom, const wxString &p_endCom);

    size_t  LineCntUW(void);
    inline
    wxString GetOriginalCharSet(void)
    {
        return m_charSet;
    }

    void SetEOLType(const char p_type);
    char GetEOLType(void);

    // bool SearchUW(const wxString& token, svSearchPozList* posList);
    // bool ReplaceUW(int idx, int sx, const wxString& oldtxt, const wxString& newtxt);

    vector<vector<styledWrapText> >* GetStyledWrapTextAt(size_t idx);
    vector<styledWrapText>* GetStyledWrapTextAt(size_t uw_idx, size_t w_idx);

    size_t GetWrapLineCountAt(size_t uw_idx);
    bool GetWrapLenOnIdxAt(size_t uw_idx, size_t p_idx, wrapLenDesc& p_wrapLenDesc);
    int InWhichWrappedLineAt(size_t uw_idx, size_t p_idx);
    bool InWhichWrappedLineColAt(size_t uw_idx, size_t p_idx, int &p_wrapline, int &p_wrapcol);
    int GetWrappedLineStartColOnIdxAt(size_t uw_idx, size_t p_idx);
    int GetWrappedLineStartColOnPositionAt(size_t uw_idx, size_t p_idx);


    // void ProcWrapLineAt(size_t uw_idx, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth);
    // void ProcStyledTextAt(size_t uw_idx);
    // void ProcStyledWrapLineTextAt(size_t uw_idx);
    // bool IsStyledWrapProcessedAt(size_t uw_idx);
    inline
    bool IsStyledWrapProcessedAt(size_t uw_idx)
    {
        return GetLine(uw_idx)->IsStyledWrapProcessed();
    }


    void ProcWrapAndStyleAt(size_t uw_idx, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void ProcWrapAndStyleAll(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);

    // bool ConvertBuffer2StringAt(size_t uw_idx);

    bool PixelX2TextColAt(size_t uw_idx, int p_pixelX, size_t& p_textCol);
    bool TextCol2PixelXAt(size_t uw_idx, size_t p_textCol, int& p_pixelX);
    bool TextCol2PixelXAtW(size_t uw_idx, size_t p_textCol, int& p_pixelX);


    // ------------------------------------------------------------------------------------- //
    // Visible related functions.
    // ------------------------------------------------------------------------------------- //
    bool VisibleAt(size_t uw_idx);
    void VisibleAt(size_t uw_idx, bool p_visible);
    bool GetPrevVisibleLine(size_t uw_idx, int &o_row);
    bool GetNextVisibleLine(size_t uw_idx, int &o_row);
    int  GetLastVisibleLine(void);

    // ------------------------------------------------------------------------------------- //
    // Wrap version member functions.
    // ------------------------------------------------------------------------------------- //
    size_t GetAvailableLineCnt_W(void);
    vector<styledWrapText>* GetStyledWrapTextAt_W(size_t idx);
    // ------------------------------------------------------------------------------------- //


    bool DetectSyntaxType(void);
    bool LoadSyntaxDesc(const char*);
    inline
    bool IsPythonSyntax(void)
    {
        wxString syn = wxString(m_synName.c_str(), wxConvUTF8);
        if (syn.CmpNoCase(_("Python"))==0)
            return true;
        else
            return false;
    }

    // ------------------------------------------------------------------------------------- //
    // Create keyword table functions.
    // ------------------------------------------------------------------------------------- //
    bool CreateKeywordTableOnLoading(svTextEditorCtrl *editor);
    bool CreateKeywordTableAll(void);
    bool CreateKeywordTableAt(size_t uw_idx);
    bool ConvertBufferICUAll(void);

    // ------------------------------------------------------------------------------------- //
    // Create hint dictionary functions.
    // ------------------------------------------------------------------------------------- //
    void CreateHintDictionaryAll(void);

    // ------------------------------------------------------------------------------------- //
    // Caret related functions.
    // ------------------------------------------------------------------------------------- //
    // int GetDiffBufTextIndex(const bufTextIndex& p_oriIdx, int p_diff, bufTextIndex& p_newIdx);
    bool GetCaretsOfRange(int32_t p_srow, int32_t p_scol, int32_t p_erow, int32_t p_ecol, vector<svCaret> **p_inCaretList, vector<svCaret> **p_outCaretList);

    void SingleCaretMoveLeft(const int p_irow, const int p_icol, int &p_orow, int &p_ocol, int &p_keepSpace);
    void SingleCaretMoveRight(const int p_irow, const int p_icol, int &p_orow, int &p_ocol, int &p_keepSpace);
    void SingleCaretMoveUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                           const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol, bool p_isWrap);
    void SingleCaretMoveDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                             const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol, bool p_isWrap);
    void SingleCaretMoveTo(const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol);
                             

    void CaretsLeft(void);
    void CaretsLeftHead(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void CaretsRight(void);
    void CaretsRightEnd(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void CaretsUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth, bool p_isWrap);
    void CaretsDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth, bool p_isWrap);

    void CaretsLineHead_W(void);
    void CaretsLineEnd_W(void);

    void RemoveHiddenLineCarets(void);
    inline
    void RemoveDuplication(void)
    {
        m_carets.RemoveDuplication();
    }

    void GetCaretData(int idx, int &row, int &vcol, int &kspace);

    void ClearCaretSelect(void);
    void SetCaretSelect(void);

    inline
    void KeepFirstCaret(void)
    {
        m_carets.KeepFirstAndClearElse();
    }

    inline
    void KeepLastCaret(void)
    {
        m_carets.KeepLastAndClearElse();
    }

    bool GetCaretSelectPixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList);
    bool GetTextRangePixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList, const vector<textRange> p_rangeText);

    // bool GetLastCaretPixelXY(int &p_x, int &p_y);

    bool CaretsHasSelect(void);

    void ResetCaretPosition(const size_t p_row, const size_t p_col);
    void AppendCaretPosition(const size_t p_row, const size_t p_col);

    void LastCaretMoveTo(const size_t p_row, const size_t p_col);
    void CaretsSelectCharacters(int p_length);
    
    bool CaretsMergeOverlap(void);

    inline
    bool CaretsIsOverlaped(const svCaret p_target)
    {
        return m_carets.IsOverlaped(p_target);
    }

    inline
    vector<svCaret> GetCaretsList(void)
    {
        return m_carets.m_caretsList;
    }

    inline
    svCaret *GetLastCaret(void)
    {
        return m_carets.GetLastCaret();
    }

    bool GetCaretsCurrentUChar(UChar **p_curText, int &p_len, int &p_offset);
    bool GetCaretsCurrentUCharLen(int &p_len, int &p_offset);
    void UpdateAvailableHint(void);
    inline
    void SetCarets(const vector<svCaret> p_carets)
    {
        if (p_carets.size()>0)
            m_carets.m_caretsList = p_carets;
    }
    inline
    int32_t GetAvailableHintCnt(void)
    {
        return m_availableHint.size();
    }
    inline
    vector<wxString> GetAvailableHint(void)
    {
        return m_availableHint;
    }

    // ------------------------------------------------------------------------------------- //
    // Edit related functions.
    // ------------------------------------------------------------------------------------- //
    void EditingInsertChar(const wxString &p_str, bool p_cont, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingSplitLine(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingTextDelete(bool p_cont);
    void EditingTextBackDelete(bool p_cont);
    void EditingSelectedTextDelete(void);
    void EditingTextCopySelected(void);
    void EditingPaste(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingDuplicateLine(void);
    void EditingLineComment(const wxString &p_lineComment, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingBlockComment(const wxString &p_startComment, const wxString &p_endComment, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingIndent(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    void EditingOutdent(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);

    // ------------------------------------------------------------------------------------- //
    // undo actions related functions.
    // ------------------------------------------------------------------------------------- //

    void UndoEditing(void);
    void UndoEditingDelete(const svUndoAction &p_act);
    void UndoEditingInsert(const svUndoAction &p_act);
    void UndoEditingCut(const svUndoAction &p_act);
    void UndoEditingPaste(const svUndoAction &p_act);
    void UndoEditingJoin(const svUndoAction &p_act);
    void UndoEditingSplit(const svUndoAction &p_act);
    void UndoEditingDeleteLine(const svUndoAction &p_act);
    void UndoEditingLineComment(const svUndoAction &p_act);

    void InitialNewUndoActions(void);

    bool CheckContinousUndoOperation(const int p_actionType);
    inline
    bool LastUndoActionsIsEmpty(void)
    {
        return m_undoActions.LastUndoActionsIsEmpty();
    }

    // ------------------------------------------------------------------------------------- //
    // find & replace related functions.
    // ------------------------------------------------------------------------------------- //
    // inline
    // void SetBufUCharAllDirty(bool p_flag=true)
    // {
    //     m_bufUCharAllDirty = p_flag;
    // }
    inline
    bool FileChangedSinceLastSearch(void)
    {
        return m_searchUndoStatus!=m_undoActions.GetUndoStatus();
    }

    bool FindMatchLocations(const svFindReplaceOption &p_fro);
    bool FindwxStrMatchLocations(const svFindReplaceOption &p_fro, vector<svInt2Pair> &p_locations);
    bool FindUCharMatchLocations(const UCharText &p_keyword, vector<svInt2Pair> &p_locations);
    bool FindRegexMatchLocations(const svFindReplaceOption &p_fro, vector<svInt2Pair> &p_locList);
    bool CreateLineRegexFindResult(vector<svIntPair> &p_foundList, vector<svIntPair> &p_crlfList, vector<svInt2Pair> &p_resultList);

    bool HasMatchedWord(void)
    {
        if (m_matchLocations.size())
            return true;
        else
            return false;
    }

    void MatchOnSelection(void);

    bool CaretMoveToMatch(int p_direction, int &p_destRow, int &p_destCol, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    bool CaretMoveToAllMatch(int &p_destRow, int &p_destCol, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);

    bool GetFindMatchPixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList);

    bool Replace(int p_direction, const svFindReplaceOption &p_fro, int &p_newRow, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);
    bool ReplaceAll(const svFindReplaceOption &p_fro, int &p_newRow, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap);

    inline
    svFindReplaceOption GetLastFindReplaceOption(void)
    {
        return m_frOption;
    }
    // ------------------------------------------------------------------------------------- //
    // Update status.
    // ------------------------------------------------------------------------------------- //
    inline
    bool IsModified(void)
    {
        return m_modified;
    }

    inline
    void Modified(const bool p_status=true)
    {
        m_modified = p_status;
    }

    bool SaveToFile(const wxString &p_filename);
    bool SaveToBigBuffer(char **p_buffer, int &p_bufLen);

    /*------------------------------------------------------------------------------------
     * Folding related functions.
     *------------------------------------------------------------------------------------*/
    bool FoldingAt(int uw_idx_s, int uw_idx_e);
    bool UnfoldingAt(int uw_idx);

    vector<pairSymbol> GetFoldingSymbol(void)
    {
        return m_foldingSymbol;
    }
    void UnfoldingAtCarets(void);

    void FindEmbracePosInRange(const int p_cr, const int p_cc, vector<pairSymbol> &p_ps, const int p_range_sr, const int p_range_er, int &p_sr, int &p_sc, int &p_er, int &p_ec, bool &p_sfound, bool &p_efound, pairSymbol &p_sFoundSymbol, pairSymbol &p_eFoundSymbol);

    vector<pairSymbol> GetEmbraceSymbol(void)
    {
        return m_embraceSymbol;
    }

    bool GetLineCommentSymbol(wxString &p_lineComment);
    bool GetBlockCommentSymbol(wxString &p_startComment, wxString &p_endComment);

    bool GetDefinitionLineNo(vector<svIntText> &p_defLineNo);


private:
    // int DetectFileEncoding(const char *filename);    
    char* DetectFileEncoding(const char *filename);    
    UErrorCode ConvFileICU(const char *ifname, UChar **obuf, int32_t *osize);
    UErrorCode ConvBufferICU(const char *ibuf, int32_t isize, UChar **obuf, int32_t *osize);

    /*
     * Keyword processing (syntax hilight) functions.
     * syntax hilight
     */
    bool InsertKeywordUnit(vector<keywordUnit>* p_keywordList, const keywordUnit* p_ku);
    // bool InsertKeywordUnit2(vector<keywordUnit>* p_keywordList, const keywordUnit& p_ku);
    bool InsertHintUnit(vector<keywordUnit>* p_hintList, const keywordUnit* p_ku);
    // bool InsertHintUnit2(vector<keywordUnit>* p_hintList, const keywordUnit& p_ku);
    bool InsertBlockTagKeywordUnit(vector<keywordUnit>* p_blockTagList, const keywordUnit* p_ku);
    bool CreateLineKeywordTable(vector<vector<keywordUnit> >* p_KWLList, vector<vector<keywordUnit> >* p_BKWLList, vector<vector<keywordUnit> >* p_HWLList, vector<keywordUnit>* p_crlfList);    

    UErrorCode REMatchPattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end, vector<keywordUnit> *p_keywordList, vector<keywordUnit> *p_bkeywordList);
    UErrorCode REMatchPattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList);
    UErrorCode REMatchKeyword_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList);
    UErrorCode REMatchKeyword_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_hintList);
    UErrorCode REFind(const UChar *ibuf, int32_t ibufSize, const UChar *p_pattern, int32_t p_patternLen, vector<svIntPair> &p_locList, bool p_case);
    UErrorCode ConvertwxStrToUChar(const wxString &p_str, UChar **p_ubuf, int32_t &p_ubufLen);
    UErrorCode ConvertUCharTowxStr(UChar *p_ubuf, int32_t p_ubufLen, wxString &p_str);


    bool CheckBlockTag(void);
    bool GetNextBlockTag(const UChar *p_tag, const int32_t p_tagLen, const size_t p_row, const size_t p_col, size_t *r_row, size_t *r_col);
    void ClearBlockKeywordUnitPID(const size_t p_start_row, const size_t p_start_col, const size_t p_end_row, const size_t p_end_col, const int p_pid);
    bool ChangeBlockKeywordUnitPID(const size_t p_start_row, const size_t p_start_col, const size_t p_end_row, const size_t p_end_col, const int p_pid, const int32_t p_stagLen, const int32_t p_etagLen);

};

#endif
