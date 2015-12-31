/*
   Copyright Notice in awvic.cpp
*/

#include <wx/dcbuffer.h>
#include "svScrollBar.h"

// svEVT_SCROLLBAR EVENT
DEFINE_EVENT_TYPE(svEVT_SCROLLBAR_PAGEUP)
DEFINE_EVENT_TYPE(svEVT_SCROLLBAR_PAGEDOWN)
DEFINE_EVENT_TYPE(svEVT_SCROLLBAR_THUMBTRACK)
IMPLEMENT_DYNAMIC_CLASS(svScrollBarEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(svScrollBar, wxWindow)
EVT_PAINT(svScrollBar::OnPaint)
EVT_ERASE_BACKGROUND(svScrollBar::OnErase)
// EVT_SIZE(svScrollBar::OnSize)
EVT_LEFT_DOWN(svScrollBar::OnMouseLeftDown)
EVT_LEFT_UP(svScrollBar::OnMouseLeftUp)
EVT_RIGHT_UP(svScrollBar::OnMouseRightUp)
EVT_MOTION(svScrollBar::OnMouseMotion)
EVT_MOUSEWHEEL(svScrollBar::OnMouseWheel)
EVT_LEFT_DCLICK(svScrollBar::OnMouseLeftDClick)
EVT_ENTER_WINDOW(svScrollBar::OnMouseEnter)
EVT_LEAVE_WINDOW(svScrollBar::OnMouseLeave)
END_EVENT_TABLE()


svScrollBar::svScrollBar(wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("scrollbar"))
:wxWindow(parent, id, pos, size, style, name)
{
    m_range = 1;
    m_pageSize = 1;
    m_thumbPosition = 0;
    m_thumbPixelSize = 10;
    m_isVertical = true;

    m_oMouseX = m_cMouseX = -1;
    m_oMouseY = m_cMouseY = -1;

    m_drag = false;
    m_dist = 0;
    m_mouseInWindow = false;

    m_theme = NULL;

}

svScrollBar::~svScrollBar()
{
}

void svScrollBar::OnErase(wxEraseEvent& evet)
{
}

void svScrollBar::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    int w, h;
    w=h=0;
    GetClientSize(&w, &h);

    wxColour defBGColor;
    wxColour defFGColor;
    wxColour BGColor;

    if (m_theme)
    {
        defBGColor = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour();
        defFGColor = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetFGColour();
    }
    else
    {
        defBGColor = *wxBLACK;
        defFGColor = *wxWHITE;
    }

    if (m_mouseInWindow|m_drag)
    {
        BGColor = defBGColor.ChangeLightness(110);
    }
    else
    {
        BGColor = defBGColor;
    }


    if (m_isVertical)   // vertical scrollbar
    {

#ifndef NDEBUG
        wxLogMessage(wxString::Format("w=%i h=%i m_thumbPosition=%i m_range=%i m_pageSize=%i",w, h, m_thumbPosition, m_range, m_pageSize));
#endif

        dc.SetBackground(wxBrush(BGColor));
        dc.Clear();

        dc.SetPen(*wxWHITE);

        // m_range+m_pageSize-1 => -1 是因為 最後一行會顯示在畫面的第一行，所以 m_range+m_pageSize 算多了一行
        wxRect r(0, (int)((double)h*((double)m_thumbPosition/(double)(m_range+m_pageSize-1))),
                 w, (int)((double)h*((double)m_pageSize/(double)(m_range+m_pageSize-1))));

        if (r.GetHeight() < SV_SCROLLBAR_MINIMAL_PIXEL)   // The most small height
            r.SetHeight(SV_SCROLLBAR_MINIMAL_PIXEL);

        m_thumbPixelSize = r.GetHeight();

        dc.GradientFillLinear(r, defBGColor.ChangeLightness(136), defBGColor.ChangeLightness(116));
    }
    else   // horizontal scrollbar
    {

#ifndef NDEBUG
        wxLogMessage(wxString::Format("w=%i h=%i m_thumbPosition=%i m_range=%i m_pageSize=%i",w, h, m_thumbPosition, m_range, m_pageSize));
#endif

        dc.SetBackground(wxBrush(BGColor));
        dc.Clear();

        dc.SetPen(*wxWHITE);

        // m_range+m_pageSize => 沒有 -1 是因為 水平捲動不會有留一行在畫面最上方的問題
        wxRect r((int)((double)w*((double)m_thumbPosition/(double)(m_range+m_pageSize))), 0, 
                 (int)((double)w*((double)m_pageSize/(double)(m_range+m_pageSize))), h);

        if (r.GetWidth() < SV_SCROLLBAR_MINIMAL_PIXEL)   // The most small width
            r.SetWidth(SV_SCROLLBAR_MINIMAL_PIXEL);

        m_thumbPixelSize = r.GetWidth();

        dc.GradientFillLinear(r, defBGColor.ChangeLightness(136), defBGColor.ChangeLightness(116), wxDOWN);
    }

}

// void svScrollBar::OnSize(wxSizeEvent& event)
// {

// }

/* ============================================================================ *
 *
 *   Mouse Processing.
 *
 * ============================================================================ */

void svScrollBar::OnMouseLeftUp(wxMouseEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnMouseLeftUp"));
// #endif

    m_drag = false;
    m_dist = 0;
    // Refresh();
    if (HasCapture())
    {
        ReleaseMouse();
    }

    Refresh();
    event.Skip();
}

// 按下滑鼠左鍵觸發一次
void svScrollBar::OnMouseLeftDown(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svScrollBar::OnMouseLeftDown"));
#endif

    int w, h;
    w=h=0;

    GetClientSize(&w, &h);

    m_cMouseX = event.GetX();
    m_cMouseY = event.GetY();

    if (m_isVertical)  // vertical scrollbar
    {
        if (m_cMouseY<h*((double)m_thumbPosition/(double)(m_range+m_pageSize)))  // page up 
        {
            m_thumbPosition-=m_pageSize;
            if (m_thumbPosition<0)
                m_thumbPosition=0;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEUP, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

        }
        else if (m_cMouseY>=h*((double)m_thumbPosition/(double)(m_range+m_pageSize)) &&  // drag
            m_cMouseY<=h*((double)m_thumbPosition/(double)(m_range+m_pageSize)) + m_thumbPixelSize)
        {
            m_drag = true;
            m_dist = h*((double)m_thumbPosition/(double)(m_range+m_pageSize)) - m_cMouseY;
#ifndef NDEBUG
            wxLogMessage(wxString::Format("mouse left down m_dist=%i", m_dist));
#endif
            CaptureMouse();
        }
        else  // page down
        {
            m_thumbPosition+=m_pageSize;
            if (m_thumbPosition>m_range)
                m_thumbPosition=m_range;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

        }
    }
    else // horizontal scrollbar
    {
        if (m_cMouseX<w*((double)m_thumbPosition/(double)(m_range+m_pageSize)))  // page up
        {
            m_thumbPosition-=m_pageSize;
            if (m_thumbPosition<0)
                m_thumbPosition=0;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEUP, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

        }
        else if (m_cMouseX>=w*((double)m_thumbPosition/(double)(m_range+m_pageSize)) && // drag
            m_cMouseX<=w*((double)m_thumbPosition/(double)(m_range+m_pageSize))+m_thumbPixelSize)
        {
            m_drag = true;
            m_dist = w*((double)m_thumbPosition/(double)(m_range+m_pageSize)) - m_cMouseX;
#ifndef NDEBUG
            wxLogMessage(wxString::Format("mouse left down m_dist=%i", m_dist));
#endif
            CaptureMouse();
        }
        else  // page down
        {
            m_thumbPosition+=m_pageSize;
            if (m_thumbPosition>m_range)
                m_thumbPosition=m_range;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);
        }       
    }

    Refresh();
    // event.Skip();
}

void svScrollBar::OnMouseRightUp(wxMouseEvent& event)
{
    // if (!m_mouseReady) return;
}

void svScrollBar::OnMouseMotion(wxMouseEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnMouseMotion"));
// #endif

    SetCursor(wxCursor(wxCURSOR_ARROW));

    int w, h;
    w=h=0;

    GetClientSize(&w, &h);

    m_cMouseX = event.GetX();
    m_cMouseY = event.GetY();
#ifndef NDEBUG
    wxLogMessage(wxString::Format("CX=%i CY=%i OX=%i OY=%i", m_cMouseX, m_cMouseY, m_oMouseX, m_oMouseY));
#endif

    if (event.LeftIsDown())
    {
        if (m_isVertical)  // vertical scrollbar
        {
            if (m_drag && m_oMouseY>=0)
            {
                int diff = m_cMouseY - m_oMouseY;
                m_thumbPosition = ((m_cMouseY+ m_dist)/(double)h) * (m_range+m_pageSize);
                if (m_thumbPosition<0)
                    m_thumbPosition=0;
                else if (m_thumbPosition>m_range)
                    m_thumbPosition=m_range;

                svScrollBarEvent event(svEVT_SCROLLBAR_THUMBTRACK, GetId());
                event.SetPosition(m_thumbPosition);
                event.SetEventObject(this);
                GetEventHandler()->ProcessEvent(event);

#ifndef NDEBUG
                wxLogMessage(wxString::Format("mouse motion m_dist=%i m_thumbPosition=%i", m_dist, m_thumbPosition));
#endif
            }
        }
        else  // horizontal scrollbar
        {
            if (m_drag && m_oMouseX>=0)
            {
                int diff = m_cMouseX - m_oMouseX;
                m_thumbPosition = ((m_cMouseX+ m_dist)/(double)w) * (m_range+m_pageSize);
                if (m_thumbPosition<0)
                    m_thumbPosition=0;
                else if (m_thumbPosition>m_range)
                    m_thumbPosition=m_range;

                svScrollBarEvent event(svEVT_SCROLLBAR_THUMBTRACK, GetId());
                event.SetPosition(m_thumbPosition);
                event.SetEventObject(this);
                GetEventHandler()->ProcessEvent(event);

#ifndef NDEBUG
                wxLogMessage(wxString::Format("mouse motion m_dist=%i m_thumbPosition=%i", m_dist, m_thumbPosition));
#endif
            }
        }
    }

    m_oMouseX = m_cMouseX;
    m_oMouseY = m_cMouseY;

    Refresh();
    event.Skip();
}

void svScrollBar::OnMouseWheel(wxMouseEvent& event)
{
    event.Skip();
}

// When mouse double click. The second click will be generate a page up/down event too.
void svScrollBar::OnMouseLeftDClick(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svScrollBar::OnLeftDClick"));
#endif

    int w, h;
    w=h=0;

    GetClientSize(&w, &h);

    m_cMouseX = event.GetX();
    m_cMouseY = event.GetY();

    if (m_isVertical)  // vertical scrollbar
    {
        if (m_cMouseY<h*((double)m_thumbPosition/(double)(m_range+m_pageSize)))  // page up 
        {
            m_thumbPosition-=m_pageSize;
            if (m_thumbPosition<0)
                m_thumbPosition=0;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEUP, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

            // m_thumbPosition-=m_pageSize;
            // if (m_thumbPosition<0)
            //     m_thumbPosition=0;

            // svScrollBarEvent event2(svEVT_SCROLLBAR_PAGEUP, GetId());
            // event2.SetEventObject(this);
            // GetEventHandler()->ProcessEvent(event2);
        }
        else if (m_cMouseY>=h*((double)m_thumbPosition/(double)(m_range+m_pageSize)) &&  // drag
            m_cMouseY<=h*((m_thumbPosition)/(double)(m_range+m_pageSize))+m_thumbPixelSize)
        {
//             m_drag = true;
//             m_dist = h*((double)m_thumbPosition/(double)m_range) - m_cMouseY;
// #ifndef NDEBUG
//             wxLogMessage(wxString::Format("mouse left down m_dist=%i", m_dist));
// #endif
//             CaptureMouse();
        }
        else  // page down
        {
            m_thumbPosition+=m_pageSize;
            if (m_thumbPosition>m_range)
                m_thumbPosition=m_range;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

            // m_thumbPosition+=m_pageSize;
            // if (m_thumbPosition>m_range-m_pageSize)
            //     m_thumbPosition=m_range-m_pageSize;

            // svScrollBarEvent event2(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            // event2.SetEventObject(this);
            // GetEventHandler()->ProcessEvent(event2);
        }
    }
    else // horizontal scrollbar
    {
        if (m_cMouseX<w*((double)m_thumbPosition/(double)(m_range+m_pageSize)))  // page up
        {
            m_thumbPosition-=m_pageSize;
            if (m_thumbPosition<0)
                m_thumbPosition=0;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEUP, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

            // m_thumbPosition-=m_pageSize;
            // if (m_thumbPosition<0)
            //     m_thumbPosition=0;

            // svScrollBarEvent event2(svEVT_SCROLLBAR_PAGEUP, GetId());
            // event2.SetEventObject(this);
            // GetEventHandler()->ProcessEvent(event2);
        }
        else if (m_cMouseX>=w*((double)m_thumbPosition/(double)(m_range+m_pageSize)) && // drag
            m_cMouseX<=w*((m_thumbPosition)/(double)(m_range+m_pageSize))+m_thumbPixelSize)
        {
//             m_drag = true;
//             m_dist = w*((double)m_thumbPosition/(double)m_range) - m_cMouseX;
// #ifndef NDEBUG
//             wxLogMessage(wxString::Format("mouse left down m_dist=%i", m_dist));
// #endif
//             CaptureMouse();
        }
        else  // page down
        {
            m_thumbPosition+=m_pageSize;
            if (m_thumbPosition>m_range)
                m_thumbPosition=m_range;

            svScrollBarEvent event(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            event.SetEventObject(this);
            GetEventHandler()->ProcessEvent(event);

            // m_thumbPosition+=m_pageSize;
            // if (m_thumbPosition>m_range-m_pageSize)
            //     m_thumbPosition=m_range-m_pageSize;

            // svScrollBarEvent event2(svEVT_SCROLLBAR_PAGEDOWN, GetId());
            // event2.SetEventObject(this);
            // GetEventHandler()->ProcessEvent(event2);
        }       
    }

    Refresh();
    // event.Skip();
}


void svScrollBar::OnMouseEnter(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svScrollBar::OnMouseEnter"));
#endif
    m_mouseInWindow = true;

    Refresh();
}

void svScrollBar::OnMouseLeave(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svScrollBar::OnMouseLeave"));
#endif

    m_mouseInWindow = false;

    Refresh();
}