/*
   Copyright Notice in awvic.cpp
*/
 
#ifndef _SVSCROLLBAR_H
#define _SVSCROLLBAR_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

// #include <wx/scrolbar.h>
#include "svTheme.h"

#define SV_SCROLLBAR_THICK 12                // The default scrollbar thickness.
#define SV_SCROLLBAR_MINIMAL_PIXEL 10        // The minimal scrollbar thumb pixel size. 

typedef struct {
    int Width;
    int Height;                 // Scroll Bar Width & Height. 
    int X;
    int Y;                      // Scroll Bar Start Position(X&Y). 
} ScrollbarInfo;

// Define a customed event
class svScrollBarEvent : public wxNotifyEvent
{
private:
    int m_position;

public:
    svScrollBarEvent(wxEventType commandType = wxEVT_NULL, int id = 0):
    wxNotifyEvent(commandType, id)
    {
        m_position = 0;
    }
    svScrollBarEvent(const svScrollBarEvent& event):
    wxNotifyEvent(event){}
    virtual wxEvent *Clone() const
    { 
        return new svScrollBarEvent(*this); 
    }

    inline
    void SetPosition(int p_position)
    {
        m_position = p_position;
    }

    inline
    int GetPosition(void)
    {
        return m_position;
    }

    DECLARE_DYNAMIC_CLASS(svScrollBarEvent);
};

typedef void (wxEvtHandler::*svScrollBarEventFunction)
(svScrollBarEvent&);

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(svEVT_SCROLLBAR_PAGEUP, 801)
DECLARE_EVENT_TYPE(svEVT_SCROLLBAR_PAGEDOWN, 802)
DECLARE_EVENT_TYPE(svEVT_SCROLLBAR_THUMBTRACK, 803)
END_DECLARE_EVENT_TYPES()


#define EVT_SVSCROLLBAR_PAGEUP(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_SCROLLBAR_PAGEUP, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svScrollBarEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVSCROLLBAR_PAGEDOWN(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_SCROLLBAR_PAGEDOWN, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svScrollBarEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVSCROLLBAR_THUMBTRACK(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_SCROLLBAR_THUMBTRACK, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svScrollBarEventFunction) & fn, \
    (wxObject *) NULL ),


class svScrollBar : public wxWindow
{
public:
    svScrollBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name);
    ~svScrollBar();
    void OnErase(wxEraseEvent& evet);
    void OnPaint(wxPaintEvent& event);
    // void OnSize(wxSizeEvent& event);
    // void OnClose(wxCloseEvent& event);

    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseRightUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseLeftDClick(wxMouseEvent& event);

    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);

    inline
    void SetRange(int p_range)
    {
        m_range = p_range;
    }

    inline
    int GetRange(void)
    {
        return m_range;
    }

    inline
    void SetPageSize(int p_pageSize)
    {
        m_pageSize = p_pageSize;
    }

    inline
    int GetPageSize(void)
    {
        return m_pageSize;
    }

    inline
    void SetThumbPosition(int p_thumbPosition)
    {
        m_thumbPosition = p_thumbPosition;
    }

    inline
    int SetThumbPosition(void)
    {
        return m_thumbPosition;
    }

    // inline
    // void SetThumbSize(int p_thumbSize)
    // {
    //     m_thumbSize = p_thumbSize;
    // }

    // inline
    // int SetThumbSize(void)
    // {
    //     return m_thumbSize;
    // }

    inline
    void SetVertical(bool p_isVertical)
    {
        m_isVertical = p_isVertical;
    }

    inline
    bool GetVertical(void)
    {
        return m_isVertical;
    }

    inline
    void SetTheme(svTheme *p_theme)
    {
        m_theme = p_theme;
    }

    inline
    int GetDefWidth(void)
    {
        return SV_SCROLLBAR_THICK;
    }

    inline
    int GetDefHeight(void)
    {
        return SV_SCROLLBAR_THICK;
    }

private:

    int m_range;
    int m_pageSize;
    int m_thumbPosition;
    int m_thumbPixelSize;
    
    bool m_isVertical;

    int m_cMouseX;
    int m_cMouseY;
    int m_oMouseX;
    int m_oMouseY;

    bool m_drag;
    int m_dist;
    bool m_mouseInWindow;

    svTheme* m_theme;

private:
    DECLARE_EVENT_TABLE()
};

#endif