/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVDDIALOG_H
#define _SVDDIALOG_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/dialog.h>
#include <wx/notebook.h>

#define ID_ABCD 10029
#define ID_NOTEBOOK1 10030
#define ID_PANEL2 10031
#define ID_TEXTCTRL5 10033
#define ID_BUTTON5 10034
#define ID_BUTTON6 10035
#define ID_BUTTON7 10036
#define ID_PANEL3 10032
#define ID_TEXTCTRL6 10037
#define ID_TEXTCTRL7 10038
#define ID_BUTTON8 10039
#define ID_BUTTON9 10040
#define ID_BUTTON10 10041
#define ID_BUTTON11 10042

enum
{
    IDD_FINDNREPLACE=wxID_HIGHEST+1,
    IDD_NOTEBOOK_FR,
    IDD_PANEL_FIND,
    IDD_TEXT_F_FIND,
    IDD_BTN_FIND_NEXT,
    IDD_BTN_FIND_PREV,
    IDD_BTN_FIND_CANCEL,
    IDD_PANEL_REPLACE,
    IDD_TEXT_R_FIND,
    IDD_TEXT_R_REPLACE,
    IDD_BTN_REPLAC_ALL,
    IDD_BTN_REPLACE_NEXT,
    IDD_BTN_REPLACE_PREV,
    IDD_BTN_REPLACE_CANCEL
};

class svDDialog : public wxDialog
{
    // DECLARE_CLASS( svDDialog )
private:
    DECLARE_EVENT_TABLE()

public:
    svDDialog();
    svDDialog( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Find & Replace"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void Init();

    bool Create( wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Find & Replace"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE );

    void CreateControls();
    //void OnOKClick(wxCommandEvent& event);
    void OnDialogClose(wxCloseEvent& event);
};

#endif
