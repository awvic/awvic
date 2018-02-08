/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVMAINFRAME_H
#define _SVMAINFRAME_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/aui/auibook.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/strconv.h>
#include <wx/encconv.h>
#include <wx/textfile.h>
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#include <wx/fontmap.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/splitter.h>

#include "svTextEditorCtrl.h"
#include "svAboutDialog.h"
#ifdef __WXMSW__
//#include "./svAuiTabArt.h"
#endif
#include "svDebugDialog01.h"
#include "svFindReplaceCtrl.h"
#include "svOpenFilesCtrl.h"
#include "svTypeHintCtrl.h"

#include <json/json.h>

//#define BUF_SIZE 256

using namespace std;

enum
{
    SVID_NOTEBOOK=wxID_HIGHEST+1,
    SVID_FINDCTRL
};


class svMainFrame : public wxFrame
{
public:
    svMainFrame(const wxString& title, const wxString& openfiles, bool p_loadLastOpenedFiles=true);
    ~svMainFrame();
    
    void OnMenuNew(wxCommandEvent& event);
    void OnMenuOpen(wxCommandEvent& event);
    void DoOnMenuOpen(const wxString& path, svFileDescOpened *p_fdo=NULL);
    void OnMenuSave(wxCommandEvent& event);
    void DoOnMenuSave(void);
    void OnMenuSaveAs(wxCommandEvent& event);
    void OnMenuClose(wxCommandEvent& event);
    void OnMenuFind(wxCommandEvent& event);
    void OnMenuSideBar(wxCommandEvent& event);
    void OnMenuSelectFont(wxCommandEvent& WXUNUSED(event));
    void OnMenuAbout(wxCommandEvent& event);
    void OnMenuDebug(wxCommandEvent& event);
    void OnMenuQuit(wxCommandEvent& event);
    
    //void OnSetFocus(wxFocusEvent& event);
    void OnActivate(wxActivateEvent& event);
    void CheckTextEditorBufferModificated(void);

    void OnSVTextModified(svTextCtrlEvent& event);
    void OnSVTextMSG(svTextCtrlEvent& event);
    void OnAuiNotebookPageClose(wxAuiNotebookEvent& event);
    void OnAuiNotebookPageChanging(wxAuiNotebookEvent& event);
    void OnAuiNotebookPageChanged(wxAuiNotebookEvent& event);
    //void OnAuiNotebookChildGetFocus(wxChildFocusEvent& event);

    svTextEditorCtrl* GetTextEditorCtrl(size_t p_id);
    svTextEditorCtrl* GetCurrentsvTextEditorCtrl(void);
    void SetCurrentsvTextEditorCtrlFocus(void);
    bool ChangeCurrentsvTextEditorCtrl(const svFileDesc &p_fd);
    bool IsCurrentsvTextEditorCtrl(const svFileDesc &p_fd);
    inline
    void OpenFilesCtrlChangeCurrentItemBG(const svFileDesc &p_fd)
    {
        if (m_openFiles)
            m_openFiles->ChangeCurrentItemBG(p_fd);    
    }

    void OnClose(wxCloseEvent& event);

    // void OnNotebookChar(wxKeyEvent& event);
    // void OnNotebookKeyDown(wxKeyEvent& event);

    void OnSashDoubleClick(wxSplitterEvent& event);
    void OnChar(wxKeyEvent& event);
    // void OnKeyDown(wxKeyEvent& event);
    // void OnSetFocus(wxFocusEvent& event);


    void ShowHideFindWindow(void);
    void ShowFindWindow(void);
    void HideFindWindow(void);
    bool FindWindowIsVisible(void);
    void ShowHideSideBar(int p_width=200);

    bool ReadAppCloseStatus(void);
    bool WriteAppCloseStatus(void);
    void SetCloseStatusPosition(void);
    void AddCloseStatusBuffer(const svFileDescOpened &m_fileDescOpened);

/*    inline
    void ShowTypeHint(vector<wxString> &p_hintList)
    {
        m_typeHintCtrl->SetHints(p_hintList);
        m_typeHintCtrl->Show();
    }
    inline
    void HideTypeHint(void)
    {
        m_typeHintCtrl->Hide();
    }*/


private:
    DECLARE_EVENT_TABLE()

private:
    wxAuiNotebook *notebook;
    // wxFont *settedFont;
    wxIcon *icn;
    wxAuiManager m_mgr;
    wxString m_defDir;
    wxPNGHandler *pngHandler;

    wxPanel *panel111;
    wxPanel *panel112;
    wxSplitterWindow *splitterWin11;
    wxTextCtrl *txtOpenFiles;

    wxBoxSizer* vSizer1121;
    wxBoxSizer* vSizer11212;
    svFindReplaceCtrl *m_findCtrl;
    svOpenFilesCtrl *m_openFiles;
    // svTypeHintCtrl *m_typeHintCtrl;

    svSeqCodeGenerator m_seqCodeGen;
    svAppCloseStatus m_closeStatus;
    bool m_loadLastOpenedFiles;

    svFindReplaceOption m_lastFindReplaceOption;

#ifdef __WXMSW__
    //omAuiTabArt* m_art;
#endif
    wxAuiTabArt *m_art;
    // wxAuiSimpleTabArt m_art;
    // svFileNameRule m_fnRules;
};

#endif
