/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVFINDREPLACECTRL_H
#define _SVFINDREPLACECTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/caret.h>
#include <wx/textfile.h>
#include <wx/dcbuffer.h>
#include <wx/tglbtn.h>
#include <wx/aui/auibook.h>
#include <memory>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#include "svBufText.h"
#include "svCommand.h"
#include "svListOfIntList.h"
#include "svTextView.h"
#include "svTheme.h"
#include "svScrollBar.h"
#include "svBaseType.h"
// #include "svMainFrame.h"

using namespace std;

// forward declaration. avoid cross reference problem.
class svMainFrame; 

enum
{
    ID_BTN_FIND=wxID_HIGHEST+1,
    ID_BTN_FIND_PREV,
    ID_BTN_FIND_ALL,
    ID_BTN_REPLACE,
    ID_BTN_REPLACE_PREV,
    ID_BTN_REPLACE_ALL,
    ID_BTN_REGEX,
    ID_BTN_CASE,
    ID_BTN_ONSELECT,
    ID_BTN_WHOLEWORD,
    ID_TXT_FIND,
    ID_TXT_REPLACE,
};

class svFindReplaceCtrl : public wxWindow
{
public:
    svFindReplaceCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name);
    ~svFindReplaceCtrl();
    // void OnSize(wxSizeEvent& event);

    inline
    void SetMainFrame(svMainFrame *p_mainFrame)
    {
        m_mainFrame = p_mainFrame;
    }

    void OnChar(wxKeyEvent& event);
    void OnClose(wxCloseEvent& event);
    void InitControls(void);

    void OnBtnFindClick(wxCommandEvent& event);
    void DoOnBtnFindClick(void);
    void OnBtnFindPrevClick(wxCommandEvent& event);
    void OnBtnFindAllClick(wxCommandEvent& event);

    void OnBtnReplaceClick(wxCommandEvent& event);
    void DoOnBtnReplaceClick(void);
    void OnBtnReplacePrevClick(wxCommandEvent& event);
    void OnBtnReplaceAllClick(wxCommandEvent& event);

    void OnEraseBackGround(wxEraseEvent& event);

    void OnTxtFindEnter(wxCommandEvent& event);
    void OnTxtReplaceEnter(wxCommandEvent& event);
    
    void OnKeyDown(wxKeyEvent& event);
    void OnTxtFindChanged(wxCommandEvent& event);
    void OnBtnValueChanged(wxCommandEvent& event);

    inline
    void ResetFocus(void)
    {
        txtFind->SetFocus();
    }

    svFindReplaceOption GetOption(void);

private:

    svMainFrame *m_mainFrame;

    // wxStaticText *lblMessage;

    wxTextCtrl *txtFind;
    wxButton *btnFind;
    wxButton *btnFindPrev;
    wxButton *btnFindAll;
    wxToggleButton *tbtnRegex;
    wxToggleButton *tbtnCaseSensitive;
    wxToggleButton *tbtnOnSelect;
    wxToggleButton *tbtnWholeWord;

    wxTextCtrl *txtReplace;
    wxButton *btnReplace;
    wxButton *btnReplacePrev;
    wxButton *btnReplaceAll;

private:
    DECLARE_EVENT_TABLE()

};


#endif
