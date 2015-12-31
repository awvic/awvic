/*
   Copyright Notice in awvic.cpp
*/

#include "svTextEdit.h"
#include <wx/tokenzr.h>

#include "stdwx.h"


// **************
// svTextEdit
// **************

svTextEdit::svTextEdit()//:
//m_VScrollCnt(0)
{

}

svTextEdit::svTextEdit(svBufText* p_buftext)
{
    m_bufText = p_buftext;
}

svTextEdit::~svTextEdit()
{
    m_bufText = NULL;
}

void svTextEdit::Reset(svBufText* p_buftext)
{
    m_bufText = p_buftext;
}

void svTextEdit::InsertChar(wxChar p_key)
{
    m_bufText->EditingInsertChar(p_key);
}