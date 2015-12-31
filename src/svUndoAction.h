/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVUNDOACTION_H
#define _SVUNDOACTION_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "svCaret.h"
#include "svBaseType.h"

#include <vector>
using namespace std;

#define MAX_ACTIONLIST 32767

enum
{
    SVID_UNDO_NOTHING=0,
    SVID_UNDO_INSERT,
    SVID_UNDO_DELETE,
    SVID_UNDO_BACKDEL,
    SVID_UNDO_CUT,
    SVID_UNDO_PASTE,
    SVID_UNDO_SPLIT,
    SVID_UNDO_JOIN,
    SVID_UNDO_DELETE_LINE,
    SVID_UNDO_LINE_COMMENT
};

// 用以紀錄可undo的動作
class svUndoAction
{
public:
    svUndoAction();
    svUndoAction(int p_type, int p_srow, int p_scol, const wxString& text);
    svUndoAction(int p_type, int p_srow, int p_scol, int p_erow, int p_ecol, const wxString& text);
    ~svUndoAction();
    void SetUndoAction(int Action, int p_srow, int p_scol, const wxString& text);
    void SetUndoAction(int Action, int p_srow, int p_scol, int p_erow, int p_ecol, const wxString& text);

    inline
    int GetType(void) const
    {
        return m_type; 
    }

    // inline
    // wxString GetName(void)
    // {
    //     switch (m_type)
    //     {
    //     case SVID_UNDO_NOTHING:
    //         return wxT("SVID_UNDO_NOTHING");
    //         break;
    //     case SVID_UNDO_INSERT:
    //         return wxT("SVID_UNDO_INSERT");
    //         break;
    //     case SVID_UNDO_DELETE:
    //         return wxT("SVID_UNDO_DELETE");
    //         break;
    //     case SVID_UNDO_BACKDEL:
    //         return wxT("SVID_UNDO_BACKDEL");
    //         break;
    //     case SVID_UNDO_PASTE:
    //         return wxT("SVID_UNDO_PASTE");
    //         break;
    //     case SVID_UNDO_SPLIT:
    //         return wxT("SVID_UNDO_SPLIT");
    //         break;
    //     case SVID_UNDO_JOIN:
    //         return wxT("SVID_UNDO_JOIN");
    //         break;
    //     default:
    //         return wxT("unknown action type");
    //     }

    // }

    inline
    wxString GetText(void) const
    {
        return m_modifiedText;
    }

    inline 
    void SetText(const wxString &p_txt)
    {
        m_modifiedText = p_txt;
    }

    inline
    int GetStartRow(void) const
    {
        return m_sRow;
    }

    inline
    int GetStartCol(void) const
    {
        return m_sCol;
    }

    inline
    int GetEndRow(void) const
    {
        return m_eRow;
    }

    inline
    int GetEndCol(void) const
    {
        return m_eCol;
    }

    inline
    void SetStartCol(int p_scol)
    {
        m_sCol = p_scol;
    }

    inline
    void SetEndCol(int p_ecol)
    {
        m_eCol = p_ecol;
    }

    inline
    void GetPosition(int& p_srow, int& p_scol, int& p_erow, int& p_ecol)
    {
        p_srow = m_sRow;
        p_scol = m_sCol;
        p_erow = m_eRow;
        p_ecol = m_eCol;
    }


private:
    int m_type;      // action type.
    int m_sRow, m_sCol;    // action start position.
    int m_eRow, m_eCol;    // action end position.  SVID_UNDO_DELETE SVID_UNDO_CUT 時會參考
    wxString m_modifiedText;
};


/* --------------------------------------------------------------------- *
 * svUndoActionList is a vector of vector of svUndoActions.
 * every vector of avUndoActions on the vector is an user operated action  
 * on editing.
 * --------------------------------------------------------------------- */

class svUndoActionList
{
public:
    svUndoActionList();
    ~svUndoActionList();
    void Append(const sv_seq_code p_seqCode);
    // void Append(const vector<svUndoAction> &p_actList, const vector<svCaret> &p_cList);
    void Add2TheLast(const svUndoAction &p_act);
    void Add2TheLast(int p_actionType, int p_row, int p_col, const wxString& text);
    // void Add2TheLast(const svCaret &p_caret);
    void Merge2TheLast(const svUndoAction &p_act, const int p_caretIdx, int p_originalType);
    inline
    void RecordCarets(const carets_info &p_c)
    {
        m_caretsList.back() = p_c;
    }
    void ShowList(void);
    int  GetLastUndoActionsCnt(void);
    vector<svUndoAction> GetLastUndoActions(int p_idx);
    vector<svUndoAction> GetLastUndoActions(void);
    carets_info GetLastCarets(void);
    sv_seq_code DeleteLastUndoActions(void);
    void Clear(void);
    bool Empty(void);
    size_t Size(void);
    bool ValidateSize(void);
    inline
    bool LastUndoActionsIsEmpty(void)
    {
        if (!m_undoList.back().size())
            return true;
        else
            return false;
    }
    svUndoStatus GetUndoStatus(void);

private:
    vector<vector<svUndoAction> > m_undoList;
    vector<carets_info> m_caretsList;
    // every undo has it's own sv_seq_code and will be unique.
    // It's the number for checking status of buffer modified or not.
    // m_seqCodeList 是存放 svSeqCode 的資料結構
    // 每一個可以 undoActions 都有一個 svSeqCode, 
    // 每增加一個 undoActions 其 svSeqCode 會累加且不會重覆
    // 當還原 undoActions 會比對其 svSeqCode 與 svBufText 的 m_savedSeqCode 是否相同
    // 如果相同，則表示 svBufText 在此時是未經過異動的
    // svBufText 在存檔時會紀錄下當時的 svSeqCode 並存放在 m_savedSeqCode
    vector<sv_seq_code> m_seqCodeList;
};

#endif
