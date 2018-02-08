/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVLISTBOXCTRL_H
#define _SVLISTBOXCTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <memory>
#include <wx/dcbuffer.h>
#include <wx/font.h>

#include "svScrollBar.h"
#include "svBaseType.h"

using namespace std;

#define SVID_LISTBOX_DEFAULT_FONT_SIZE  12
#define SVID_LISTBOX_ITEM_BORDER 30

// class svTextEditorCtrl;
// class svMainFrame; 

// enum
// {
//     SVID_SMOOTH_NONE=0,
//     SVID_SMOOTH_LOW,
//     SVID_SMOOTH_MEDIUM,
//     SVID_SMOOTH_HIGH,
//     SVID_SMOOTH_DEFAULT
// };

// Define a customed event
/*class svListBoxCtrlEvent : public wxNotifyEvent
{
private:
    bool m_modified;

public:
    svListBoxCtrlEvent(wxEventType commandType = wxEVT_NULL, int id = 0):
    wxNotifyEvent(commandType, id)
    {
        m_modified = false;
    }
    svListBoxCtrlEvent(const svListBoxCtrlEvent& event):
    wxNotifyEvent(event){}
    virtual wxEvent *Clone() const
    {
        return new svListBoxCtrlEvent(*this); 
    }
    inline
    void Modified(bool p_modified)
    {
        m_modified = p_modified;
    }
    inline
    bool IsModified(void)
    {
        return m_modified;
    }

    DECLARE_DYNAMIC_CLASS(svListBoxCtrlEvent);
};

typedef void (wxEvtHandler::*svListBoxCtrlEventFunction)
(svListBoxCtrlEvent&);

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_EVENT, 811)
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_MODIFIED_CHANGED, 812)
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_MSG, 813)
END_DECLARE_EVENT_TYPES()

#define EVT_SVTEXTCTRL_EVENT(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_EVENT, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svListBoxCtrlEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVTEXTCTRL_MODIFIED_CHANGED(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_MODIFIED_CHANGED, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svListBoxCtrlEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVTEXTCTRL_MSG(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_MSG, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svListBoxCtrlEventFunction) & fn, \
    (wxObject *) NULL ),
*/
enum
{
    SVID_SVVSB02=wxID_HIGHEST+1,
    SVID_NULL
};

enum
{
    SVID_LISTBOX_NO_SCB=0,
    SVID_LISTBOX_VSCB=1,
    SVID_LISTBOX_HSCB=2,
    SVID_LISTBOX_BOTH_SCB=3
};

enum
{
    ID_LBOX_LIST_BOX=wxID_HIGHEST+1
};


// class svListBoxCtrl : public wxFrame
// class svListBoxCtrl : public wxWindow
class svListBoxCtrl : public wxControl
{
public:
    svListBoxCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("svListBoxCtrl"));
    ~svListBoxCtrl();

    void InitControls(void);

    void OnErase(wxEraseEvent& evet);
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnMouseEnterSB(wxMouseEvent& event);

    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    void PrepareDCBuffer(bool p_smooth);
    // void DoSmoothRefresh(voidconst int p_smooth=SVID_SMOOTH_DEFAULT, char p_funcType=SVID_SMOOTH_SIN);
    void DoSmoothRefresh(void);

    void OnVsvScrollThumbTrack(svScrollBarEvent& event);
    void OnVsvScrollPageUp(svScrollBarEvent& event);
    void OnVsvScrollPageDown(svScrollBarEvent& event);
    void OnHsvScrollThumbTrack(svScrollBarEvent& event);
    void OnHsvScrollPageUp(svScrollBarEvent& event);
    void OnHsvScrollPageDown(svScrollBarEvent& event);

    void SetTheme(const wxColour p_fgColour, const wxColour p_bgColour, const wxColour p_cColour, const wxColour p_fColour, int p_fontSize=SVID_LISTBOX_DEFAULT_FONT_SIZE);

    void PageUp(void);
    void PageDown(void);
    void LineUp(void);
    void LineDown(void);

    inline
    void Clear(void)
    {
        m_items.clear();
        m_filterItems.clear();
    }
    void Append(const wxString &p_item);
    void Insert(const wxString &p_item, unsigned int p_pos);
    void Delete(unsigned int p_pos);
    void SetItems(const vector<wxString> &p_items);


    void SetMaxHeight(unsigned int p_height);
    void FitHeight(void);

    inline
    unsigned int GetCount(void)
    {
        return m_items.size();
    }

    inline
    unsigned int GetFilteredCount(void)
    {
        return m_filterItems.size();
    }

    inline
    wxString GetString(unsigned int p_idx)
    {
        if (p_idx<m_items.size())
            return m_items.at(p_idx);
        else
            return _("");
    }

    inline
    wxString GetFilteredString(unsigned int p_idx)
    {
        if (p_idx<m_filterItems.size())
            return m_items.at(m_filterItems.at(p_idx));
        else
            return _("");
    }

    // inline
    // void ShowVScrollbar(bool p_flag=true)
    // {
    //     m_vsb = p_flag;
    // }

    // inline
    // bool ShowVScrollbar(void)
    // {
    //     return m_vsb;
    // }

    void SetFilter(const wxString &p_filter);

    inline
    wxString GetFilter(void)
    {
        return m_filter;
    }

    

private:

    svScrollBar *m_vsb;                // Vertical scrollbar.

    vector<wxString> m_items;          // All items.
    vector<unsigned int> m_filterItems;   // Items index been filtered on m_items.

    wxString m_filter;                 // Filter pattern for filter items.

    // for smooth scroll.
    wxMemoryDC *m_bufferDC;            // buffer DC for onPaint called.
    int m_linesPerPage;
    int m_scb_type;

    ScrollbarInfo m_vsbi;

    /* ------------------------------------------- */
    wxFont m_font;
    wxColour m_fgColour;
    wxColour m_bgColour;
    wxColour m_choseColour;
    wxColour m_filterColour;
    int m_fontSize;
    int m_charHeight;

    wxListBox* m_lcHints;

private:
    DECLARE_EVENT_TABLE()

};

#endif
