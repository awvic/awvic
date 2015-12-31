/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVDEBUGDIALOG01_H
#define _SVDEBUGDIALOG01_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/dialog.h>

enum
{
    SVID_TCDEBUG1=wxID_HIGHEST+1,
    SVID_TCDEBUG2//,
    //SVID_OK
};


class svDebugDialog01 : public wxDialog
{
    // DECLARE_CLASS( svDebugDialog01 )
private:
    DECLARE_EVENT_TABLE()

    wxBoxSizer* boxSizer2;
    wxBoxSizer* boxSizer3;
    wxTextCtrl* tcDebug1;
    wxButton* btnOK;
    wxBoxSizer* boxSizer6;
    wxTextCtrl* tcDebug2;

public:
    svDebugDialog01();
    svDebugDialog01( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Debug Panel"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void Init();

    bool Create( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Debug Panel"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void CreateControls(void);
    void OnOKClick(wxCommandEvent& event);
    void OnDialogClose(wxCloseEvent& event);
    bool DebugInfo01(void);
    bool DebugInfo02(void);
};

#endif
