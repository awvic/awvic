/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVPOZLIST_H
#define _SVPOZLIST_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <vector>
using namespace std;

enum
{
    SVID_NEXT=0,
    SVID_PREV
};

class svPoz
{
public:
    int row;
    int col;
    svPoz();
    svPoz(int r, int c);
};

class svPozLen : public svPoz
{
public:
    int len;
    svPozLen(int r, int c, int l);
};

class svPozList
{
protected:
    vector<svPozLen> m_pozList;

public:
    svPozList();
    ~svPozList();
    void Clear(void);
    void Append(const svPozLen& pos);
    void Append(svPozLen* pos);
    int Size(void);
    svPozLen Get(int idx);
};

class svSearchPozList : public svPozList
{
private:
    wxString* m_token;

public:
    svSearchPozList();
    ~svSearchPozList();
    wxString* GetToken(void);
    void SetToken(const wxString& token);
    void Clear(void);
    bool GetInTextAreaPozIdx(int ssx, int ssy, int sex, int sey, int& s, int& e);
    bool GetSearchedTextPos(int cx, int cy, int& fx, int& fy, int dir);
    void ResetSearchedTextPos(int row, int col, int oldLen, int newLen);
};

#endif
