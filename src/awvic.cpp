/* 
    
Copyright (c) 2015, Chih Chen Fang
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL CHIH CHEN FANG BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   
*/

/* 
 * ----------------------------------------------------------------------------------------*
 *
 *          AWVIC is a text editor created by Chih Chen Fang.
 *
 * ----------------------------------------------------------------------------------------*
 *
 * Welcome to my ugly code!
 * 
 * 1. Don't remove any comments in the language you cann't read.
 * 2. The only thing you can learn from these code probably is to avoiding the way it did.
 *
 *
 */
#include "wx/wx.h"
#include "wx/file.h"
#include "wx/cmdline.h"
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "./svMainFrame.h"
#include "stdwx.h"
#include "svCommonLib.h"
#include "svPreference.h"
#include <wx/snglinst.h>

svPreference g_preference;

class AWorseVIClone : public wxApp
{
public:
    ~AWorseVIClone();
    virtual bool OnInit();
    virtual int OnExit();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

private:
    wxLocale* m_locale;
    FILE* m_pLogFile;
    wxSingleInstanceChecker *m_checker;
};

DECLARE_APP(AWorseVIClone)

IMPLEMENT_APP(AWorseVIClone)

bool AWorseVIClone::OnInit()
{

    wxString apppath;
    apppath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fp(apppath);
    wxString path = fp.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

    g_preference.SetAppPath(path);
    wxApp::SetAppName("Awvic");
    wxApp::SetAppDisplayName("Awvic");

    // path = wxStandardPaths::Get().GetExecutablePath();
    path = wxStandardPaths::Get().GetUserConfigDir();

#ifdef __WXMSW__
#ifndef NDEBUG
    path += "\\awvic_debug";   // Windows files (debug)
#else
    path += "\\awvic";   // Windows files
#endif
#else
    path += "/.awvic";   // Unix files
#endif

    if (!wxDirExists(path))
    {
        if (wxFileExists(path))
        {
            wxRemoveFile(path);
        }
        wxMkdir(path);
    }

    g_preference.SetUserConfigPath(path);

    path = g_preference.GetSyntaxPath();
    if (!wxDirExists(path))
    {
        if (wxFileExists(path))
        {
            wxRemoveFile(path);
        }
        wxMkdir(path);
    }    

    path = g_preference.GetThemePath();
    if (!wxDirExists(path))
    {
        if (wxFileExists(path))
        {
            wxRemoveFile(path);
        }
        wxMkdir(path);
    }    

    m_locale = NULL;
    m_locale = new wxLocale(wxLANGUAGE_DEFAULT);
    // m_locale = new wxLocale(wxLANGUAGE_ENGLISH);
    //m_locale->AddCatalogLookupPathPrefix(wxT("./locale/"));
    m_locale->AddCatalogLookupPathPrefix(wxT("../locale/"));
    m_locale->AddCatalog(wxT("AWVIC"));

    // open log file.
    m_pLogFile = fopen( g_preference.GetUserConfigPath() + "log.txt", "w+" );

    delete wxLog::SetActiveTarget(new wxLogStderr(m_pLogFile));
    wxLogMessage(wxT("Log Start"));
    wxLogMessage(g_preference.GetUserConfigPath());
    wxLogMessage(apppath);

    // Parameters, awvic will open it initially.
    wxString files(wxT(""));
    wxString fn(wxT(""));
    for(int i=1;i<argc;i++)
    {
        fn = argv[i];
        wxLogMessage(fn);
        if (wxFile::Exists(fn))
        {
            wxFileName filename(fn);
            filename.MakeAbsolute();
            files += filename.GetFullPath() + wxT('|');      
        }    
    }

    // Loading configuration preference
    // g_preference.LoadGlobalPreference(svCommonLib::wxStrFilename2Char(g_preference.GetAppPath() + g_preference.GetGlobalPreferenceFileName()));
    // g_preference.LoadUserPreference(svCommonLib::wxStrFilename2Char(g_preference.GetConfigPath() + g_preference.GetUserPreferenceFileName()));
    g_preference.LoadGlobalPreference();
    g_preference.LoadUserPreference();


    bool loadLastOpenedFiles = true;
    m_checker = new wxSingleInstanceChecker;
    if (m_checker->IsAnotherRunning())
    {
        loadLastOpenedFiles = false;
        delete m_checker;
        m_checker = NULL;
    }

    svMainFrame *frame = new svMainFrame(wxT("A Worse VI Clone Editor 0.01"), files, loadLastOpenedFiles);

    frame->Show(true);

    return true;
}

int AWorseVIClone::OnExit()
{
    delete m_checker;
    return 0;
}

// Doesn't works.
void AWorseVIClone::OnInitCmdLine(wxCmdLineParser& parser)
{
    wxLogMessage(wxT("OnInitCmdLine"));
}

// Doesn't works.
bool AWorseVIClone::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxLogMessage(wxT("OnCmdLineParsed"));

    wxArrayString files;
    wxString fn(wxT(""));
    for (unsigned int i = 0; i < parser.GetParamCount(); i++)
    {
        fn = parser.GetParam(i);
        wxLogMessage(fn);
        if (wxFile::Exists(fn))
        {
            files.Add(fn);
            wxLogMessage(fn);
        }
    }

    return true;
}

AWorseVIClone::~AWorseVIClone()
{
    delete m_locale;

    // close log file.
    delete wxLog::SetActiveTarget(NULL);
    if (m_pLogFile != NULL)
    {
        fclose(m_pLogFile);
    }

}

