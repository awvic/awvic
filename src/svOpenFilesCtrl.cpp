/*
   Copyright Notice in awvic.cpp
*/


// *********************************************************************
//
// A Find & Replace control for awvic.
//
// *********************************************************************

#include "svOpenFilesCtrl.h"
#include <wx/dcbuffer.h>
#include <wx/log.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include "wx/filedlg.h"
#include "wx/file.h"
#include "wx/strconv.h"
#include "wx/encconv.h"
#include "wx/textfile.h"
#include "wx/colour.h"
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


BEGIN_EVENT_TABLE(svOpenFilesCtrl, wxWindow)
// EVT_PAINT(svOpenFilesCtrl::OnPaint)
// EVT_ERASE_BACKGROUND(svOpenFilesCtrl::OnErase)
// EVT_SIZE(svOpenFilesCtrl::OnSize)
// EVT_CHAR(svOpenFilesCtrl::OnChar)
// EVT_CLOSE(svOpenFilesCtrl::OnClose)
// EVT_TREE_SEL_CHANGED(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnSelChanged)
// EVT_TREE_SEL_CHANGING(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnSelChanging)
// EVT_TREE_ITEM_ACTIVATED(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnItemActivated)
// EVT_SET_FOCUS(svOpenFilesCtrl::OnFocused)
// EVT_LEFT_DOWN(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnMouseDown)
// EVT_RIGHT_DOWN(svOpenFilesCtrl::OnMouseDown)
// EVT_LEFT_UP(svOpenFilesCtrl::OnMouseUp)
// EVT_RIGHT_UP(svOpenFilesCtrl::OnMouseUp)
// EVT_TREE_ITEM_RIGHT_CLICK(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnItemRightClicked)
// EVT_TREE_ITEM_EXPANDED(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnItemExpanded)
// EVT_TREE_ITEM_COLLAPSED(ID_TREE_CTRL_OPENFILE, svOpenFilesCtrl::OnItemCollapsed)
END_EVENT_TABLE()


svOpenFilesCtrl::svOpenFilesCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"))
:wxWindow(parent, id, pos, size, style, name)
{
    m_mainFrame = NULL;
    InitControls();
}

svOpenFilesCtrl::~svOpenFilesCtrl()
{
    // if (m_tcOpenFiles) delete m_tcOpenFiles;  << wxWidget will handle it.
}

void svOpenFilesCtrl::InitControls(void)
{
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxVERTICAL );
    
    wxStaticText *lblMessage = new wxStaticText( this, wxID_ANY, wxT("Open Files"), wxDefaultPosition, wxDefaultSize, 0 );
    lblMessage->Wrap( -1 );
    bSizer1->Add( lblMessage, 0, wxALL|wxEXPAND, 5 );
    
    m_tcOpenFiles = new wxTreeCtrl( this, ID_TREE_CTRL_OPENFILE, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_NO_LINES|wxNO_BORDER|wxTR_SINGLE );
    m_tcOpenFiles->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
    m_tcOpenFiles->AddRoot(wxT("Open files"));

    m_tcOpenFiles->Bind(wxEVT_LEFT_DOWN , &svOpenFilesCtrl::OnMouseLeftDown, this);

    bSizer1->Add( m_tcOpenFiles, 1, wxALL|wxEXPAND, 5 );
    
    this->SetSizer( bSizer1 );
    this->Layout();

    m_bgColor = m_tcOpenFiles->GetBackgroundColour();
    m_curBgColor = *wxBLUE;
}

// void svOpenFilesCtrl::OnSize(wxSizeEvent& event)
// {

// }

void svOpenFilesCtrl::OnClose(wxCloseEvent& event)
{
}

void svOpenFilesCtrl::InsertNewFile(const svFileDesc &p_fd)
{
    size_t s = m_tcOpenFiles->GetChildrenCount(m_tcOpenFiles->GetRootItem(), false);
    wxTreeItemId id = m_tcOpenFiles->InsertItem (m_tcOpenFiles->GetRootItem(), s, p_fd.m_displayName);
    // wxTreeItemId id = m_tcOpenFiles->PrependItem(m_tcOpenFiles->GetRootItem(), p_fd.m_displayName);
    m_filesList.emplace(id, p_fd);
    ChangeCurrentItemBG(p_fd);
}

bool svOpenFilesCtrl::CheckDupFile(const svFileDesc &p_fd)
{
    for (std::map<wxTreeItemId, svFileDesc>::iterator it=m_filesList.begin();
         it!=m_filesList.end();
         ++it)
    {
        if (it->second == p_fd)
            return true;
    }
    return false;
}

void svOpenFilesCtrl::DeleteFile(const svFileDesc &p_file)
{
    for (std::map<wxTreeItemId, svFileDesc>::iterator it=m_filesList.begin();
         it!=m_filesList.end(); 
         ++it)
    {
        if (it->second.m_fullPathName==p_file.m_fullPathName)
        {
            m_tcOpenFiles->Delete(it->first);
            m_filesList.erase(it);
            break;
        }
    }
}

// Change fileDesc on SaveAs command.
void svOpenFilesCtrl::ChangeFileDesc(const svFileDesc &p_ofile, const svFileDesc &p_nfile)
{
    for (std::map<wxTreeItemId, svFileDesc>::iterator it=m_filesList.begin();
         it!=m_filesList.end(); 
         ++it)
    {
        if (it->second.m_fullPathName==p_ofile.m_fullPathName)
        {
            it->second.m_fullPathName = p_nfile.m_fullPathName;
            it->second.m_displayName = p_nfile.m_displayName;

            m_tcOpenFiles->SetItemText(it->first, p_nfile.m_displayName);
            // m_filesList.erase(it);

            break;
        }
    }
}

void svOpenFilesCtrl::ChangeCurrentItemBG(const svFileDesc &p_fd)
{
    wxTreeItemId curItem;
    for (std::map<wxTreeItemId, svFileDesc>::iterator it=m_filesList.begin();
         it!=m_filesList.end();
         ++it)
    {
        if (it->second.m_fullPathName==p_fd.m_fullPathName)
        {
            if (it->first.IsOk())
            {
                curItem = it->first;
                // m_tcOpenFiles->SelectItem(it->first);
                // m_tcOpenFiles->SelectItem(it->first, false);
                // m_tcOpenFiles->SetFocusedItem(it->first);
                // m_tcOpenFiles->ToggleItemSelection(it->first);
                break;
            }
        }
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId root = m_tcOpenFiles->GetRootItem();
    wxTreeItemId c = m_tcOpenFiles->GetFirstChild(root, cookie);
    while (c.IsOk())
    {
        if (c==curItem)
        {
            m_tcOpenFiles->SetItemBold(c, true);
            m_tcOpenFiles->SetItemBackgroundColour(c, *wxLIGHT_GREY);
        }
        else
        {
            m_tcOpenFiles->SetItemBold(c, false);
            m_tcOpenFiles->SetItemBackgroundColour(c, m_tcOpenFiles->GetBackgroundColour());
        }
        c = m_tcOpenFiles->GetNextChild(root, cookie);
    }
}

// void svOpenFilesCtrl::OnSelChanged(wxTreeEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnSelChanged");
// #endif
    
//     wxTreeItemId id = event.GetItem();
//     // wxTreeItemId id = m_tcOpenFiles->GetFocusedItem();
//     // wxTreeItemId id = m_tcOpenFiles->GetSelection();

//     if (!id.IsOk()) return;

//     std::map<wxTreeItemId, svFileDesc>::iterator it = m_filesList.find(id);
//     if (it != m_filesList.end())
//     {
//         svFileDesc fd = it->second;
//         m_mainFrame->ChangeCurrentsvTextEditorCtrl(fd);
//         // m_tcOpenFiles->ClearFocusedItem();
//     }

//     // m_mainFrame->SetCurrentsvTextEditorCtrlFocus();

// }

// void svOpenFilesCtrl::OnSelChanging(wxTreeEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnSelChanging");
// #endif
//     wxTreeItemId id = event.GetItem();
//     // wxTreeItemId id = m_tcOpenFiles->GetFocusedItem();
//     // wxTreeItemId id = m_tcOpenFiles->GetSelection();

//     if (!id.IsOk()) return;

//     std::map<wxTreeItemId, svFileDesc>::iterator it = m_filesList.find(id);
//     if (it != m_filesList.end())
//     {
//         svFileDesc fd = it->second;
//         m_mainFrame->ChangeCurrentsvTextEditorCtrl(fd);
//         // m_tcOpenFiles->ClearFocusedItem();
//     }

//     m_mainFrame->SetCurrentsvTextEditorCtrlFocus();
//     // event.Veto();
// }

// void svOpenFilesCtrl::OnItemActivated(wxTreeEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnItemActivated");
// #endif

//     wxTreeItemId id = event.GetItem();

//     if (!id.IsOk()) return;

//     std::map<wxTreeItemId, svFileDesc>::iterator it = m_filesList.find(id);
//     if (it != m_filesList.end())
//     {
//         svFileDesc fd = it->second;
//         m_mainFrame->ChangeCurrentsvTextEditorCtrl(fd);
//         m_tcOpenFiles->ClearFocusedItem();
//         m_mainFrame->SetCurrentsvTextEditorCtrlFocus();
//     }

// }

// void svOpenFilesCtrl::OnFocused(wxFocusEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnFocused");
// #endif
//     m_mainFrame->SetCurrentsvTextEditorCtrlFocus();
// }


// When an item is clicked. 
// Changing the wxAuiNotebook to the specified TextEditorControl.
void svOpenFilesCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage("svOpenFilesCtrl::OnMouseLeftDown");
#endif

    wxPoint mouse_position = event.GetPosition();
    int temp_num = wxTREE_HITTEST_ONITEMLABEL;

    wxTreeItemId id = m_tcOpenFiles->HitTest(mouse_position, temp_num);

    // if(id && tree->ItemHasChildren(id))
    if(id)
    {
        svFileDesc fd;
        std::map<wxTreeItemId, svFileDesc>::iterator it = m_filesList.find(id);
        if (it != m_filesList.end())
        {
            fd = it->second;
            m_mainFrame->ChangeCurrentsvTextEditorCtrl(fd);
            // m_tcOpenFiles->ClearFocusedItem();
            ChangeCurrentItemBG(fd);
        }

        // Changing the background color of the current item. NOT WORKING!
        // for (std::map<wxTreeItemId, svFileDesc>::iterator it=m_filesList.begin();
        //      it!=m_filesList.end();
        //      ++it)
        // {
        //     if (it->second.m_fullPathName==fd.m_fullPathName)
        //     {
        //         if (it->first.IsOk())
        //         {
        //             m_tcOpenFiles->SetItemBackgroundColour(it->first, m_curBgColor);
        //         }
        //     }
        //     else
        //     {
        //         if (it->first.IsOk())
        //         {
        //             m_tcOpenFiles->SetItemBackgroundColour(it->first, m_bgColor);
        //             break;
        //         }

        //     }
        // }

    }
    // else
    //     event.Skip();
}

// void svOpenFilesCtrl::OnMouseUp(wxMouseEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnMouseUp");
// #endif
//     m_mainFrame->SetCurrentsvTextEditorCtrlFocus();
// }

void svOpenFilesCtrl::OnItemRightClicked(wxTreeEvent& event)
{
#ifndef NDEBUG
    wxLogMessage("svOpenFilesCtrl::OnItemClicked");
#endif
    wxTreeItemId id = event.GetItem();

    if (!id.IsOk()) return;

    std::map<wxTreeItemId, svFileDesc>::iterator it = m_filesList.find(id);
    if (it != m_filesList.end())
    {
        svFileDesc fd = it->second;
        m_mainFrame->ChangeCurrentsvTextEditorCtrl(fd);
    }

}

// void svOpenFilesCtrl::OnItemCollapsed(wxTreeEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnItemCollapsed");
// #endif
// }

// void svOpenFilesCtrl::OnItemExpanded(wxTreeEvent& event)
// {
// #ifndef NDEBUG
//     wxLogMessage("svOpenFilesCtrl::OnItemExpanded");
// #endif
// }

