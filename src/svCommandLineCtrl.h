/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVCOMMANDLINECTRL_H
#define _SVCOMMANDLINECTRL_H

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
#include "wx/aui/auibook.h"
#include <memory>
#include "wx/stdpaths.h"
#include "wx/filefn.h"
#include "wx/filename.h"
#include "svBufText.h"
#include "svCommand.h"
#include "svListOfIntList.h"
#include "svTextView.h"
#include "svTheme.h"
#include "svScrollBar.h"
// #include "svMainFrame.h"

using namespace std;

class svTextEditorCtrl;

#define SVID_COMMAND_LINE_MAX_HEIGHT 500

enum
{
    SVID_CMD_NONE=0,
    SVID_CMD_GOTO_LINE,
    SVID_CMD_GOTO_DEFINITION,
    SVID_CMD_REGULAR,
    SVID_CMD_OTHERS
};

enum
{
    SVID_TXT_COMMAND=wxID_HIGHEST+1,
    SVID_TXT_COMMAND_LIST_BOX
};

typedef struct svLineCommand
{
    int m_commandType;
    int m_lineNo;
    wxString m_text1;
    wxString m_text2;

    svLineCommand()
    {
        m_commandType = 0;
        m_lineNo = 0;
    }

} svLineCommand;

class svCommandLineCtrl : public wxWindow
{
public:
    svCommandLineCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name);
    ~svCommandLineCtrl();
    // void OnSize(wxSizeEvent& event);

    // inline
    // void SetMainFrame(svMainFrame *p_mainFrame)
    // {
    //     m_mainFrame = p_mainFrame;
    // }

    void OnClose(wxCloseEvent& event);
    // void OnChar(wxKeyEvent& event);
    void InitControls(void);


    void OnCommandText(wxCommandEvent& event);
    void OnCommandTextEnter(wxCommandEvent& event);
    void OnCommandKeyDown(wxKeyEvent& event);
    // void OnBtnFindClick(wxCommandEvent& event);
    // void OnBtnFindPrevClick(wxCommandEvent& event);
    // void OnBtnFindAllClick(wxCommandEvent& event);

    // void OnBtnReplaceClick(wxCommandEvent& event);
    // void OnBtnReplacePrevClick(wxCommandEvent& event);
    // void OnBtnReplaceAllClick(wxCommandEvent& event);

    // void OnEraseBackGround(wxEraseEvent& event);
    // void OnChar(wxKeyEvent& event);
    void SetFocus(void);
    bool CheckCommand(svLineCommand &p_cmd);


    void SetCommandType(char p_commandType);

    // void OnLabelMouseLeftDown(wxMouseEvent& event);
    // void OnLabelKeyDown(wxKeyEvent& event);

    void OnListBox(wxCommandEvent &event);

private:

    wxWindow *m_parent;
    wxStaticText* lblMsg;
    wxTextCtrl *txtCommand;
    wxScrolledWindow* m_scrolledWindow1;
    wxStaticText *m_staticText1;
    wxStaticText *m_staticText2;
    wxBoxSizer* bSizer6;
    wxBoxSizer* vSizer12;
    wxListBox* m_listBox1;

    char m_commandType;

    int m_selectIndex;

    vector<svIntText> m_definitionLines;  // All available definetions.
    vector<svIntText> m_filterLines;      // Definitions been filtered.

    static const int m_border=5;

private:
    DECLARE_EVENT_TABLE()

};


#endif
