/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include <wx/file.h>
#include "svPreference.h"

#include "stdwx.h"

#include <json/json.h>
#include <iostream>
#include <fstream>

#include "svCommonLib.h"


svPreference::svPreference()
{
    m_tabSize = 4;
    m_tabToSpace = false;
    m_alignPrevLine = false;

    m_fontSize = 11;
    m_fontFace = _("Monospace");
    
    m_wrap = true;
    m_wrapColumn = 0; // 0 means automatic. Decided by the width of DC width.
}

svPreference::~svPreference()
{
    m_fnRules.clear();
}

bool svPreference::LoadGlobalPreference(void)
{
    // For error logging.
    wxString filename = GetAppPath() + GetGlobalPreferenceFileName();

    std::ifstream f(filename.mb_str());
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
            wxString ErrMsg = wxString::Format(wxT("svPreference::LoadGlobalPreference Error - Exception opening file:" + filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    catch (std::exception& e)
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadGlobalPreference - Exception opening/reading file:" + filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadGlobalPreference - Failed to parse configuration: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }

    // Get the value of the member of root named 'encoding', return 'UTF-8' if there is no such member.
    std::string encoding = root.get("encoding", "UTF-8").asString();

    m_tabSize = root.get("tab_size", 4).asInt();
    // const Json::Value closeMemo = root["close_memo"];
    // m_closeMemoFileName = wxString(closeMemo.asString().c_str(), wxConvUTF8);
    m_closeMemoFileName = wxString(root.get("close_memo", "awvic.svmemo").asString().c_str(), wxConvUTF8);
    m_tabToSpace = root.get("tab_to_space", false).asBool();
    m_alignPrevLine = root.get("align_prev_line", false).asBool();
    m_fontSize = root.get("font_size", 11).asInt();
    m_fontFace = wxString(root.get("font_face", "Monospace").asString().c_str(), wxConvUTF8);
    m_wrap = root.get("wrap", false).asBool();
    m_wrapColumn = root.get("wrap_column", 0).asInt();
    m_themeName = wxString(root.get("theme", "simple").asString().c_str(), wxConvUTF8);

    const Json::Value rules = root["syntax_filename_rule"];
    for ( unsigned int index = 0; index < rules.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value rule_val  = rules[index];

        char *syntax = NULL;
        syntax = strdup(rule_val["syntax"].asString().c_str());

        const Json::Value descs = rule_val["desc"];
        for ( unsigned int j = 0; j < descs.size(); ++j )  // Iterates over the sequence elements.
        {
            const Json::Value desc_val  = descs[j];

            syntax_fn_rule rd;
            // syntax_fn_rule has a constructor to assign NULL to char pointer.

            try
            {
                rd.fname = strdup(desc_val["comment"].asString().c_str());
                rd.syntax = strdup(syntax);
                rd.type =  desc_val["type"].asInt();
                m_fnRules.push_back(rd);
            }
            catch(const std::exception &e )
            {
                wxString ErrMsg = wxString::Format(wxT("svPreference::LoadGlobalPreference - Parsing Json error:" + wxString(e.what())));
                wxLogMessage(ErrMsg);
            }
        }

        if (syntax) free(syntax);

    }

    return true;

}


// Read overwritable user preference from user.ini
bool svPreference::LoadUserPreference(void)
{
    // For error logging.
    wxString filename = GetUserConfigPath() + GetUserPreferenceFileName();

    // If file not exist, create a blank json file.
    if (!wxFile::Exists(filename))
    {
        std::ofstream f(filename.mb_str());
        f << "{\n}";
        f.close();   
        return false; 
    }

    std::ifstream f(filename.mb_str());
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
            wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference Error - Exception opening file:" + filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    catch (std::exception& e)
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference - Exception opening/reading file:" + filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference - Failed to parse configuration: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }

    // Get the value of the member of root named 'encoding', return 'UTF-8' if there is no such member.
    std::string encoding = root.get("encoding", "UTF-8").asString();

    m_tabSize = root.get("tab_size", m_tabSize).asInt();
    m_tabToSpace = root.get("tab_to_space", m_tabToSpace).asBool();
    m_alignPrevLine = root.get("align_prev_line", m_alignPrevLine).asBool();
    m_fontSize = root.get("font_size", m_fontSize).asInt();
    m_fontFace = wxString(root.get("font_face", std::string(m_fontFace.mb_str())).asString().c_str(), wxConvUTF8);
    m_wrap = root.get("wrap", m_wrap).asBool();
    m_wrapColumn = root.get("wrap_column", m_wrapColumn).asInt();

    return true;

}

// overwrite user preference to specified filename.
// only overwrite font_size and font_face.
// 將 user preference 回寫檔案
// 目前只會強制將 g_preference 的 font_size font_face 蓋過原檔案
// 只供 svMainFrame::OnMenuSetupFont 呼叫
bool svPreference::RewriteUserPreference(void)
{
    // Write user preference file.
    wxString filename = GetUserConfigPath() + GetUserPreferenceFileName();

    std::ifstream f(filename.mb_str());
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
            wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference Error - Exception opening file:" + filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    catch (std::exception& e)
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference - Exception opening/reading file:" + filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::LoadUserPreference - Failed to parse configuration: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }
    
    try
    {
        root["font_size"] = m_fontSize;
        root["font_face"] = std::string(m_fontFace.mb_str());

        Json::StyledWriter writer;
        std::string outputConfig = writer.write(root);

        std::ofstream f(filename.mb_str());
        f << outputConfig;
        f.close();
    }
    catch(const std::exception &e )
    {
        wxString ErrMsg = wxString::Format(wxT("svPreference::RewriteUserPreference error:" + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    return true;
}


char *svPreference::GetSyntaxFNRule(const char *p_filename)
{
    const char *ext = NULL;
    ext = strrchr(p_filename, '.');
    char *syn = NULL;

    if (m_fnRules.size()==0)
    {
        wxLogMessage(wxT("svPreference::GetSyntaxFNRule error: m_fnRules NULL!"));
    }

    for (std::vector<syntax_fn_rule>::iterator it=m_fnRules.begin();
         it != m_fnRules.end();
         ++it)
    {
        // if (p_filename.EndsWith(wxString.FromAscii(it->fname)))
        if (ext && !strcmp(ext+1, it->fname))
        {
            syn = strdup(it->syntax);
            return syn;
        }
    }

    // If nothing found, return text as default syntax.
    const char* text= "text";
    syn = (char*) malloc(sizeof(char)*(strlen(text)+1));
    strcpy(syn, text);
    return syn;
}

int svPreference::GetTabSize(void)
{
    return m_tabSize;
}

wxString svPreference::TabToSpaceTranslate(int p_col)
{
    if (!m_tabSize) return wxString("");

    int mod = m_tabSize - (p_col % m_tabSize);
    char blank = 32;
    return wxString(wxUniChar(blank), mod);

}

