/*
   Copyright Notice in awvic.cpp
*/

// *********************************************************************
//
// A Find & Replace control for awvic.
//
// *********************************************************************

#include "svFindReplaceCtrl.h"
#include <wx/dcbuffer.h>
#include <wx/log.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include "wx/filedlg.h"
#include "wx/file.h"
#include "wx/strconv.h"
#include "wx/encconv.h"
#include "wx/textfile.h"
#include <time.h>
#include "svTextEditorCtrl.h"
#include "svMainFrame.h"


#include "stdwx.h"
#include "svPreference.h"


// m_bufText_EVENT
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_EVENT)
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_MODIFIED)
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_MSG)
// IMPLEMENT_DYNAMIC_CLASS(SVTextEvent, wxNotifyEvent)


BEGIN_EVENT_TABLE(svFindReplaceCtrl, wxWindow)
// EVT_PAINT(svFindReplaceCtrl::OnPaint)
// EVT_ERASE_BACKGROUND(svFindReplaceCtrl::OnErase)
// EVT_SIZE(svFindReplaceCtrl::OnSize)
EVT_CHAR(svFindReplaceCtrl::OnChar)
EVT_CLOSE(svFindReplaceCtrl::OnClose)
EVT_KEY_DOWN(svFindReplaceCtrl::OnKeyDown)

// EVT_CHILD_FOCUS(svFindReplaceCtrl::OnChildFocused)
// EVT_SET_FOCUS(svFindReplaceCtrl::OnSetFocus)

// EVT_LEFT_DOWN(svFindReplaceCtrl::OnMouseLeftDown)
// EVT_LEFT_UP(svFindReplaceCtrl::OnMouseLeftUp)
// EVT_RIGHT_UP(svFindReplaceCtrl::OnMouseRightUp)
// EVT_MOTION(svFindReplaceCtrl::OnMouseMotion)
// EVT_MOUSEWHEEL(svFindReplaceCtrl::OnMouseWheel)
EVT_BUTTON( ID_BTN_FIND, svFindReplaceCtrl::OnBtnFindClick )
EVT_BUTTON( ID_BTN_FIND_PREV, svFindReplaceCtrl::OnBtnFindPrevClick )
EVT_BUTTON( ID_BTN_FIND_ALL, svFindReplaceCtrl::OnBtnFindAllClick )
EVT_BUTTON( ID_BTN_REPLACE, svFindReplaceCtrl::OnBtnReplaceClick )
EVT_BUTTON( ID_BTN_REPLACE_PREV, svFindReplaceCtrl::OnBtnReplacePrevClick )
EVT_BUTTON( ID_BTN_REPLACE_ALL, svFindReplaceCtrl::OnBtnReplaceAllClick )
// EVT_ERASE_BACKGROUND(svFindReplaceCtrl::OnEraseBackGround)

EVT_TEXT_ENTER( ID_TXT_FIND, svFindReplaceCtrl::OnTxtFindEnter )
EVT_TEXT_ENTER( ID_TXT_REPLACE, svFindReplaceCtrl::OnTxtReplaceEnter )

EVT_TEXT( ID_TXT_FIND, svFindReplaceCtrl::OnTxtFindChanged )

EVT_TOGGLEBUTTON( ID_BTN_REGEX, svFindReplaceCtrl::OnBtnValueChanged )
EVT_TOGGLEBUTTON( ID_BTN_CASE, svFindReplaceCtrl::OnBtnValueChanged )
EVT_TOGGLEBUTTON( ID_BTN_ONSELECT, svFindReplaceCtrl::OnBtnValueChanged )
EVT_TOGGLEBUTTON( ID_BTN_WHOLEWORD, svFindReplaceCtrl::OnBtnValueChanged )

END_EVENT_TABLE()


svFindReplaceCtrl::svFindReplaceCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"))
:wxWindow(parent, id, pos, size, style, name)
{
    m_mainFrame = NULL;
    InitControls();
}

svFindReplaceCtrl::~svFindReplaceCtrl()
{
    // wxWidgets will handle it. Comment them all.
    // if (lblMessage) delete lblMessage;

    // if (txtFind) delete txtFind;
    // if (btnFind) delete btnFind;
    // if (btnFindPrev) delete btnFindPrev;
    // if (btnFindAll) delete btnFindAll;
    // if (tbtnRegex) delete tbtnRegex;
    // if (tbtnCaseSensitive) delete tbtnCaseSensitive;
    // if (tbtnOnSelect) delete tbtnOnSelect;
    // if (tbtnWholeWord) delete tbtnWholeWord;

    // if (txtReplace) delete txtReplace;
    // if (btnReplace) delete btnReplace;
    // if (btnReplacePrev) delete btnReplacePrev;
    // if (btnReplaceAll) delete btnReplaceAll;
}

void svFindReplaceCtrl::InitControls(void)
{

    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    // lblMessage = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    // lblMessage->Wrap( -1 );
    // vSizer1->Add( lblMessage, 0, wxALL, 3 );
    
    wxPanel *pFind12 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* hSizer121;
    hSizer121 = new wxBoxSizer( wxHORIZONTAL );
    
    wxBoxSizer* hSizer1211;
    hSizer1211 = new wxBoxSizer( wxVERTICAL );
    
    hSizer1211->SetMinSize( wxSize( 340,-1 ) ); 
    txtFind = new wxTextCtrl( pFind12, ID_TXT_FIND, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), wxTE_PROCESS_ENTER );
    hSizer1211->Add( txtFind, 0, wxALL, 1 );
    
    
    hSizer1211->Add( 40, 0, 1, wxALIGN_LEFT|wxFIXED_MINSIZE, 5 );
    
    
    hSizer121->Add( hSizer1211, 1, 0, 5 );
    
    wxBoxSizer* hSizer1212;
    hSizer1212 = new wxBoxSizer( wxHORIZONTAL );
    
    btnFind = new wxButton( pFind12, ID_BTN_FIND, wxT("Find"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1212->Add( btnFind, 0, wxALL, 1 );
    
    btnFindPrev = new wxButton( pFind12, ID_BTN_FIND_PREV, wxT("Find Prev"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1212->Add( btnFindPrev, 0, wxALL, 1 );
    
    btnFindAll = new wxButton( pFind12, ID_BTN_FIND_ALL, wxT("Find All"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1212->Add( btnFindAll, 0, wxALL, 1 );
    
    
    hSizer1212->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxBoxSizer* hSizer12122;
    hSizer12122 = new wxBoxSizer( wxHORIZONTAL );
    
    tbtnRegex = new wxToggleButton( pFind12, ID_BTN_REGEX, wxT("R"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
    hSizer12122->Add( tbtnRegex, 0, wxALL, 1 );
    
    tbtnCaseSensitive = new wxToggleButton( pFind12, ID_BTN_CASE, wxT("C"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
    hSizer12122->Add( tbtnCaseSensitive, 0, wxALL, 1 );
    
    
    hSizer1212->Add( hSizer12122, 1, wxEXPAND, 5 );
    
    
    hSizer121->Add( hSizer1212, 1, wxALIGN_LEFT|wxEXPAND, 5 );
    
    
    pFind12->SetSizer( hSizer121 );
    pFind12->Layout();
    hSizer121->Fit( pFind12 );
    vSizer1->Add( pFind12, 1, wxEXPAND | wxALL, 1 );
    
    wxPanel *pReplace13 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* hSizer131;
    hSizer131 = new wxBoxSizer( wxHORIZONTAL );
    
    wxBoxSizer* hSizer1311;
    hSizer1311 = new wxBoxSizer( wxVERTICAL );
    
    hSizer1311->SetMinSize( wxSize( 340,-1 ) ); 
    txtReplace = new wxTextCtrl( pReplace13, ID_TXT_REPLACE, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), wxTE_PROCESS_ENTER );
    hSizer1311->Add( txtReplace, 0, wxALL, 1 );

    hSizer1311->Add( 40, 0, 1, wxALIGN_LEFT|wxFIXED_MINSIZE, 5 );
    
    
    hSizer131->Add( hSizer1311, 1, wxEXPAND, 5 );
    
    wxBoxSizer* hSizer1312;
    hSizer1312 = new wxBoxSizer( wxHORIZONTAL );
    
    btnReplace = new wxButton( pReplace13, ID_BTN_REPLACE, wxT("Replace"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1312->Add( btnReplace, 0, wxALL, 1 );
    
    btnReplacePrev = new wxButton( pReplace13, ID_BTN_REPLACE_PREV, wxT("Replace Prev"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1312->Add( btnReplacePrev, 0, wxALL, 1 );
    
    btnReplaceAll = new wxButton( pReplace13, ID_BTN_REPLACE_ALL, wxT("Replace All"), wxDefaultPosition, wxDefaultSize, 0 );
    hSizer1312->Add( btnReplaceAll, 0, wxALL, 1 );
    
    
    hSizer1312->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxBoxSizer* vSizer13122;
    vSizer13122 = new wxBoxSizer( wxHORIZONTAL );
    
    tbtnOnSelect = new wxToggleButton( pReplace13, ID_BTN_ONSELECT, wxT("S"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
    vSizer13122->Add( tbtnOnSelect, 0, wxALL, 1 );
    
    tbtnWholeWord = new wxToggleButton( pReplace13, ID_BTN_WHOLEWORD, wxT("W"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
    vSizer13122->Add( tbtnWholeWord, 0, wxALL, 1 );
    
    
    hSizer1312->Add( vSizer13122, 1, wxEXPAND, 5 );
    
    
    hSizer131->Add( hSizer1312, 1, wxEXPAND, 5 );
    
    
    pReplace13->SetSizer( hSizer131 );
    pReplace13->Layout();
    hSizer131->Fit( pReplace13 );
    vSizer1->Add( pReplace13, 1, wxEXPAND | wxALL, 1 );
    
    
    
    txtFind->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnFind->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnFindPrev->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnFindAll->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    txtReplace->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnReplace->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnReplacePrev->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    btnReplaceAll->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    tbtnRegex->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    tbtnCaseSensitive->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    tbtnOnSelect->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    tbtnWholeWord->Bind(wxEVT_KEY_DOWN, &svFindReplaceCtrl::OnKeyDown, this);
    
    this->SetSizer( vSizer1 );
    this->Layout();

}

// void svFindReplaceCtrl::OnSize(wxSizeEvent& event)
// {

// }

void svFindReplaceCtrl::OnChar(wxKeyEvent& event)
{

	int key = event.GetKeyCode();


    if (key==WXK_RETURN)
    {
        
    }

    // event.Skip() means the event will be processed by the parent control.
    // Don't call event.Skip() if everything you want had been processed.
    
    // if (cmd->Name()!=CMD_TXT_UP &&
    //     cmd->Name()!=CMD_TXT_DOWN)
    // {
    //     event.Skip();
    // }

}


void svFindReplaceCtrl::OnClose(wxCloseEvent& event)
{
}


void svFindReplaceCtrl::OnBtnFindClick(wxCommandEvent& event)
{
    DoOnBtnFindClick();
}

void svFindReplaceCtrl::DoOnBtnFindClick(void)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        // editor->DoFindNextWordwxStr(GetOption());
        editor->DoFindNext();
        editor->SetFocus();
    }
}

void svFindReplaceCtrl::OnBtnFindPrevClick(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        editor->DoFindPrev();
        editor->SetFocus();
    }
}

void svFindReplaceCtrl::OnBtnFindAllClick(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        // editor->DoFindAll(GetOption());
        editor->DoFindAll();
        editor->SetFocus();
    }
}

void svFindReplaceCtrl::OnBtnReplaceClick(wxCommandEvent& event)
{
    // if (!m_mainFrame) return;

    // svTextEditorCtrl *editor;
    // editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    // if (editor)
    // {
    //     editor->DoReplaceNextWordwxStr(txtFind->GetValue(), txtReplace->GetValue());
    //     editor->SetFocus();
    // }

    DoOnBtnReplaceClick();
}

void svFindReplaceCtrl::DoOnBtnReplaceClick(void)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        // editor->DoReplaceNextWordwxStr(GetOption());
        editor->DoReplaceNext(GetOption());
        editor->SetFocus();
    }
}

void svFindReplaceCtrl::OnBtnReplacePrevClick(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        // editor->DoReplacePrevWordwxStr(GetOption());
        editor->DoReplacePrev(GetOption());
        editor->SetFocus();
    }
}

void svFindReplaceCtrl::OnBtnReplaceAllClick(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
    {
        editor->DoReplaceAll(GetOption());
        editor->SetFocus();
    }
}


void svFindReplaceCtrl::OnEraseBackGround(wxEraseEvent& event)
{
    
}

void svFindReplaceCtrl::OnTxtFindEnter(wxCommandEvent& event)
{
    DoOnBtnFindClick();
}

void svFindReplaceCtrl::OnTxtReplaceEnter(wxCommandEvent& event)
{
    DoOnBtnReplaceClick();
}

svFindReplaceOption svFindReplaceCtrl::GetOption(void)
{
    svFindReplaceOption fro;

    fro.m_wxStr2Find = txtFind->GetValue();
    fro.m_wxStr2Replace = txtReplace->GetValue(); 

    fro.m_case = tbtnCaseSensitive->GetValue();
    fro.m_regex = tbtnRegex->GetValue();
    fro.m_inSelect = tbtnOnSelect->GetValue();
    fro.m_wholeWord = tbtnWholeWord->GetValue();
    fro.m_from = svFindReplaceOption::SVID_FIND_FROM_PANEL;

    return fro;
}

void svFindReplaceCtrl::OnKeyDown(wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    //wxChar ukey = event.GetUnicodeKey();

    if (key==27) // ESC
    {
        m_mainFrame->HideFindWindow();
    }
    else if ((key==70 || key==102) && event.ControlDown())  // Ctrl + F/f
    {
        m_mainFrame->HideFindWindow();
    }
    else
    {
        event.Skip();
    }

}

void svFindReplaceCtrl::OnTxtFindChanged(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
        editor->DoFindMatchLocations(GetOption());

}

void svFindReplaceCtrl::OnBtnValueChanged(wxCommandEvent& event)
{
    if (!m_mainFrame) return;

    svTextEditorCtrl *editor;
    editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
    if (editor)
        editor->DoFindMatchLocations(GetOption());
    txtFind->SetFocus();

}

// void svFindReplaceCtrl::OnChildFocused(wxChildFocusEvent& event)
// {

// }

// void svFindReplaceCtrl::OnSetFocus(wxFocusEvent& event)
// {

// }