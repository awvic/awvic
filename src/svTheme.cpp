/*
   Copyright Notice in awvic.cpp
*/

#include "svTheme.h"
#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <json/json.h>
#include "stdwx.h"


//svRGB
svRGB::svRGB():
m_red(0),
m_green(0),
m_blue(0)
{
}

svRGB::svRGB(unsigned char p_r, unsigned char p_g, unsigned char p_b):
m_red(p_r),
m_green(p_g),
m_blue(p_b)
{
}

svRGB::svRGB(const svRGB& p_rgb):
m_red(p_rgb.GetR()),
m_green(p_rgb.GetG()),
m_blue(p_rgb.GetB())
{
    // m_red = p_rgb.GetR();
    // m_green = p_rgb.GetG();
    // m_blue = p_rgb.GetB();
}

void svRGB::SetRGB(unsigned char p_r, unsigned char p_g, unsigned char p_b)
{
    m_red = p_r;
    m_green = p_g;
    m_blue = p_b;
}

/*unsigned char svRGB::GetR() const
{
    return m_red;
}

unsigned char svRGB::GetG() const
{
    return m_green;
}

unsigned char svRGB::GetB() const
{
    return m_blue;
}
*/
// svThemeclass
svThemeClass::svThemeClass(const string& p_class):
m_class(p_class),
m_fg(0,0,0),
m_bg(0,0,0)
{

}

svThemeClass::svThemeClass(const string& p_class, const svRGB& p_fg, const svRGB& p_bg):
m_class(p_class),
m_fg(p_fg),
m_bg(p_bg)
{

}

svThemeClass::svThemeClass(const svThemeClass& p_themeClass):
m_class(p_themeClass.GetClass()),
m_fg(p_themeClass.GetFG()),
m_bg(p_themeClass.GetBG())
{

}

/*void svThemeClass::SetClass(const string& p_class)
{
    m_class = p_class;
}

string svThemeClass::GetClass(void) const
{
    return m_class;
}

void svThemeClass::SetFG(const svRGB& p_fg)
{
    m_fg.SetRGB(p_fg.GetR(), p_fg.GetG(), p_fg.GetB());
}

void svThemeClass::SetBG(const svRGB& p_bg)
{
    m_bg.SetRGB(p_bg.GetR(), p_bg.GetG(), p_bg.GetB());
}

svRGB svThemeClass::GetFG() const
{
    return m_fg;
}

svRGB svThemeClass::GetBG() const
{
    return m_bg;
}

wxColour svThemeClass::GetFGColour() const
{
    wxColour c = wxColour(m_fg.GetR(), m_fg.GetG(), m_fg.GetB());
    return c;
}

wxColour svThemeClass::GetBGColour() const
{
    wxColour c = wxColour(m_bg.GetR(), m_bg.GetG(), m_bg.GetB());
    return c;
}
*/
// svTheme
svTheme::svTheme()
{
}

svTheme::svTheme(const string& p_name):
m_name(p_name)
{
}

svTheme::~svTheme()
{
    m_listofClass.clear();
}

/*void svTheme::SetName(const string& p_name)
{
    m_name = p_name;
}

string svTheme::GetName(void)
{
    return m_name;
}
*/
bool svTheme::LoadFile(const wxString& p_filename)
{
    m_filename = p_filename;

    std::ifstream f(m_filename.mb_str().data());
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
            //std::cout << "Exception  opening/reading file:" << e.what();
            wxString ErrMsg = wxString::Format(wxT("Exception opening file:" + m_filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    // catch (std::ios_base::failure& e)
    catch (exception& e)
    {
        //std::cout << "Exception  opening/reading file:" << e.what();
        wxString ErrMsg = wxString::Format(wxT("Exception opening/reading file:" + m_filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        // std::cout  << "Failed to parse configuration\n"
        //            << reader.getFormattedErrorMessages();
        wxString ErrMsg = wxString::Format(wxT("Failed to parse configuration: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }

    // Get the value of the member of root named 'encoding', return 'UTF-8' if there is no
    // such member.
    std::string encoding = root.get("encoding", "UTF-8" ).asString();
    // Get the value of the member of root named 'encoding', return a 'null' value if
    // there is no such member.
    const Json::Value name = root["name"];
    m_name = name.asString() ;

    // for ( int index = 0; index < plugins.size(); ++index )  // Iterates over the sequence elements.
    //    std::cout  << "plugin:" << plugins[index].asString() ;

    const Json::Value tclass = root["class"];
    for ( int index = 0; index < (int)tclass.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value jsonC  = tclass[index];
        try
        {
            svThemeClass c = svThemeClass(jsonC["id"].asString());
            c.SetFG(svRGB(jsonC["fg"][0].asInt(), jsonC["fg"][1].asInt(), jsonC["fg"][2].asInt()));
            c.SetBG(svRGB(jsonC["bg"][0].asInt(), jsonC["bg"][1].asInt(), jsonC["bg"][2].asInt()));
            m_listofClass.push_back(c);
        }
        catch(const std::exception &e )
        {
            // std::cout << "Unhandled exception:" << e.what() << "\n";
            wxString ErrMsg = wxString::Format(wxT("Unhandled exception :" + wxString(e.what())));
            wxLogMessage(ErrMsg);
            return false;
        }
    }

    return 0;

}

svThemeClass* svTheme::GetClass(const string& p_class)
{
    vector<svThemeClass>::iterator ite;
    for ( ite=m_listofClass.begin(); ite != m_listofClass.end(); ite++ ){
        if (p_class == ite->GetClass())
        {
            return &*ite;
        }
    }
    return NULL;
}

// void svTheme::Clear()
// {
//     m_listofClass.clear();
// }

