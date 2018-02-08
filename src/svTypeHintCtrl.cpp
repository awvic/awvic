/*
   Copyright Notice in awvic.cpp
*/

// *********************************************************************
//
// A Find & Replace control for awvic.
//
// *********************************************************************

#include "svTypeHintCtrl.h"
#include <wx/log.h>
#include <wx/tokenzr.h>
#include <wx/string.h>
#include "svTextEditorCtrl.h"
#include "svMainFrame.h"

#include "stdwx.h"
#include "svPreference.h"
#include <vector>

using namespace std;


BEGIN_EVENT_TABLE(svTypeHintCtrl, wxWindow)
// EVT_PAINT(svTypeHintCtrl::OnPaint)
// EVT_ERASE_BACKGROUND(svTypeHintCtrl::OnErase)
// EVT_SIZE(svTypeHintCtrl::OnSize)
EVT_CHAR(svTypeHintCtrl::OnChar)
EVT_CLOSE(svTypeHintCtrl::OnClose)
EVT_KEY_DOWN(svTypeHintCtrl::OnKeyDown)
EVT_SET_FOCUS(svTypeHintCtrl::OnSetFocus)

// EVT_LEFT_DOWN(svTypeHintCtrl::OnMouseLeftDown)
// EVT_LEFT_UP(svTypeHintCtrl::OnMouseLeftUp)
// EVT_RIGHT_UP(svTypeHintCtrl::OnMouseRightUp)
// EVT_MOTION(svTypeHintCtrl::OnMouseMotion)
// EVT_MOUSEWHEEL(svTypeHintCtrl::OnMouseWheel)

EVT_LISTBOX_DCLICK(ID_HINT_LIST_BOX, svTypeHintCtrl::OnListBoxDClick)
EVT_LEFT_DCLICK(svTypeHintCtrl::OnMouseLeftDClick)
EVT_LEFT_DOWN(svTypeHintCtrl::OnMouseLeftDown)
EVT_LEFT_UP(svTypeHintCtrl::OnMouseLeftUp)

END_EVENT_TABLE()


// svTypeHintCtrl::svTypeHintCtrl(svMainFrame *p_mainFrame, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("svTypeHintCtrl"))
svTypeHintCtrl::svTypeHintCtrl(svTextEditorCtrl *p_txtCtrl, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
// svTypeHintCtrl::svTypeHintCtrl(svMainFrame *p_mainFrame, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
// :wxDialog( parent, id, name, pos, size, style )
:wxFrame( parent, id, name, pos, size, style )
// :wxWindow( parent, id, pos, size, style )
{
    m_txtCtrl = p_txtCtrl;
    InitControls();
}

svTypeHintCtrl::~svTypeHintCtrl()
{
}

void svTypeHintCtrl::InitControls(void)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer4;
    bSizer4 = new wxBoxSizer( wxVERTICAL );
    
    // m_lcHints = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_ICON|wxLC_NO_HEADER );
    // m_lcHints = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_LIST );
    m_lcHints = new wxListBox( this, ID_HINT_LIST_BOX, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxLB_HSCROLL );    bSizer4->Add( m_lcHints, 1, wxALL|wxEXPAND, 0 );
    
/*    wxListItem col;
    col.SetId(0);
    col.SetText( _("Hint") );
    col.SetWidth(50);
    m_lcHints->InsertColumn(0, col);
    m_lcHints->SetColumnWidth(0, wxLIST_AUTOSIZE);*/

    this->SetSizer( bSizer4 );
    this->Layout();
    
    this->Centre( wxBOTH );

    m_lcHints->Bind(wxEVT_KEY_DOWN, &svTypeHintCtrl::OnKeyDown, this);
    m_lcHints->Bind(wxEVT_CHAR, &svTypeHintCtrl::OnChar, this);
    // m_lcHints->Bind(wxEVT_LEFT_DCLICK, &svTypeHintCtrl::OnMouseLeftDClick, this);
    // m_lcHints->Bind(wxEVT_LEFT_DOWN, &svTypeHintCtrl::OnMouseLeftDown, this);
    // m_lcHints->Bind(wxEVT_LEFT_UP, &svTypeHintCtrl::OnMouseLeftUp, this);
}


/*void svTypeHintCtrl::OnChar(wxKeyEvent& event)
{

    int key = event.GetKeyCode();


    if (key==WXK_RETURN)
    {
        
    }

}*/

void svTypeHintCtrl::OnClose(wxCloseEvent& event)
{
}

void svTypeHintCtrl::SetHints(vector<wxString> &p_hintList)
{
    // m_lcHints->ClearAll();
    m_lcHints->Clear();

    wxArrayString as;
    int id=0;
    for (std::vector<wxString>::iterator it=p_hintList.begin();
         it!=p_hintList.end();
         ++it)
    {
        // wxListItem item;
        // item.SetId(id);
        // item.SetText(*it);
        // m_lcHints->InsertItem(item);
        // id++;
        as.Add(*it);
    }
    m_lcHints->InsertItems(as, 0);
    if (as.GetCount()>0)
        m_lcHints->SetSelection(0);
}

void svTypeHintCtrl::OnKeyDown(wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    //wxChar ukey = event.GetUnicodeKey();

    if (key==27) // ESC
    {
        Hide();
    }
    else if (key==WXK_UP)
    {
        int sel = m_lcHints->GetSelection();
        if (sel>0)
            --sel;
        m_lcHints->SetSelection(sel);
    }
    else if (key==WXK_DOWN)
    {
        int max = m_lcHints->GetCount();
        int sel = m_lcHints->GetSelection();
        if (sel<max-1)
            ++sel;
        m_lcHints->SetSelection(sel);
    }
    else if (key==WXK_RETURN || 
             key==WXK_TAB)
    {
        int selNo = m_lcHints->GetSelection();
        if (selNo!=wxNOT_FOUND)
        {
            m_txtCtrl->DoTextInsertHint(m_lcHints->GetString(selNo));
        }
        Hide();
        m_txtCtrl->SetFocus();
    }
    else
    {
        /*event.Skip();
        wxKeyEvent e;
        e = event;
        m_txtCtrl->OnKeyDown(e);*/

        event.Skip();
        GetParent()->ProcessWindowEvent(event);
        // wxLogMessage(wxString::Format("svTypeHintCtrl::OnKeyDown() %i %i", e.m_x, e.m_y));
    }

}

void svTypeHintCtrl::OnSetFocus(wxFocusEvent& event)
{
    m_lcHints->SetFocus();   
}

void svTypeHintCtrl::OnChar(wxKeyEvent& event)
{
    /*event.Skip();
    wxKeyEvent e;
    e = event;
    m_txtCtrl->OnChar(e);*/

    event.Skip();
    GetParent()->ProcessWindowEvent(event);
    // wxLogMessage(wxString::Format("svTypeHintCtrl::OnChar() %i %i", e.m_x, e.m_y));

}

void svTypeHintCtrl::OnListBoxDClick(wxCommandEvent& event)
{
    int selNo = m_lcHints->GetSelection();
    if (selNo!=wxNOT_FOUND)
    {
        m_txtCtrl->DoTextInsertHint(m_lcHints->GetString(selNo));
    }
    event.StopPropagation();
    Hide();
    m_txtCtrl->SetFocus();
}

void svTypeHintCtrl::OnMouseLeftDClick(wxMouseEvent& event)
{
    int selNo = m_lcHints->GetSelection();
    if (selNo!=wxNOT_FOUND)
    {
        m_txtCtrl->DoTextInsertHint(m_lcHints->GetString(selNo));
    }
    event.StopPropagation();
    Hide();
    m_txtCtrl->SetFocus();
}

void svTypeHintCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
    event.StopPropagation();
}

void svTypeHintCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
    event.StopPropagation();
}
