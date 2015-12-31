/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVOPENFILESCTRL_H
#define _SVOPENFILESCTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <memory>

#include <wx/caret.h>
#include <wx/textfile.h>
#include <wx/dcbuffer.h>
#include <wx/tglbtn.h>
#include <wx/aui/auibook.h>
#include <wx/treectrl.h>
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

using namespace std;

// forward declaration. avoid cross reference problem.
class svMainFrame; 

enum
{
    ID_TREE_CTRL_OPENFILE=wxID_HIGHEST+1,
    ID_ABC
};

class svOpenFilesCtrl : public wxWindow
{
public:
    svOpenFilesCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name);
    ~svOpenFilesCtrl();
    // void OnSize(wxSizeEvent& event);
    void OnClose(wxCloseEvent& event);
    void InitControls(void);
    // void OnSelChanged(wxTreeEvent& event);
    // void OnSelChanging(wxTreeEvent& event);
    // void OnItemActivated(wxTreeEvent& event);

    inline
    void SetMainFrame(svMainFrame *p_mainFrame)
    {
        m_mainFrame = p_mainFrame;
    }

    void InsertNewFile(const svFileDesc &p_file);
    bool CheckDupFile(const svFileDesc &p_file);
    void DeleteFile(const svFileDesc &p_file);
    void ChangeFileDesc(const svFileDesc &p_ofile, const svFileDesc &p_nfile);
    void ChangeCurrentItemBG(const svFileDesc &p_fd);
    void OnItemRightClicked(wxTreeEvent& event);
    // void OnItemCollapsed(wxTreeEvent& event);
    // void OnItemExpanded(wxTreeEvent& event);

    // void OnFocused(wxFocusEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    // void OnMouseUp(wxMouseEvent& event);

private:
    svMainFrame    *m_mainFrame;
    wxTreeCtrl     *m_tcOpenFiles;
    map<wxTreeItemId, svFileDesc> m_filesList;

    wxColor m_bgColor;
    wxColor m_curBgColor;

private:
    DECLARE_EVENT_TABLE()

};

#endif
