/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVPREFERENCE_H
#define _SVPREFERENCE_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#pragma warning(disable: 4996)
#endif

#include <vector>
using namespace std;

/*
 * This struct is for filename to syntax look up rule.
 * 檔案名與對應的語法的regular expression規則
 * awvic 不只用副檔名來判斷該用哪種syntax hilight
 */
typedef struct syntax_fn_rule{

    char *fname;
    char *syntax;
    int type;

    syntax_fn_rule()
    {
        fname = NULL;
        syntax = NULL;
        type = 0;
    }
    syntax_fn_rule(const syntax_fn_rule& obj)  // copy constructor
    {
        if (obj.fname)
            fname = strdup(obj.fname);
        else 
            fname = NULL;
        if (obj.syntax)
            syntax = strdup(obj.syntax);
        else
            syntax = NULL;
        type = obj.type;
    }
    syntax_fn_rule& operator=(const syntax_fn_rule& obj)
    {
        if (fname) free(fname);
        if (obj.fname)
            fname = strdup(obj.fname);
        else
            fname = NULL;
        if (syntax) free(syntax);
        if (obj.syntax)
            syntax = strdup(obj.syntax);
        else
            syntax = NULL;
        type = obj.type;
        return *this;
    }
    ~syntax_fn_rule()
    {
        if (fname) free(fname);
        if (syntax) free(syntax);
    }    
} syntax_fn_rule;

class svPreference
{
private:
    vector<syntax_fn_rule> m_fnRules;
    
    int m_tabSize;                    // How many spaces presents 1 tab.

    bool m_tabToSpace;                // insert space instead tab

    wxString m_appPath;               // Awvic application running path.
    wxString m_configPath;            // Awvic user config path.
    wxString m_syntaxPath;            // Awvic syntax path.
    wxString m_themePath;             // Awvic theme path.

    wxString m_themeName;             // Awvic theme name.

    wxString m_closeMemoFileName;     // File name for closing memo.

    bool m_alignPrevLine;
    // If m_alignPrevLine is true
    //    newline will automatic insert spaces or tabs to make carets move to the position as previous line first non space character's position.
    //    if carets column position is zero and tab is hit, awvic will automatic insert spaces or tabs to make carets move to the position as previous line first non space character's position.
    // If m_alignPrevLine is false
    //    not any special process descripted aboved will happen.


    int m_fontSize;
    wxString m_fontFace;
    
    bool m_wrap;
    int m_wrapColumn;


public:
    svPreference();
    ~svPreference();
    bool LoadGlobalPreference(void);
    bool LoadUserPreference(void);
    bool RewriteUserPreference(void);
    char *GetSyntaxFNRule(const char *p_filename);
    int GetTabSize(void);
    inline
    void SetAppPath(const wxString &p_path)
    {
        m_appPath = p_path;

#ifdef __WXMSW__
        if (!m_appPath.EndsWith("\\"))
            m_appPath += "\\";
#else
        if (!m_appPath.EndsWith("/"))
            m_appPath += "/";
#endif

        m_syntaxPath = m_appPath + "syntaxs";
        m_themePath = m_appPath + "themes";

#ifdef __WXMSW__
        if (!m_syntaxPath.EndsWith("\\"))
            m_syntaxPath += "\\";
        if (!m_themePath.EndsWith("\\"))
            m_themePath += "\\";
#else
        if (!m_syntaxPath.EndsWith("/"))
            m_syntaxPath += "/";
        if (!m_themePath.EndsWith("/"))
            m_themePath += "/";
#endif

    }
    inline
    wxString GetAppPath(void)
    {
        return m_appPath;
    }
    inline
    wxString GetSyntaxPath(void)
    {
        return m_syntaxPath;
    }
    inline
    wxString GetThemePath(void)
    {
        return m_themePath;
    }
    inline
    wxString GetThemeFilePath(void)
    {
        return m_themePath + m_themeName + _(".svthm");
    }
    inline
    void SetUserConfigPath(const wxString &p_path)
    {
        m_configPath = p_path;
#ifdef __WXMSW__
        if (!m_configPath.EndsWith("\\"))
            m_configPath += "\\";
#else
        if (!m_configPath.EndsWith("/"))
            m_configPath += "/";
#endif
    }
    inline
    wxString GetUserConfigPath(void)
    {
        return m_configPath;
    }
    inline
    void SetCloseMemo(const wxString &p_file)
    {
        m_closeMemoFileName = p_file;
    }
    inline
    wxString GetCloseMemo(void)
    {
        return m_closeMemoFileName;
    }
    inline
    bool GetTabToSpace(void)
    {
        return m_tabToSpace;
    }
    inline
    void SetTabToSpace(bool p_tabToSpace)
    {
        m_tabToSpace = p_tabToSpace;
    }
    wxString TabToSpaceTranslate(int p_col);
    inline
    bool GetAlignPrevLine(void)
    {
        return m_alignPrevLine;
    }
    inline
    void SetAlignPrevLine(bool p_alignPrevLine)
    {
        m_alignPrevLine = p_alignPrevLine;
    }
    inline
    int GetFontSize(void)
    {
        return m_fontSize;
    }
    inline
    wxString GetFontFace(void)
    {
        return m_fontFace;
    }
    inline
    void SetFontSize(int p_size)
    {
        m_fontSize = p_size;
    }
    inline
    void SetFontFace(const wxString p_face)
    {
        m_fontFace = p_face;
    }
    inline
    wxString GetGlobalPreferenceFileName(void)
    {
        return _("awvic.ini");
    }
    inline
    wxString GetUserPreferenceFileName(void)
    {
        return _("user.ini");
    }
    inline
    bool GetIsWrap(void)
    {
        return m_wrap;
    }
    inline
    int GetWrapColumn(void)
    {
        return m_wrapColumn;
    }
};

#endif
