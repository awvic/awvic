/*
   Copyright Notice in awvic.cpp
*/

#include "svTextStyleList.h"

#include "stdwx.h"

/*
* svTextStyleList
* A class for list of txtStyleDesc.
*/

svTextStyleList::svTextStyleList()
{
}

svTextStyleList::~svTextStyleList()
{
    m_styleList.clear();
}

void svTextStyleList::Clear(void)
{
    m_styleList.clear();
}

void svTextStyleList::Append(const txtStyleDesc& data)
{
    m_styleList.push_back(data);
}

txtStyleDesc svTextStyleList::Get(int idx)
{
    return m_styleList.at(idx);
}

int svTextStyleList::Size(void)
{
    return m_styleList.size();
}

