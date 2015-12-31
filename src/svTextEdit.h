/* 
 * Class svTextEdit present the text edit behavior on the screen.
*/

#ifndef _SVTEXTEDIT_H
#define _SVTEXTEDIT_H


#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include "svLineText.h"
#include "svBufText.h"
#include "svListOfIntList.h"

#include <vector>
using namespace std;

class svTextEdit
{
private:
    svBufText* m_bufText;           // text buffer to be edit. 


public:
    svTextEdit();
    svTextEdit(svBufText* p_buftext);
    ~svTextEdit();
    void Reset(svBufText* p_buftext);
    void InsertChar(wxChar p_key);

};

#endif
