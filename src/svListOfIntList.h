/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVLISTOFINTLIST_H
#define _SVLISTOFINTLIST_H


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

class svIntList
{
private:
    vector<unsigned int> m_intList;

public:
    svIntList();
    ~svIntList();
    void Clear(void);
    void Append(unsigned int data);
    int Size(void);
    unsigned int Get(int idx);
    unsigned int Sum(int idx);
    unsigned int Sum(int sidx, int eidx);
    unsigned int SumByLen(int sidx, int eidx);
    unsigned int DeSum(int idx, bool round);
    unsigned int DeSum(int isum, int start_pos, bool round);
    unsigned int DeSumLeft(int isum, int start_pos, int max_pos);

};

class svListOfIntList
{
private:
    vector<svIntList> m_ilList;

public:
    svListOfIntList();
    ~svListOfIntList();
    void Append(svIntList* data);
    svIntList* GetList(int idx);
    unsigned int GetItem(int row, int col);
    void Clear(void);
    unsigned int Sum(int row, int idx);
    unsigned int DeSum(int row, int idx, bool round);
    
};

#endif
