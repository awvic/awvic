/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVTYPEHINTCTRL_H
#define _SVTYPEHINTCTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/stdpaths.h>
#include <wx/listctrl.h>

#include "svBufText.h"
#include "svCommand.h"
#include "svTextView.h"
#include "svBaseType.h"


using namespace std;

// forward declaration. avoid cross reference problem.
class svMainFrame; 

enum
{
    ID_HINT_LIST_BOX=wxID_HIGHEST+1
};

// class svTypeHintCtrl : public wxDialog
class svTypeHintCtrl : public wxFrame
{
public:
    // svTypeHintCtrl(svMainFrame *p_mainFrame, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 180,180 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxT("svTypeHintCtrl"));
    svTypeHintCtrl(svTextEditorCtrl *p_mainFrame, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 180,180 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxT("svTypeHintCtrl"));
    ~svTypeHintCtrl();
    // void OnSize(wxSizeEvent& event);

    // void OnChar(wxKeyEvent& event);
    void OnClose(wxCloseEvent& event);
    void InitControls(void);

    void SetHints(vector<wxString> &p_hintList);
    void OnKeyDown(wxKeyEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnListBoxDClick(wxCommandEvent& event);
    void OnMouseLeftDClick(wxMouseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);

private:

    svTextEditorCtrl *m_txtCtrl;

    // wxStaticText *lblMessage;

    // wxListCtrl* m_lcHints;
    wxListBox* m_lcHints;

private:
    DECLARE_EVENT_TABLE()

};


#endif
