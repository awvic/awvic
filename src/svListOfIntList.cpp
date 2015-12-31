/*
   Copyright Notice in awvic.cpp
*/

#include "svListOfIntList.h"

#include "stdwx.h"

/*
* svIntList
* A class for list of Integer.
*/

svIntList::svIntList()
{
}

svIntList::~svIntList()
{
    m_intList.clear();
}

void svIntList::Clear(void)
{
    m_intList.clear();
}

void svIntList::Append(unsigned int data)
{
    m_intList.push_back(data);
}

unsigned int svIntList::Get(int idx)
{
    return m_intList.at(idx);
}

int svIntList::Size(void)
{
    return m_intList.size();
}

unsigned int svIntList::Sum(int idx)
{
    unsigned int sum=0;
    for (int i=0; i<(int)m_intList.size() && i<idx; ++i) {
        sum += m_intList.begin() [i];
    }
    return sum;
}

// Sum(1, 7)
// 0 1 2 3 4 5 6 7 8 9 10 
//   x x x x x x
unsigned int svIntList::Sum(int sidx, int eidx)
{
    unsigned int sum=0;
    if (sidx==eidx)
        return 0;
    
    if (sidx >= (int)m_intList.size() ||
            eidx > (int)m_intList.size())
    {
        wxLogMessage(wxString::Format(wxT("svIntList index out of boundry: size=%i start=%i end=%i "), (int)m_intList.size(), (int)sidx, (int)eidx));
        return 0;
    }

    for (int i=sidx; i<(int)m_intList.size() && i<eidx; ++i) {
        sum += m_intList.begin() [i];
    }
    return sum;
}

// Sum(2, 4)
// 0 1 2 3 4 5 6 7 8 9 10 
//     x x x x 
unsigned int svIntList::SumByLen(int sidx, int len)
{
    unsigned int sum=0;
    if (!len) // len == 0
        return 0;
    int eidx = sidx + len;
    if (sidx >= (int)m_intList.size() ||
            eidx > (int)m_intList.size())
    {
        wxLogMessage(wxString::Format(wxT("svIntList index out of boundry: size=%i start=%i len=%i "), (int)m_intList.size(), (int)sidx, (int)len));
        return 0;
    }

    for (int i=sidx; i<(int)m_intList.size() && i<eidx; ++i) {
        sum += m_intList.begin() [i];
    }
    return sum;
}


// if round == true then close to nearst position(left or right) else alway shift to right.
unsigned int svIntList::DeSum(int isum, bool round)
{
    if (isum<=0) return 0;
    unsigned int sum=0;
    int i=0;
    for (i=0; i<(int)m_intList.size(); ++i) {
        unsigned int inc = m_intList.begin() [i];
        sum += inc;
        if ((int)sum>=isum)
        {
            if (inc==1 || !round)
            return i+1;
            else
            {
                if (sum-isum>=(float)inc/2)
                return i;
                else
                return i+1;
            }
        }
    }
    return i;
}

// close to nearst position(left).
// call only by svLineText::Space2ColW
unsigned int svIntList::DeSumLeft(int isum, int start_pos, int max_pos)
{
    if (isum<=0) return start_pos;
    if (start_pos<0 || start_pos>=(int)m_intList.size()) return 0;
    unsigned int sum=0;
    int i=0;
    // for (i=start_pos; i<(int)m_intList.size(); ++i) {
    for (i=start_pos; i<max_pos; ++i) {
        unsigned int inc = m_intList.begin() [i];
        sum += inc;
        if ((int)sum==isum)
        {
            return i+1;  // Including this position, So add 1;
        }
        else if ((int)sum>isum)
        {
            return i;
        }
    }
    return i;
}

// if round == true then close to nearst position(left or right) else alway shift to right.
unsigned int svIntList::DeSum(int isum, int start_pos, bool round)
{
    if (isum<=0) return start_pos;
    if (start_pos<0 || start_pos>=(int)m_intList.size()) return 0;
    unsigned int sum=0;
    int i=0;
    for (i=start_pos; i<(int)m_intList.size(); ++i) {
        unsigned int inc = m_intList.begin() [i];
        sum += inc;
        if ((int)sum>=isum)
        {
            if (inc==1 || !round)
            return i+1;
            else
            {
                if (sum-isum>=(float)inc/2)
                return i;
                else
                return i+1;
            }
        }
    }
    return i;
}


/*
* svListOfIntList
* A class for list of svIntList. ( A class for list of list of Integer. )
*/

svListOfIntList::svListOfIntList()
{
}

svListOfIntList::~svListOfIntList()
{
    m_ilList.clear();
}

void svListOfIntList::Append(svIntList* data)
{
    m_ilList.push_back(*data);
}

svIntList* svListOfIntList::GetList(int idx)
{
    return &m_ilList.at(idx);
}

unsigned int svListOfIntList::GetItem(int row, int col)
{
    return m_ilList.at(row).Get(col);
}

void svListOfIntList::Clear(void)
{
    m_ilList.clear();
}

unsigned int svListOfIntList::Sum(int row, int idx)
{
    svIntList il = m_ilList.at(row);
    return il.Sum(idx);
}

unsigned int svListOfIntList::DeSum(int row, int sum, bool round)
{
    svIntList il = m_ilList.at(row);
    return il.DeSum(sum, round);
}

