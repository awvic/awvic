/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVACTION_H
#define _SVACTION_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "svCaret.h"

#include <time.h>
#include <vector>
using namespace std;

#define MAX_ACTION_SIZE 100
#define SVID_DUP_SEC  3            // 檢查重覆動作時的秒數(只計算幾秒內的重覆動作) 

enum
{
    SVID_ACTION_NOTHING=0,
    SVID_ACTION_INSERT,
    SVID_ACTION_INSERT_HINT,
    SVID_ACTION_DELETE,
    SVID_ACTION_BACKDEL,
    SVID_ACTION_COPY,
    SVID_ACTION_CUT,
    SVID_ACTION_PASTE,
    SVID_ACTION_SPLIT,
    SVID_ACTION_JOIN,
    SVID_ACTION_DUP_LINE,
    SVID_ACTION_LINE_COMMENT,
    SVID_ACTION_BLOCK_COMMENT,
    SVID_ACTION_INDENT,
    SVID_ACTION_OUTDENT,

    SVID_ACTION_UNDO,

    SVID_ACTION_CARET_UP,
    SVID_ACTION_CARET_DOWN,
    SVID_ACTION_CARET_LEFT,
    SVID_ACTION_CARET_RIGHT,

    SVID_ACTION_CARET_HEAD_OF_LINE,
    SVID_ACTION_CARET_END_OF_LINE,
    SVID_ACTION_CARET_RESET,

    SVID_ACTION_PAGE_UP,
    SVID_ACTION_PAGE_DOWN,

    SVID_ACTION_H_SCROLL_LINE_UP,
    SVID_ACTION_H_SCROLL_PAGE_UP,
    SVID_ACTION_H_SCROLL_THUMB_TRACK,
    SVID_ACTION_H_SCROLL_LINE_DOWN,
    SVID_ACTION_H_SCROLL_PAGE_DOWN,

    SVID_ACTION_V_SCROLL_LINE_UP,
    SVID_ACTION_V_SCROLL_PAGE_UP,
    SVID_ACTION_V_SCROLL_THUMB_TRACK,
    SVID_ACTION_V_SCROLL_LINE_DOWN,
    SVID_ACTION_V_SCROLL_PAGE_DOWN,

    SVID_ACTION_FIND_CURRENT_WORD,
    SVID_ACTION_FIND_CURRENT_WORD_NEXT,
    SVID_ACTION_FIND_CURRENT_WORD_PREV,
    SVID_ACTION_FIND_REGEX,

    SVID_ACTION_FIND_NEXT,
    SVID_ACTION_FIND_PREV


};

// 用以紀錄使用者操作的動作，可能部份會與svAction重疉
class svAction
{
public:
    svAction();
    svAction(int p_type);
    svAction(int p_type, int p_srow, int p_scol);
    svAction(int p_type, int p_srow, int p_scol, int p_erow, int p_ecol);
    ~svAction();

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
    //     case SVID_ACTION_NOTHING:
    //         return wxT("SVID_ACTION_NOTHING");
    //         break;
    //     case SVID_ACTION_INSERT:
    //         return wxT("SVID_ACTION_INSERT");
    //         break;
    //     case SVID_ACTION_DELETE:
    //         return wxT("SVID_ACTION_DELETE");
    //         break;
    //     case SVID_ACTION_BACKDEL:
    //         return wxT("SVID_ACTION_BACKDEL");
    //         break;
    //     case SVID_ACTION_PASTE:
    //         return wxT("SVID_ACTION_PASTE");
    //         break;
    //     case SVID_ACTION_SPLIT:
    //         return wxT("SVID_ACTION_SPLIT");
    //         break;
    //     case SVID_ACTION_JOIN:
    //         return wxT("SVID_ACTION_JOIN");
    //         break;
    //     default:
    //         return wxT("unknown action type");
    //     }

    // }

    // inline
    // wxString GetText(void) const
    // {
    //     return m_modifiedText;
    // }

    // inline 
    // void SetText(const wxString &p_txt)
    // {
    //     m_modifiedText = p_txt;
    // }

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

    inline
    time_t GetTime(void)
    {
        return m_time;
    }

    inline
    void SetTime(const time_t &p_time)
    {
        m_time = p_time;
    }

private:
    int m_type;            // action type.
    // m_sRow, m_sCol, m_eRow, m_eCol is reserved and not been used.
    int m_sRow, m_sCol;    // action start position.
    int m_eRow, m_eCol;    // action end position.  SVID_ACTION_DELETE SVID_ACTION_CUT 時會參考
    time_t m_time;
};


/* --------------------------------------------------------------------- *
 * svActionList is a vector of vector of svActions.
 * every vector of avActions on the vector is an action users operated 
 * on editing.
 * --------------------------------------------------------------------- */

class svActionList
{
public:
    svActionList();
    ~svActionList();
    void Append(const svAction &p_act);
    void Append(int p_actionType, int p_row, int p_col);
    svAction GetLastActions(int p_idx);
    svAction GetLastActions(void);
    void Clear(void);
    bool Empty(void);
    size_t Size(void);
    bool DuplicatedAction(const int p_type, const int p_count);
    bool LastActionIs(const int p_type);

private:
    vector<svAction>  m_actionList;
};

#endif
