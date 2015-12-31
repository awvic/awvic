/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVABOUTDIALOG_H
#define _SVABOUTDIALOG_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/dialog.h>
#include <wx/hyperlink.h>

enum
{
    SVID_TCVERSION=wxID_HIGHEST+1,
    SVID_TCINFO,
    SVID_OK
};


class svAboutDialog : public wxDialog
{
    // DECLARE_CLASS( svAboutDialog )
private:
    DECLARE_EVENT_TABLE()

    wxStaticText* lbl01;
    wxStaticText* lbl02;
    wxStaticText* lbl03;
    wxHyperlinkCtrl* hl01;

public:
    svAboutDialog();
    svAboutDialog( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("About awvic"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void Init();

    bool Create( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("About awvic"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void CreateControls();
};

#endif
