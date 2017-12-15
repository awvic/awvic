/*
   Copyright Notice in awvic.cpp
*/

// *********************************************************************
//
// A Command line control for awvic.
//
// *********************************************************************

#include "svCommandLineCtrl.h"
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
#include <algorithm>
#include "svTextEditorCtrl.h"


#include "stdwx.h"
#include "svPreference.h"


// m_bufText_EVENT
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_EVENT)
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_MODIFIED)
// DEFINE_EVENT_TYPE(wxEVT_SVTEXT_MSG)
// IMPLEMENT_DYNAMIC_CLASS(SVTextEvent, wxNotifyEvent)


BEGIN_EVENT_TABLE(svCommandLineCtrl, wxWindow)
// EVT_PAINT(svCommandLineCtrl::OnPaint)
// EVT_ERASE_BACKGROUND(svCommandLineCtrl::OnErase)
// EVT_SIZE(svCommandLineCtrl::OnSize)
EVT_TEXT_ENTER(SVID_TXT_COMMAND, svCommandLineCtrl::OnCommandTextEnter)
EVT_TEXT(SVID_TXT_COMMAND, svCommandLineCtrl::OnCommandText)
//EVT_CHAR(svCommandLineCtrl::OnChar)
EVT_CLOSE(svCommandLineCtrl::OnClose)
// EVT_LEFT_DOWN(svCommandLineCtrl::OnMouseLeftDown)
// EVT_LEFT_UP(svCommandLineCtrl::OnMouseLeftUp)
// EVT_RIGHT_UP(svCommandLineCtrl::OnMouseRightUp)
// EVT_MOTION(svCommandLineCtrl::OnMouseMotion)
// EVT_MOUSEWHEEL(svCommandLineCtrl::OnMouseWheel)
// EVT_BUTTON( ID_BTN_FIND, svCommandLineCtrl::OnBtnFindClick )
// EVT_BUTTON( ID_BTN_FIND_PREV, svCommandLineCtrl::OnBtnFindPrevClick )
// EVT_BUTTON( ID_BTN_FIND_ALL, svCommandLineCtrl::OnBtnFindAllClick )
// EVT_BUTTON( ID_BTN_REPLACE, svCommandLineCtrl::OnBtnReplaceClick )
// EVT_BUTTON( ID_BTN_REPLACE_PREV, svCommandLineCtrl::OnBtnReplacePrevClick )
// EVT_BUTTON( ID_BTN_REPLACE_ALL, svCommandLineCtrl::OnBtnReplaceAllClick )
EVT_LISTBOX(SVID_TXT_COMMAND_LIST_BOX, svCommandLineCtrl::OnListBox)
END_EVENT_TABLE()


svCommandLineCtrl::svCommandLineCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"))
:wxWindow(parent, id, pos, size, style, name)
{
    m_parent = parent;

    m_commandType = SVID_CMD_REGULAR;
    InitControls();

    m_selectIndex = -1;
}

svCommandLineCtrl::~svCommandLineCtrl()
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

void svCommandLineCtrl::InitControls(void)
{

/*    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    txtCommand = new wxTextCtrl( this, SVID_CMD_COMMAND, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    vSizer1->Add( txtCommand, 1, wxALL|wxEXPAND, 5 );
    
    wxBoxSizer* vSizer12;
    vSizer12 = new wxBoxSizer( wxVERTICAL );
    
    m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Test message 01"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );
    vSizer12->Add( m_staticText1, 1, wxALL|wxEXPAND, 5 );
    
    m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Test message 02"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText2->Wrap( -1 );
    vSizer12->Add( m_staticText2, 1, wxALL|wxEXPAND, 5 );
    
    
    vSizer1->Add( vSizer12, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( vSizer1 );
    this->Layout();*/

/*    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer( wxHORIZONTAL );
    
    lblMsg = new wxStaticText( this, wxID_ANY, wxT("?"), wxDefaultPosition, wxSize( 20,-1 ), 0 );
    lblMsg->Wrap( -1 );
    bSizer5->Add( lblMsg, 0, wxALL, 5 );
    
    txtCommand = new wxTextCtrl( this, SVID_TXT_COMMAND, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    bSizer5->Add( txtCommand, 1, wxALL|wxEXPAND, 5 );
    
    
    vSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
    
    wxBoxSizer* vHintSizer;
    vHintSizer = new wxBoxSizer( wxVERTICAL );
    
    m_scrolledWindow1 = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow1->SetScrollRate( 5, 5 );
    // wxBoxSizer* vSizer12;
    vSizer12 = new wxBoxSizer( wxVERTICAL );
    
    m_staticText1 = new wxStaticText( m_scrolledWindow1, wxID_ANY, wxT("Test message 01"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );
    vSizer12->Add( m_staticText1, 1, wxALL|wxEXPAND, 5 );
    
    m_staticText2 = new wxStaticText( m_scrolledWindow1, wxID_ANY, wxT("Test message 02"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText2->Wrap( -1 );
    vSizer12->Add( m_staticText2, 1, wxALL|wxEXPAND, 5 );
    
    
    m_scrolledWindow1->SetSizer( vSizer12 );
    m_scrolledWindow1->Layout();
    vSizer12->Fit( m_scrolledWindow1 );
    vHintSizer->Add( m_scrolledWindow1, 1, wxEXPAND | wxALL, 5 );
    
    
    vSizer1->Add( vHintSizer, 1, wxEXPAND, 5 );*/
    












/*
    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer( wxVERTICAL );
    
    // wxBoxSizer* bSizer6;
    bSizer6 = new wxBoxSizer( wxHORIZONTAL );
    
    lblMsg = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxSize( 20,-1 ), 0 );
    lblMsg->Wrap( -1 );
    bSizer6->Add( lblMsg, 0, wxALL, 5 );
    
    txtCommand = new wxTextCtrl( this, SVID_TXT_COMMAND, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    bSizer6->Add( txtCommand, 1, wxALL, 5 );
    
    
    bSizer5->Add( bSizer6, 0, wxEXPAND, 5 );
    
    wxBoxSizer* vHintSizer;
    vHintSizer = new wxBoxSizer( wxVERTICAL );
    
    m_scrolledWindow1 = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
    m_scrolledWindow1->SetScrollRate( 5, 5 );
    // wxBoxSizer* vSizer12;
    vSizer12 = new wxBoxSizer( wxVERTICAL );
    
    m_staticText1 = new wxStaticText( m_scrolledWindow1, wxID_ANY, wxT("Test message 01"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText1->Wrap( -1 );
    vSizer12->Add( m_staticText1, 1, wxALL|wxEXPAND, 5 );
    
    m_staticText2 = new wxStaticText( m_scrolledWindow1, wxID_ANY, wxT("Test message 02"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText2->Wrap( -1 );
    vSizer12->Add( m_staticText2, 1, wxALL|wxEXPAND, 5 );
    
    
    m_scrolledWindow1->SetSizer( vSizer12 );
    m_scrolledWindow1->Layout();
    vSizer12->Fit( m_scrolledWindow1 );
    vHintSizer->Add( m_scrolledWindow1, 1, wxEXPAND | wxALL, 5 );
    
    
    bSizer5->Add( vHintSizer, 1, wxEXPAND, 5 );
    
    
    vSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( vSizer1 );

    // default size, will be changed on execution time.
    this->SetSize(300, 400);
    this->Layout();*/














    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer( wxVERTICAL );
    
    // wxBoxSizer* bSizer6;
    bSizer6 = new wxBoxSizer( wxHORIZONTAL );
    
    lblMsg = new wxStaticText( this, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxSize( 20,-1 ), 0 );
    lblMsg->Wrap( -1 );
    bSizer6->Add( lblMsg, 0, wxALL, 5 );
    
    txtCommand = new wxTextCtrl( this, SVID_TXT_COMMAND, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    bSizer6->Add( txtCommand, 1, wxALL, 5 );
    
    
    bSizer5->Add( bSizer6, 0, wxEXPAND, 5 );
    
    wxBoxSizer* vHintSizer;
    vHintSizer = new wxBoxSizer( wxVERTICAL );
    
    m_listBox1 = new wxListBox( this, SVID_TXT_COMMAND_LIST_BOX, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
    vHintSizer->Add( m_listBox1, 1, wxALL|wxEXPAND, 0 );
    
    
    bSizer5->Add( vHintSizer, 1, wxEXPAND, 5 );
    
    
    vSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( vSizer1 );
    this->SetSize( 300, 400);
    this->Layout();


    txtCommand->Connect( SVID_TXT_COMMAND, wxEVT_KEY_DOWN, 
          wxKeyEventHandler( svCommandLineCtrl::OnCommandKeyDown ), NULL, this );
    m_listBox1->Connect( SVID_TXT_COMMAND_LIST_BOX, wxEVT_KEY_DOWN, 
          wxKeyEventHandler( svCommandLineCtrl::OnCommandKeyDown ), NULL, this );
    // txtCommand->Connect( SVID_TXT_COMMAND, wxEVT_CHAR, 
    //       wxKeyEventHandler( svCommandLineCtrl::OnChar ), NULL, this );
    // m_listBox1->Connect( SVID_TXT_COMMAND_LIST_BOX, wxEVT_CHAR, 
    //       wxKeyEventHandler( svCommandLineCtrl::OnChar ), NULL, this );

}

// void svCommandLineCtrl::OnSize(wxSizeEvent& event)
// {

// }


void svCommandLineCtrl::OnClose(wxCloseEvent& event)
{
}

// void svCommandLineCtrl::OnChar(wxKeyEvent& event)
// {
//     // int key = event.GetKeyCode();
//     // wxChar ukey = event.GetUnicodeKey();

//     // if (key == WXK_ESCAPE)
//     // {
//     //  this->Show(false);
//     // }

//     // event.Skip();


//     // if ( event.GetKeyCode() == WXK_ESCAPE )
//     // {
//     //    this->Show(false);
//     //    m_parent->SetFocus();
//     //    return;
//     // }
//     // else
//     // {
//         if (event.GetId()==SVID_TXT_COMMAND)
//         {
//             if (event.GetKeyCode() == WXK_PAGEUP || 
//                 event.GetKeyCode() == WXK_PAGEDOWN || 
//                 event.GetKeyCode() == WXK_UP || 
//                 event.GetKeyCode() == WXK_DOWN)
//             {
//                 wxKeyEvent e = event;
//                 e.SetId(SVID_TXT_COMMAND_LIST_BOX);
//                 e.SetEventObject(m_listBox1);
//                 OnChar(e);


//                 wxKeydEvent e(wxEVT_CHAR, SVID_TXT_COMMAND_LIST_BOX);
//                 e.SetEventObject(m_listBox1);
//                 // Do send it
//                 ProcessWindowEvent(e);
//             }
//         }
//     // }
//     event.Skip();
// }

void svCommandLineCtrl::OnCommandText(wxCommandEvent& event)
{
    wxString cmd = txtCommand->GetValue();
    cmd.Trim();
    if (m_commandType==SVID_CMD_GOTO_DEFINITION)
    {

        m_filterLines.clear();

        // filter by text input.
        for (std::vector<svIntText>::iterator it=m_definitionLines.begin();
             it!=m_definitionLines.end();
             ++it)
        {
            if (svCommonLib::wxStrMatchPattern(it->m_text, cmd))
            {
                m_filterLines.push_back(*it);
            }
        }


        if (m_filterLines.size()>0)
        {

            m_listBox1->Clear();

            for (vector<svIntText>::iterator it=m_filterLines.begin();
                 it!=m_filterLines.end();
                 ++it)
            {

                m_listBox1->Append(it->m_text);
            }
            m_listBox1->SetSelection(0);
            m_selectIndex = 0;

            this->SetSize(400, 300);
            this->Layout();
        }
        else
        {
            // No definition found.
            int width = this->GetSize().GetWidth();
            int height = this->GetSize().GetHeight();
            this->SetSize(width, bSizer6->GetSize().GetHeight());
            this->Layout();
            // txtCommand->SetValue(_(""));
            m_selectIndex = -1;
        }

    }

    
    event.Skip();
}


void svCommandLineCtrl::OnCommandTextEnter(wxCommandEvent& event)
{
    svLineCommand cmd;
    if (CheckCommand(cmd))
    {
        if (cmd.m_commandType==SVID_CMD_GOTO_LINE)
        {
            txtCommand->SetValue("");
            ((svTextEditorCtrl *)m_parent)->GotoLine(cmd.m_lineNo);
        }
        else if (cmd.m_commandType==SVID_CMD_GOTO_DEFINITION)
        {
            // int x_start, y_start;
            // x_start=y_start=0;
            // int dest_i=0;
            // m_scrolledWindow1->GetViewStart(&x_start, &y_start);
            // int x_unit, y_unit;
            // x_unit=y_unit=0;
            // m_scrolledWindow1->GetScrollPixelsPerUnit(&x_unit, &y_unit);

            // if (m_filterLines.size()>0)
            // {
            //     wxSizerItem *lbl = vSizer12->GetItem((size_t)0);
            //     if (lbl)
            //     {
            //         // dest_y = i * (lbl->GetSize().GetHeight() + m_border*2);
            //         dest_i = (y_start*y_unit) / lbl->GetSize().GetHeight();
            //         int mod = (y_start*y_unit) % lbl->GetSize().GetHeight();
            //         if (mod>0) ++dest_i;
            //         if (dest_i>=0 && dest_i<(int)m_filterLines.size())
            //             ((svTextEditorCtrl *)m_parent)->GotoLine( m_filterLines.at(dest_i).m_lineNo);
            //     }
            // }
            if (m_listBox1->GetCount()>0)
            {
                int sel = m_listBox1->GetSelection();
                ((svTextEditorCtrl *)m_parent)->GotoLine( m_filterLines.at(sel).m_lineNo);
            }
        }
        else
        {

        }
        this->Show(false);
        m_parent->SetFocus();
    }
    // m_parent->ExecuteCommand();

    // event.Skip();
}

void svCommandLineCtrl::OnCommandKeyDown(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_ESCAPE )
    {
       this->Show(false);
       m_parent->SetFocus();
       return;
    }
    else
    {
        // if (event.GetId()==SVID_TXT_COMMAND)
        // {
        //     if (event.GetKeyCode() == WXK_PAGEUP || 
        //         event.GetKeyCode() == WXK_PAGEDOWN || 
        //         event.GetKeyCode() == WXK_UP || 
        //         event.GetKeyCode() == WXK_DOWN)
        //     {
        //         wxKeyEvent e = event;
        //         e.SetId(SVID_TXT_COMMAND_LIST_BOX);
        //         OnCommandKeyDown(e);
        //     }
        // }
    }
    event.Skip();
}


// void svCommandLineCtrl::OnBtnFindClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoFindNextWordwxStr(txtFind->GetValue());
//         editor->SetFocus();
//     }
// }

// void svCommandLineCtrl::OnBtnFindPrevClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoFindPrevWordwxStr(txtFind->GetValue());
//         editor->SetFocus();
//     }
// }

// void svCommandLineCtrl::OnBtnFindAllClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoFindAllWordwxStr(txtFind->GetValue());
//         editor->SetFocus();
//     }
// }

// void svCommandLineCtrl::OnBtnReplaceClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoReplaceNextWordwxStr(txtFind->GetValue(), txtReplace->GetValue());
//         editor->SetFocus();
//     }
// }

// void svCommandLineCtrl::OnBtnReplacePrevClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoReplacePrevWordwxStr(txtFind->GetValue(), txtReplace->GetValue());
//         editor->SetFocus();
//     }
// }

// void svCommandLineCtrl::OnBtnReplaceAllClick(wxCommandEvent& event)
// {
//     if (!m_mainFrame) return;

//     svTextEditorCtrl *editor;
//     editor = m_mainFrame->GetCurrentsvTextEditorCtrl();
//     if (editor)
//     {
//         editor->DoReplaceAllWordwxStr(txtFind->GetValue(), txtReplace->GetValue());
//         editor->SetFocus();
//     }
// }


// void svCommandLineCtrl::OnEraseBackGround(wxEraseEvent& event)
// {
    
// }


void svCommandLineCtrl::SetFocus(void)
{
    txtCommand->SetFocus();
}

bool svCommandLineCtrl::CheckCommand(svLineCommand &p_cmd)
{
    wxString cmd = txtCommand->GetValue();
    cmd.Trim();

    if (m_commandType==SVID_CMD_GOTO_LINE)
    {
        unsigned long lineNo;
        if (cmd.ToULong(&lineNo))
        {
            p_cmd.m_commandType = m_commandType;
            p_cmd.m_lineNo = (int) lineNo - 1;
            return true;
        }
        return false;
    }
    else if (m_commandType==SVID_CMD_GOTO_DEFINITION)
    {
        p_cmd.m_commandType = m_commandType;
        return true;
    }

    return false;
}

void svCommandLineCtrl::SetCommandType(char p_commandType)
{
    m_commandType = p_commandType;
    if (m_commandType==SVID_CMD_NONE)
    {
        lblMsg->SetLabel(_("N"));
    }
    else if (m_commandType==SVID_CMD_GOTO_LINE)
    {
        lblMsg->SetLabel(_("G"));
        //vSizer12->Clear(true);
        //vSizer12->Layout();
        this->SetSize(this->GetSize().GetWidth(), bSizer6->GetSize().GetHeight());
        this->Layout();
        txtCommand->SetValue(_(""));
    }
    else if (m_commandType==SVID_CMD_GOTO_DEFINITION)
    {
        lblMsg->SetLabel(_("R"));
        m_definitionLines.clear();
        m_filterLines.clear();
        if (((svTextEditorCtrl *)m_parent)->GetDefinitionLineNo(m_definitionLines))
        {
            m_filterLines = m_definitionLines;

            int labelTotalHeight = 0;

            m_listBox1->Clear();

            for (vector<svIntText>::iterator it=m_filterLines.begin();
                 it!=m_filterLines.end();
                 ++it)
            {

                m_listBox1->Append(it->m_text);

                // stext->Connect( wxEVT_LEFT_DOWN, 
                //       wxMouseEventHandler( svCommandLineCtrl::OnLabelMouseLeftDown ), NULL, this );


                // txtCommand->Connect( wxEVT_KEY_DOWN, 
                //       wxKeyEventHandler( svCommandLineCtrl::OnLabelKeyDown ), NULL, this );

                // labelTotalHeight += stext->GetSize().GetHeight() + 2 * m_border;
            }
            m_listBox1->SetSelection(0);
            m_selectIndex = 0;

            // resize the command line windows size according to the available definition found.
            // int width = this->GetSize().GetWidth();
            // int height = this->GetSize().GetHeight();
            // if (labelTotalHeight + m_border * 2 + bSizer6->GetSize().GetHeight() < height)
            //     this->SetSize(width, labelTotalHeight + m_border * 2 + bSizer6->GetSize().GetHeight());
            // else if (labelTotalHeight + m_border * 2 + bSizer6->GetSize().GetHeight() > SVID_COMMAND_LINE_MAX_HEIGHT)
            //     this->SetSize(width, SVID_COMMAND_LINE_MAX_HEIGHT);

            // int width = this->GetSize().GetWidth();
            // int height = this->GetSize().GetHeight();

            this->SetSize(400, 300);

            // vHintSizer.Show(true);
            this->Layout();
            txtCommand->SetValue(_(""));
        }
        else
        {
            // No definition found.
            // vHintSizer.Show(false);
            int width = this->GetSize().GetWidth();
            int height = this->GetSize().GetHeight();
            this->SetSize(width, bSizer6->GetSize().GetHeight());
            this->Layout();
            txtCommand->SetValue(_(""));
            m_selectIndex = -1;
        }
    }
    else if (m_commandType==SVID_CMD_REGULAR)
    {
        lblMsg->SetLabel(_("C"));
    }
    else
    {
        lblMsg->SetLabel(_("?"));
    }
    
}

// void svCommandLineCtrl::OnLabelMouseLeftDown(wxMouseEvent& event)
// {
//     if (m_commandType!=SVID_CMD_GOTO_DEFINITION)
//         return;

//     // The mouse click position of vSizer12
//     wxStaticText *lbl = (wxStaticText *)event.GetEventObject();
//     wxPoint pos = vSizer12->GetItem(lbl)->GetPosition();
//     txtCommand->SetValue(lbl->GetLabel());

//     // Mouse click in which wxStaticText
//     // int idx = pos.y / (lbl->GetSize().GetHeight() + lbl->GetBorder());
//     int idx = pos.y / (lbl->GetSize().GetHeight() + m_border*2);

//     // txtCommand->AppendText(wxString::Format(" pos=%i %i item index=%i Height=%i", pos.x, pos.y, idx, lbl->GetSize().GetHeight()));

//     if (idx<(int)m_filterLines.size())
//     {
//         // Legal index.
//         // txtCommand->SetValue("");
//         ((svTextEditorCtrl *)m_parent)->GotoLine( m_filterLines.at(idx).m_lineNo);
//     }
//     else
//     {
//         // Illegal index.
//         wxLogMessage(wxString::Format("svCommandLineCtrl::OnLabelMouseLeftDown Error: m_filterLines index out of range: %i of %i", idx, m_filterLines.size()-1));
//     }

//     txtCommand->SetFocus();
    
// }

// void svCommandLineCtrl::OnLabelKeyDown(wxKeyEvent& event)
// {
//     if ( event.GetKeyCode() == WXK_ESCAPE )
//     {
//        this->Show(false);
//        m_parent->SetFocus();
//        return;
//     }
//     event.Skip();
// }

void svCommandLineCtrl::OnListBox(wxCommandEvent& event)
{

    int sel = event.GetSelection();
    // txtCommand->SetValue(m_filterLines.at(sel).m_text);
    ((svTextEditorCtrl *)m_parent)->GotoLine( m_filterLines.at(sel).m_lineNo);

}