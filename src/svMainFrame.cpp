/*
   Copyright Notice in awvic.cpp
*/

#include "svMainFrame.h"
#include "svBaseType.h"

#include "stdwx.h"
#include "svPreference.h"
#include <wx/arrstr.h>

enum
{
    Menu_NEW = 1,
    Menu_OPEN,
    Menu_SAVE,
    Menu_SAVEAS,
    Menu_CLOSE,
    Menu_EXIT,
    Menu_FIND,
    Menu_REPLACE,
    Menu_SELECTFONT = 100,
    Menu_ABOUT = 200,
    Menu_DEBUG = 210
};

BEGIN_EVENT_TABLE(svMainFrame, wxFrame)
EVT_MENU(Menu_NEW,        svMainFrame::OnMenuNew)
EVT_MENU(Menu_OPEN,       svMainFrame::OnMenuOpen)
EVT_MENU(Menu_SAVE,       svMainFrame::OnMenuSave)
EVT_MENU(Menu_SAVEAS,     svMainFrame::OnMenuSaveAs)
EVT_MENU(Menu_CLOSE,      svMainFrame::OnMenuClose)
EVT_MENU(Menu_EXIT,       svMainFrame::OnMenuQuit)
EVT_MENU(Menu_FIND,       svMainFrame::OnMenuFind)
EVT_MENU(Menu_REPLACE,    svMainFrame::OnMenuSideBar)
EVT_MENU(Menu_SELECTFONT, svMainFrame::OnMenuSelectFont)
EVT_MENU(Menu_ABOUT,      svMainFrame::OnMenuAbout)
EVT_MENU(Menu_DEBUG,      svMainFrame::OnMenuDebug)
EVT_CLOSE(                svMainFrame::OnClose)
EVT_SVTEXTCTRL_MODIFIED_CHANGED(wxID_ANY, svMainFrame::OnSVTextModified) 
EVT_SVTEXTCTRL_MSG(wxID_ANY,      svMainFrame::OnSVTextMSG) 
EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY,   svMainFrame::OnAuiNotebookPageClose)
EVT_AUINOTEBOOK_PAGE_CHANGING(wxID_ANY, svMainFrame::OnAuiNotebookPageChanging)
EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, svMainFrame::OnAuiNotebookPageChanged)
EVT_SPLITTER_DCLICK(wxID_ANY, svMainFrame::OnSashDoubleClick)
EVT_CHAR(                 svMainFrame::OnChar)
//EVT_SET_FOCUS(            svMainFrame::OnSetFocus)
//EVT_ACTIVATE(             svMainFrame::OnActivate)
END_EVENT_TABLE()

//#include "ompte.xpm"
svMainFrame::svMainFrame(const wxString& title, const wxString& openfiles, bool p_loadLastOpenedFiles)
// : wxFrame(NULL, wxID_ANY, title)
: wxFrame(NULL, wxID_ANY, title,
                 wxDefaultPosition, wxSize(800, 600),
                 wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{

    wxString apppath;
    apppath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fp(apppath);
    wxString path = fp.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
    
    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow(this);

    //SetIcon(wxIcon(ompte_xpm));
    pngHandler = new wxPNGHandler();
    wxImage::AddHandler(pngHandler);
    icn = new wxIcon();
    icn->LoadFile(path + wxT("icon.png"), wxBITMAP_TYPE_PNG);
    SetIcon(*icn);
    SetSize(wxRect(60, 60, 800, 600));

    // Initial MenuBar
    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append(Menu_NEW, _("&New\tAlt-N"),
    _("New file."));
    fileMenu->Append(Menu_OPEN, _("&Open\tAlt-O"),
    _("Open file."));
    fileMenu->Append(Menu_SAVE, _("&Save\tAlt-S"),
    _("Save file."));
    fileMenu->Append(Menu_SAVEAS, _("Save &As\tAlt-A"),
    _("Save As file."));
    fileMenu->Append(Menu_CLOSE, _("&Close\tAlt-C"),
    _("Close file."));
    fileMenu->Append(Menu_EXIT, _("E&xit\tAlt-X"),
    _("Quit ompte."));

    wxMenu* findMenu = new wxMenu;
    findMenu->Append(Menu_FIND, _("Find"),
    _("Show Find Window"));
    findMenu->Append(Menu_REPLACE, _("Side Bar"),
    _("Show Opended Files Window"));

    wxMenu* settingsMenu = new wxMenu;
    settingsMenu->Append(Menu_SELECTFONT, _("Select Font"),
    _("Select Font"));

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(Menu_ABOUT, _("&About...\tF1"),
    _("Show about dialog"));
    // helpMenu->Append(Menu_DEBUG, _("&Debug...\tF12"),
    // _("Debug dialog"));

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _("&File"));
    menuBar->Append(findMenu, _("V&iew"));
    menuBar->Append(settingsMenu, _("Preferences"));
    menuBar->Append(helpMenu, _("&Help"));

    SetMenuBar(menuBar);





    wxBoxSizer* vSizer1;
    vSizer1 = new wxBoxSizer( wxVERTICAL );
    
    splitterWin11 = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE );
    splitterWin11->SetSashGravity(0.2);

    panel111 = new wxPanel( splitterWin11, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* vSizer1111;
    vSizer1111 = new wxBoxSizer( wxVERTICAL );
    
    m_openFiles = new svOpenFilesCtrl(panel111, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, "Open Files");
    m_openFiles->SetMainFrame(this);
    vSizer1111->Add( m_openFiles, 1, wxALL|wxEXPAND, 0 );
    
    panel111->SetSizer( vSizer1111 );
    panel111->Layout();
    vSizer1111->Fit( panel111 );
    panel112 = new wxPanel( splitterWin11, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    
    vSizer1121 = new wxBoxSizer( wxVERTICAL );
    







    // insert a wxNotebook
    notebook = new wxAuiNotebook(panel112, SVID_NOTEBOOK, wxDefaultPosition, 
                                 wxDefaultSize, wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_MIDDLE_CLICK_CLOSE | wxWANTS_CHARS);

#ifdef __WXMSW__
    // cannot works in gtk, mark it for debug purpose 
    //m_art = NULL;
    //m_art = new omAuiTabArt();
    //notebook->SetArtProvider(m_art);
#endif 
    m_art = NULL;
    // m_art = new wxAuiSimpleTabArt();
    m_art = new wxAuiDefaultTabArt();
    notebook->SetArtProvider(m_art);
    notebook->Refresh();

    //notebook->Bind(wxEVT_CHILD_FOCUS, &svMainFrame::OnAuiNotebookChildGetFocus, this);





   
    vSizer1121->Add( notebook, 1, wxEXPAND | wxALL, 0 );

   
    vSizer11212 = new wxBoxSizer( wxVERTICAL );
    
    m_findCtrl = new svFindReplaceCtrl(panel112, SVID_FINDCTRL,
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL, "My Find & Replace Control");
  
    m_findCtrl->SetMainFrame(this);

    // m_typeHintCtrl = new svTypeHintCtrl(this, this, wxID_ANY, wxDefaultPosition, wxSize(150, 140), wxBORDER_NONE|wxCLIP_CHILDREN|wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT );
    
    vSizer11212->Add( m_findCtrl, 0, wxALL, 5 );
    
    vSizer1121->Add( vSizer11212, 0, wxEXPAND | wxALL, 0 );
    
    vSizer11212->Hide(m_findCtrl);
    
    panel112->SetSizer( vSizer1121 );
    panel112->Layout();
    vSizer1121->Fit( panel112 );
    splitterWin11->Initialize(panel111);
    splitterWin11->Initialize(panel112);
    splitterWin11->SplitVertically( panel111, panel112, 200 );
    
    vSizer1->Add( splitterWin11, 1, wxEXPAND, 0 );
    
    splitterWin11->Unsplit(panel111);
    
    this->SetSizer( vSizer1 );
    this->Layout();
    





































    CreateStatusBar(3);
    SetStatusText(wxT("ready"));

    SetMinSize(wxSize(250, 220));
    CentreOnScreen();

    // "commit" all changes made to wxAuiManager
    m_mgr.Update();
    m_defDir = wxGetCwd();

    // Reopen last time opened files.
    m_loadLastOpenedFiles = p_loadLastOpenedFiles;  // Read last opened files or not.
    ReadAppCloseStatus();

    // Open files from command line parameter.
    wxString fn(wxT(""));
    wxStringTokenizer tkz(openfiles, wxT("|"));
    if (tkz.CountTokens()>0)
    {
        while(tkz.HasMoreTokens())
        {
            fn = tkz.GetNextToken();
            svFileDescOpened fdo;
            fdo.m_firstLineNo = 0;
            fdo.m_carets.push_back(svCaret(0,0));
            DoOnMenuOpen(fn, &fdo);
        }

        // open file from parameter not display well (OnSize event processing)
        // We force it to resize();
        int pc = notebook->GetPageCount();
        if (pc>0)
        {
            notebook->SetSelection(pc);
            svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
            txtCtrl->DoOnSize();
            txtCtrl->SetFocus();
        }

    }

// #ifdef __WXMSW__    
//     m_fnRules.LoadRules(extFileName.mb_str());
// #else
//     m_fnRules.LoadRules(extFileName.ToUTF8());
// #endif

    //this->Bind(wxEVT_ACTIVATE, &svMainFrame::OnActivate, this);
    this->Connect( wxID_ANY, wxEVT_ACTIVATE, wxActivateEventHandler(svMainFrame::OnActivate) );

}

svMainFrame::~svMainFrame()
{
    m_mgr.UnInit();  
    if (notebook) delete notebook;
    if (icn) delete icn;

    if (m_findCtrl) delete m_findCtrl;
    // if (m_typeHintCtrl) delete m_typeHintCtrl;
    // if (m_art) delete m_art;
#ifdef __WXMSW__
    ////if (m_art!=NULL) delete m_art;
#endif 

}

void svMainFrame::OnMenuNew(wxCommandEvent& event)
{
    svTextEditorCtrl* curTextCtrl = GetCurrentsvTextEditorCtrl();
    if (curTextCtrl) curTextCtrl->HideTypeHint();
    
    wxPanel *window = new wxPanel(this->notebook, wxID_ANY, wxDefaultPosition, 
                                  wxDefaultSize, wxWANTS_CHARS|wxBORDER_NONE);
    svTextEditorCtrl* textCtrl = 
        new svTextEditorCtrl(this, window, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                             wxWANTS_CHARS|wxBORDER_SUNKEN, wxT("Text"));

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
    // Create text ctrl with minimal size 100x60
    topSizer->Add( textCtrl,
                   1, // make vertically stretchable
                   wxEXPAND| // make horizontally stretchable
                   wxALL, // and make border all around
                   0 ); // set border width to 10
                   // 2 ); // set border width to 10

    window->SetSizer(topSizer);
    wxString l_displayName = wxString::Format("Text %02i.txt", m_seqCodeGen.GetCurCode());
    this->notebook->AddPage(window, l_displayName, true);

    wxString appPath;
    appPath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fp(appPath);
    wxString path = fp.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

    svFileDesc fd(path+l_displayName, l_displayName);
    fd.SetLastModificationTime();
    textCtrl->SetFileDesc(fd);

    if (!textCtrl->InitNewFile(textCtrl->GetFullPathName()))
    {
        wxString msg;
        msg.Printf(wxT("Initialize new file error!"), wxVERSION_STRING);
        wxMessageBox(msg, wxT("Error.."), wxOK | wxICON_INFORMATION, this);
        return;
    }

    // On open and new file will not refind again.
    // we manual find it with m_lastFindReplaceOption.
    if (m_lastFindReplaceOption.m_from != svFindReplaceOption::SVID_FIND_NOT_SET)
    {
        textCtrl->DoFindMatchLocations(m_lastFindReplaceOption);
    }

    textCtrl->SetFocus();
    m_openFiles->InsertNewFile(fd);
}

void svMainFrame::OnMenuOpen(wxCommandEvent& event)
{
    wxString caption = wxT("Choose a file...");
    wxString wildcard = wxT("Text files (*.*)|*.*");
    wxString defaultDir = m_defDir;
    wxString defaultFilename = wxEmptyString;

    wxString txt;
    wxFileDialog dialog(this, caption, defaultDir, defaultFilename, 
                        wildcard, wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString fileArr;
        dialog.GetPaths(fileArr);
        for (size_t i=0; i<fileArr.GetCount(); i++)
        {
            DoOnMenuOpen(fileArr.Item(i));
        }
    }

}

void svMainFrame::DoOnMenuOpen(const wxString &path, svFileDescOpened *p_fdo)
{
    wxPanel *window;
    svTextEditorCtrl* textCtrl;

    svTextEditorCtrl* curTextCtrl = GetCurrentsvTextEditorCtrl();
    if (curTextCtrl) curTextCtrl->HideTypeHint();

    if (wxFile::Exists(path)){

        wxFileName file(path);
        svFileDesc fd(path, file.GetFullName());
        fd.SetLastModificationTime();
        if (!m_openFiles->CheckDupFile(fd))   // Not exist file.
        {
            window = new wxPanel(this->notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxBORDER_NONE);
            textCtrl = new svTextEditorCtrl(this, window, wxID_ANY, 
                                            wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxBORDER_SUNKEN, wxT("text"));

            // textCtrl->SetFocus();
            textCtrl->SetFileDesc(fd);

            wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
            topSizer->Add( textCtrl,
                           1, // make vertically stretchable
                           wxEXPAND| // make horizontally stretchable
                           wxALL, // and make border all around
                           0 ); // set border width to 10
                           //2 ); // set border width to 10
            
            window->SetSizer(topSizer);
            this->notebook->AddPage(window, textCtrl->GetDisplayName(), true);

            if (!textCtrl->ReadTextFile(path))
            {
                wxString msg;
                msg.Printf(wxT("File not found!"), wxVERSION_STRING);
                wxMessageBox(msg, wxT("Error.."), wxOK | wxICON_INFORMATION, this);
                return;
            }

            // svFileDescOpened is not null, initial carets from p_fdo.
            if (p_fdo)
            {
                textCtrl->ResetBufTextCarets(&*p_fdo);
            }

            SetStatusText(textCtrl->GetTextFileCharSet());
            
            // textCtrl->SetFocus();

            // On open and new file will not refind again.
            // we manual find it with m_lastFindReplaceOption.
            if (m_lastFindReplaceOption.m_from != svFindReplaceOption::SVID_FIND_NOT_SET)
            {
                textCtrl->DoFindMatchLocations(m_lastFindReplaceOption);
            }

            textCtrl->DoSmoothRefresh();

            m_openFiles->InsertNewFile(fd);
            textCtrl->SetFocus();
            // this->DeletePendingEvents();
            // textCtrl->DeletePendingEvents();
        }
        else  // Exist file.
        {
            ChangeCurrentsvTextEditorCtrl(fd);
            m_openFiles->ChangeCurrentItemBG(fd);
        }
    }
}

void svMainFrame::OnMenuSave(wxCommandEvent& event)
{
    DoOnMenuSave();
}

void svMainFrame::DoOnMenuSave(void)
{
    if (this->notebook->GetPageCount() <= 0){
        return;
    }

    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    svFileDesc ofd = txtCtrl->GetFileDesc();
    txtCtrl->SaveFile();
    svFileDesc nfd = txtCtrl->GetFileDesc();
    if (ofd!=nfd)
        m_openFiles->ChangeFileDesc(ofd, nfd);
}

void svMainFrame::OnMenuSaveAs(wxCommandEvent& event)
{
    if (this->notebook->GetPageCount() <= 0){
        return;
    }

    svTextEditorCtrl* textCtrl=GetCurrentsvTextEditorCtrl();

    svFileDesc ofd = textCtrl->GetFileDesc();

    // strange enough. shoud be modified later.
    // I just want a never duplicate file name.
    textCtrl->SaveFile(wxT("_$#12332123_") + textCtrl->GetFullPathName());

    svFileDesc nfd = textCtrl->GetFileDesc();
    if (ofd!=nfd)  // Update open files windows contents.
        m_openFiles->ChangeFileDesc(ofd, nfd);
}

void svMainFrame::OnMenuClose(wxCommandEvent& event)
{
    if (this->notebook->GetPageCount() <= 0){
        return;
    }
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    if (!txtCtrl->Close())
        return;
    this->notebook->DeletePage((size_t)this->notebook->GetSelection());
}

void svMainFrame::OnMenuFind(wxCommandEvent& event)
{
    //ShowFindWindow();
    ShowHideFindWindow();
}

void svMainFrame::OnMenuSideBar(wxCommandEvent& event)
{
    ShowHideSideBar();
}

void svMainFrame::OnMenuSelectFont(wxCommandEvent& WXUNUSED(event))
{
    if (!(this->notebook->GetPageCount()>0)) return;

    extern svPreference g_preference;
    wxFontData fdata;
    wxFont setupFont;
    setupFont.SetFaceName(g_preference.GetFontFace());
    setupFont.SetPointSize(g_preference.GetFontSize());
    fdata.SetInitialFont(setupFont);
    wxFontDialog dialog(this, fdata);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        wxFont font = retData.GetChosenFont();
        if (g_preference.GetFontFace()!=font.GetFaceName() ||
            g_preference.GetFontSize()!=font.GetPointSize())
        {
            g_preference.SetFontFace(font.GetFaceName());
            g_preference.SetFontSize(font.GetPointSize());

            // change every textEditorCtrl's font
            svTextEditorCtrl* textCtrl = NULL;
            size_t count = this->notebook->GetPageCount();
            for (size_t i=0; i<count; i++)
            {
                wxWindow *w = this->notebook->GetPage(i);
                if (!w) break;

                wxWindowList lst = w->GetChildren();
                wxWindowList::iterator iter;
                for (iter = lst.begin(); iter != lst.end(); ++iter)
                {
                    wxObject *current = *iter;
                    if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
                    {
                        textCtrl = (svTextEditorCtrl*)current;
                        textCtrl->SetupFont();
                    }
                }
            }

            textCtrl = GetCurrentsvTextEditorCtrl();
            // textCtrl->SetupFont();
            textCtrl->DoOnSize();
            textCtrl->DoSmoothRefresh();

            g_preference.RewriteUserPreference();
        }
    }
}

void svMainFrame::OnMenuAbout(wxCommandEvent& event)
{
    svAboutDialog* aboutDlg = new svAboutDialog(NULL);
    aboutDlg->ShowModal();
    delete aboutDlg;
}

void svMainFrame::OnMenuDebug(wxCommandEvent& event)
{
    svDebugDialog01* debugDlg = new svDebugDialog01(NULL);
    debugDlg->ShowModal();
    delete debugDlg;
}

void svMainFrame::OnMenuQuit(wxCommandEvent& event)
{
    Close();
}

void svMainFrame::OnSVTextModified(svTextCtrlEvent& event)
{
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    this->notebook->SetPageText((size_t)this->notebook->GetSelection(), txtCtrl->GetDisplayName());
}

void svMainFrame::OnSVTextMSG(svTextCtrlEvent& event)
{
    // svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    // SetStatusText(txtCtrl->GetMSG(0), 0);
    // SetStatusText(txtCtrl->GetMSG(1), 1);
    // SetStatusText(txtCtrl->GetCaretLocationMSG(), 2);
}

void svMainFrame::OnAuiNotebookPageChanging(wxAuiNotebookEvent& event)
{
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    if (txtCtrl)
    {
        m_lastFindReplaceOption = txtCtrl->GetBufferFindReplaceOption();
        txtCtrl->HideTypeHint();
    }
}

void svMainFrame::OnAuiNotebookPageChanged(wxAuiNotebookEvent& event)
{
    // svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    // SetStatusText(txtCtrl->GetMSG(0), 0);
    // SetStatusText(txtCtrl->GetMSG(1), 1);
    // SetStatusText(txtCtrl->GetCaretLocationMSG(), 2);
    // txtCtrl->SetFocus();
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    svFileDesc fd = txtCtrl->GetFileDesc();
    m_openFiles->ChangeCurrentItemBG(fd);
    SetStatusText(txtCtrl->GetTextFileCharSet());


    // On notebook changed.
    // Check it's find replace option.
    // If different. research again with the old notebook's option.
    int oldID = event.GetOldSelection();
    if (oldID != wxNOT_FOUND)
    {
        svTextEditorCtrl *oldTxtCtrl = (svTextEditorCtrl *) GetTextEditorCtrl(oldID);
        // On DoMenuOpen|DoMenuNew, auiNotebook changed event may fired when svTextEditorCtrl not fully initialized. Check it out.
        if (txtCtrl->TextViewIsSet())
        {
            svFindReplaceOption nFRO = txtCtrl->GetBufferFindReplaceOption();
            if ( !(nFRO==m_lastFindReplaceOption) &&
                 m_lastFindReplaceOption.m_from != svFindReplaceOption::SVID_FIND_NOT_SET)
            {
                txtCtrl->DoFindMatchLocations(m_lastFindReplaceOption);
            }
        }
    }
}

void svMainFrame::OnChar(wxKeyEvent& event)
{
    int key = event.GetKeyCode();
    wxChar ukey = event.GetUnicodeKey();
    // int cmdName;

    char charkey = 'A' + key - 1;
    if (event.ControlDown() && charkey == 'F')  // Ctrl+F
    {
        ShowFindWindow();
    }
    else if (event.ControlDown() && charkey == 'B')  // Ctrl+B
    {
        ShowHideSideBar();
    }
}

void svMainFrame::OnAuiNotebookPageClose(wxAuiNotebookEvent& event)
{
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    svFileDesc fd;
    fd.m_fullPathName = txtCtrl->GetFullPathName();
    fd.m_displayName = txtCtrl->GetDisplayName();
    if (!txtCtrl->Close())
        event.Veto();
    else
    {
        m_openFiles->DeleteFile(fd);
    }
}

svTextEditorCtrl* svMainFrame::GetTextEditorCtrl(size_t p_id)
{
    svTextEditorCtrl* txtCtrl;

    wxWindow *w = this->notebook->GetPage(p_id);
    if (!w) return NULL;

    wxWindowList lst = w->GetChildren();
    wxWindowList::iterator iter;
    for (iter = lst.begin(); iter != lst.end(); ++iter)
    {
        wxObject *current = *iter;
        if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
        {
            txtCtrl = (svTextEditorCtrl*)current;
            return txtCtrl;
        }
    }
    return NULL;
}

svTextEditorCtrl* svMainFrame::GetCurrentsvTextEditorCtrl(void)
{
    svTextEditorCtrl* txtCtrl;

    wxWindow *w = this->notebook->GetCurrentPage();
    if (!w) return NULL;

    wxWindowList lst = w->GetChildren();
    wxWindowList::iterator iter;
    for (iter = lst.begin(); iter != lst.end(); ++iter)
    {
        wxObject *current = *iter;
        if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
        {
            txtCtrl = (svTextEditorCtrl*)current;
            return txtCtrl;
        }
    }
    return NULL;
}

void svMainFrame::SetCurrentsvTextEditorCtrlFocus(void)
{
    svTextEditorCtrl* txtCtrl;

    txtCtrl = GetCurrentsvTextEditorCtrl();
    if (txtCtrl)
        txtCtrl->SetFocus();
}

bool svMainFrame::ChangeCurrentsvTextEditorCtrl(const svFileDesc &p_fd)
{
    size_t newPage = 0;
    svTextEditorCtrl* txtCtrl;

    wxWindow *w = this->notebook->GetCurrentPage();
    if (!w) return false;

    // Won't file auinotebook changing & auinotebook changed event.
    // We do it by ourself.
    svTextEditorCtrl *curTxtCtrl = GetCurrentsvTextEditorCtrl();
    curTxtCtrl->HideTypeHint();
    m_lastFindReplaceOption = curTxtCtrl->GetBufferFindReplaceOption();

    size_t pCount = notebook->GetPageCount();
    for(int i=0; i<(int)pCount; i++)
    {
        // 結構請參考當初建立時的架構
        wxWindowList lst = notebook->GetPage(i)->GetChildren();
        wxWindowList::iterator iter;
        for (iter = lst.begin(); iter!=lst.end(); ++iter)
        {
            wxObject *current = *iter;
            if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
            {
                txtCtrl = (svTextEditorCtrl*)current;
                if (txtCtrl->GetFullPathName() == p_fd.m_fullPathName)
                {
                    // newPage = this->notebook->GetPageIndex(txtCtrl);
                    this->notebook->ChangeSelection(i);
                    txtCtrl->SetFocus();
                    // Won't file auinotebook changing & auinotebook changed event.
                    // We do it by ourself.
                    if (m_lastFindReplaceOption.m_from != svFindReplaceOption::SVID_FIND_NOT_SET)
                    {
                        txtCtrl->DoFindMatchLocations(m_lastFindReplaceOption);
                    }                    
                    return true;
                }
            }
        }
    }
    return false;
}

bool svMainFrame::IsCurrentsvTextEditorCtrl(const svFileDesc &p_fd)
{
    size_t newPage = 0;

    wxWindow *w = this->notebook->GetCurrentPage();
    if (!w) return false;

    // Won't file auinotebook changing & auinotebook changed event.
    // We do it by ourself.
    svTextEditorCtrl *curTxtCtrl = GetCurrentsvTextEditorCtrl();
    return (curTxtCtrl->GetFileDesc()==p_fd);
}

void svMainFrame::OnClose(wxCloseEvent& event)
{
    m_closeStatus.ClearBuffer();
    SetCloseStatusPosition();

    // Handle when some textctrl modified but unsaved.
    svTextEditorCtrl* txtCtrl;
    size_t cnt = this->notebook->GetPageCount();
    for (size_t i=0; i<cnt; i++){
        wxWindowList lst = this->notebook->GetPage(i)->GetChildren();
        wxWindowList::iterator iter;
        for (iter = lst.begin(); iter != lst.end(); ++iter)
        {
            wxObject *current = *iter;
            if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
            {
                txtCtrl = (svTextEditorCtrl*)current;
                if (!txtCtrl->Close())
                {
                    event.Veto();
                    return;
                }
            }
        }
    }

    WriteAppCloseStatus();

    event.Skip();
}

void svMainFrame::OnSashDoubleClick(wxSplitterEvent& event)
{
    // Sash Double Click 時，關閉左方 panel
    if (panel111->IsShown())
    {
        splitterWin11->Unsplit(panel111);
    }
}

void svMainFrame::ShowHideFindWindow(void)
{
    if (!(this->notebook->GetPageCount()>0)) return;

    if (vSizer11212->IsShown(m_findCtrl))
    {
        vSizer11212->Hide(m_findCtrl);
        SetCurrentsvTextEditorCtrlFocus();
    }
    else
    {
        vSizer11212->Show(m_findCtrl);
        m_findCtrl->ResetFocus();
    }

    vSizer1121->Layout();
}

void svMainFrame::ShowFindWindow(void)
{
    if (!(this->notebook->GetPageCount()>0)) return;

    if (vSizer11212->IsShown(m_findCtrl))
    {
        m_findCtrl->ResetFocus();
    }
    else
    {
        vSizer11212->Show(m_findCtrl);
        m_findCtrl->ResetFocus();
    }

    vSizer1121->Layout();
}

void svMainFrame::HideFindWindow(void)
{
    if (!(this->notebook->GetPageCount()>0)) return;

    if (vSizer11212->IsShown(m_findCtrl))
    {
        vSizer11212->Hide(m_findCtrl);
        SetCurrentsvTextEditorCtrlFocus();
    }


    vSizer1121->Layout();
}

bool svMainFrame::FindWindowIsVisible(void)
{
    if (!(this->notebook->GetPageCount()>0)) return false; // why?

    return vSizer11212->IsShown(m_findCtrl);
}


void svMainFrame::ShowHideSideBar(int p_width)
{
    if (panel111->IsShown())
    {
        splitterWin11->Unsplit(panel111);
        SetCurrentsvTextEditorCtrlFocus();
    }
    else
    {
        splitterWin11->SplitVertically( panel111, panel112, p_width );
    }
}

/* --------------------------------------------------------------------------
 * 
 * Functions for recording awvic closing status.
 *
 * -------------------------------------------------------------------------- */

// Read Awvic last time close status.
bool svMainFrame::ReadAppCloseStatus(void)
{

    extern svPreference g_preference;
    wxString p_filename = g_preference.GetUserConfigPath() + g_preference.GetCloseMemo();

    std::ifstream f(p_filename.mb_str());
    std::string config_doc;

    try
    {
        if (!f.fail())
        {
            f.seekg(0, std::ios::end);
            int size = f.tellg();
            config_doc.reserve(size);
            f.seekg(0, std::ios::beg);

            config_doc.assign((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
            }
        else
        {
            wxString ErrMsg = wxString::Format(wxT("svMainFrame::ReadAppCloseStatus Error - Exception opening file:" + p_filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    catch (std::exception& e)
    {
        wxString ErrMsg = wxString::Format(wxT("svMainFrame::ReadAppCloseStatus - Exception opening/reading file:" + p_filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        wxString ErrMsg = wxString::Format(wxT("svMainFrame::ReadAppCloseStatus - Parsing Json error01: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }

    // Get the value of the member of root named 'encoding', return 'UTF-8' if there is no such member.
    std::string encoding = root.get("encoding", "UTF-8" ).asString();

    m_closeStatus.m_appPosition.X = root["frame"].get("X", 0).asInt();
    m_closeStatus.m_appPosition.Y = root["frame"].get("Y", 0).asInt();
    m_closeStatus.m_appPosition.Height = root["frame"].get("Height", 0).asInt();
    m_closeStatus.m_appPosition.Width = root["frame"].get("Width", 0).asInt();

    m_closeStatus.m_openFiles = root["frame"].get("open_files", false).asBool();
    m_closeStatus.m_openFilesWidth = root["frame"].get("open_files_width", 0).asInt();
    m_closeStatus.m_findCtrl = root["frame"].get("find_ctrl", false).asBool();

    /* for buffers : opened files */
    m_closeStatus.ClearBuffer();

    const Json::Value buffers = root["buffers"];
    for ( unsigned int index = 0; index < buffers.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value buffer_val  = buffers[index];

        svFileDescOpened fdo;
        try
        {
            fdo.m_fullPathName = wxString(buffer_val["full_path"].asString().c_str(), wxConvUTF8);
            fdo.m_displayName = wxString(buffer_val["display_name"].asString().c_str(), wxConvUTF8);
            fdo.m_firstLineNo = buffer_val.get("first_line", 0).asInt();

            const Json::Value carets = buffer_val["carets"];
            for ( unsigned int index2 = 0; index2 < carets.size(); ++index2 )  // Iterates over the sequence elements.
            {
                const Json::Value caret  = carets[index2];

                
                try
                {
                    int l_row = caret.get("row", 0).asInt();
                    int l_visual_col = caret.get("visual_col", 0).asInt();
                    int l_keep_space = caret.get("keep_space", 0).asInt();
                    int l_select_row = caret.get("select_row", 0).asInt();
                    int l_select_col = caret.get("select_col", 0).asInt();
                    int l_select_keep_space = caret.get("select_keep_space", 0).asInt();

                    svCaret c(l_row, l_visual_col, l_keep_space, l_select_row, l_select_col, l_select_keep_space);

                    fdo.AddCaret(c);

                }
                catch(const std::exception &e )
                {
                    wxString ErrMsg = wxString::Format(wxT("svMainFrame::ReadAppCloseStatus - Parsing Json error02:" + wxString(e.what())));
                    wxLogMessage(ErrMsg);
                }

            }

            m_closeStatus.AddBuffer(fdo);

        }
        catch(const std::exception &e )
        {
            wxString ErrMsg = wxString::Format(wxT("svMainFrame::ReadAppCloseStatus - Parsing Json error03:" + wxString(e.what())));
            wxLogMessage(ErrMsg);
        }

    }

    m_closeStatus.m_activeBuffer = root.get("buffer_active", 0).asInt();

    // Reset frame size and position.
    svRect r = m_closeStatus.GetPosition();

    this->SetPosition(wxPoint(r.X, r.Y));
    this->SetSize(r.Width, r.Height);


    if (m_loadLastOpenedFiles)
    {
        for(std::vector<svFileDescOpened>::iterator it=m_closeStatus.m_buffers.begin();
            it!=m_closeStatus.m_buffers.end();
            ++it)
        {
            DoOnMenuOpen(it->m_fullPathName, &*it);
        }
    }

    if (m_closeStatus.m_openFiles)
    {
        ShowHideSideBar(m_closeStatus.m_openFilesWidth);
    }

    if (m_closeStatus.m_findCtrl)
    {
        ShowFindWindow();
    }

    if (notebook->GetPageCount()>0 && m_loadLastOpenedFiles)
    {
        notebook->SetSelection(m_closeStatus.m_activeBuffer);
        svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
        txtCtrl->SetFocus();
    }

    return true;
}


// Write Awvic status before closing.
bool svMainFrame::WriteAppCloseStatus(void)
{
    if (!m_loadLastOpenedFiles)  // If we do not read last opened files, we do not write it back.
        return false;

    // Write data to file.

    extern svPreference g_preference;
    wxString p_filename = g_preference.GetUserConfigPath() + g_preference.GetCloseMemo();

    Json::Value root;

    // HAHA! I cut & paste the floowing comment from jsoncpp docs.
    // ...
    // At application shutdown to make the new configuration document:
    // Since Json::Value has implicit constructor for all value types, it is not
    // necessary to explicitly construct the Json::Value object:

    try
    {
        root["frame"]["X"] = m_closeStatus.GetPosition().X;
        root["frame"]["Y"] = m_closeStatus.GetPosition().Y;
        root["frame"]["Height"] = m_closeStatus.GetPosition().Height;
        root["frame"]["Width"] = m_closeStatus.GetPosition().Width;

        root["frame"]["open_files"] = m_closeStatus.m_openFiles;
        root["frame"]["open_files_width"] = m_closeStatus.m_openFilesWidth;
        root["frame"]["find_ctrl"] = m_closeStatus.m_findCtrl;

        root["encoding"] = std::string(_("UTF-8").mb_str());

        Json::Value buf(Json::arrayValue);

        for(std::vector<svFileDescOpened>::iterator it=m_closeStatus.m_buffers.begin();
            it!=m_closeStatus.m_buffers.end();
            ++it)
        {
            Json::Value fDesc;

            fDesc["full_path"] = std::string(it->m_fullPathName.mb_str());
            fDesc["display_name"] = std::string(it->m_displayName.mb_str());

            Json::Value carets(Json::arrayValue);

            for(std::vector<svCaret>::iterator it2=it->m_carets.begin();
                it2!=it->m_carets.end();
                ++it2)
            {
                Json::Value c;
                c["row"] = it2->GetRow();
                c["visual_col"] = it2->GetVisualCol();
                c["keep_space"] = it2->GetKeepSpace();
                c["select_row"] = it2->GetSelectRow();
                c["select_col"] = it2->GetSelectCol();
                c["select_keep_space"] = it2->GetSelectKeepSpace();
                carets.append(c);
            }

            fDesc["carets"] = carets;
            fDesc["first_line"] = it->m_firstLineNo;

            buf.append(fDesc);
        }

        root["buffers"] = buf;

        root["buffer_active"] = m_closeStatus.m_activeBuffer;

        Json::StyledWriter writer;
        std::string outputConfig = writer.write(root);

        std::ofstream f(p_filename.mb_str());
        f << outputConfig;
        f.close();

    }
    catch(const std::exception &e )
    {
        wxString ErrMsg = wxString::Format(wxT("svMainFrame::WriteAppCloseStatus error:" + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    return true;
}

void svMainFrame::SetCloseStatusPosition(void)
{
    svRect r;

    this->GetSize(&r.Width, &r.Height);
    this->GetPosition(&r.X, &r.Y);

    m_closeStatus.SetPosition(r);

    m_closeStatus.m_activeBuffer = notebook->GetSelection();

    if (panel111->IsShown())
    {
        m_closeStatus.m_openFiles = true;
        int w, h;
        w=h=0;
        m_openFiles->GetSize(&w, &h);
        m_closeStatus.m_openFilesWidth = w;
    }
    else
    {
        m_closeStatus.m_openFiles = false;
        m_closeStatus.m_openFilesWidth = 0;
    }

    if (vSizer11212->IsShown(m_findCtrl))
    {
        m_closeStatus.m_findCtrl = true;
    }
    else
        m_closeStatus.m_findCtrl = false;

}

void svMainFrame::AddCloseStatusBuffer(const svFileDescOpened &m_fileDescOpened)
{
    m_closeStatus.AddBuffer(m_fileDescOpened);
}

// When auiNotebook child get foucus.
// Check if the opened files been update by other program. << Not implement now. Check it on svMainFrame OnActivate.
/*void svMainFrame::OnAuiNotebookChildGetFocus(wxChildFocusEvent& event)
{
#ifndef NDEBUG
    svTextEditorCtrl* txtCtrl=GetCurrentsvTextEditorCtrl();
    wxLogMessage(wxString::Format("svMainFrame::OnAuiNotebookChildGetFocus cur=%s"+txtCtrl->GetFileDesc().m_fullPathName));
#endif
    event.Skip();
}*/

/*void svMainFrame::OnSetFocus(wxFocusEvent& event)
{
    // Doesn't work! Why?
#ifndef NDEBUG
    wxLogMessage("svMainFrame::OnSetFocus(wxFocusEvent& event)");
#endif
    event.Skip();
}
*/

// When svMainFrame activate.
// Check if any opened files been update by other program.
void svMainFrame::OnActivate(wxActivateEvent& event)
{
    // Avoid recursive trigger OnActive event.
    bool res = this->Disconnect( wxID_ANY, wxEVT_ACTIVATE, wxActivateEventHandler(svMainFrame::OnActivate) );

    if (event.GetActive())
    {
#ifndef NDEBUG
        wxLogMessage(wxString::Format("svMainFrame::OnActivate"));
#endif
        CheckTextEditorBufferModificated();
    }
    event.Skip();
    this->Connect( wxID_ANY, wxEVT_ACTIVATE, wxActivateEventHandler(svMainFrame::OnActivate) );
}

void svMainFrame::CheckTextEditorBufferModificated(void)
{
    size_t newPage = 0;
    svTextEditorCtrl* txtCtrl;

    //wxWindow *w = this->notebook->GetCurrentPage();
    //if (!w) return false;

    size_t pCount = notebook->GetPageCount();
    for(int i=0; i<(int)pCount; i++)
    {
        // 結構請參考當初建立時的架構
        wxWindowList lst = notebook->GetPage(i)->GetChildren();
        wxWindowList::iterator iter;
        for (iter = lst.begin(); iter!=lst.end(); ++iter)
        {
            wxObject *current = *iter;
            if (current->IsKindOf(CLASSINFO(svTextEditorCtrl)))
            {
                txtCtrl = (svTextEditorCtrl*)current;
                txtCtrl->Reset();
            }
        }
    }
}