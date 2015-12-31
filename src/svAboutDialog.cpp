/*
   Copyright Notice in awvic.cpp
*/

#include "svAboutDialog.h"
#include <wx/platinfo.h>

#include "stdwx.h"

BEGIN_EVENT_TABLE( svAboutDialog, wxDialog )
// EVT_BUTTON( SVID_OK, svAboutDialog::OnOKClick )
// EVT_CLOSE(           svAboutDialog::OnDialogClose )
END_EVENT_TABLE()


svAboutDialog::svAboutDialog()
:wxDialog()
{
    CreateControls();
}

svAboutDialog::svAboutDialog( wxWindow* parent,
wxWindowID id,
const wxString& caption,
const wxPoint& pos,
const wxSize& size,
long style )
:wxDialog(parent, id, caption, pos, size, style)
{
    CreateControls();
}

void svAboutDialog::Init()
{
}

bool svAboutDialog::Create( wxWindow* parent,
wxWindowID id,
const wxString& caption,
const wxPoint& pos,
const wxSize& size,
long style )
{
    //super->Create(parent, id, caption, pos, size, style);
    CreateControls();
    return true;
}


void svAboutDialog::CreateControls()
{
    this->SetBackgroundColour( wxColour( 39, 40, 34 ) );
    
    wxBoxSizer* bSizer1;
    bSizer1 = new wxBoxSizer( wxVERTICAL );
    
    
    bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer2;
    bSizer2 = new wxBoxSizer( wxHORIZONTAL );
    
    
    bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
    
    lbl01 = new wxStaticText( this, wxID_ANY, wxT("AWVIC"), wxDefaultPosition, wxDefaultSize, 0 );
    lbl01->Wrap( -1 );
    lbl01->SetFont( wxFont( 12, 70, 90, 92, false, wxEmptyString ) );
    lbl01->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
    
    bSizer2->Add( lbl01, 0, wxALL, 5 );
    
    
    bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    bSizer1->Add( bSizer2, 1, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    
    
    bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer3;
    bSizer3 = new wxBoxSizer( wxHORIZONTAL );
    
    
    bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
    
    lbl02 = new wxStaticText( this, wxID_ANY, wxT("A Text Editor."), wxDefaultPosition, wxDefaultSize, 0 );
    lbl02->Wrap( -1 );
    lbl02->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT ) );
    
    bSizer3->Add( lbl02, 0, wxALL, 5 );
    
    
    bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    bSizer1->Add( bSizer3, 1, wxALIGN_CENTER|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
    
    
    bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
    
    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer( wxHORIZONTAL );
    
    
    bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );
    
    hl01 = new wxHyperlinkCtrl( this, wxID_ANY, wxT("www.awvic.org"), wxT("http://www.awvic.org"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    bSizer5->Add( hl01, 0, wxALL, 5 );
    
    
    bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    bSizer1->Add( bSizer5, 1, wxEXPAND, 5 );
    
    
    bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
    
    
    this->SetSizer( bSizer1 );
    this->SetSize(300, 200);
    this->Layout();
    
    this->CentreOnParent();

}
