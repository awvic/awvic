/*
   Copyright Notice in awvic.cpp
*/

#include "svPozList.h"

#include "stdwx.h"

svPoz::svPoz()
{
}

svPoz::svPoz(int r, int c):
row(r),
col(c)
{
}

svPozLen::svPozLen(int r, int c, int l)
:svPoz(r, c)
{
    len = l;
}


svPozList::svPozList()
{
}

svPozList::~svPozList()
{
    m_pozList.clear();
}

void svPozList::Clear(void)
{
    m_pozList.clear();
}

void svPozList::Append(const svPozLen& pos)
{
    m_pozList.push_back(pos);
}

void svPozList::Append(svPozLen* pos)
{
    m_pozList.push_back(*pos);
}

int svPozList::Size(void)
{
    return m_pozList.size();
}


svPozLen svPozList::Get(int idx)
{
    return m_pozList.at(idx);
}

svSearchPozList::svSearchPozList()
:svPozList()
{
    m_token=NULL;
}

svSearchPozList::~svSearchPozList()
{
    if (m_token!=NULL) 
    delete m_token;
}

wxString* svSearchPozList::GetToken(void)
{
    if (m_token!=NULL)
    return m_token;
    else
    return NULL;
}

void svSearchPozList::SetToken(const wxString& token)
{
    if (m_token!=NULL) delete m_token;
    m_token = new wxString(token);
}

void svSearchPozList::Clear(void)
{
    this->svPozList::Clear();
    if (m_token!=NULL)
    {
        delete m_token;
        m_token=NULL;
    }
}

bool svSearchPozList::GetInTextAreaPozIdx(int ssx, int ssy, 
int sex, int sey,
int& s, int& e)
{
    vector<svPozLen>::iterator i;
    bool found = false;
    int min = 32765;
    int max = -32765;
    int idx = 0;

    for (i=m_pozList.begin(); i<m_pozList.end(); ++i)
    {
        //if (((*i).row >= ssy) && ((*i).row <= sey))
        if ((i->row >= ssy) && (i->row <= sey))
        {
            found = true;
            if (idx<min) min = idx;
            if (idx>max) max = idx;
        }
        if (i->row>sey) break;
        idx++;
    }

    if (found)
    {
        s = min;
        e = max;
    }

    return found;

}

//
// cy = caret Y; cx = caret X;
bool svSearchPozList::GetSearchedTextPos(int cx, int cy, 
int& fx, int& fy,
int dir)
{
    if (m_pozList.size()==0) return false;

    vector<svPozLen>::iterator i;
    int idx = 0;

    for (i=m_pozList.begin(); i<m_pozList.end(); ++i)
    {
        if (i->row < cy)
        {
        }
        else if (i->row == cy)
        {
            if (i->col > cx)
            break;
        }
        else if (i->row > cy)
        {
            break;
        }
        idx++;
    }

    if (dir==SVID_NEXT)
    {
        if (idx==m_pozList.size())
        {
            fx = m_pozList.at(0).col;
            fy = m_pozList.at(0).row;
        }
        else
        {
            fx = m_pozList.at(idx).col;
            fy = m_pozList.at(idx).row;
        }
    }
    else if (dir==SVID_PREV)
    {
        if (idx==0) idx = m_pozList.size();
        idx--;
        if ((m_pozList.at(idx).row==cy) &&
                (m_pozList.at(idx).col==cx))
        {
            if (idx==0)
            {
                fx = m_pozList.back().col;
                fy = m_pozList.back().row;
            }
            else
            {
                fx = m_pozList.at(idx-1).col;
                fy = m_pozList.at(idx-1).row;
            }
        }
        else
        {
            fx = m_pozList.at(idx).col;
            fy = m_pozList.at(idx).row;
        }
    }

    return true;

}


void svSearchPozList::ResetSearchedTextPos(int row, int col, int oldLen, int newLen)
{
    if (m_pozList.size()==0) return;

    vector<svPozLen>::iterator i;

    for (i=m_pozList.begin(); i<m_pozList.end(); ++i)
    {
        if (i->row < row)
        {
        }
        else if (i->row == row)
        {
            if (i->col <= col)
            {
            }
            else if (i->col > col)
            {
                i->col += (newLen-oldLen);
            }
        }
        else if (i->row > row)
        {
            break;
        }
    }
}
