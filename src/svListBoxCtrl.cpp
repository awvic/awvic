/*
   Copyright Notice in awvic.cpp
*/

/* -------------------------------------------------------------------- *
 * awvic text control                     .                             *
 * -------------------------------------------------------------------- */

#include "svListBoxCtrl.h"
#include <wx/dcbuffer.h>
#include <wx/log.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include "wx/strconv.h"

#include "stdwx.h"
// #include "svPreference.h"

#include <math.h>       /* sin */

#include "svCommonLib.h"

// #include "svTextEditorCtrl.h"

// m_bufText_EVENT
/*DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_EVENT)
DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_MODIFIED_CHANGED)
DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_MSG)
IMPLEMENT_DYNAMIC_CLASS(svTextCtrlEvent, wxNotifyEvent)*/


// BEGIN_EVENT_TABLE(svListBoxCtrl, wxWindow)
BEGIN_EVENT_TABLE(svListBoxCtrl, wxControl)
EVT_PAINT(svListBoxCtrl::OnPaint)
// EVT_ERASE_BACKGROUND(svListBoxCtrl::OnErase)
// EVT_SIZE(svListBoxCtrl::OnSize)
// EVT_CHAR(svListBoxCtrl::OnChar)
EVT_KEY_DOWN(svListBoxCtrl::OnKeyDown)
EVT_CLOSE(svListBoxCtrl::OnClose)
EVT_SET_FOCUS(svListBoxCtrl::OnSetFocus)

// EVT_SVSCROLLBAR_THUMBTRACK(SVID_SVVSB02, svListBoxCtrl::OnVsvScrollThumbTrack) 
// EVT_SVSCROLLBAR_PAGEUP(SVID_SVVSB02, svListBoxCtrl::OnVsvScrollPageUp)
// EVT_SVSCROLLBAR_PAGEDOWN(SVID_SVVSB02, svListBoxCtrl::OnVsvScrollPageDown)

// EVT_LEFT_DOWN(svListBoxCtrl::OnMouseLeftDown)
// EVT_LEFT_UP(svListBoxCtrl::OnMouseLeftUp)
// EVT_RIGHT_UP(svListBoxCtrl::OnMouseRightUp)
// EVT_MOTION(svListBoxCtrl::OnMouseMotion)
//EVT_MOTION(svListBoxCtrl::OnMouseMotion)
// EVT_MOUSEWHEEL(svListBoxCtrl::OnMouseWheel)
//EVT_LEAVE_WINDOW(svListBoxCtrl::OnMouseLeaveWindow)
//EVT_m_bufText_EVENT(wxID_ANY, svListBoxCtrl::OnModified)
END_EVENT_TABLE()


svListBoxCtrl::svListBoxCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style, const wxString& name)
:wxControl(parent, id, pos, size, style)
// :wxWindow(parent, id, pos, size, style, name)
// :wxFrame(parent, id, name, pos, size, style)
{
    m_linesPerPage = 0;

    m_scb_type = SVID_LISTBOX_NO_SCB;

    m_vsbi.Width = m_vsbi.Height = 0;

    m_vsbi.X = m_vsbi.Y = 0;

    m_vsb = new svScrollBar(this, SVID_SVVSB02, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "sb01");
    m_vsb->SetVertical(true);
    m_vsb->SetSize(10, 300);
    m_vsb->SetRange(100);
    m_vsb->SetPageSize(10);
    m_vsb->SetThumbPosition(0);
    // m_vsb->SetThumbSize(10);

    m_vsb->GetSize(&(m_vsbi.Width), &(m_vsbi.Height));


    m_font = parent->GetFont();
    m_fgColour = parent->GetForegroundColour();
    m_bgColour = parent->GetBackgroundColour();
    m_choseColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    m_filterColour = *wxRED;
    m_fontSize = SVID_LISTBOX_DEFAULT_FONT_SIZE;
    m_font.SetPointSize(m_fontSize);
    m_charHeight = m_font.GetPixelSize().GetHeight();

    m_bufferDC = NULL;

    InitControls();

}

svListBoxCtrl::~svListBoxCtrl()
{
    if (m_vsb) delete m_vsb;

    if (m_bufferDC) delete m_bufferDC;
}

void svListBoxCtrl::InitControls(void)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer4;
    bSizer4 = new wxBoxSizer( wxVERTICAL );
    
    m_lcHints = new wxListBox( this, ID_LBOX_LIST_BOX, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxLB_HSCROLL );    bSizer4->Add( m_lcHints, 1, wxALL|wxEXPAND, 0 );
    
    this->SetSizer( bSizer4 );
    this->Layout();
    
    this->Centre( wxBOTH );

    m_lcHints->Bind(wxEVT_KEY_DOWN, &svListBoxCtrl::OnKeyDown, this);
    m_lcHints->Bind(wxEVT_CHAR, &svListBoxCtrl::OnChar, this);


    // DEBUG BEGIN
    // m_lcHints->ClearAll();
    m_lcHints->Clear();

    wxArrayString as;
    as.Add(_("test 01"));
    as.Add(_("test 02"));
    m_lcHints->InsertItems(as, 0);
    if (as.GetCount()>0)
        m_lcHints->SetSelection(0);

    // DEBUG END

}

void svListBoxCtrl::OnErase(wxEraseEvent& evet)
{
}

void svListBoxCtrl::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    if (m_bufferDC)
    {
        wxCoord tw, th;
        dc.GetSize (&tw, &th);

        dc.SetClippingRegion(0, 0, tw, th);
        dc.Blit(0, 0, tw, th, m_bufferDC, 0, 0);
    }
    else
    {
        wxCoord tw, th;
        dc.GetSize (&tw, &th);

        dc.DrawText("Strange!", 0, 0);
    }

}

void svListBoxCtrl::OnSize(wxSizeEvent& event)
{
    int cw, ch;
    cw=ch=0;
    GetClientSize(&cw, &ch);

    // Resize Scrollbar.
    m_vsbi.Height = ch;

    m_vsbi.X = cw - m_vsbi.Width - 1;
    m_vsbi.Y = 0;

    m_vsb->SetSize(wxRect(m_vsbi.X, m_vsbi.Y, m_vsb->GetDefWidth(), m_vsbi.Height));

    //repaint client area.
    DoSmoothRefresh();

}

void svListBoxCtrl::OnKeyDown(wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    //wxChar ukey = event.GetUnicodeKey();

    if (key==27) // ESC
    {
        Hide();
    }
    else if (key==WXK_UP)
    {
        int sel = m_lcHints->GetSelection();
        if (sel>0)
            --sel;
        m_lcHints->SetSelection(sel);
    }
    else if (key==WXK_DOWN)
    {
        int max = m_lcHints->GetCount();
        int sel = m_lcHints->GetSelection();
        if (sel<max-1)
            ++sel;
        m_lcHints->SetSelection(sel);
    }
    // else if (key==WXK_RETURN || 
    //          key==WXK_TAB)
    // {
    //     int selNo = m_lcHints->GetSelection();
    //     if (selNo!=wxNOT_FOUND)
    //     {
    //         m_txtCtrl->DoTextInsertHint(m_lcHints->GetString(selNo));
    //     }
    //     Hide();
    //     m_txtCtrl->SetFocus();
    // }
    else
    {
        // event.Skip();
        // wxKeyEvent e;
        // e = event;
        GetParent()->ProcessWindowEvent(event); // passing the key event to  parent window(svTextEditorCtrl)
        // wxLogMessage(wxString::Format("svTypeHintCtrl::OnKeyDown() %i %i", e.m_x, e.m_y));
    }

}


void svListBoxCtrl::OnSetFocus(wxFocusEvent& event)
{
    m_lcHints->SetFocus();
}

void svListBoxCtrl::OnChar(wxKeyEvent& event)
{

    event.Skip();
    GetParent()->ProcessWindowEvent(event); // passing the key event to  parent window(
    // svCommand* cmd;
    // cmd = NULL;

    // cmd = EmergeTextCommand(event);
    // ProcessTextCommand(cmd);


    // if (cmd) // cmd != NULL  <= I am a C/C++ novice. Reminding myself.
    // { 
    //     delete cmd; 
    //     cmd=NULL;
    // }

    // DoSmoothRefresh();

}

/*
 * Process TEXT MODE input
 */
// svCommand* svListBoxCtrl::EmergeTextCommand(const wxKeyEvent& event)
// {

    // int key = event.GetKeyCode();
    // wxChar ukey = event.GetUnicodeKey();
    // int cmdName;


    // m_setCaretOnTxtArea = true;

    // cmdName = CMD_UNDEFINED;

    // if (key == WXK_LEFT)
    // {
    //     if (event.ControlDown())
    //         cmdName = CMD_TXT_LEFT_HEAD;
    //     else
    //         cmdName = CMD_TXT_LEFT;
    // }
    // else if (key == WXK_RIGHT)
    // {
    //     if (event.ControlDown())
    //         cmdName = CMD_TXT_RIGHT_END;
    //     else
    //         cmdName = CMD_TXT_RIGHT;
    // }
    // else if (key == WXK_UP)
    // {
    //     cmdName = CMD_TXT_UP;
    // }
    // else if (key == WXK_DOWN)
    // {
    //     cmdName = CMD_TXT_DOWN;
    // }
    // else if (key == WXK_PAGEUP)
    // {
    //     cmdName = CMD_TXT_PAGEUP;
    // }
    // else if (key == WXK_PAGEDOWN)
    // {
    //     cmdName = CMD_TXT_PAGEDOWN;
    // }
    // else if (key == WXK_END)
    // {
    //     if (event.ControlDown())
    //     cmdName = CMD_TXT_BOTTOM;
    //     else
    //     cmdName = CMD_TXT_LINE_END;
    // }
    // else if (key == WXK_HOME)
    // {
    //     if (event.ControlDown())
    //     cmdName = CMD_TXT_TOP;
    //     else
    //     cmdName = CMD_TXT_LINE_HEAD;
    // }
    // else if (key == WXK_RETURN)
    // {
    //     cmdName = CMD_TXT_SPLIT;
    // }

    // svCommand* cmd = new svCommand(cmdName, event);
    // return cmd;

//     return NULL;
// }


// void svListBoxCtrl::ProcessTextCommand(svCommand* cmd)
// {
    // switch(cmd->Name())
    // {
    // case CMD_TXT_LEFT:
    // case CMD_TXT_LEFT_HEAD:
    // case CMD_TXT_RIGHT:
    // case CMD_TXT_RIGHT_END:
    // case CMD_TXT_UP:
    // case CMD_TXT_DOWN:
    // case CMD_TXT_PAGEUP:
    // case CMD_TXT_PAGEDOWN:
    // case CMD_TXT_LINE_END:
    // case CMD_TXT_LINE_HEAD:
    // case CMD_TXT_TOP:
    // case CMD_TXT_BOTTOM:
    // case CMD_TXT_CUR_TOP_SCREEN:
    // case CMD_TXT_CUR_MIDDLE_SCREEN:
    // case CMD_TXT_CUR_BOTTOM_SCREEN:
    // case CMD_TXT_TOP_SCREEN:
    // case CMD_TXT_MIDDLE_SCREEN:
    // case CMD_TXT_BOTTOM_SCREEN:
    //     DoTextNavi(cmd);
    //     break;
    // }

// }

// void svListBoxCtrl::DoTextNavi(svCommand* cmd)
// {

    // if (!cmd->ShiftDown())
    // {
    //     m_textView->ClearCaretSelect();
    // }
    // else
    // {
    //     m_textView->SetCaretSelect();
    // }

    // // m_setCaretOnTxtArea = true;

    // // int tmpX, tmpY;

    // switch (cmd->Name())
    // {
    // case CMD_TXT_LEFT:
    //     m_textView->CaretsLeft();
    //     if (m_typeHintCtrl->IsShown() &&
    //         m_bufText->GetAvailableHintCnt()>0)
    //     {}
    //     else HideTypeHint();
    //     break;
        
    // case CMD_TXT_LEFT_HEAD:
    //     m_textView->CaretsLeftHead();
    //     if (m_typeHintCtrl->IsShown() &&
    //         m_bufText->GetAvailableHintCnt()>0)
    //     {}
    //     else HideTypeHint();
    //     break;

    // case CMD_TXT_RIGHT:
    //     m_textView->CaretsRight();
    //     if (m_typeHintCtrl->IsShown() &&
    //         m_bufText->GetAvailableHintCnt()>0)
    //     {}
    //     else HideTypeHint();
    //     break;

    // case CMD_TXT_RIGHT_END:
    //     m_textView->CaretsRightEnd();
    //     if (m_typeHintCtrl->IsShown() &&
    //         m_bufText->GetAvailableHintCnt()>0)
    //     {}
    //     else HideTypeHint();
    //     break;

    // case CMD_TXT_UP:
    //     HideTypeHint();
    //     m_textView->CaretsUp();
    //     break;

    // case CMD_TXT_DOWN:
    //     HideTypeHint();
    //     m_textView->CaretsDown();
    //     break;

    // case CMD_TXT_PAGEUP:
    //     HideTypeHint();
    //     m_textView->PageUp();
    //     break;

    // case CMD_TXT_PAGEDOWN:
    //     HideTypeHint();
    //     m_textView->PageDown();
    //     break;
      
    // }

// }

void svListBoxCtrl::OnClose(wxCloseEvent& event)
{

}

void svListBoxCtrl::OnMouseEnterSB(wxMouseEvent& event)
{
    SetCursor(wxCursor(wxCURSOR_ARROW));
    event.Skip();
}


// --------------------------------------------------------------------------------------- //
// void svListBoxCtrl::AdjustScrollbar(void)
// {
    // m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_vsb->SetPageSize(m_textView->GetLinesPerPage());
    // m_vsb->SetRange(m_bufText->LineCntUW());
// }

/* ============================================================================ *
 *
 *   Mouse Processing.
 *
 * ============================================================================ */

// 按下滑鼠左鍵觸發一次
void svListBoxCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseLeftDown"));
#endif

    event.Skip();
}

void svListBoxCtrl::OnMouseWheel(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseWheel"));
#endif
    
    event.Skip();
}

/*
 * Preparing all part needed to be display on the OnPaint event.
 * Including Line Number Indicator, Column Number Indicator, Text and Carets.
 * 將要在畫面上呈現的各部份先行準備為一個個的memoryDC
 * OnPaint時再將各併合各部份
 * 這樣作的目的是為了比較平滑的畫面捲動效果
 * 控制平滑捲動的是 DoSmoothScrool function.
 *
 * PrepareDCBuffer 總共準備了三倍於畫面大小的wxMemoryDC
 * 分別是畫面的上一頁，本頁及下一頁
 * 是用來作平滑捲動用的
 * 可能有些耗用了記憶體及CPU處理時間
 *
 * p_smooth = false 時不作 smooth scroll, 也就不準備上一頁及下一頁的資料
 * 
 */
void svListBoxCtrl::PrepareDCBuffer(bool p_smooth)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svListBoxCtrl::PrepareDCBuffer 01"));
#endif

    // 準備相關DC時是按前一頁、本頁、下一頁分別處理

    if (m_bufferDC!=NULL) delete m_bufferDC;
    m_bufferDC = new wxMemoryDC();

    // 區分前一頁，本頁，下一頁，baseH是三者的基準y值
    int baseH = 0;

    // 每一文字行高度佔的像素值
    // int lineHeight = m_hChar + m_rowInterval;
    int borderHeight = (int) (m_charHeight * (((double)SVID_LISTBOX_ITEM_BORDER)/(double)100));
    int lineHeight = m_charHeight + 2 * borderHeight;

    // 長度額外加上一個頁面的長度，處理最後一行出現在頁面第一行時的狀況
    // wxBitmap bitmapDC(GetMaxPixelWidth(), GetAllPageTextListCount() * lineHeight + m_tai.Height);
    wxBitmap bitmapDC(GetMaxWidth(), GetMaxHeight());

    m_bufferDC->SelectObject(bitmapDC);
    m_bufferDC->SetFont(m_font);
    m_bufferDC->SetBrush(wxBrush(*wxWHITE));
    m_bufferDC->SetPen(wxPen(*wxBLACK, 1));


    // 長度額外加上一個頁面的長度，處理最後一行出現在頁面第一行時的狀況



    
    // Loading the theme colors.


    // Clear all background
    m_bufferDC->SetBackground(m_bgColour);
    m_bufferDC->Clear();


    m_bufferDC->SetTextForeground(m_fgColour);
    m_bufferDC->SetBrush(m_fgColour);
    m_bufferDC->SetPen(m_fgColour);

// #ifdef NDEBUG
// #ifdef __WXMSW__
//     wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i MLD=%s CUR=%Iu %Iu LN=%i"), m_mouseX, m_mouseY, m_tai.X, m_tai.Y, m_tai.Width, m_tai.Height, m_areaI, m_vsbi.X, m_vsbi.Y, m_vsbi.Width, m_vsbi.Height, m_mouseLeftIsDown?"true":"false", debug_textIdx.unwrap_idx, debug_textIdx.wrap_idx, m_linesPerTxtArea);
// #else
//     wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i MLD=%s CUR=%zu %zu LN=%i"), m_mouseX, m_mouseY, m_tai.X, m_tai.Y, m_tai.Width, m_tai.Height, m_areaI, m_vsbi.X, m_vsbi.Y, m_vsbi.Width, m_vsbi.Height, m_mouseLeftIsDown?"true":"false", debug_textIdx.unwrap_idx, debug_textIdx.wrap_idx, m_linesPerTxtArea);
// #endif
// #endif

    // draw text.
    m_font.SetStyle(wxFONTSTYLE_NORMAL);

    // for (std::vector<wxString>::iterator it=m_filterItems.begin();
    //      it!=m_filterItems.end();
    //      ++it)
    for (int i=0; i<(int)m_filterItems.size(); i++)
    {
        // m_bufferDC->SetTextForeground(defFGColor);
        m_bufferDC->DrawText(m_items.at(m_filterItems.at(i)), 0, baseH + borderHeight + (i * lineHeight)); 
    }

#ifndef NDEBUG
        wxLogMessage(wxT("svListBoxCtrl::PrepareDCBuffer 10"));
#endif

}

/*
 * Calulate start and end positon and dividing it into several peroid position.
 * Drawing sequentially and make smooth scroll felling.
 * 將要在畫面上呈現的各部份計算其起啟位置及停止位置
 * 將其分割為數個中間值並予以呈現
 * 這樣作的目的是為了比較平滑的畫面捲動效果
 * 
 * 目前使用 sin 函務進行中間值的分割
 *
 *
 * p_smooth = true => smooth scroll, else force not to smooth scroll
 */
void svListBoxCtrl::DoSmoothRefresh(void)
{
    // AdjustScrollbar();

    PrepareDCBuffer(true);

    Refresh();  // YOU HAVE TO CALL REFRESH() THEN CALL UPDATE()!!!
    Update();   // immediately Refresh();


/*#ifndef NDEBUG            
            wxLogMessage(wxString::Format(wxT("SmoothRefresh() m_fai.Width=%i"), m_fai.Width));
#endif  

    int lineHeight = m_hChar + m_rowInterval;

    int p_oldHScrollPixel = m_textView->GetOldHScrollPixel();
    int p_HScrollPixel = m_textView->GetHScrollPixel();

    // m_bTextAreaDC_x = m_textView->GetOldHScrollPixel();
    m_bTextAreaDC_x = p_oldHScrollPixel;
    m_bTextAreaDC_y = m_textView->GetPrevPageTextListCount() * lineHeight + 1;

    // 判斷該如何垂直捲動
    int p_VScrollLineCnt = m_textView->GetVScrollLineCount();
    int p_VScrollPixelDistance = p_VScrollLineCnt * lineHeight;

    // 推算垂直捲原來的啟始位置
    int p_oldVScrollPixel = m_textView->GetPrevPageTextListCount() * lineHeight - (p_VScrollLineCnt * lineHeight);

    if ((p_oldHScrollPixel!=p_HScrollPixel || p_VScrollPixelDistance!=0) &&
        p_smooth!=SVID_SMOOTH_NONE)
    {
        int peroid = 0;

        if (abs(p_VScrollPixelDistance) <= 2 * lineHeight &&
            abs(p_oldHScrollPixel-p_HScrollPixel) <= 3 * lineHeight )
        {
            peroid = SVID_VERY_LOW_FPS;
        }
        else
        {
            switch(p_smooth)
            {
                case SVID_SMOOTH_LOW:
                    peroid = SVID_LOW_FPS;
                    break;
                case SVID_SMOOTH_MEDIUM:
                    peroid = SVID_MEDIUM_FPS;
                    break;
                case SVID_SMOOTH_HIGH:
                    peroid = SVID_HIGH_FPS;
                    break;
                case SVID_SMOOTH_DEFAULT:
                    peroid = SVID_MEDIUM_FPS;
                    break;
            }
        }

        int deltax = 0;
        int deltay = 0;
        for(int i=1; i<=peroid; ++i)
        {
            // wxLogMessage(wxString::Format(wxT("SmoothRefresh() m_textView->GetHScrollPixel() - m_textView->GetOldHScrollPixel()=%i"), m_textView->GetHScrollPixel() - m_textView->GetOldHScrollPixel()));
            // wxLogMessage(wxString::Format(wxT("m_textView->GetOldHScrollPixel()=%i"), m_textView->GetOldHScrollPixel()));
            // wxLogMessage(wxString::Format(wxT("m_textView->GetHScrollPixel()=%i"), m_textView->GetHScrollPixel()));

            if (p_funcType==SVID_SMOOTH_SIN)
            {
                deltax = (p_HScrollPixel-p_oldHScrollPixel) * sin ((double)i/peroid*(PI/2));
                deltay = (p_VScrollPixelDistance) * sin ((double)i/peroid*(PI/2));
            }
            else // average
            {
                deltax = (p_HScrollPixel-p_oldHScrollPixel) * ((double)i/peroid);
                deltay = (p_VScrollPixelDistance) * ((double)i/peroid);
            }
            m_bTextAreaDC_x = p_oldHScrollPixel + deltax;
            m_bTextAreaDC_y = p_oldVScrollPixel + deltay + 1;

            if (m_bufferDC) delete m_bufferDC;
            m_bufferDC = new wxMemoryDC();
            int bw, bh;
            bw=bh=0;
            this->GetClientSize(&bw, &bh);
            wxBitmap bitmapDC(bw, bh);
            m_bufferDC->SelectObject(bitmapDC);
            m_bufferDC->SetFont(m_font);
            m_bufferDC->SetBrush(wxBrush(*wxBLACK));
            m_bufferDC->SetPen(wxPen(*wxBLACK, 1));
            m_bufferDC->SetBackground(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
            m_bufferDC->Clear();

            m_bufferDC->SetClippingRegion(0, 0, bw, bh);
            // 放文字區資料
            m_bufferDC->Blit(m_lniai.Width+m_fai.Width, m_cniai.Height, m_tai.Width, m_tai.Height, m_bTextAreaDC, m_bTextAreaDC_x, m_bTextAreaDC_y);
            // 放行號區資料
            m_bufferDC->Blit(0, m_cniai.Height, m_lniai.Width, m_lniai.Height, m_bLineNumIndDC, 0, m_bTextAreaDC_y);
            // 放folding區資料
            m_bufferDC->Blit(m_lniai.Width, m_cniai.Height, m_fai.Width, m_fai.Height, m_bFoldingIndDC, 0, m_bTextAreaDC_y);
            // 放上方區(原欄位區)資料
            m_bufferDC->Blit(m_cniai.X, m_cniai.Y, m_cniai.Width, m_cniai.Height, m_bColNumIndDC, 0, 0);

            Refresh();  // YOU HAVE TO CALL REFRESH() THEN CALL UPDATE()!!!
            Update();   // immediately Refresh();
            // wxLogMessage(wxString::Format(wxT("SmoothRefresh() m_bTextAreaDC_x=%i m_bTextAreaDC_y=%i"), m_bTextAreaDC_x, m_bTextAreaDC_y));
#ifndef NDEBUG            
            wxLogMessage(wxString::Format(wxT("SmoothRefresh() deltax=%i"), deltax));
#endif            
        }
    }
    else
    {
        if (m_bufferDC) delete m_bufferDC;
        m_bufferDC = new wxMemoryDC();
        int bw, bh;
        bw=bh=0;
        this->GetClientSize(&bw, &bh);
        wxBitmap bitmapDC(bw, bh);
        m_bufferDC->SelectObject(bitmapDC);
        m_bufferDC->SetFont(m_font);
        m_bufferDC->SetBrush(wxBrush(*wxBLACK));
        m_bufferDC->SetPen(wxPen(*wxBLACK, 1));
        m_bufferDC->SetBackground(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
        m_bufferDC->Clear();

        m_bufferDC->SetClippingRegion(0, 0, bw, bh);
        // 放文字區資料
        m_bufferDC->Blit(m_lniai.Width+m_fai.Width, m_cniai.Height, m_tai.Width, m_tai.Height, m_bTextAreaDC, m_bTextAreaDC_x, m_bTextAreaDC_y);
        // 放行號區資料
        m_bufferDC->Blit(0, m_cniai.Height, m_lniai.Width, m_lniai.Height, m_bLineNumIndDC, 0, m_bTextAreaDC_y);
        // 放folding區資料
        m_bufferDC->Blit(m_lniai.Width, m_cniai.Height, m_fai.Width, m_fai.Height, m_bFoldingIndDC, 0, m_bTextAreaDC_y);
        // 放上方區(原欄位區)資料
        m_bufferDC->Blit(m_cniai.X, m_cniai.Y, m_cniai.Width, m_cniai.Height, m_bColNumIndDC, 0, 0);

        Refresh();
        Update();
    }

    m_textView->ResetOldHScrollPixel();
    m_textView->ResetVScrollLineCount();
*/
}


/* ----------------------------------------------------------------------- *
 *
 *                         Scroll Bar Processing.
 *       #define OS_SCROLLBAR for using wxWidgets scrollbar (OS style)
 *                  Or not defined for using customized style.
 *
 * ----------------------------------------------------------------------- */

void svListBoxCtrl::OnVsvScrollThumbTrack(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnVsvScrollThumbTrack"));
// #endif
//     if (!m_mouseReady) return;

//     int pos = event.GetPosition();
//     int old_firstLine = m_textView->GetFirstUWLineNo();
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("OnVsvScrollThumbTrack pos=%i", pos));
// #endif

//     // // for smmoth scoll
//     // if (old_firstLine>pos)
//     // {
//     //     m_textView->SetFirstLineIndex(pos+1);
//     //     m_textView->MoveFirstLineIndex(-1);
//     // }
//     // else
//     // {
//     //     m_textView->SetFirstLineIndex(pos-1);
//     //     m_textView->MoveFirstLineIndex(1);
//     // }

//     if (old_firstLine==pos)
//     {
//         event.Skip(); 
//         return;
//     }
//     else if (old_firstLine>pos && old_firstLine-pos<=SVID_SCROLLTHUMB_SMOOTH)       // for smmoth scoll
//     {
//         // m_textView->SetFirstLineIndex(pos+1);
//         m_textView->MoveFirstLineIndex(pos-old_firstLine);
//     }
//     else if (old_firstLine<pos && pos-old_firstLine<=SVID_SCROLLTHUMB_SMOOTH)      // for smmoth scoll
//     {
//         // m_textView->SetFirstLineIndex(pos-1);
//         m_textView->MoveFirstLineIndex(pos-old_firstLine);
//     }
//     else
//     {
//         m_textView->SetFirstLineIndex(pos);  // no smooth scroll
//     }

//     m_textView->Refresh();
//     m_textView->UpdateCaretsInfo();
//     m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_THUMB_TRACK));

//     m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());

//     DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);
//     this->SetFocus();
    event.Skip();    
}

void svListBoxCtrl::OnVsvScrollPageUp(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnVsvScrollPageUp"));
// #endif
//     if (!m_mouseReady) return;

//     m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
//     m_textView->UpdateCaretsInfo();

//     m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
//     m_textView->Refresh();
//     m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_UP));
//     // 重覆按鍵情況下加速捲動
//     if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_HIGH_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_NONE);
//     else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_LOW_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_LOW);
//     else
//         DoSmoothRefresh();
//     this->SetFocus();
    event.Skip();    
}

void svListBoxCtrl::OnVsvScrollPageDown(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnVsvScrollPageDown"));
// #endif
//     if (!m_mouseReady) return;

//     m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage());
//     m_textView->UpdateCaretsInfo();

//     m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
//     m_textView->Refresh();
//     m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_DOWN));
//     // 重覆按鍵情況下加速捲動
//     if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_NONE);
//     else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_LOW);
//     else
//         DoSmoothRefresh();
//     this->SetFocus();
    event.Skip();    
}

void svListBoxCtrl::OnHsvScrollThumbTrack(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnHsvScrollThumbTrack"));
// #endif
//     if (!m_mouseReady) return;

//     int newPos = event.GetPosition();
//     // int maxHExtend = m_hsb->GetRange()-(m_hsb->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
//     // if (newPos > maxHExtend) newPos = maxHExtend;
//     if (newPos < 0) newPos = 0;
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("OnHsvScrollThumbTrack ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
// #endif    
//     m_textView->SetHScrollPixel(newPos); 
//     // m_textView->UpdateCaretsInfo();

//     m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
//     // m_textView->Refresh();
//     m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_THUMB_TRACK));
//     DoSmoothRefresh(SVID_SMOOTH_NONE);
//     this->SetFocus();
    event.Skip();    
}

void svListBoxCtrl::OnHsvScrollPageUp(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnHsvScrollPageUp"));
// #endif
//     if (!m_mouseReady) return;

//     int newPos = m_textView->GetHScrollPixel() + (-1) * m_hsb->GetPageSize()*2/3;
//     if (newPos < 0) newPos = 0;
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("OnHsvScrollLineDown old new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
// #endif
//     m_textView->SetHScrollPixel(newPos); 
//     // m_textView->UpdateCaretsInfo();

//     m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
//     // m_textView->Refresh();

//     m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_UP));
//     // 重覆按鍵情況下加速捲動
//     if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_HIGH_DUP))
//         //DoSmoothRefresh(SVID_SMOOTH_NONE);
//         DoSmoothRefresh(SVID_SMOOTH_LOW);
//     else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_LOW_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_LOW);
//     else
//         //DoSmoothRefresh();
//         DoSmoothRefresh(SVID_SMOOTH_HIGH);

//     this->SetFocus();
    event.Skip();    
}

void svListBoxCtrl::OnHsvScrollPageDown(svScrollBarEvent& event)
{
// #ifndef NDEBUG
//     wxLogMessage(wxT("OnHsvScrollPageDown"));
// #endif
//     if (!m_mouseReady) return;

//     int newPos = m_textView->GetHScrollPixel() + m_hsb->GetPageSize()*1/3;
//     // int maxHExtend = m_hsb->GetRange()-(m_hsb->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
//     // if (newPos > maxHExtend) newPos = maxHExtend;
//     if (newPos < 0) newPos = 0;
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("OnHsvScrollLineDown ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
// #endif
//     m_textView->SetHScrollPixel(newPos); 
//     // m_textView->UpdateCaretsInfo();

//     m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
//     // m_textView->Refresh();
//     m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_DOWN));
//     // 重覆按鍵情況下加速捲動
//     if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_NONE);
//     else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
//         DoSmoothRefresh(SVID_SMOOTH_LOW);
//     else
//         DoSmoothRefresh();
//     this->SetFocus();
    event.Skip();    
}


void svListBoxCtrl::SetTheme(const wxColour p_fgColour, const wxColour p_bgColour, const wxColour p_cColour, const wxColour p_fColour, int p_fontSize)
{
    m_fgColour = p_fgColour;
    m_bgColour = p_bgColour;
    m_choseColour = p_cColour;
    m_filterColour = p_fColour;
    m_fontSize = p_fontSize;

    m_font.SetPointSize(m_fontSize);
    m_charHeight = m_font.GetPixelSize().GetHeight();
}

void svListBoxCtrl::SetFilter(const wxString &p_filter)
{
    m_filter = p_filter;
    m_filterItems.clear();
    if (m_filter.IsEmpty())
    {
        for (unsigned int i=0; i<m_items.size(); i++)
        {
            m_filterItems.push_back(i);
        }
    }
    else
    {
        // filtering.
        for (unsigned int i=0; i<m_items.size(); i++)
        {
            if (svCommonLib::wxStrMatchPattern(m_items.at(i), m_filter))
            {
                m_filterItems.push_back(i);
            }
        }
    }
}

void svListBoxCtrl::Append(const wxString &p_item)
{
    m_items.push_back(p_item);

    if (m_filter.IsEmpty())
    {
        m_filterItems.push_back(m_items.size()-1);
    }
    else
    {
        // filtering.
        if (svCommonLib::wxStrMatchPattern(m_items.back(), m_filter))
        {
            m_filterItems.push_back(m_items.size()-1);
        }
    }

}

void svListBoxCtrl::Insert(const wxString &p_item, unsigned int p_pos)
{
    if (p_pos>=m_items.size())
    {
        Append(p_item);
        return;
    }

    m_items.insert(m_items.begin()+p_pos, p_item);

    // filtering.
    if (svCommonLib::wxStrMatchPattern(p_item, m_filter))
    {
        int insertPos = -1;
        for (unsigned int i=0; i<m_filterItems.size(); i++)
        {
            if (m_filterItems.at(i)>=p_pos && insertPos==-1)
            {
                insertPos = i;
            }
            if (m_filterItems.at(i)>=p_pos)
            {
                m_filterItems.at(i) = m_filterItems.at(i) + 1;
            }
        }
    }
}

void svListBoxCtrl::Delete(unsigned int p_pos)
{
    if (p_pos>=m_items.size())
    {
        return;
    }

    m_items.erase(m_items.begin()+p_pos);

    // filtering.
    for (unsigned int i=0; i<m_filterItems.size(); i++)
    {
        if (m_filterItems.at(i)==p_pos)
        {
            m_filterItems.erase(m_filterItems.begin()+i);
            return;
        }
    }
}

void svListBoxCtrl::SetItems(const vector<wxString> &p_items)
{
    Clear();
    m_items = p_items;
    SetFilter(m_filter);
}
