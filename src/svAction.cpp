/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include "svAction.h"

#include "stdwx.h"

svAction::svAction()
{
}

svAction::svAction(int p_type):
m_type(p_type),
m_sRow(0),
m_sCol(0),
m_eRow(0),
m_eCol(0)
{
}

svAction::svAction(int p_type, int p_srow, int p_scol):
m_type(p_type),
m_sRow(p_srow),
m_sCol(p_scol),
m_eRow(p_srow),
m_eCol(p_scol)
{
}

svAction::svAction(int p_type, int p_srow, int p_scol, int p_erow, int p_ecol):
m_type(p_type),
m_sRow(p_srow),
m_sCol(p_scol),
m_eRow(p_erow),
m_eCol(p_ecol)
{
}

svAction::~svAction()
{
}

/* ------------------------------------------------------------------------- *
 * 
 * Class svActionList
 * 
 * ------------------------------------------------------------------------- */
svActionList::svActionList()
{
}

svActionList::~svActionList()
{
    m_actionList.clear();
}

// push back a blank new node of m_actionList
void svActionList::Append(const svAction &p_action)
{
    while (m_actionList.size()>=MAX_ACTION_SIZE)
        m_actionList.erase(m_actionList.begin());

    svAction a = p_action;
    time_t t;
    time(&t);
    a.SetTime(t);
    m_actionList.push_back(a);
}

// Get last action  0 : last; -1 : last-1; ... 
svAction svActionList::GetLastActions(int idx)
{
    return m_actionList.at(m_actionList.size()+idx-1);
}

svAction svActionList::GetLastActions(void)
{
    return m_actionList.back();
}


void svActionList::Clear(void)
{
    m_actionList.clear();
}

bool svActionList::Empty(void)
{
    return m_actionList.empty();
}

size_t svActionList::Size(void)
{
    return m_actionList.size();
}

// 檢查最近的 p_count svAction 是否為相同的動作
// 只檢查 SVID_DUP_SEC 秒數內的動作
// 以避免 user 停頓，但仍被當成重覆動作
bool svActionList::DuplicatedAction(const int p_type, const int p_count)
{
    if ((int)m_actionList.size()<p_count)
        return false;

    time_t now;

    time(&now);

    for (vector<svAction>::reverse_iterator it=m_actionList.rbegin();
         it!=m_actionList.rbegin()+p_count;
         ++it)
    {
        if (it->GetType()!=p_type ||
            difftime(now, it->GetTime())>SVID_DUP_SEC)
        {
            return false;
        }
    }

    return true;
}

bool svActionList::LastActionIs(const int p_type)
{
    if (!m_actionList.size())
        return false;

    return (m_actionList.back().GetType()==p_type);
}