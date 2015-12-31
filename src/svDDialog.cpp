/*
   Copyright Notice in awvic.cpp
*/

#include "svDDialog.h"
#include <wx/platinfo.h>

#include "stdwx.h"

BEGIN_EVENT_TABLE( svDDialog, wxDialog )
//EVT_BUTTON( OMIDD_OK, svDDialog::OnOKClick )
EVT_CLOSE(           svDDialog::OnDialogClose )
END_EVENT_TABLE()


svDDialog::svDDialog()
:wxDialog()
{
    Init();
}

svDDialog::svDDialog( wxWindow* parent,
wxWindowID id,
const wxString& caption,
const wxPoint& pos,
const wxSize& size,
long style )
//              :wxDialog(parent, id, caption, pos, size, style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

void svDDialog::Init()
{
}

bool svDDialog::Create( wxWindow* parent,
wxWindowID id,
const wxString& caption,
const wxPoint& pos,
const wxSize& size,
long style )
{
    ////@begin abcd creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    ////@end abcd creation
    return true;
}


void svDDialog::CreateControls()
{
    SetSize(wxRect(0, 0, 400, 200));
    svDDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemDialog1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    wxPanel* itemPanel4 = new wxPanel( itemNotebook3, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemPanel4->SetSizer(itemBoxSizer5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer6, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel4, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemPanel4, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemTextCtrl8, 1, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton10 = new wxButton( itemPanel4, ID_BUTTON5, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemPanel4, ID_BUTTON6, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemPanel4, ID_BUTTON7, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("Tab"));

    wxPanel* itemPanel13 = new wxPanel( itemNotebook3, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
    itemPanel13->SetSizer(itemBoxSizer14);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer14->Add(itemBoxSizer15, 0, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel13, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemStaticText16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl17 = new wxTextCtrl( itemPanel13, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemTextCtrl17, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer14->Add(itemBoxSizer18, 1, wxGROW|wxALL, 5);
    wxStaticText* itemStaticText19 = new wxStaticText( itemPanel13, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl20 = new wxTextCtrl( itemPanel13, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemTextCtrl20, 1, wxALIGN_TOP|wxALL, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer14->Add(itemBoxSizer21, 0, wxALIGN_LEFT|wxALL, 5);
    wxButton* itemButton22 = new wxButton( itemPanel13, ID_BUTTON8, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton23 = new wxButton( itemPanel13, ID_BUTTON9, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton24 = new wxButton( itemPanel13, ID_BUTTON10, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton25 = new wxButton( itemPanel13, ID_BUTTON11, _("Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemNotebook3->AddPage(itemPanel13, _("Tab"));

    itemBoxSizer2->Add(itemNotebook3, 1, wxGROW|wxALL, 1);


    CenterOnScreen();
}

/*void svDDialog::OnOKClick(wxCommandEvent& event)
{
if (IsModal())
    EndModal(wxID_OK);
else
{
    SetReturnCode(wxID_OK);
    this->Show(false);
}
}*/

void svDDialog::OnDialogClose(wxCloseEvent& event)
{
    Destroy();
}

