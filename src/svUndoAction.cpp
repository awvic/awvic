/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include "svUndoAction.h"

#include "stdwx.h"

// svUndoAction::svUndoAction():
// m_modifiedText(wxT(""))
svUndoAction::svUndoAction()
{
}

svUndoAction::svUndoAction(int p_type, int p_srow, int p_scol, const wxString& inText):
m_type(p_type),
m_sRow(p_srow),
m_sCol(p_scol),
m_eRow(p_srow),
m_eCol(p_scol),
m_modifiedText(inText)
{
}

svUndoAction::svUndoAction(int p_type, int p_srow, int p_scol, int p_erow, int p_ecol, const wxString& inText):
m_type(p_type),
m_sRow(p_srow),
m_sCol(p_scol),
m_eRow(p_erow),
m_eCol(p_ecol),
m_modifiedText(inText)
{
}

svUndoAction::~svUndoAction()
{
}

void svUndoAction::SetUndoAction(int p_type, int p_srow, int p_scol, const wxString& inText)
{
    m_type = p_type;
    m_sRow = p_srow;
    m_sCol = p_scol;
    m_eRow = p_srow;
    m_eCol = p_scol;

    m_modifiedText = inText;
}

void svUndoAction::SetUndoAction(int p_type, int p_srow, int p_scol, int p_erow, int p_ecol, const wxString& inText)
{
    m_type = p_type;
    m_sRow = p_srow;
    m_sCol = p_scol;
    m_eRow = p_erow;
    m_eCol = p_ecol;

    m_modifiedText = inText;
}

/* ------------------------------------------------------------------------- *
 * 
 * Class svUndoActionList
 * 
 * ------------------------------------------------------------------------- */
svUndoActionList::svUndoActionList()
{
}

svUndoActionList::~svUndoActionList()
{
    m_undoList.clear();
    m_caretsList.clear();
    m_seqCodeList.clear();
}

// push back a blank new node of m_undoList
void svUndoActionList::Append(const sv_seq_code p_seqCode)
{
    if (!ValidateSize())
    {
        wxLogMessage("svUndoActionList::Append m_undoList.size() != m_caretsList.size()!");
    }

    vector<svUndoAction> p_actList;
    m_undoList.push_back(p_actList);

    carets_info p_c;
    m_caretsList.push_back(p_c);

    m_seqCodeList.push_back(p_seqCode);
}

// Add an undo action to the last vector of m_undoList.
void svUndoActionList::Add2TheLast(const svUndoAction &p_act)
{
    m_undoList.back().push_back(p_act);
}

// Add an undo action to the last vector of m_undoList.
void svUndoActionList::Add2TheLast(int p_actionType, int X, int Y, const wxString& text)
{
    svUndoAction act(p_actionType, X, Y, text);
    Add2TheLast(act);
}

// 只有 EditingInsertChar EditingDelete EditingBackDelete 且不跨行的operation時才會呼叫本函式
// 這個函式用以將相同undo動作者合併其資料
// 因為有多重 caret, 所以要找到正確的 caret 位置判斷
// 如不符合相同undo動作的條件，則新增一個
// 只考慮在同一行的狀況
void svUndoActionList::Merge2TheLast(const svUndoAction &p_act, const int p_caretIdx, int p_originalType)
{
    if (((int)m_undoList.back().size())>p_caretIdx && 
        m_undoList.back().at(p_caretIdx).GetType()==p_act.GetType())
    {
        int newCol = 0;

        switch(p_originalType)
        {
            case SVID_UNDO_INSERT:
                m_undoList.back().at(p_caretIdx).SetText(m_undoList.back().at(p_caretIdx).GetText() + p_act.GetText());
                newCol = m_undoList.back().at(p_caretIdx).GetEndCol() + m_undoList.back().at(p_caretIdx).GetText().Length();
                m_undoList.back().at(p_caretIdx).SetEndCol(newCol);
                break;
            case SVID_UNDO_DELETE:
                m_undoList.back().at(p_caretIdx).SetText(m_undoList.back().at(p_caretIdx).GetText() + p_act.GetText());
                break;
            case SVID_UNDO_BACKDEL:
                m_undoList.back().at(p_caretIdx).SetText(p_act.GetText() + m_undoList.back().at(p_caretIdx).GetText());
                newCol = p_act.GetStartCol();
                m_undoList.back().at(p_caretIdx).SetStartCol(newCol);
                break;
        }
    }
    else
    {
        m_undoList.back().push_back(p_act);
    }
}

int svUndoActionList::GetLastUndoActionsCnt(void)
{
    if (!m_undoList.size())
        return m_undoList.back().size();
    else
        return 0;
}

// Get last action  0 : last; -1 : last-1; ... 
vector<svUndoAction> svUndoActionList::GetLastUndoActions(int idx)
{
    return m_undoList.at(m_undoList.size()+idx-1);
}

vector<svUndoAction> svUndoActionList::GetLastUndoActions(void)
{
    return m_undoList.back();
}

carets_info svUndoActionList::GetLastCarets(void)
{
    return m_caretsList.back();
}

sv_seq_code svUndoActionList::DeleteLastUndoActions(void)
{
    if (!ValidateSize())
    {
        wxLogMessage("svUndoActionList::DeleteLastUndoActions m_undoList.size() != m_caretsList.size()!");
    }

    if (!m_undoList.empty())
    {
        m_undoList.back().clear();
        m_undoList.pop_back();

        m_caretsList.pop_back();

        sv_seq_code sc = m_seqCodeList.back();
        m_seqCodeList.pop_back();
        return sc;
    }

    return 0;
}

void svUndoActionList::Clear(void)
{
    m_undoList.clear();
    m_caretsList.clear();
}

void svUndoActionList::ShowList(void)
{
    // vector<int>::iterator ipos;
    // vector<svUndoAction>::iterator apos;

    // wxString msg;

    // int offset = 0;
    // for (ipos=m_undoActCnt.begin(); ipos<m_undoActCnt.end(); ++ipos)
    // {
    //     int cnt = *ipos;
    //     msg += wxString::Format(wxT("%i"), cnt);
    //     for (int i=0; i<cnt; i++)
    //     {
    //         msg += wxT(" ") + m_list.at(offset+i).GetActionName()
    //         + wxString::Format(wxT(" %i %i %i %i "), m_list.at(offset+i).GetStartRow(),
    //         m_list.at(offset+i).GetStartCol(), m_list.at(offset+i).GetEndRow(), m_list.at(offset+i).GetEndCol())
    //         + wxT(" ") + m_list.at(offset+i).GetText();
    //     }
    //     offset += cnt;
    //     msg += wxT("\n");
    // }

    // wxLogMessage(msg);

}

bool svUndoActionList::Empty(void)
{
    return m_undoList.empty();
}

size_t svUndoActionList::Size(void)
{
    return m_undoList.size();
}

bool svUndoActionList::ValidateSize(void)
{
    if (m_undoList.size()==m_caretsList.size() &&
        m_undoList.size()==m_seqCodeList.size())
        return true;
    else
        return false;
}

svUndoStatus svUndoActionList::GetUndoStatus(void)
{
    svUndoStatus us;

    us.m_undoCount = 0;
    us.m_lastUndoActionCount = 0;
    us.m_lastUndoTextLength = 0;

    if (m_undoList.size())
    {
        us.m_undoCount = m_undoList.size();
        if (m_undoList.back().size())
        {
            us.m_lastUndoActionCount = m_undoList.back().size();
            us.m_lastUndoTextLength = m_undoList.back().back().GetText().Length();
        }
    }

    return us;
}