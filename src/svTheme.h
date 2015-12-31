/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVTHEME_H
#define _SVTHEME_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <wx/colour.h>
#include <vector>
//#include <wx/textfile.h>
using namespace std;

class svRGB
{
private:
    unsigned char m_red;
    unsigned char m_green;
    unsigned char m_blue;
public:
    svRGB();
    svRGB(unsigned char p_r, unsigned char p_g, unsigned char p_b);
    svRGB(const svRGB& p_rgb);
    void SetRGB(unsigned char p_r, unsigned char p_g, unsigned char p_b);
    inline
    unsigned char GetR() const
    {
        return m_red;
    }
    inline
    unsigned char GetG() const
    {
        return m_green;
    }
    inline
    unsigned char GetB() const
    {
        return m_blue;
    }
    // inline
    // svRGB operator+(unsigned char p_offset)
    // {
    //     svRGB n;
    //     n.SetRGB(m_red+p_offset, m_green+p_offset, m_blue+p_offset);
    //     return n;
    // }
};

class svThemeClass
{
private:
    string m_class;
    svRGB m_fg;
    svRGB m_bg;
public:
    svThemeClass(const string& p_class);
    svThemeClass(const string& p_class, const svRGB& p_fg, const svRGB& p_bg);
    svThemeClass(const svThemeClass& p_themeClass);
    inline
    void SetClass(const string& p_class)
    {
         m_class = p_class;
    }
    inline
    string GetClass(void) const
    {
        return m_class;
    }
    inline
    void SetFG(const svRGB& p_fg)
    {
        m_fg.SetRGB(p_fg.GetR(), p_fg.GetG(), p_fg.GetB());
    }
    inline
    void SetBG(const svRGB& p_bg)
    {
        m_bg.SetRGB(p_bg.GetR(), p_bg.GetG(), p_bg.GetB());
    }
    inline
    svRGB GetFG() const
    {
        return m_fg;
    }
    inline
    svRGB GetBG() const
    {
        return m_bg;
    }
    inline
    wxColour GetFGColour() const
    {
        wxColour c = wxColour(m_fg.GetR(), m_fg.GetG(), m_fg.GetB());
        return c;
    }
    inline
    wxColour GetBGColour() const
    {
        wxColour c = wxColour(m_bg.GetR(), m_bg.GetG(), m_bg.GetB());
        return c;
    }
};


// class for svTheme.
// The class format for color theme.
class svTheme
{
private:
    string m_name;
    wxString m_filename;
    vector<svThemeClass> m_listofClass;

public:
    svTheme();
    svTheme(const string& p_name);
    ~svTheme();
    inline
    void SetName(const string& p_name)
    {
        m_name = p_name;
    }
    inline
    string GetName(void)
    {
        return m_name;
    }

    bool LoadFile(const wxString& p_filename);

    svThemeClass* GetClass(const string& p_class);
    inline
    void Clear()
    {
        m_listofClass.clear();
    }
};

#endif
