/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVTEXTSTYLELIST_H
#define _SVTEXTSTYLELIST_H

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

typedef struct{
    size_t startPos;
    size_t len;
    wxString styleName;
} txtStyleDesc;

class svTextStyleList
{
private:
    vector<txtStyleDesc> m_styleList;

public:
    svTextStyleList();
    ~svTextStyleList();
    void Clear(void);
    void Append(const txtStyleDesc& data);
    int Size(void);
    txtStyleDesc Get(int idx);

};

#endif
