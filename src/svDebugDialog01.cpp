/*
   Copyright Notice in awvic.cpp
*/

#include "svDebugDialog01.h"
#include <wx/platinfo.h>
#include <wx/xml/xml.h>

#include "stdwx.h"

BEGIN_EVENT_TABLE( svDebugDialog01, wxDialog )
EVT_BUTTON( wxID_OK, svDebugDialog01::OnOKClick )
EVT_CLOSE(           svDebugDialog01::OnDialogClose )
END_EVENT_TABLE()


svDebugDialog01::svDebugDialog01()
:wxDialog()
{
    CreateControls();
}

svDebugDialog01::svDebugDialog01( wxWindow* parent,
wxWindowID id,
const wxString& caption,
const wxPoint& pos,
const wxSize& size,
long style )
:wxDialog(parent, id, caption, pos, size, style)
{
    CreateControls();
}

void svDebugDialog01::Init()
{
}

bool svDebugDialog01::Create( wxWindow* parent,
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


void svDebugDialog01::CreateControls(void)
{

    SetSize(wxRect(0, 0, 600, 500));
    boxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(boxSizer2);

    boxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    boxSizer2->Add(boxSizer3, 1, wxEXPAND|wxALL, 5);

    tcDebug2 = new wxTextCtrl( this, SVID_TCDEBUG2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    tcDebug2->SetSize(wxRect(0, 0, 120, 30));
    boxSizer3->Add(tcDebug2, 1, wxEXPAND|wxALL, 5);

    btnOK = new wxButton( this, wxID_OK, wxT("ok"), wxDefaultPosition, wxDefaultSize, 0 );
    boxSizer3->Add(btnOK, 0, wxALIGN_BOTTOM|wxALL, 5);

    boxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    boxSizer2->Add(boxSizer6, 1, wxEXPAND|wxALL, 5);

    tcDebug1 = new wxTextCtrl( this, SVID_TCDEBUG1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    boxSizer6->Add(tcDebug1, 1, wxEXPAND|wxALL, 5);

    CenterOnScreen();

    DebugInfo01();
    DebugInfo02();
}

void svDebugDialog01::OnOKClick(wxCommandEvent& event)
{
    if (IsModal())
    EndModal(wxID_OK);
    else
    {
        SetReturnCode(wxID_OK);
        this->Show(false);
    }
}

void svDebugDialog01::OnDialogClose(wxCloseEvent& event)
{
    Destroy();
}

// doesn't work.
bool svDebugDialog01::DebugInfo01(void)
{
    // wxXmlDocument doc;
    // if (!doc.Load(wxT("stylers.xml")))
    //   return false;

    // if (doc.GetRoot()->GetName() != wxT("NotepadPlus"))
    //   return false;
    // else
    //   tcDebug1->AppendText(wxT("NotepadPlus xml root found.\n"));

    // wxXmlNode *child = doc.GetRoot()->GetChildren();
    // while (child)
    // {
    //   if (child->GetName() == wxT("LexerStyles"))
    //   {
    //     wxXmlNode *cchild = child->GetChildren();
    //     wxXmlNode *ccchild = cchild->GetChildren();
    //     while (ccchild)
    //     {
    //       wxXmlProperty *pro = ccchild->GetProperties();
    //       while(pro)
    //       {
    //         tcDebug1->AppendText(pro->GetName() + wxT(" ") + pro->GetValue() + wxT("\n"));
    //         pro = pro->GetNext();
    //       }
    //       ccchild = ccchild->GetNext();
    //     }
    //   }
    //   else if (child->GetName() == wxT("tag2"))
    //   {
    //     // process tag2
    //   }
    //   child = child->GetNext();
    // }

    return true;
}

bool svDebugDialog01::DebugInfo02(void)
{
/*    svTextStyle* textStyle = new svTextStyle(wxT("CPP"));
    textStyle->LoadFile(wxT("style_cpp.om"));
    svStyle* s1 = textStyle->GetTextStyle(wxT("#"));
    wxColour c1 = s1->GetfgColor();
    svStyle* s2 = textStyle->GetTextStyle(wxT("("));
    wxColour c2 = s2->GetfgColor();
    svStyle* s3 = textStyle->GetTextStyle(wxT("else"));
    wxColour c3 = s3->GetfgColor();
    svStyle* s4 = textStyle->GetTextStyle(wxT("test"));
    wxColour c4 = s4->GetfgColor();*/
    return true;
}
