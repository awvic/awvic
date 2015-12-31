/*
   Copyright Notice in awvic.cpp
*/

/* -------------------------------------------------------------------- *
 * awvic text control                     .                             *
 * -------------------------------------------------------------------- */

#include "svTextEditorCtrl.h"
#include <wx/dcbuffer.h>
#include <wx/log.h>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include "wx/filedlg.h"
#include "wx/file.h"
#include "wx/strconv.h"
#include "wx/encconv.h"
#include "wx/textfile.h"
#include <time.h>


#include "stdwx.h"
#include "svPreference.h"
#include "svAction.h"

#include <math.h>       /* sin */

#include "svMainFrame.h"

// m_bufText_EVENT
DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_EVENT)
DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_MODIFIED_CHANGED)
DEFINE_EVENT_TYPE(svEVT_TEXTCTRL_MSG)
IMPLEMENT_DYNAMIC_CLASS(svTextCtrlEvent, wxNotifyEvent)


BEGIN_EVENT_TABLE(svTextEditorCtrl, wxWindow)
EVT_PAINT(svTextEditorCtrl::OnPaint)
EVT_ERASE_BACKGROUND(svTextEditorCtrl::OnErase)
EVT_SIZE(svTextEditorCtrl::OnSize)
EVT_CHAR(svTextEditorCtrl::OnChar)
EVT_KEY_DOWN(svTextEditorCtrl::OnKeyDown)
EVT_CLOSE(svTextEditorCtrl::OnClose)
EVT_SET_FOCUS(svTextEditorCtrl::OnSetFocus)
#ifdef OS_SCROLLBAR
EVT_COMMAND_SCROLL_LINEUP(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollLineUp)
EVT_COMMAND_SCROLL_LINEDOWN(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollLineDown)
EVT_COMMAND_SCROLL_PAGEUP(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollPageUp)
EVT_COMMAND_SCROLL_PAGEDOWN(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollPageDown)
EVT_COMMAND_SCROLL_TOP(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollTop)
EVT_COMMAND_SCROLL_BOTTOM(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollBottom)
EVT_COMMAND_SCROLL_THUMBTRACK(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollThumbTrack)
EVT_COMMAND_SCROLL_THUMBRELEASE(SVID_VSCROLLBAR, svTextEditorCtrl::OnVScrollThumbRelease)
EVT_COMMAND_SCROLL_LINEUP(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollLineUp)
EVT_COMMAND_SCROLL_LINEDOWN(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollLineDown)
EVT_COMMAND_SCROLL_PAGEUP(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollPageUp)
EVT_COMMAND_SCROLL_PAGEDOWN(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollPageDown)
EVT_COMMAND_SCROLL_TOP(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollTop)
EVT_COMMAND_SCROLL_BOTTOM(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollBottom)
EVT_COMMAND_SCROLL_THUMBTRACK(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollThumbTrack)
EVT_COMMAND_SCROLL_THUMBRELEASE(SVID_HSCROLLBAR, svTextEditorCtrl::OnHScrollThumbRelease)
#else
EVT_SVSCROLLBAR_THUMBTRACK(SVID_SVVSB, svTextEditorCtrl::OnVsvScrollThumbTrack) 
EVT_SVSCROLLBAR_PAGEUP(SVID_SVVSB, svTextEditorCtrl::OnVsvScrollPageUp)
EVT_SVSCROLLBAR_PAGEDOWN(SVID_SVVSB, svTextEditorCtrl::OnVsvScrollPageDown)
EVT_SVSCROLLBAR_THUMBTRACK(SVID_SVHSB, svTextEditorCtrl::OnHsvScrollThumbTrack) 
EVT_SVSCROLLBAR_PAGEUP(SVID_SVHSB, svTextEditorCtrl::OnHsvScrollPageUp)
EVT_SVSCROLLBAR_PAGEDOWN(SVID_SVHSB, svTextEditorCtrl::OnHsvScrollPageDown)
#endif
EVT_LEFT_DOWN(svTextEditorCtrl::OnMouseLeftDown)
EVT_LEFT_UP(svTextEditorCtrl::OnMouseLeftUp)
EVT_RIGHT_UP(svTextEditorCtrl::OnMouseRightUp)
EVT_MOTION(svTextEditorCtrl::OnMouseMotion)
//EVT_MOTION(svTextEditorCtrl::OnMouseMotion)
EVT_MOUSEWHEEL(svTextEditorCtrl::OnMouseWheel)
//EVT_LEAVE_WINDOW(svTextEditorCtrl::OnMouseLeaveWindow)
//EVT_m_bufText_EVENT(wxID_ANY, svTextEditorCtrl::OnModified)
END_EVENT_TABLE()


svTextEditorCtrl::svTextEditorCtrl(svMainFrame *mainFrame, wxWindow *parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"))
:wxWindow(parent, id, pos, size, style, name)
{
    m_mainFrame = mainFrame;
    m_linesPerTxtArea = 0;

    m_cdai.Width = m_cdai.Height = 0;

    m_scb_type = SVID_TEXT_NO_SCB;

    m_hsbi.Width = m_hsbi.Height = 0;
    m_vsbi.Width = m_vsbi.Height = 0;

    m_hsbi.X = m_hsbi.Y = 0;
    m_vsbi.X = m_vsbi.Y = 0;

    m_tai.Width = m_tai.Height = 0;
    m_tai.X = m_tai.Y = 0;
    m_tai.OffsetX = 0;


    m_cniai.Width = m_cniai.Height = 0;
    m_cniai.X = m_cniai.Y = 0;

    m_lniai.Width = m_lniai.Height = 0;
    m_lniai.X = m_lniai.Y = 0;

    m_fai.Width = m_fai.Height = 0;
    m_fai.X = m_fai.Y = 0;

    m_border.Width = m_border.Height = 1;

#ifdef OS_SCROLLBAR

    vScrollbar = new wxScrollBar(this, SVID_VSCROLLBAR, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
    hScrollbar = new wxScrollBar(this, SVID_HSCROLLBAR, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);

    vScrollbar->Connect( SVID_VSCROLLBAR, wxEVT_ENTER_WINDOW, wxMouseEventHandler(svTextEditorCtrl::OnMouseEnterSB) );
    hScrollbar->Connect( SVID_HSCROLLBAR, wxEVT_ENTER_WINDOW, wxMouseEventHandler(svTextEditorCtrl::OnMouseEnterSB) );

    vScrollbar->GetSize(&(m_vsbi.Width), &(m_vsbi.Height));
    hScrollbar->GetSize(&(m_hsbi.Width), &(m_hsbi.Height));

#else

    m_vsb = new svScrollBar(this, SVID_SVVSB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "sb01");
    m_vsb->SetVertical(true);
    m_vsb->SetSize(10, 300);
    m_vsb->SetRange(100);
    m_vsb->SetPageSize(10);
    m_vsb->SetThumbPosition(0);
    // m_vsb->SetThumbSize(10);


    m_hsb = new svScrollBar(this, SVID_SVHSB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "sb02");
    m_hsb->SetVertical(false);
    m_hsb->SetSize(300, 10);
    m_hsb->SetRange(100);
    m_hsb->SetPageSize(10);
    m_hsb->SetThumbPosition(0);
    // m_hsb->SetThumbSize(10);

    m_vsb->GetSize(&(m_vsbi.Width), &(m_vsbi.Height));
    m_hsb->GetSize(&(m_hsbi.Width), &(m_hsbi.Height));

#endif

    m_commandLineCtrl = new svCommandLineCtrl(this, SVID_CMD_LINE, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, "Command Line");
    // m_commandLineCtrl->SetSize(500, 400);
    m_commandLineCtrl->Show(false);
    m_showCommandLine = false;

    // m_bufText = new svBufText();
    m_txtMaxW = 100L;
    m_setCaretOnTxtArea = false;

    m_selSX = m_selSY = m_selEX = m_selEY = 0;

    m_mouseX = m_mouseY = 0;
    m_areaI = 0;

    m_mouseLeftIsDown=false;
    m_wheelScrollLines=3;   // how many lines scrolled for a mouse wheel.

    m_modified=false;

    m_showDBGMSG=false;

    m_mode = SVID_TEXT_MODE;

    m_dbgcnt=0;

    m_pgmLang = wxT("");
    m_style = wxT("");

    // m_textStyle = new svTextStyle(wxT("text"));

    m_contextMenu = NULL;

    extern svPreference g_preference;
    m_tabWidth = g_preference.GetTabSize(); // shoud be variaty

    m_mouseReady = false;

    m_maxColumn = 200;
    m_lineWrap = 0;

    m_bufText = new svBufText();

    // default setting.
    // m_font = wxFont();
// #ifdef __WXMSW__
//     m_font.SetFaceName(wxT("Consolas"));
// #else
//     m_font.SetFaceName(wxT("Monospace"));
// #endif
    // m_font.SetPointSize(11);

    // m_font.SetFaceName(g_preference.GetFontFace());
    // m_font.SetPointSize(g_preference.GetFontSize());
    // m_font.SetStyle(wxFONTSTYLE_NORMAL);

    m_textView = NULL;
    m_spaceWidth = 0;
    m_charHeight = 0;
    SetupFont();

    m_hChar = m_font.GetPixelSize().GetHeight();
    m_rowInterval = (m_hChar * m_rowIntPct) / 100;


    // wxString fname = wxString::Format(wxT("style_%s.om"), m_style.Lower());
    wxString apppath;
    apppath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fp(apppath);
    wxString path = fp.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);



    m_textView = new svTextView();

    m_theme = new svTheme();
    // m_theme->LoadFile(path +wxT("simple.svthm"));
    // m_theme->LoadFile(g_preference.GetThemePath() +wxT("simple.svthm"));
    m_theme->LoadFile(g_preference.GetThemeFilePath());

#ifndef OS_SCROLLBAR
    m_vsb->SetTheme(m_theme);
    m_hsb->SetTheme(m_theme);
#endif

    // m_fnRules = NULL;

/*    char *syntaxName = NULL;
#ifdef __WXMSW__
    syntaxName = m_fnRules->GetRule(m_bufText->GetFileName().mb_str());
    wxString fn = path + wxString::FromAscii(syntaxName) + wxT(".svsyn");
    m_bufText->LoadSyntaxDesc(fn.mb_str());
#else
    syntaxName = m_fnRules->GetRule(m_bufText->GetFileName().ToUTF8());
    wxString fn = path + wxString(syntaxName, wxConvUTF8) + wxT(".svsyn");
    m_bufText->LoadSyntaxDesc(fn.ToUTF8());
#endif
    if (syntaxName) free(syntaxName);
*/


    // for smooth scroll testing.
    m_bTextAreaDC = NULL;
    m_bLineNumIndDC = NULL;
    m_bFoldingIndDC = NULL;
    m_bColNumIndDC = NULL;
    m_bufferDC = NULL;
    m_bTextAreaDC_x = m_bTextAreaDC_y = 0;


    m_mouseMotionTimer = new svMouseMotionTimer(this);

    // m_typeHintCtrl = new svTypeHintCtrl(m_mainFrame, this, wxID_ANY, wxDefaultPosition, wxSize(150, 140), wxBORDER_NONE|wxCLIP_CHILDREN|wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT );
    m_typeHintCtrl = new svTypeHintCtrl(this, this, wxID_ANY, wxDefaultPosition, wxSize(150, 140), wxBORDER_NONE|wxCLIP_CHILDREN|wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT );

    // m_typeHintCtrl->Bind(wxEVT_KEY_DOWN, &svTextEditorCtrl::OnKeyDown, this);
    // m_typeHintCtrl->Bind(wxEVT_CHAR, &svTextEditorCtrl::OnChar, this);

}

svTextEditorCtrl::~svTextEditorCtrl()
{
#ifdef OS_SCROLLBAR
    if (hScrollbar) delete hScrollbar;
    if (vScrollbar) delete vScrollbar;
#else
    if (m_vsb) delete m_vsb;
    if (m_hsb) delete m_hsb;
#endif
    if (m_bufText) delete m_bufText;
    if (m_textView) delete m_textView;

    if (m_commandLineCtrl) delete m_commandLineCtrl;
    // delete m_textEdit;
    //delete m_act;
    // delete m_actList;
    if (m_contextMenu) delete m_contextMenu;
    // if (m_textStyle!=NULL) delete m_textStyle;
    if (m_theme) delete m_theme;

    if (m_bTextAreaDC) delete m_bTextAreaDC;
    if (m_bLineNumIndDC) delete m_bLineNumIndDC;
    if (m_bFoldingIndDC) delete m_bFoldingIndDC;
    if (m_bColNumIndDC) delete m_bColNumIndDC;
    if (m_bufferDC) delete m_bufferDC;
    // m_wxCaretList.clear();
    // delete caret;
    delete m_mouseMotionTimer;

    if (m_typeHintCtrl) delete m_typeHintCtrl;

}

void svTextEditorCtrl::OnErase(wxEraseEvent& evet)
{
}

void svTextEditorCtrl::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    if (!m_mouseReady)            // also means first time load.
    {
        m_mouseReady = true;
    }

    if (m_bufferDC)
    {
        wxCoord tw, th;
        dc.GetSize (&tw, &th);

        dc.SetClippingRegion(0, 0, tw, th);
        dc.Blit(0, 0, tw, th, m_bufferDC, 0, 0);
    }
    else
    {
        dc.DrawText("Stange!", 0, 0);
    }

}

void svTextEditorCtrl::OnSize(wxSizeEvent& event)
{
    // every page on wxAuiNotebook receive OnSize event when wxAuiNotebook Resize.
    // But that slow down the performance.
    // So, We only resize the svTextEditor when it's visible on screen.
    // And manul resie it when it focused.
    if (IsShownOnScreen())  
    {
        DoOnSize();
    }

}

void svTextEditorCtrl::DoOnSize(void)
{
    GetClientSize(&m_cdai.Width, &m_cdai.Height);

    DoResizeChildArea();

    //repaint client area.
    m_textView->Refresh();
    DoSmoothRefresh();
}

void svTextEditorCtrl::DoResizeChildArea(void)
{
    // Cal Text Area Info.

    // Resize Scrollbar.
    m_hsbi.Width = m_cdai.Width - m_vsbi.Width;
    // m_hsbi.Width = m_cdai.Width - m_vsbi.Width - (m_border.Width*2);
    m_vsbi.Height = m_cdai.Height - m_hsbi.Height;
    // m_vsbi.Height = m_cdai.Height - m_hsbi.Height - (m_border.Height*2);

    // m_hsbi.X = m_border.Width - 1;
    m_hsbi.X = 0;
    m_hsbi.Y = m_cdai.Height - m_hsbi.Height - m_border.Height + 1; // - 2;
    m_vsbi.X = m_cdai.Width - m_vsbi.Width - m_border.Height + 1; // - 2;
    // m_vsbi.Y = m_border.Height - 1;
    m_vsbi.Y = 0;

#ifdef OS_SCROLLBAR
    vScrollbar->SetSize(wxRect(m_vsbi.X, m_vsbi.Y, m_vsbi.Width, m_vsbi.Height));
    hScrollbar->SetSize(wxRect(m_hsbi.X, m_hsbi.Y, m_hsbi.Width, m_hsbi.Height));
    // vScrollbar->GetSize(&(m_vsbi.Width), &(m_vsbi.Height));
    // hScrollbar->GetSize(&(m_hsbi.Width), &(m_hsbi.Height));
#else
    m_vsb->SetSize(wxRect(m_vsbi.X, m_vsbi.Y, m_vsb->GetDefWidth(), m_vsbi.Height));
    m_hsb->SetSize(wxRect(m_hsbi.X, m_hsbi.Y, m_hsbi.Width, m_hsb->GetDefHeight()));
    // m_vsb->GetSize(&(m_vsbi.Width), &(m_vsbi.Height));
    // m_hsb->GetSize(&(m_hsbi.Width), &(m_hsbi.Height));
#endif

    // m_cniai => column number indicator area infomation
    m_cniai.Width = m_cdai.Width - (m_border.Width*2);
    m_cniai.Height = (m_hChar / 2); // + (m_rowInterval * 2); 
    m_cniai.X = m_border.Width  - 1;
    m_cniai.Y = m_border.Height - 1;

    // Keeping svCommandLineCtrl's height and varity the width according to the svTextEditorCtrl's width.
    m_commandLineCtrl->SetSize(m_cdai.Width/5*3, m_commandLineCtrl->GetSize().GetHeight());
    m_commandLineCtrl->SetPosition(wxPoint(m_cdai.Width/5+1, 0));

    DoResizeLineNoIndicatorArea();
}


void svTextEditorCtrl::DoResizeLineNoIndicatorArea(void)
{

    // m_lniai => line number indicator area information.
    // 下一行有雞生蛋、蛋生雞問題，當 svTextEditorCtrl initial 時, m_textView 的狀態是未知的，執行下一行會產生不可預期的錯誤
    if (m_textView->HasBuffer())
    {
        m_lniai.Width = m_textView->GetLineNoIndicatorAreaMaxWidth();
    }
    else
    {
        m_lniai.Width = 0; // svTextEditorCtrl initial and first time OnSize event.
    }
    m_lniai.Height = m_cdai.Height - (m_border.Height*2) - m_cniai.Height - m_hsbi.Height;
    m_lniai.X = m_border.Width - 1; 
    m_lniai.Y = m_border.Height + m_cniai.Height - 1;
    m_lniai.BorderWidth = 5;

    m_fai.Height = m_cdai.Height - (m_border.Height*2) - m_cniai.Height - m_hsbi.Height;
    m_fai.Width = SVID_FOLDING_AREA_WIDTH;
    m_fai.X = m_border.Width - 1 + m_lniai.Width; 
    m_fai.Y = m_border.Height + m_cniai.Height - 1;
    m_fai.BorderWidth = 0;

    // m_tai => text area information
    m_tai.Width = m_cdai.Width - (m_border.Width*2) - m_lniai.Width - m_vsbi.Width;
    m_tai.Height = m_cdai.Height - (m_border.Height*2) - m_cniai.Height - m_hsbi.Height;
    m_tai.X = m_border.Width + m_lniai.Width + m_fai.Width - 1;
    m_tai.Y = m_border.Height + m_cniai.Height - 1;

    m_linesPerTxtArea = m_tai.Height / (m_hChar + m_rowInterval);

    m_textView->SetPageLines(m_linesPerTxtArea);
    extern svPreference g_preference;
    if (g_preference.GetIsWrap())
    {
        if (g_preference.GetWrapColumn()==0)
        {
            m_textView->SetMaxPixelWidth(m_tai.Width - m_vsbi.Width, g_preference.GetIsWrap());
        }
        else
        {
            // Note that under wxMSW if you passed to SetPixelSize() (or to the ctor) a wxSize object with a null width value, you'll get a null width in the returned object.
            m_textView->SetMaxPixelWidth(g_preference.GetWrapColumn() * m_spaceWidth, g_preference.GetIsWrap());
        }
    }
    else
    {
        m_textView->SetMaxPixelWidth(m_tai.Width, g_preference.GetIsWrap());
    }
    m_textView->SetVisiblePixelWidth(m_tai.Width);
}

void svTextEditorCtrl::OnChar(wxKeyEvent& event)
{

    svCommand* cmd;
    cmd = NULL;
    // switch(m_mode)
    // {
    // case SVID_TEXT_MODE:
    //     cmd = EmergeTextCommand(event);
    //     ProcessTextCommand(cmd);
    //     break;
    // case SVID_VI_MODE:
    //     //cmd = EmergeVICommand(&event);
    //     //ProcessVICommand(cmd);
    //     break;
    // case SVID_HEX_MODE:
    //     //cmd = EmergeHEXCommand(&event);
    //     //ProcessHEXCommand(cmd);
    //     break;
    // }

    cmd = EmergeTextCommand(event);
    ProcessTextCommand(cmd);


    if (cmd) // cmd != NULL  <= I am a C/C++ novice. Reminding myself.
    { 
        delete cmd; 
        cmd=NULL;
    }
    // Refresh();

    // user keeping page up or page down
    // disable smooth scrool to fast the screen reaction.
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_PAGE_UP, SVID_HIGH_DUP) ||
        m_textView->DuplicatedAction(SVID_ACTION_PAGE_DOWN, SVID_HIGH_DUP))
        //DoSmoothRefresh(SVID_SMOOTH_NONE);
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else if (m_textView->DuplicatedAction(SVID_ACTION_PAGE_UP, SVID_LOW_DUP) ||
        m_textView->DuplicatedAction(SVID_ACTION_PAGE_DOWN, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        //DoSmoothRefresh();
        DoSmoothRefresh(SVID_SMOOTH_HIGH);

    // event.Skip() means the event will be processed by the parent control.
    // Don't call event.Skip() if everything you want had been processed.
    
    // if (cmd->Name()!=CMD_TXT_UP &&
    //     cmd->Name()!=CMD_TXT_DOWN)
    // {
    //     event.Skip();
    // }

}

void svTextEditorCtrl::OnKeyDown(wxKeyEvent& event)
{

    svCommand* cmd;
    cmd = NULL;

    cmd = EmergeTextCommand2(event);
    if (cmd) // cmd != NULL  <= I am a C/C++ novice. Reminding myself.
    {
        ProcessTextCommand(cmd);

        delete cmd; 
        cmd=NULL;

        DoSmoothRefresh(SVID_SMOOTH_NONE);
    }
    else
    {
        event.Skip();    
    }

}

/*
 * Process TEXT MODE input
 */
svCommand* svTextEditorCtrl::EmergeTextCommand(const wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    wxChar ukey = event.GetUnicodeKey();
    int cmdName;


    m_setCaretOnTxtArea = true;

    cmdName = CMD_UNDEFINED;

    if (key == WXK_LEFT)
    {
        if (event.ControlDown())
            cmdName = CMD_TXT_LEFT_HEAD;
        else
            cmdName = CMD_TXT_LEFT;
    }
    else if (key == WXK_RIGHT)
    {
        if (event.ControlDown())
            cmdName = CMD_TXT_RIGHT_END;
        else
            cmdName = CMD_TXT_RIGHT;
    }
    else if (key == WXK_UP)
    {
        cmdName = CMD_TXT_UP;
    }
    else if (key == WXK_DOWN)
    {
        cmdName = CMD_TXT_DOWN;
    }
    else if (key == WXK_PAGEUP)
    {
        cmdName = CMD_TXT_PAGEUP;
    }
    else if (key == WXK_PAGEDOWN)
    {
        cmdName = CMD_TXT_PAGEDOWN;
    }
    else if (key == WXK_END)
    {
        if (event.ControlDown())
        cmdName = CMD_TXT_BOTTOM;
        else
        cmdName = CMD_TXT_LINE_END;
    }
    else if (key == WXK_HOME)
    {
        if (event.ControlDown())
        cmdName = CMD_TXT_TOP;
        else
        cmdName = CMD_TXT_LINE_HEAD;
    }
    else if (key == WXK_RETURN)
    {
        cmdName = CMD_TXT_SPLIT;
    }
    else if (key == WXK_DELETE)
    {
        cmdName = CMD_TXT_DELETE;
    }
    else if (key == WXK_BACK)
    {
        cmdName = CMD_TXT_BACKDEL;
    }
    else if (key == WXK_ESCAPE)
    {
        if (m_typeHintCtrl->IsShown())
        {
            cmdName = CMD_TXT_HIDE_HINT;   // Hide typing hint window.
        }
        else if (m_commandLineCtrl->IsShown())
        {
            cmdName = CMD_TXT_HIDE_COMMAND_LINE;   // Hide command line window.
        }
        else if (m_mainFrame->FindWindowIsVisible())
        {
            cmdName = CMD_TXT_HIDE_FIND;   // Show or hide find replace windows.
        }
        else
            cmdName = CMD_TXT_RESET_CARETS;     // Reset carets => keep first caret and remove else.  
    }
    else if (key == WXK_F10)
    {
        cmdName = CMD_TXT_DEBUG01;
    }
    else if (key == WXK_F11)
    {
        cmdName = CMD_TXT_DEBUG02;
    }
    else if (key == WXK_F3)
    {
        if (event.ControlDown())
            cmdName = CMD_TXT_FIND_CURRENT_WORD;         // Ctrl+F3
        else if (event.ShiftDown())
            cmdName = CMD_TXT_FIND_PREV_WORD;            // Shift+F3
        else if (event.AltDown())
            cmdName = CMD_TXT_FIND_CURRENT_WORD_ALL;     // Alt+F3 
        else
            cmdName = CMD_TXT_FIND_NEXT_WORD;            // F3
    }
    else if (key == WXK_TAB)
    {
        cmdName = CMD_TXT_INS_TAB;
    }
    else if (key>0 && key<27)  // ASCII control code
    {
        char charkey = 'A' + key - 1;
        if (event.ControlDown() && charkey == 'C')  // Ctrl+C
        {
            cmdName = CMD_TXT_COPY;
        }
        else if (event.ControlDown() && charkey == 'S')  // Ctrl+S
        {
            cmdName = CMD_TXT_SAVE;
        }
        else if (event.ControlDown() && charkey == 'V') // Ctrl+V
        {
            cmdName = CMD_TXT_PASTE;
        }
        else if (event.ControlDown() && charkey == 'X') // Ctrl+X
        {
            cmdName = CMD_TXT_CUT;
        }
        // else if (event.ControlDown() && charkey == 'L') // Ctrl+L for debug
        // {
        //     cmdName = CMD_TXT_TOP_SCREEN;
        // }
        // else if (event.ControlDown() && charkey == 'O') // Ctrl+O for debug
        // {
        //     cmdName = CMD_TXT_MIDDLE_SCREEN;
        // }
        else if (event.ControlDown() && charkey == 'G') // Ctrl+G 
        {
            cmdName = CMD_SHOW_COMMAND_LINE_GOTO;
        }
        else if (event.ControlDown() && charkey == 'P') // Ctrl+P 
        {
            cmdName = CMD_SHOW_COMMAND_LINE;
        }
        else if (event.ControlDown() && charkey == 'R') // Ctrl+R 
        {
            cmdName = CMD_SHOW_COMMAND_LINE_GOTO_DEFINITION;
        }
        else if (event.ControlDown() && charkey == 'Z') // Ctrl+Z
        {
            cmdName = CMD_TXT_UNDO;
        }
        else if (event.ControlDown() && charkey == 'F') // Ctrl+F
        {
            cmdName = CMD_TXT_SEARCH;
        }
        else if (event.ControlDown() && charkey == 'B') // Ctrl+F
        {
            cmdName = CMD_TXT_SIDEBAR;
        }
        else if (event.ControlDown() && event.ShiftDown() && charkey == 'D') // Ctrl+Shift+D
        {
            cmdName = CMD_TXT_DUP_LINE;
        }
    }
    else if (key>=32 && key<127) // ASCII printable code
    {
        // unicode 在這段資料與 ASCII 相同
        // cmdName = CMD_TXT_INSERT;

        if (event.ControlDown() && key == 47) // Ctrl+/
        {
            wxLogMessage(wxString::Format(wxT("svTextEditorCtrl:: Ctrl+ // EmergeTextCommand code: 0x%i"), key));
            cmdName = CMD_TXT_LINE_COMMENT;
        }
        else
        {
            cmdName = CMD_TXT_INS_UNI;    
        }
    }
    else if (ukey>=0x100) // UNICODE < 0x100 is C0 Controls, Basic Latin, C1 Controls, Latin-1 Supplement
    {
        cmdName = CMD_TXT_INS_UNI;
    }
    else
    {
        wxLogMessage(wxString::Format(wxT("svTextEditorCtrl::EmergeTextCommand error: unhandled key code: 0x%x"), ukey));
    }

    svCommand* cmd = new svCommand(cmdName, event);
    return cmd;

}

/*
 * Process TEXT MODE input
 */
svCommand* svTextEditorCtrl::EmergeTextCommand2(const wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    wxChar ukey = event.GetUnicodeKey();
    int cmdName;

    if (key==47)
    {
        if (event.ControlDown() && event.ShiftDown()) // Ctrl+Shift+/
        {
            cmdName = CMD_TXT_BLOCK_COMMENT;

            svCommand* cmd = new svCommand(cmdName, event);
            return cmd;
        }
        else if (event.ControlDown()) // Ctrl+/
        {
            cmdName = CMD_TXT_LINE_COMMENT;

            svCommand* cmd = new svCommand(cmdName, event);
            return cmd;
        }

        // Question: Where does '/' goes?
    }
    else if (key==91)
    {
        if (event.ControlDown()) // Ctrl+[
        {
            cmdName = CMD_TXT_OUTDENT;

            svCommand* cmd = new svCommand(cmdName, event);
            return cmd;
        }
    }
    else if (key==93)
    {
        if (event.ControlDown()) // Ctrl+]
        {
            cmdName = CMD_TXT_INDENT;

            svCommand* cmd = new svCommand(cmdName, event);
            return cmd;
        }
    }

    return NULL;

}

void svTextEditorCtrl::ProcessTextCommand(svCommand* cmd)
{
    switch(cmd->Name())
    {
    case CMD_TXT_LEFT:
    case CMD_TXT_LEFT_HEAD:
    case CMD_TXT_RIGHT:
    case CMD_TXT_RIGHT_END:
    case CMD_TXT_UP:
    case CMD_TXT_DOWN:
    case CMD_TXT_PAGEUP:
    case CMD_TXT_PAGEDOWN:
    case CMD_TXT_LINE_END:
    case CMD_TXT_LINE_HEAD:
    case CMD_TXT_TOP:
    case CMD_TXT_BOTTOM:
    case CMD_TXT_CUR_TOP_SCREEN:
    case CMD_TXT_CUR_MIDDLE_SCREEN:
    case CMD_TXT_CUR_BOTTOM_SCREEN:
    case CMD_TXT_TOP_SCREEN:
    case CMD_TXT_MIDDLE_SCREEN:
    case CMD_TXT_BOTTOM_SCREEN:
        DoTextNavi(cmd);
        break;
    case CMD_TXT_SPLIT:
        DoTextSplitLine(cmd);
        break;
    // case CMD_TXT_INSERT:
    //     DoTextInsert(cmd);
    //     break;
    case CMD_TXT_INS_UNI:
        DoTextInsert(cmd);
        // DoTextInsertUnicode(cmd);
        break;
    case CMD_TXT_DELETE:
        DoTextDelete(cmd);
        break;
    case CMD_TXT_BACKDEL:
        DoTextBackDelete(cmd);
        break;
    case CMD_TXT_COPY:
        DoTextCopy(cmd);
        break;
    case CMD_TXT_PASTE:
        DoTextPaste(cmd);
        break;
    case CMD_TXT_CUT:
        DoTextCut(cmd);
        break;
    case CMD_TXT_DUP_LINE:
        DoTextDuplicateLine(cmd);
        break;
    case CMD_TXT_UNDO:
        DoTextUndo(cmd);
        break;
    case CMD_TXT_INS_TAB:
        DoTextInsert(cmd);
        break;
    case CMD_TXT_LINE_COMMENT:
        DoTextLineComment(cmd);
        break;
    case CMD_TXT_BLOCK_COMMENT:
        DoTextBlockComment(cmd);
        break;
    case CMD_TXT_INDENT:
        DoTextIndent(cmd);
        break;
    case CMD_TXT_OUTDENT:
        DoTextOutdent(cmd);
        break;
    case CMD_TXT_RESET_CARETS:
        DoTextResetCarets(cmd);
        break;
    case CMD_TXT_HIDE_HINT:
        HideTypeHint();
        break;
    case CMD_TXT_HIDE_COMMAND_LINE:
        DoShowHideCommandLine(SVID_CMD_REGULAR);
        break;
    case CMD_TXT_HIDE_FIND:
        m_mainFrame->HideFindWindow();
        break;
    // case CMD_TXT_DEBUG01:
    //     m_actList->ShowList();
    //     break;
    // case CMD_TXT_DEBUG02:
    //     m_showDBGMSG=!m_showDBGMSG;
    //     break;
    case CMD_TXT_SAVE:
        // SaveFile();
        m_mainFrame->DoOnMenuSave();
        break;
    case CMD_TXT_FIND_CURRENT_WORD:
        // DoFindCurrentWord(cmd);
        DoFindCurrentWord();
        break;
    case CMD_TXT_FIND_CURRENT_WORD_ALL:
        // DoFindCurrentWordAll(cmd);
        DoFindCurrentWord();
        DoFindAll();
        break;
    case CMD_TXT_FIND_NEXT_WORD:
        DoFindNext();
        break;
    case CMD_TXT_FIND_PREV_WORD:
        DoFindPrev();
        break;
    case CMD_TXT_SEARCH:
        m_mainFrame->ShowFindWindow();
        break;
    case CMD_TXT_SIDEBAR:
        m_mainFrame->ShowHideSideBar();
        break;
    case CMD_SHOW_COMMAND_LINE_GOTO:
        DoShowHideCommandLine(SVID_CMD_GOTO_LINE);
        break;
    case CMD_SHOW_COMMAND_LINE:
        DoShowHideCommandLine(SVID_CMD_REGULAR);
        break;
    case CMD_SHOW_COMMAND_LINE_GOTO_DEFINITION:
        DoShowHideCommandLine(SVID_CMD_GOTO_DEFINITION);
        break;
    //case CMD_TXT_DEBUG01:
        //ShowTypeHint(m_bufText->GetAvailableHint());
        // m_mainFrame->ShowTypeHint(m_bufText->GetAvailableHint());
        break;
    case CMD_TXT_DEBUG02:
        // m_mainFrame->HideTypeHint();
        break;
    }

    CheckTextModified();

}

void svTextEditorCtrl::CheckTextModified(void)
{
    if (m_modified!=m_bufText->IsModified())
    {
        svTextCtrlEvent event(svEVT_TEXTCTRL_MODIFIED_CHANGED, GetId());
        event.Modified(m_bufText->IsModified());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);

        m_modified = m_bufText->IsModified();
    }
}

void svTextEditorCtrl::DoTextNavi(svCommand* cmd)
{

    if (!cmd->ShiftDown())
    {
        m_textView->ClearCaretSelect();
    }
    else
    {
        m_textView->SetCaretSelect();
    }

    // m_setCaretOnTxtArea = true;

    // int tmpX, tmpY;

    switch (cmd->Name())
    {
    case CMD_TXT_LEFT:
        m_textView->CaretsLeft();
        if (m_typeHintCtrl->IsShown() &&
            m_bufText->GetAvailableHintCnt()>0)
        {}
        else HideTypeHint();
        break;
        
    case CMD_TXT_LEFT_HEAD:
        m_textView->CaretsLeftHead();
        if (m_typeHintCtrl->IsShown() &&
            m_bufText->GetAvailableHintCnt()>0)
        {}
        else HideTypeHint();
        break;

    case CMD_TXT_RIGHT:
        m_textView->CaretsRight();
        if (m_typeHintCtrl->IsShown() &&
            m_bufText->GetAvailableHintCnt()>0)
        {}
        else HideTypeHint();
        break;

    case CMD_TXT_RIGHT_END:
        m_textView->CaretsRightEnd();
        if (m_typeHintCtrl->IsShown() &&
            m_bufText->GetAvailableHintCnt()>0)
        {}
        else HideTypeHint();
        break;

    case CMD_TXT_UP:
        HideTypeHint();
        m_textView->CaretsUp();
        break;

    case CMD_TXT_DOWN:
        HideTypeHint();
        m_textView->CaretsDown();
        break;

    case CMD_TXT_PAGEUP:
        HideTypeHint();
        m_textView->PageUp();
        break;

    case CMD_TXT_PAGEDOWN:
        HideTypeHint();
        m_textView->PageDown();
        break;

    case CMD_TXT_LINE_END:
        HideTypeHint();
        m_textView->CaretsLineEnd();
        break;

    case CMD_TXT_LINE_HEAD:
        HideTypeHint();
        m_textView->CaretsLineHead();
        break;

    // case CMD_TXT_TOP:
    //     m_displayText->MoveTop(m_txtX, m_txtY, true);
    //     break;

    // case CMD_TXT_BOTTOM:
    //     m_displayText->MoveBottom(m_txtX, m_txtY, true);
    //     break;
        
    // case CMD_TXT_CUR_TOP_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_zt, true);
    //     break;
        
    // case CMD_TXT_CUR_MIDDLE_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_zz, true);
    //     break;
        
    // case CMD_TXT_CUR_BOTTOM_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_zb, true);
    //     break;
        
    // case CMD_TXT_TOP_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_H, true);
    //     break;
        
    // case CMD_TXT_MIDDLE_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_M, true);
    //     break;
        
    // case CMD_TXT_BOTTOM_SCREEN:
    //     m_displayText->Move(m_txtX, m_txtY, SVID_L, true);
    //     break;
        
    }

    // if (cmd->ShiftDown())
    // {
    //     SetSelectedEnd();
    // }

}


void svTextEditorCtrl::DoTextInsert(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingInsertChar(wxString(ukey));

    if (m_bufText->GetAvailableHintCnt()>0)
    {
        vector<wxString> hintList = m_bufText->GetAvailableHint();
        ShowTypeHint(hintList);
        int p_x, p_y;
        if (m_textView->GetLastCaretWordPixelXY(p_x, p_y, m_hChar+m_rowInterval))
        {
            p_x += m_lniai.Width + m_fai.Width;
            p_y += m_cniai.Height;
            ClientToScreen(&p_x, &p_y);
            m_typeHintCtrl->Move(p_x, p_y);
        }
    }
    else
        HideTypeHint();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoTextInsertHint(const wxString p_str)
{
    int currentWordLen=0;
    int currentWordOffset=0;
    if (m_bufText->GetCaretsCurrentUCharLen(currentWordLen, currentWordOffset))
    {
        m_textView->EditingInsertHintChar(p_str, currentWordLen, currentWordOffset);

        if (m_bufText->GetAvailableHintCnt()>0)
        {
            vector<wxString> hintList = m_bufText->GetAvailableHint();
            ShowTypeHint(hintList);
            int p_x, p_y;
            if (m_textView->GetLastCaretWordPixelXY(p_x, p_y, m_hChar+m_rowInterval))
            {
                p_x += m_lniai.Width + m_fai.Width;
                p_y += m_cniai.Height;
                ClientToScreen(&p_x, &p_y);
                m_typeHintCtrl->Move(p_x, p_y);
            }
        }
        else
            HideTypeHint();
        m_textView->Refresh();
        DoSmoothRefresh(SVID_SMOOTH_LOW);
        // m_textView->CaretsRight();
    }
}

void svTextEditorCtrl::DoTextSplitLine(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingSplitLine();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoTextDelete(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextDelete();

    if (m_typeHintCtrl->IsShown())
    {
        if (m_bufText->GetAvailableHintCnt()>0)
        {
            vector<wxString> hintList = m_bufText->GetAvailableHint();
            ShowTypeHint(hintList);
            int p_x, p_y;
            if (m_textView->GetLastCaretWordPixelXY(p_x, p_y, m_hChar+m_rowInterval))
            {
                p_x += m_lniai.Width + m_fai.Width;
                p_y += m_cniai.Height;
                ClientToScreen(&p_x, &p_y);
                m_typeHintCtrl->Move(p_x, p_y);
            }
        }
        else
        {
            HideTypeHint();
        }
    }

    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoTextBackDelete(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextBackDelete();

    if (m_typeHintCtrl->IsShown())
    {
        if (m_bufText->GetAvailableHintCnt()>0)
        {
            vector<wxString> hintList = m_bufText->GetAvailableHint();
            ShowTypeHint(hintList);
            int p_x, p_y;
            if (m_textView->GetLastCaretWordPixelXY(p_x, p_y, m_hChar+m_rowInterval))
            {
                p_x += m_lniai.Width + m_fai.Width;
                p_y += m_cniai.Height;
                ClientToScreen(&p_x, &p_y);
                m_typeHintCtrl->Move(p_x, p_y);
            }
        }
        else
        {
            HideTypeHint();
        }
    }
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoTextCopy(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextCopySelected();
    // m_textView->Refresh();
}


void svTextEditorCtrl::DoTextPaste(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextPaste();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextCut(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextCut();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextDuplicateLine(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextDuplicateLine();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextLineComment(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextLineComment();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextBlockComment(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextBlockComment();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextIndent(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextIndent();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextOutdent(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextOutdent();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextResetCarets(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->EditingTextResetCarets();
    // m_textView->Refresh();
}

void svTextEditorCtrl::DoTextUndo(svCommand* cmd)
{
    int key = cmd->GetKeyCode();
    wxChar ukey = cmd->GetUnicodeKey();

    m_textView->UndoEditing();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoFindMatchLocations(const svFindReplaceOption &p_frOption)
{
    m_textView->FindMatchLocations(p_frOption);
    DoSmoothRefresh(SVID_SMOOTH_NONE);
}

void svTextEditorCtrl::DoFindCurrentWord(void)
{
    svFindReplaceOption p_fro;
    p_fro.m_from = svFindReplaceOption::SVID_FIND_FROM_KEYSTROKE;
    m_textView->FindMatchLocations(p_fro);
}

void svTextEditorCtrl::DoFindNext(void)
{
    m_textView->FindNext();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoFindPrev(void)
{
    m_textView->FindPrev();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoFindAll(void)
{
    m_textView->FindAll();
    // m_textView->Refresh();
    // m_textView->CaretsRight();
}

void svTextEditorCtrl::DoReplaceNext(const svFindReplaceOption &p_frOption)
{
    m_textView->ReplaceNext(p_frOption);
    DoSmoothRefresh();
    CheckTextModified();
}

void svTextEditorCtrl::DoReplacePrev(const svFindReplaceOption &p_frOption)
{
    m_textView->ReplacePrev(p_frOption);
    DoSmoothRefresh();
    CheckTextModified();
}

void svTextEditorCtrl::DoReplaceAll(const svFindReplaceOption &p_frOption)
{
    m_textView->ReplaceAll(p_frOption);
    DoSmoothRefresh();
    CheckTextModified();
}

void svTextEditorCtrl::DoShowHideCommandLine(char p_commandType)
{
    if (m_commandLineCtrl->IsShown())
    {
        m_commandLineCtrl->Show(false);
    }
    else
    {
        m_commandLineCtrl->SetCommandType(p_commandType);
        m_commandLineCtrl->Show(true);
        m_commandLineCtrl->SetFocus();
    }
    DoSmoothRefresh();
}

void svTextEditorCtrl::OnClose(wxCloseEvent& event)
{
    svFileDescOpened fdo;
    fdo.m_fullPathName = m_fileDesc.m_fullPathName;
    fdo.m_displayName = m_fileDesc.m_displayName;
    fdo.AddCarets(m_bufText->GetCaretsList());
    fdo.m_firstLineNo = m_textView->GetFirstUWLineNo();

    if (m_bufText->IsModified())
    {
        int ans;
        // wxMessageDialog msgdlg(this, _("Save file before closing?"), _("Message"), wxYES_NO|wxCANCEL);
        wxMessageDialog msgdlg(this, m_fileDesc.m_displayName + _(" is modified, save it before closing?"), _("Message"), wxYES_NO|wxCANCEL);
        ans = msgdlg.ShowModal();
        switch(ans){
        case wxID_YES:
            SaveFile();
            m_mainFrame->AddCloseStatusBuffer(fdo);
            break;
        case wxID_NO:
            m_mainFrame->AddCloseStatusBuffer(fdo);
            break;
        case wxID_CANCEL:
            event.Veto();
            break;
        }
    }
    else
    {
        fdo.m_fullPathName = m_fileDesc.m_fullPathName;
        fdo.m_displayName = m_fileDesc.m_displayName;
        fdo.AddCarets(m_bufText->GetCaretsList());
        fdo.m_firstLineNo = m_textView->GetFirstUWLineNo();
        m_mainFrame->AddCloseStatusBuffer(fdo);
    }
}

void svTextEditorCtrl::OnSetFocus(wxFocusEvent& event)
{
    DoOnSize();
    // m_bufText->SetBufUCharAllDirty();  // eaziest way, not the best way.
    // when svTextEditorCtrl get focused. set svBufText.m_bufUCharAll to dirty.
    //m_mainFrame->ChangeCurrentsvTextEditorCtrl(m_fileDesc);

}

void svTextEditorCtrl::OnMouseEnterSB(wxMouseEvent& event)
{
    SetCursor(wxCursor(wxCURSOR_ARROW));
    event.Skip();
}


// --------------------------------------------------------------------------------------- //
void svTextEditorCtrl::AdjustScrollbar(void)
{
#ifdef OS_SCROLLBAR
    vScrollbar->SetScrollbar(m_textView->GetFirstUWLineNo(), m_textView->GetLinesPerPage(), m_bufText->LineCntUW(), m_textView->GetLinesPerPage());
    hScrollbar->SetScrollbar(m_textView->GetHScrollPixel(), m_textView->GetVisiblePixelWidth(),  m_textView->GetMaxPixelWidth() + (m_textView->GetVisiblePixelWidth()*1/3), m_textView->GetVisiblePixelWidth());
#else
    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_vsb->SetPageSize(m_textView->GetLinesPerPage());
    m_vsb->SetRange(m_bufText->LineCntUW());

    m_hsb->SetThumbPosition(m_textView->GetHScrollPixel());
    m_hsb->SetPageSize(m_textView->GetVisiblePixelWidth());
    // m_hsb->SetRange(m_textView->GetMaxPixelWidth() + (m_textView->GetVisiblePixelWidth()*1/3));
    m_hsb->SetRange(m_textView->GetMaxPixelWidth());
#endif
}

bool svTextEditorCtrl::InitNewFile(const wxString& filename)
{
#ifdef OS_SCROLLBAR
    hScrollbar->Show(false);
    vScrollbar->Show(false);
#else
    m_vsb->Show(false);
    m_hsb->Show(false);
#endif
    DisplayLoadingBackground("Preparing file...", 0);

    ResetSyntaxHilight(filename);

    if (!m_bufText->InitNewFile(filename, this))
        return false;

    svTextCtrlEvent event(svEVT_TEXTCTRL_MODIFIED_CHANGED, GetId());
    event.Modified(m_bufText->IsModified());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);

    m_modified = m_bufText->IsModified();        

#ifdef OS_SCROLLBAR
    hScrollbar->Show(true);
    vScrollbar->Show(true);
#else
    m_vsb->Show(true);
    m_hsb->Show(true);
#endif

    m_textView->Reset(m_bufText, m_linesPerTxtArea, m_font, m_tai.Width, m_tabWidth, m_charHeight, m_spaceWidth);
    m_textView->Refresh();
    DoSmoothRefresh();

    return true;
}

bool svTextEditorCtrl::ReadTextFile(const wxString& filename)
{
#ifdef OS_SCROLLBAR
    hScrollbar->Show(false);
    vScrollbar->Show(false);
#else
    m_vsb->Show(false);
    m_hsb->Show(false);
#endif
    DisplayLoadingBackground("Preparing file...", 0);

    ResetSyntaxHilight(filename);

    if (!m_bufText->ReadTextFile(filename, this))
    // if (!m_bufText->ReadTextFile(filename, *this))
        return false;
    // m_displayText->SetFirstLineNo(0);

#ifdef OS_SCROLLBAR
    hScrollbar->Show(true);
    vScrollbar->Show(true);
#else
    m_vsb->Show(true);
    m_hsb->Show(true);
#endif

    m_textView->Reset(m_bufText, m_linesPerTxtArea, m_font, m_tai.Width, m_tabWidth, m_charHeight, m_spaceWidth);
    m_textView->Refresh();
    DoSmoothRefresh();
    // m_textEdit->Reset(m_bufText);
        // testing

    return true;
}

bool svTextEditorCtrl::Reset(void)
{
    // Check if the physical file's last modification time is differ from buffer's.
    if (!m_fileDesc.IsModificationTimeChanged())
        return false;
        
    int ans;
    wxMessageDialog msgdlg(this, m_fileDesc.m_displayName + _(" is modified, reload it?"), _("Message"), wxYES_NO);
    ans = msgdlg.ShowModal();
    if (ans==wxID_YES)
    {
        if (ReadTextFile(m_fileDesc.m_fullPathName))
        {
            m_fileDesc.SetLastModificationTime();
            m_textView->ResetCaretPosition(0, 0);
            return true;
        }
    }
    
    m_fileDesc.SetNeedReset(false);  // The file will no more hint to be reload when last modification is different with physical file and buffer.
    return false;    
}

bool svTextEditorCtrl::SaveFile(void)
{
    return SaveFile(m_fileDesc.m_fullPathName);
}

bool svTextEditorCtrl::SaveFile(const wxString& filename)
{

    wxTextFile file;
    if (!wxFileExists(filename))
    {
        // File not existed. Maybe a new files or save as new files.
        extern svPreference g_preference;

        char *o_syntaxName = NULL;
        char *n_syntaxName = NULL;
        o_syntaxName = g_preference.GetSyntaxFNRule(filename.mb_str());

        wxString caption = _("Choose a filename to save...");
        wxString wildcard = 
        wxT("Text files (*.*)|*.*");
        wxString defaultDir = wxGetCwd();
        wxString defaultFilename = wxEmptyString;

        wxFileDialog dialog(this, caption, defaultDir, defaultFilename,
        wildcard, wxFD_SAVE);
        if (dialog.ShowModal() != wxID_OK)
            return false;
        else
        {
            wxString path = dialog.GetPath();
            int filterIndex = dialog.GetFilterIndex();

            svFileDesc fd(path, dialog.GetFilename());
            SetFileDesc(fd);
            // SetFileName(path);
            // SetDisplayName(dialog.GetFilename());
            // file.Create(GetFileName());
            m_bufText->SaveToFile(GetFullPathName());
            m_fileDesc.SetLastModificationTime();
        }

        n_syntaxName = g_preference.GetSyntaxFNRule(GetFullPathName().mb_str());

        // If file extension is changed. Reseting it syntax.
        if (strcmp(n_syntaxName, o_syntaxName)!=0)
        {
            ResetSyntaxHilight(GetFullPathName());
            m_bufText->CreateKeywordTableAll();
            m_textView->Refresh();
            DoSmoothRefresh(SVID_SMOOTH_NONE);
        }

        if (o_syntaxName) free(o_syntaxName);
        if (n_syntaxName) free(n_syntaxName);

        // Change the main frame tab display name.
        svTextCtrlEvent event(svEVT_TEXTCTRL_MODIFIED_CHANGED, GetId());
        event.Modified(m_bufText->IsModified());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);

        m_modified = m_bufText->IsModified();
    }
    else
    {
        m_bufText->SaveToFile(GetFullPathName());
        m_fileDesc.SetLastModificationTime();
    }

    if (m_modified!=m_bufText->IsModified())
    {
        svTextCtrlEvent event(svEVT_TEXTCTRL_MODIFIED_CHANGED, GetId());
        event.Modified(m_bufText->IsModified());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);

        m_modified = m_bufText->IsModified();        
    }

    return false;

}

void svTextEditorCtrl::SetFileDesc(const svFileDesc &p_fd)
{
    m_fileDesc = p_fd;
}

wxString svTextEditorCtrl::GetDisplayName(void)
{
    // return m_modified?wxT("*")+m_displayName:m_displayName;
    return m_bufText->IsModified()?wxT("*")+m_fileDesc.m_displayName:m_fileDesc.m_displayName;
}

wxString svTextEditorCtrl::GetFullPathName(void)
{
    // return m_fileName;
    return m_fileDesc.m_fullPathName;
}

wxString svTextEditorCtrl::GetTextFileCharSet(void)
{
    return m_bufText->GetOriginalCharSet();
}

void svTextEditorCtrl::ResetSyntaxHilight(const wxString& filename)
{
    // wxString apppath;
    // apppath = wxStandardPaths::Get().GetExecutablePath();
    // wxFileName fp(apppath);
    // wxString path = fp.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);

    extern svPreference g_preference;

    char *syntaxName = NULL;
    syntaxName = g_preference.GetSyntaxFNRule(filename.mb_str());
    wxString fn = g_preference.GetSyntaxPath() + wxString::FromAscii(syntaxName) + wxT(".svsyn");
    m_bufText->LoadSyntaxDesc(fn.mb_str());
    if (syntaxName) free(syntaxName);


/*    if (!m_fnRules)
    {
        wxLogMessage(wxT("svTextEditorCtrl::ResetSyntaxHilight error: m_fnRules NULL!"));
    }

    char *syntaxName = NULL;
#ifdef __WXMSW__
    syntaxName = m_fnRules->GetRule(filename.mb_str());
    wxString fn = path + wxString::FromAscii(syntaxName) + wxT(".svsyn");
    m_bufText->LoadSyntaxDesc(fn.mb_str());
#else
    syntaxName = m_fnRules->GetRule(filename.ToUTF8());
    wxString fn = path + wxString(syntaxName, wxConvUTF8) + wxT(".svsyn");
    m_bufText->LoadSyntaxDesc(fn.ToUTF8());
#endif
    if (syntaxName) free(syntaxName);*/
}



/* ============================================================================ *
 *
 *   Mouse Processing.
 *
 * ============================================================================ */

void svTextEditorCtrl::OnMouseLeftUp(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseLeftUp"));
#endif
    if (!m_mouseReady) return;

    // wxObject *eventObject = event.GetEventObject();
    // if (!eventObject->IsKindOf(CLASSINFO(svTextEditorCtrl))) return;

    m_mouseLeftIsDown=false;

    if(HasCapture())
    {
        ReleaseMouse();
    }

    if(m_mouseMotionTimer->IsRunning())
    {
        m_mouseMotionTimer->Stop();
    }

    if (m_textView->CaretsMergeOverlap())
    {
        m_textView->UpdateCaretsInfo();
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    }

    // event.Skip();
}

// 按下滑鼠左鍵觸發一次
void svTextEditorCtrl::OnMouseLeftDown(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseLeftDown"));
#endif

    if (!m_mouseReady) return;

    // When textEditorCtrl is clicked, set wxAuiNotebook focused to the tab it is.
    if (!m_mainFrame->IsCurrentsvTextEditorCtrl(m_fileDesc))
    {
        m_mainFrame->ChangeCurrentsvTextEditorCtrl(m_fileDesc);
        m_mainFrame->OpenFilesCtrlChangeCurrentItemBG(m_fileDesc);
    }
    
    // wxObject *eventObject = event.GetEventObject();
    // if (!eventObject->IsKindOf(CLASSINFO(svTextEditorCtrl))) return;

    HideTypeHint();

    int old_m_areaI = m_areaI;

    m_areaI = MouseXY2Area(m_mouseX, m_mouseY, false);

    // // cal which zone mouse position is in.
    // if ((m_mouseX >= m_tai.X && m_mouseX <= m_tai.X + m_tai.Width-1) &&
    //     (m_mouseY >= m_tai.Y && m_mouseY <= m_tai.Y + m_tai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_IBEAM));
    //     m_areaI = SVID_TXTAREA;
    // }
    // else if ((m_mouseX >= m_lniai.X && m_mouseX <= m_lniai.X + m_lniai.Width-1) &&
    //          (m_mouseY >= m_lniai.Y && m_mouseY <= m_lniai.Y + m_lniai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_LINNUM;
    // }
    // else if ((m_mouseX >= m_fai.X && m_mouseX <= m_fai.X + m_fai.Width-1) &&
    //          (m_mouseY >= m_fai.Y && m_mouseY <= m_fai.Y + m_fai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_FOLDING;
    // }
    // else if ((m_mouseX >= m_cniai.X && m_mouseX <= m_cniai.X + m_cniai.Width-1 ) &&
    //          (m_mouseY >= m_cniai.Y && m_mouseY <= m_cniai.Y + m_cniai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_COLNUM;
    // }
    // else  // mouse out of client area.
    // {
    //     m_areaI = SVID_OUTOFBORDER;
    // }
    

    // if (m_areaI == SVID_TXTAREA)
    // if (m_areaI == SVID_TXTAREA || m_areaI == SVID_LINNUM || m_areaI == SVID_FOLDING)
    if (m_areaI == SVID_TXTAREA || m_areaI == SVID_LINNUM)
    {
        bufTextIndex caretPos;
        int mouse_x, mouse_y;
        mouse_x=mouse_y=0;
        if (m_areaI==SVID_TXTAREA)
        {
            mouse_x = m_mouseX-m_lniai.Width-m_fai.Width;
            mouse_y = m_mouseY-m_cniai.Height;
        }
        else  // SVID_LINNUM && SVID_FOLDING
        {
            mouse_x = 0;
            mouse_y = m_mouseY-m_cniai.Height;
        }

        if (m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos))
        {
            if (!event.ControlDown())
            {
                // 則重設所有的游標，只留下這一個新的游標
                m_textView->ResetCaretPosition(caretPos.unwrap_idx, caretPos.wrap_idx);
                m_textView->SetCaretSelect();
            }
            else
            {
                // 滑鼠按下左鍵並且按住鍵盤的Ctl鍵
                svCaret c = svCaret(caretPos.unwrap_idx, caretPos.wrap_idx);
                if (m_textView->CaretsIsOverlaped(svCaret(c)))
                {
                    // // 如果滑鼠點到的位置是在別的游標選取文字之內
                    // // 則重設所有的游標，只留下這一個新的游標
                    // m_textView->ResetCaretPosition(caretPos.unwrap_idx, caretPos.wrap_idx);
                    // m_textView->SetCaretSelect();

                    // 要改為 Drag (Copy) 到目的地位置
                    // Not implement yet.
                }
                else
                {
                    // 新增一個新的游標
                    // 並初始開始選取的位置(留待 OnMouseMotion 判斷是否移動滑鼠+滑鼠左鍵=>選取文字)
                    m_textView->AppendCaretPosition(caretPos.unwrap_idx, caretPos.wrap_idx);
                    m_textView->SetCaretSelect();
                }
            }
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseLeftDown 02"));
#endif
            
            // Refresh();
            DoSmoothRefresh(SVID_SMOOTH_NONE);
        }
    }
    else if (m_areaI == SVID_FOLDING)        
    {
        // click for folding.

        size_t r=0;
        if (m_textView->PixelXY2TextRowUW(m_mouseX-m_lniai.Width, m_mouseY-m_cniai.Height, m_hChar+m_rowInterval, r))
        {
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnMouseLeftDown On folding area. row=%i", (int)r));
#endif
            m_textView->DoFolding(r);
            DoSmoothRefresh(SVID_SMOOTH_NONE);
        }

    }

    CaptureMouse();
    m_mouseLeftIsDown=true;

    // else if (m_areaI == SVID_LINNUM && MouseXY2ScrTextXY(event.GetX(), event.GetY()))
    // {
    //     m_txtX = 0;
    //     m_mouseLeftIsDown=true;
    //     ResetSelected();
    //     Refresh();
    // }
    // event.Skip();
}

void svTextEditorCtrl::OnMouseRightUp(wxMouseEvent& event)
{
    if (!m_mouseReady) return;

    // wxObject *eventObject = event.GetEventObject();
    // if (!eventObject->IsKindOf(CLASSINFO(svTextEditorCtrl))) return;


    // if (m_contextMenu!=NULL)
    // {
    //     delete m_contextMenu;
    //     m_contextMenu = NULL;
    // }
    // m_contextMenu = GetContextMenu();
    // PopupMenu(m_contextMenu);
}

void svTextEditorCtrl::OnMouseMotion(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseMotion"));
#endif

    if (!m_mouseReady) return;

    // wxObject *eventObject = event.GetEventObject();
    // if (eventObject && !eventObject->IsKindOf(CLASSINFO(svTextEditorCtrl))) return;

    m_mouseX = event.GetX();
    m_mouseY = event.GetY();

#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("svTextEditorCtrl::OnMouseMotion: mouse x:%i y=%i, m_tai.X=%i, m_tai.Y=%i, m_lniai.X=%i, m_lniai.Y=%i, m_fai.X=%i, m_fai.Y=%i"), m_mouseX, m_mouseY, m_tai.X, m_tai.Y, m_lniai.X, m_lniai.Y, m_fai.X, m_fai.Y));
#endif  


    int old_m_areaI = m_areaI;

    m_areaI = MouseXY2Area(m_mouseX, m_mouseY);
    // // cal which zone mouse position is in.
    // SetCursor(wxCursor(wxCURSOR_ARROW));
    // if ((m_mouseX >= m_tai.X && m_mouseX <= m_tai.X + m_tai.Width-1) &&
    //     (m_mouseY >= m_tai.Y && m_mouseY <= m_tai.Y + m_tai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_IBEAM));
    //     m_areaI = SVID_TXTAREA;
    // }
    // else if ((m_mouseX >= m_lniai.X && m_mouseX <= m_lniai.X + m_lniai.Width-1) &&
    //          (m_mouseY >= m_lniai.Y && m_mouseY <= m_lniai.Y + m_lniai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_LINNUM;
    // }
    // else if ((m_mouseX >= m_fai.X && m_mouseX <= m_fai.X + m_fai.Width-1) &&
    //          (m_mouseY >= m_fai.Y && m_mouseY <= m_fai.Y + m_fai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_FOLDING;
    // }
    // else if ((m_mouseX >= m_cniai.X && m_mouseX <= m_cniai.X + m_cniai.Width-1 ) &&
    //          (m_mouseY >= m_cniai.Y && m_mouseY <= m_cniai.Y + m_cniai.Height-1))
    // {
    //     SetCursor(wxCursor(wxCURSOR_ARROW));
    //     m_areaI = SVID_COLNUM;
    // }
    // else  // mouse out of client area.
    // {
    //     m_areaI = SVID_OUTOFBORDER;
    // }
    
/*    if (m_areaI == SVID_TXTAREA || m_areaI == SVID_LINNUM)
    {
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("svTextEditorCtrl::OnMouseMotion: mouse x:%i y=%i, lnw=%i cnh=%i"), m_mouseX, m_mouseY, m_lniai.Width, m_cniai.Height));
#endif        
        if (!m_textView->PixelXY2TextRowColUW(m_mouseX-m_lniai.Width, m_mouseY-m_cniai.Height, m_hChar+m_rowInterval, debug_textIdx))
        {
            debug_textIdx.unwrap_idx = 0;
            debug_textIdx.wrap_idx = 0;
        }
    }
*/
    if (event.LeftIsDown())
    {
        if(!m_mouseMotionTimer->IsRunning())
        {
            m_mouseMotionTimer->Start(SVID_MOUSE_MOTION_TIMER_INTERVAL);
        }

        if (m_areaI == SVID_TXTAREA || m_areaI == SVID_LINNUM || m_areaI == SVID_FOLDING)
        {
            bufTextIndex caretPos;
            int mouse_x, mouse_y;
            mouse_x=mouse_y=0;
            if (m_areaI==SVID_TXTAREA)
            {
                mouse_x = m_mouseX-m_lniai.Width-m_fai.Width;
                mouse_y = m_mouseY-m_cniai.Height;
            }
            else  // SVID_LINNUM && SVID_FOLDING
            {
                mouse_x = 0;
                mouse_y = m_mouseY-m_cniai.Height;
            }
            if (m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos))
            {
                // 滑鼠移動+滑鼠左鍵=>選取文字，將游標移到滑鼠的位置
                m_textView->LastCaretMoveTo(caretPos.unwrap_idx, caretPos.wrap_idx);
                // m_textView->CaretsMergeOverlap();  // 移到 MouseLeftUp
            }
            DoSmoothRefresh(SVID_SMOOTH_NONE);
            #ifndef NDEBUG
                wxLogMessage(wxT("OnMouseMotion 01"));
            #endif
        }
        else  // mouse out of client area.
        {
            bufTextIndex caretPos;
            int mouse_x, mouse_y;
            mouse_x=mouse_y=0;

            if (m_mouseY<0)  // automatic scrool up 3 lines.
            {
                int lineHeight = m_hChar+m_rowInterval;
                int l = 0;

                if (m_mouseY>-lineHeight)
                    l = -1;
                else if (m_mouseY>lineHeight * -3)
                    l = -3;
                else
                    l = -5;

                m_textView->MoveFirstLineIndex(l);
                m_textView->UpdateCaretsInfo();

#ifdef OS_SCROLLBAR
                vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
#else
                m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
#endif
                m_textView->Refresh();
                // DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);

                mouse_x = 0;
                mouse_y = 0;

                if (m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos))
                {
                    // 滑鼠移動+滑鼠左鍵=>選取文字，將游標移到滑鼠的位置
                    m_textView->LastCaretMoveTo(caretPos.unwrap_idx, caretPos.wrap_idx);
                    // m_textView->CaretsMergeOverlap();  // 移到 MouseLeftUp
                }
                DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);
                #ifndef NDEBUG
                    wxLogMessage(wxT("OnMouseMotion 02"));
                #endif
            }
            else if (m_mouseY>m_tai.Y + m_tai.Height)    // automatic scrool down 3 lines.
            {
                if (m_textView->GetLastUWLineNo()<(int)m_bufText->LineCntUW())
                {
                    int lineHeight = m_hChar+m_rowInterval;
                    int l = 0;

                    if (m_mouseY - m_tai.Y - m_tai.Height < lineHeight)
                        l = 1;
                    else if (m_mouseY - m_tai.Y - m_tai.Height < 3 * lineHeight)
                        l = 3;
                    else
                        l = 5;

                    m_textView->MoveFirstLineIndex(l);
                    m_textView->UpdateCaretsInfo();

#ifdef OS_SCROLLBAR
                    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
#else
                    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
#endif
                    m_textView->Refresh();
                    // DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);

                }

                mouse_x = m_tai.Width;
                mouse_y = m_tai.Height;
#ifndef NDEBUG
                    wxLogMessage(wxString::Format("m_textView->PixelXY2TextRowColUW=%i", m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos)));
#endif
                if (m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos))
                {
                    // 滑鼠移動+滑鼠左鍵=>選取文字，將游標移到滑鼠的位置
                    m_textView->LastCaretMoveTo(caretPos.unwrap_idx, caretPos.wrap_idx);
#ifndef NDEBUG
                    wxLogMessage(wxString::Format("m_textView->LastCaretMoveTo(%i, %i)", caretPos.unwrap_idx, caretPos.wrap_idx));
#endif
                    // m_textView->CaretsMergeOverlap();  // 移到 MouseLeftUp
                }
                DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);
                #ifndef NDEBUG
                    wxLogMessage(wxT("OnMouseMotion 03"));
                #endif
            }
            else
            {
                if (m_mouseX<0)    // mouse on the left side of client area.
                {
                    mouse_x = 0;
                    mouse_y = m_mouseY-m_cniai.Height;
                }
                else  // mouse on the left side of client area.
                {
                    mouse_x = m_tai.Width;
                    mouse_y = m_mouseY-m_cniai.Height;
                }
                if (m_textView->PixelXY2TextRowColUW(mouse_x, mouse_y, m_hChar+m_rowInterval, caretPos))
                {
                    // 滑鼠移動+滑鼠左鍵=>選取文字，將游標移到滑鼠的位置
                    m_textView->LastCaretMoveTo(caretPos.unwrap_idx, caretPos.wrap_idx);
                    // m_textView->CaretsMergeOverlap();  // 移到 MouseLeftUp
                }
                DoSmoothRefresh(SVID_SMOOTH_NONE);
                #ifndef NDEBUG
                    wxLogMessage(wxT("OnMouseMotion 04"));
                #endif
            }

        }
        // DoSmoothRefresh(SVID_SMOOTH_NONE);
    }
    else if (m_areaI!=old_m_areaI)
    {
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    }

    // event.Skip();
}

void svTextEditorCtrl::OnMouseWheel(wxMouseEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnMouseWheel"));
#endif
    
    if (!m_mouseReady) return;

    // wxObject *eventObject = event.GetEventObject();
    // if (!eventObject->IsKindOf(CLASSINFO(svTextEditorCtrl))) return;


#ifndef NDEBUG
    //ShowMsg(wxString::Format(wxT("wheel=%i %i %i"), event.m_wheelRotation, event.m_wheelDelta, event.m_linesPerAction));
    wxLogMessage(wxString::Format("OnMouseWheel rotation=%i", event.m_wheelRotation));
#endif

    // m_wheelRotation -n for line up and +n for line down
    // int line = event.m_wheelRotation / 80 * -1;
    int line = event.m_wheelRotation / SVID_MOUSE_WHEEL_SPEED * -1;
    m_textView->MoveFirstLineIndex(line);
    m_textView->UpdateCaretsInfo();

#ifdef OS_SCROLLBAR
    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
#else
    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
#endif
    m_textView->Refresh();
    // DoSmoothRefresh();
    DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);
    this->SetFocus();
    event.Skip();
}

// cal which zone mouse position is in.
int svTextEditorCtrl::MouseXY2Area(int p_mouseX, int p_mouseY, bool p_setCursor)
{
    if (p_setCursor) SetCursor(wxCursor(wxCURSOR_ARROW));
    if ((p_mouseX >= m_tai.X && p_mouseX <= m_tai.X + m_tai.Width-1) &&
        (p_mouseY >= m_tai.Y && p_mouseY <= m_tai.Y + m_tai.Height-1))
    {
        if (p_setCursor) SetCursor(wxCursor(wxCURSOR_IBEAM));
        return SVID_TXTAREA;
    }
    else if ((p_mouseX >= m_lniai.X && p_mouseX <= m_lniai.X + m_lniai.Width-1) &&
             (p_mouseY >= m_lniai.Y && p_mouseY <= m_lniai.Y + m_lniai.Height-1))
    {
        if (p_setCursor) SetCursor(wxCursor(wxCURSOR_ARROW));
        return SVID_LINNUM;
    }
    else if ((p_mouseX >= m_fai.X && p_mouseX <= m_fai.X + m_fai.Width-1) &&
             (p_mouseY >= m_fai.Y && p_mouseY <= m_fai.Y + m_fai.Height-1))
    {
        if (p_setCursor) SetCursor(wxCursor(wxCURSOR_ARROW));
        return SVID_FOLDING;
    }
    else if ((p_mouseX >= m_cniai.X && p_mouseX <= m_cniai.X + m_cniai.Width-1 ) &&
             (p_mouseY >= m_cniai.Y && p_mouseY <= m_cniai.Y + m_cniai.Height-1))
    {
        if (p_setCursor) SetCursor(wxCursor(wxCURSOR_ARROW));
        return SVID_COLNUM;
    }
    else  // mouse out of client area.
    {
        if (p_setCursor) SetCursor(wxCursor(wxCURSOR_ARROW));
        return SVID_OUTOFBORDER;
    }
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
void svTextEditorCtrl::PrepareDCBuffer(bool p_smooth)
{
#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 01"));
#endif

    // 準備相關DC時是按前一頁、本頁、下一頁分別處理

    if (m_bTextAreaDC!=NULL) delete m_bTextAreaDC;
    if (m_bLineNumIndDC!=NULL) delete m_bLineNumIndDC;
    if (m_bFoldingIndDC!=NULL) delete m_bFoldingIndDC;
    if (m_bColNumIndDC!=NULL) delete m_bColNumIndDC;
    m_bTextAreaDC = new wxMemoryDC();
    m_bLineNumIndDC = new wxMemoryDC();
    m_bFoldingIndDC = new wxMemoryDC();
    m_bColNumIndDC = new wxMemoryDC();

    // 區分前一頁，本頁，下一頁，baseH是三者的基準y值
    int baseH = 0;

    // 每一文字行高度佔的像素值
    int lineHeight = m_hChar + m_rowInterval;

    // 長度額外加上一個頁面的長度，處理最後一行出現在頁面第一行時的狀況
    wxBitmap bitmapTA(m_textView->GetMaxPixelWidth(), m_textView->GetAllPageTextListCount() * lineHeight + m_tai.Height);

    // exception handle (avoiding gtk error message)
    // a width=0 or height=0 wxBitmap will dump some gtk error message.
    if (bitmapTA.GetWidth()<=0)
        bitmapTA.SetWidth(10);
    if (bitmapTA.GetHeight()<=0)
        bitmapTA.SetHeight(10);

    m_bTextAreaDC->SelectObject(bitmapTA);
    m_bTextAreaDC->SetFont(m_font);
    m_bTextAreaDC->SetBrush(wxBrush(*wxWHITE));
    m_bTextAreaDC->SetPen(wxPen(*wxBLACK, 1));

    // 先 comment 掉，觀察看看
    // if (m_lniai.Width != m_textView->GetLineNoIndicatorAreaMaxWidth())
    // {
    //     DoResizeLineNoIndicatorArea();
    // }
    m_lniai.Width = m_textView->GetLineNoIndicatorAreaMaxWidth();
    // The width of line Number indicator area is determed on PrepareDC function.
    // So, other variables relay on m_lniai.Width need to be recaculated.
    m_fai.X = m_border.Width - 1 + m_lniai.Width; 
    m_tai.X = m_border.Width + m_lniai.Width + m_fai.Width - 1;

    // 長度額外加上一個頁面的長度，處理最後一行出現在頁面第一行時的狀況
    wxBitmap bitmapLI(m_lniai.Width, m_textView->GetAllPageTextListCount() * lineHeight + m_tai.Height);

    // exception handle (avoiding gtk error message)
    // a width=0 or height=0 wxBitmap will dump some gtk error message.
    if (bitmapLI.GetWidth()<=0)
        bitmapLI.SetWidth(10);
    if (bitmapLI.GetHeight()<=0)
        bitmapLI.SetHeight(10);

    m_bLineNumIndDC->SelectObject(bitmapLI);
    m_bLineNumIndDC->SetFont(m_font);
    m_bLineNumIndDC->SetBrush(wxBrush(*wxWHITE));
    m_bLineNumIndDC->SetPen(wxPen(*wxBLACK, 1));

    // 長度額外加上一個頁面的長度，處理最後一行出現在頁面第一行時的狀況
    wxBitmap bitmapFI(m_fai.Width, m_textView->GetAllPageTextListCount() * lineHeight + m_tai.Height);

    // exception handle (avoiding gtk error message)
    // a width=0 or height=0 wxBitmap will dump some gtk error message.
    if (bitmapFI.GetWidth()<=0)
        bitmapFI.SetWidth(10);
    if (bitmapFI.GetHeight()<=0)
        bitmapFI.SetHeight(10);

    m_bFoldingIndDC->SelectObject(bitmapFI);
    m_bFoldingIndDC->SetFont(m_font);
    m_bFoldingIndDC->SetBrush(wxBrush(*wxWHITE));
    m_bFoldingIndDC->SetPen(wxPen(*wxBLACK, 1));

    wxBitmap bitmapCI(m_textView->GetMaxPixelWidth(), m_cniai.Height);


    // exception handle (avoiding gtk error message)
    // a width=0 or height=0 wxBitmap will dump some gtk error message.
    if (bitmapCI.GetWidth()<=0)
        bitmapCI.SetWidth(10);
    if (bitmapCI.GetHeight()<=0)
        bitmapCI.SetHeight(10);

    m_bColNumIndDC->SelectObject(bitmapCI);
    m_bColNumIndDC->SetFont(m_font);
    m_bColNumIndDC->SetBrush(wxBrush(*wxWHITE));
    m_bColNumIndDC->SetPen(wxPen(*wxBLACK, 1));


    // 取得在畫面中的caret資料
    vector<caretPosOnView> *carets = m_textView->CalVisibleCaretsPos();
    // int verticalPixelOffset = m_textView->CalHScrollPixel(carets);
    // 水平捲動的重算依各功能需求自行呼叫
    int verticalPixelOffset = m_textView->GetHScrollPixel();
    
    // Loading the theme colors.
    wxColour defBGColor = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour();
    wxColour defFGColor = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetFGColour();
    wxColour defLIBGColor = m_theme->GetClass(wxString(wxT("_LINE_IND")).ToStdString())->GetBGColour();
    wxColour defLIFGColor = m_theme->GetClass(wxString(wxT("_LINE_IND")).ToStdString())->GetFGColour();

    // Clear all background
    m_bColNumIndDC->SetBackground(defBGColor);
    m_bColNumIndDC->Clear();

    m_bLineNumIndDC->SetBackground(defBGColor);
    m_bLineNumIndDC->Clear();

    m_bFoldingIndDC->SetBackground(defBGColor);
    m_bFoldingIndDC->Clear();


    m_bTextAreaDC->SetBackground(defBGColor);
    m_bTextAreaDC->Clear();


    // draw column line number indicator
/*    wxRect CIRect = wxRect(0, 0, m_cniai.Width, m_cniai.Height);
    wxRect gradientRect = CIRect;
    gradientRect.SetHeight(gradientRect.GetHeight()/2);
    m_bColNumIndDC->GradientFillLinear(gradientRect,
    wxColour(132,125,132), wxColour(74,69,74), wxSOUTH);
    gradientRect.Offset(0, gradientRect.GetHeight());
    m_bColNumIndDC->GradientFillLinear(gradientRect,
    wxColour(0,0,0), wxColour(57,56,57), wxSOUTH);
     
    m_bColNumIndDC->SetPen(wxPen(GetBackgroundColour()));
    m_bColNumIndDC->SetBrush(*wxTRANSPARENT_BRUSH);
    m_bColNumIndDC->DrawRectangle(0, 0, CIRect.GetWidth(), CIRect.GetHeight());
    m_bColNumIndDC->SetFont(GetFont());
    m_bColNumIndDC->SetTextForeground(GetForegroundColour());

*/
    m_bColNumIndDC->SetTextForeground(*wxRED);

#ifndef NDEBUG
#ifdef __WXMSW__
    wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i MLD=%s CUR=%Iu %Iu LN=%i"), m_mouseX, m_mouseY, m_tai.X, m_tai.Y, m_tai.Width, m_tai.Height, m_areaI, m_vsbi.X, m_vsbi.Y, m_vsbi.Width, m_vsbi.Height, m_mouseLeftIsDown?"true":"false", debug_textIdx.unwrap_idx, debug_textIdx.wrap_idx, m_linesPerTxtArea);
#else
    wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i MLD=%s CUR=%zu %zu LN=%i"), m_mouseX, m_mouseY, m_tai.X, m_tai.Y, m_tai.Width, m_tai.Height, m_areaI, m_vsbi.X, m_vsbi.Y, m_vsbi.Width, m_vsbi.Height, m_mouseLeftIsDown?"true":"false", debug_textIdx.unwrap_idx, debug_textIdx.wrap_idx, m_linesPerTxtArea);
#endif
    // DebugMsg = wxString::Format(wxT("Caret1:r=%i vc=%i ks=%i Caret2:r=%i vc=%i ks=%i"), r1, vc1, ks1, r2, vc2, ks2);
    // DebugMsg = wxString::Format(wxT("Caret1:r=%i vc=%i ks=%i Caret2:r=%i vc=%i ks=%i vofs=%i"), r1, vc1, ks1, r2, vc2, ks2, verticalPixelOffset);
    // DebugMsg = wxString::Format(wxT("Caret1=%i %i %i Caret2=%i %i %i"), r1, c1, vc1, r2, c2, vc2);
    // wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i %i L|E=%i"), m_mouseX, m_mouseY, m_txtAreaX, m_txtAreaY, m_txtAreaW, m_txtAreaH, m_areaI, m_vscbX, m_vscbY, m_vscbW, m_vscbH, m_mouseLeftIsDown, m_mouseLeaveWindowCnt);
    //wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i TX=%i %i WH=%i %i AREA=%i VB=%i %i %i %i %i "), m_mouseX, m_mouseY, m_txtAreaX, m_txtAreaY, m_txtAreaW, m_txtAreaH, m_areaI, m_vscbX, m_vscbY, m_vscbW, m_vscbH, m_mouseLeftIsDown);
    //wxString DebugMsg = wxString::Format(wxT("M:%i %i T:%i %i W:%i H:%i AREA=%i VB=%i %i %i %i SCB=%i LN=%i %i C=%i %i"), m_mouseX, m_mouseY, m_txtAreaX, m_txtAreaY, m_txtAreaW, m_txtAreaH, m_areaI, m_vscbX, m_vscbY, m_vscbW, m_vscbH, m_scb_type, GetTextLineCnt(), m_linesPerTxtArea, m_caretX, m_caretY);
    //wxString DebugMsg = wxString::Format(wxT("ST=%i %i %i %i MLD=%i Caret=%i %i SCB=%i"), m_selSX, m_selSY, m_selEX, m_selEY, m_mouseLeftIsDown, m_txtX, m_txtY, m_scb_type);
    //wxString DebugMsg = wxString::Format(wxT("hsb=%i %i %i %i "), -m_txtAreaOffsetX, m_txtAreaW, m_txtMaxW, m_txtAreaW/2);
    // wxString DebugMsg = wxString::Format(wxT("Mouse=%i %i vsb X=%i Y=%i W=%i H=%i"), m_mouseX, m_mouseY, m_vscbX, m_vscbY, m_vscbW, m_vscbH);

/*    DebugMsg = "";
    vector<wxString> h = m_bufText->GetAvailableHint();
    for (std::vector<wxString>::iterator it=h.begin();
         it!=h.end();
         ++it)
    {
        DebugMsg += *it + " ";
    }
    
    m_bColNumIndDC->DrawText(DebugMsg, 0, 0); */
    
#endif


#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 02"));
#endif

    vector<int> pixelList;
    vector<styledWrapText>* wt = NULL;
    size_t ln=0;
    size_t old_ln=-1;



    // draw previous page
    // 畫前一頁資料
    if (p_smooth)
    {
        // Line number indicator background 
        m_bLineNumIndDC->SetPen(defLIBGColor);
        m_bLineNumIndDC->SetBrush(defLIBGColor);
        m_bLineNumIndDC->SetTextForeground(defLIFGColor);
        wxRect rlniai(0, baseH, m_lniai.Width, m_textView->GetPrevPageTextListCount() * lineHeight);

        // draw line number indicator area.
        m_font.SetStyle(wxFONTSTYLE_NORMAL);

        ln=0;
        old_ln=-1;

        for (int i=0; i<m_textView->GetPrevPageTextListCount(); i++)
        {
            ln = m_textView->GetStyledWrapTextLineNoUW_PP((size_t)i);
            if (ln!=old_ln && ln!=0) // When a line contain only line break bytes. or wrap line doesn't exist.
            {
#ifdef __WXMSW__
                m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %Iu"), ln), 0 , baseH + (i * lineHeight)); 
#else
                m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %zu"), ln), 0 , baseH + (i * lineHeight)); 
#endif
            }
            old_ln = ln;
        }

#ifndef NDEBUG
        wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 03"));
#endif

        // draw Text Area background.
        m_bTextAreaDC->SetBrush(defBGColor);
        m_bTextAreaDC->SetPen(defBGColor);
        m_bTextAreaDC->SetTextForeground(defFGColor);

        // draw indent indicator line.
        m_bTextAreaDC->SetPen(wxPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour(), 1, wxPENSTYLE_DOT));
        m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetTextIndentIndicatorCharLen_PP(i, pixelList))
            {
                for (vector<int>::iterator it = pixelList.begin();
                     it!=pixelList.end();
                     ++it)
                {
                    m_bTextAreaDC->DrawLine(wxPoint(*it*m_spaceWidth , baseH + (i * lineHeight) + 1), wxPoint(*it*m_spaceWidth, baseH + ((i+1) * lineHeight)));         
                }
            }
        }

        // draw selected text background
        m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        for (int i=0; i<m_textView->GetPrevPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetCaretSelectPixel_PP(i, pixelList))
            {
                int cnt = pixelList.size();
                cnt /= 2;
                for (int j=0; j<cnt; j++)
                {
                    int sp = pixelList.at(j*2);
                    int ep = pixelList.at(j*2+1);
                    wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                    m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
                }
            }
        }

        // draw finded keyword background
        pixelList.clear();
        m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_FIND")).ToStdString())->GetFGColour());
        // m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
        m_bTextAreaDC->SetBrush(*wxTRANSPARENT_BRUSH);
        for (int i=0; i<m_textView->GetPrevPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetFindKeywordPixel_PP(i, pixelList))
            {
                int cnt = pixelList.size();
                cnt /= 2;
                for (int j=0; j<cnt; j++)
                {
                    int sp = pixelList.at(j*2);
                    int ep = pixelList.at(j*2+1);
                    wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                    m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
                }
            }
        }

        // draw text.
        m_font.SetStyle(wxFONTSTYLE_NORMAL);

        wt = NULL;
        for (int i=0; i<m_textView->GetPrevPageTextListCount(); i++)
        {
            wt = NULL;
            wt = m_textView->GetStyledWrapText_PP((size_t)i);
            if (wt) // When a line contain only line break bytes. or wrap line doesn't exist.
            {
                int lastAccumPixelSum = 0;
                for (size_t j=0; j<(*wt).size(); j++)
                {
                    svThemeClass* tc = m_theme->GetClass((*wt).at(j).w_styleName.ToStdString());
                    if (!tc)
                    {
                        tc = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString()); 
                    }
                    wxColour color = tc->GetFGColour();
                    m_bTextAreaDC->SetTextForeground(color);
                    // wxLogMessage(wxT("draw text: ") + (*wt).at(j).w_text + wxString::Format(wxT("%i"), (*wt).at(j).w_accumPixelWidth) + " " + wxString::Format(wxT("%i"), s.GetWidth()));
                    if ((*wt).at(j).w_text==wxT("\t"))
                    {
                        m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                        m_bTextAreaDC->DrawText(wxT(">"), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    }
                    // else if ((*wt).at(j).w_text==wxT(" "))
                    // {
                    //     m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                    //     m_bTextAreaDC->DrawText(wxT("."), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    // }
                    else                    
                    {
                        m_bTextAreaDC->DrawText((*wt).at(j).w_text, (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    }

                    lastAccumPixelSum = (*wt).at(j).w_accumPixelWidth + (*wt).at(j).w_pixelWidth;
                }

                // draw tail folding indicator.
                if (m_textView->HadFoldingTail_PP(i))
                {
                    // wxRect rectFoldingTail(lastAccumPixelSum, baseH + (i * lineHeight), 12, lineHeight);
                    // m_bTextAreaDC->DrawRoundedRectangle(rectFoldingTail, 2.2);
                    m_bTextAreaDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());
                    m_bTextAreaDC->DrawText("~", lastAccumPixelSum, baseH + (i * lineHeight));
                }
            }
        }

        // draw folding indicator
        if (m_areaI==SVID_FOLDING)
        {
            m_bFoldingIndDC->SetPen(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
            m_bFoldingIndDC->SetBrush(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
            m_bFoldingIndDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());
            for (int i=0; i<m_textView->GetPrevPageTextListCount(); i++)
            {
                if (m_textView->HadFolding_PP(i))
                {
                    // wxRect rInd(1, baseH + (i * lineHeight), 10, lineHeight);
                    // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                    m_bFoldingIndDC->DrawText("+", 2, baseH + (i * lineHeight));
                }
                else if (m_textView->CanFolding_PP(i))
                {
                    // wxRect rInd(3, baseH + (i * lineHeight), 7, lineHeight);
                    // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                    m_bFoldingIndDC->DrawText("=", 2, baseH + (i * lineHeight));
                }
            }
        }        

#ifndef NDEBUG
        wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 04"));
#endif

    }  // end of if p_smooth


    // draw current page
    // 繪製本頁

    // increment the base Y coordinator.
    baseH = m_textView->GetPrevPageTextListCount() * lineHeight;


    // draw line number indicator background
    m_bLineNumIndDC->SetPen(defLIBGColor);
    m_bLineNumIndDC->SetBrush(defLIBGColor);
    m_bLineNumIndDC->SetTextForeground(defLIFGColor);

    // draw line number.
    m_font.SetStyle(wxFONTSTYLE_NORMAL);

    ln=0;
    old_ln=-1;

    for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
    {
        ln = m_textView->GetStyledWrapTextLineNoUW((size_t)i);
        if (ln!=old_ln && ln!=0) // When a line contain only line break bytes. or wrap line doesn't exist.
        {
#ifdef __WXMSW__
            m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %Iu"), ln), 0 , baseH + (i * lineHeight)); 
#else
            m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %zu"), ln), 0 , baseH + (i * lineHeight)); 
#endif
        }
        old_ln = ln;
    }


#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 05"));
#endif

    // draw Text Area background.
    m_bTextAreaDC->SetBrush(defBGColor);
    m_bTextAreaDC->SetPen(defBGColor);
    m_bTextAreaDC->SetTextForeground(defFGColor);


    // draw indent indicator line.
    m_bTextAreaDC->SetPen(wxPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour(), 1, wxPENSTYLE_DOT));
    m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
    for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
    {
        pixelList.clear();
        if (m_textView->GetTextIndentIndicatorCharLen(i, pixelList))
        {
            for (vector<int>::iterator it = pixelList.begin();
                 it!=pixelList.end();
                 ++it)
            {
                m_bTextAreaDC->DrawLine(wxPoint(*it*m_spaceWidth , baseH + (i * lineHeight) + 1), wxPoint(*it*m_spaceWidth, baseH + ((i+1) * lineHeight)));         
            }
        }
    }
    
    
    // draw selected text background
    m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
    m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
    for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
    {
        pixelList.clear();
        if (m_textView->GetCaretSelectPixel(i, pixelList))
        {
            int cnt = pixelList.size();
            cnt /= 2;
            for (int j=0; j<cnt; j++)
            {
                int sp = pixelList.at(j*2);
                int ep = pixelList.at(j*2+1);
                wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
            }
        }
    }

    // draw finded keyword background
    m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_FIND")).ToStdString())->GetFGColour());
    // m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
    m_bTextAreaDC->SetBrush(*wxTRANSPARENT_BRUSH);
    for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
    {
        pixelList.clear();
        if (m_textView->GetFindKeywordPixel(i, pixelList))
        {
            int cnt = pixelList.size();
            cnt /= 2;
            for (int j=0; j<cnt; j++)
            {
                int sp = pixelList.at(j*2);
                int ep = pixelList.at(j*2+1);
                wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
            }
        }
    }

#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 06"));
#endif

    // draw embrace symbol background
    vector<textRange> *textRange = m_textView->CalVisibleCaretsEmbraceSymbolPos();

    if (textRange->size())
    {
        m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_EMBRACE")).ToStdString())->GetFGColour());
        m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_EMBRACE")).ToStdString())->GetFGColour());
        for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetTextRangePixel(i, pixelList, *textRange))
            {
                int cnt = pixelList.size();
                cnt /= 2;
                for (int j=0; j<cnt; j++)
                {
                    int sp = pixelList.at(j*2);
                    int ep = pixelList.at(j*2+1);
                    // wxRect selectCaret(sp, baseH + (i * lineHeight) + 2, ep-sp, lineHeight);
                    // m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);
                    
                    wxPoint f(sp, baseH + (i+1)*lineHeight+2), t(ep, baseH + (i+1)*lineHeight+2);
                    m_bTextAreaDC->DrawLine(f, t);
                    
                }
            }
        }
    }

    textRange->clear();
    delete textRange;

    // draw text.

    m_font.SetStyle(wxFONTSTYLE_NORMAL);

    for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
    {
        wt = NULL;
        wt = m_textView->GetStyledWrapText((size_t)i);
        if (wt) // When a line contain only line break bytes. or wrap line doesn't exist.
        {
            int lastAccumPixelSum = 0;
            for (size_t j=0; j<(*wt).size(); j++)
            {
                svThemeClass* tc = m_theme->GetClass((*wt).at(j).w_styleName.ToStdString());
                if (!tc)
                {
                    tc = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString()); 
                }
                wxColour color = tc->GetFGColour();
                m_bTextAreaDC->SetTextForeground(color);
                // wxLogMessage(wxT("draw text: ") + (*wt).at(j).w_text + wxString::Format(wxT("%i"), (*wt).at(j).w_accumPixelWidth) + " " + wxString::Format(wxT("%i"), s.GetWidth()));
                if ((*wt).at(j).w_text==wxT("\t"))
                {
                    m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                    m_bTextAreaDC->DrawText(wxT(">"), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                }
                // else if ((*wt).at(j).w_text==wxT(" "))
                // {
                //     m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                //     m_bTextAreaDC->DrawText(wxT("."), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                // }
                else                    
                {
                    m_bTextAreaDC->DrawText((*wt).at(j).w_text, (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                }

                lastAccumPixelSum = (*wt).at(j).w_accumPixelWidth + (*wt).at(j).w_pixelWidth;
            }

            // draw tail folding indicator.
            if (m_textView->HadFoldingTail(i))
            {
                // wxRect rectFoldingTail(lastAccumPixelSum, baseH + (i * lineHeight), 12, lineHeight);
                // m_bTextAreaDC->DrawRoundedRectangle(rectFoldingTail, 2.2);
                m_bTextAreaDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());
                m_bTextAreaDC->DrawText("~", lastAccumPixelSum, baseH + (i * lineHeight));
            }
        }

    }

/*#ifndef NDEBUG
    // draw debug mouse X Y
    m_bTextAreaDC->SetTextForeground(*wxRED);
    // m_bTextAreaDC->DrawText(wxString::Format("m_x=%i, m_y=%i m_areaI=%i timer=%d", m_mouseX, m_mouseY, m_areaI, m_mouseMotionTimer->IsRunning()), 20, baseH + 20); 
    m_bTextAreaDC->DrawText(wxString::Format("m_x=%i, m_y=%i m_tai.Y=%i, m_tai.Heigh=%i", m_mouseX, m_mouseY, m_tai.Y + m_tai.Height), 20, baseH + 20); 
#endif*/

    // draw caret
#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 07"));
#endif

    if (carets->size()>0)
    {
        for(std::vector<caretPosOnView>::iterator it=carets->begin();
            it!=carets->end();
            ++it)
        {
            m_bTextAreaDC->SetPen(*wxWHITE);
            m_bTextAreaDC->SetBrush(*wxWHITE);
            wxRect rectCaret(it->accumPixelWidth, baseH + (it->row * lineHeight) + 1, 1, lineHeight);
            m_bTextAreaDC->DrawRectangle(rectCaret);
        }
    }
    carets->clear();
    delete carets;

    // draw folding indicator
    if (m_areaI==SVID_FOLDING)
    {
        m_bFoldingIndDC->SetPen(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
        m_bFoldingIndDC->SetBrush(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
        m_bFoldingIndDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());

        for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
        {
            if (m_textView->HadFolding(i))
            {
                // wxRect rInd(1, baseH + (i * lineHeight), 10, lineHeight);
                // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                m_bFoldingIndDC->DrawText("+", 2, baseH + (i * lineHeight));
            }
            else if (m_textView->CanFolding(i))
            {
                // wxRect rInd(3, baseH + (i * lineHeight), 7, lineHeight);
                // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                m_bFoldingIndDC->DrawText("=", 2, baseH + (i * lineHeight));
            }
        }
    }


#ifndef NDEBUG
    wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 08"));
#endif






    // draw next page
    // 繪製畫面下一頁

    if (p_smooth)
    {


        // increment the Y coordinate
        baseH += m_textView->GetCurPageTextListCount() * lineHeight;

        // draw the line number indicator background
        m_bLineNumIndDC->SetPen(defLIBGColor);
        m_bLineNumIndDC->SetBrush(defLIBGColor);
        m_bLineNumIndDC->SetTextForeground(defLIFGColor);

        // draw line number.
        m_font.SetStyle(wxFONTSTYLE_NORMAL);

        ln=0;
        old_ln=-1;

        for (int i=0; i<m_textView->GetNextPageTextListCount(); i++)
        {
            ln = m_textView->GetStyledWrapTextLineNoUW_NP((size_t)i);
            if (ln!=old_ln && ln!=0) // When a line contain only line break bytes. or wrap line doesn't exist.
            {
#ifdef __WXMSW__
                m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %Iu"), ln), 0, baseH + (i * lineHeight)); 
#else
                m_bLineNumIndDC->DrawText(wxString::Format(wxT(" %zu"), ln), 0, baseH + (i * lineHeight)); 
#endif
            }
            old_ln = ln;
        }

#ifndef NDEBUG
        wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 09"));
#endif
        // draw Text Area background.
        m_bTextAreaDC->SetBrush(defBGColor);
        m_bTextAreaDC->SetPen(defBGColor);
        m_bTextAreaDC->SetTextForeground(defFGColor);

        // draw indent indicator line.
        m_bTextAreaDC->SetPen(wxPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour(), 1, wxPENSTYLE_DOT));
        m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        for (int i=0; i<m_textView->GetCurPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetTextIndentIndicatorCharLen_NP(i, pixelList))
            {
                for (vector<int>::iterator it = pixelList.begin();
                     it!=pixelList.end();
                     ++it)
                {
                    m_bTextAreaDC->DrawLine(wxPoint(*it*m_spaceWidth , baseH + (i * lineHeight) + 1), wxPoint(*it*m_spaceWidth, baseH + ((i+1) * lineHeight)));         
                }
            }
        }

        // draw searched text background
        m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_SELECTED")).ToStdString())->GetBGColour());
        for (int i=0; i<m_textView->GetNextPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetCaretSelectPixel_NP(i, pixelList))
            {
                int cnt = pixelList.size();
                cnt /= 2;
                for (int j=0; j<cnt; j++)
                {
                    int sp = pixelList.at(j*2);
                    int ep = pixelList.at(j*2+1);
                    wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                    m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
                }
            }
        }

        // draw finded keyword background
        m_bTextAreaDC->SetPen(m_theme->GetClass(wxString(wxT("_FIND")).ToStdString())->GetFGColour());
        // m_bTextAreaDC->SetBrush(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
        m_bTextAreaDC->SetBrush(*wxTRANSPARENT_BRUSH);
        for (int i=0; i<m_textView->GetNextPageTextListCount(); i++)
        {
            pixelList.clear();
            if (m_textView->GetFindKeywordPixel_NP(i, pixelList))
            {
                int cnt = pixelList.size();
                cnt /= 2;
                for (int j=0; j<cnt; j++)
                {
                    int sp = pixelList.at(j*2);
                    int ep = pixelList.at(j*2+1);
                    wxRect selectCaret(sp, baseH + (i * lineHeight) + 1, ep-sp, lineHeight);
                    m_bTextAreaDC->DrawRoundedRectangle (selectCaret, 2.2);                
                }
            }
        }

        // draw text.

        m_font.SetStyle(wxFONTSTYLE_NORMAL);

        for (int i=0; i<m_textView->GetNextPageTextListCount(); i++)
        {
            wt = NULL;
            wt = m_textView->GetStyledWrapText_NP((size_t)i);
            if (wt) // When a line contain only line break bytes. or wrap line doesn't exist.
            {
                int lastAccumPixelSum = 0;
                for (size_t j=0; j<(*wt).size(); j++)
                {
                    svThemeClass* tc = m_theme->GetClass((*wt).at(j).w_styleName.ToStdString());
                    if (!tc)
                    {
                        tc = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString()); 
                    }
                    wxColour color = tc->GetFGColour();
                    m_bTextAreaDC->SetTextForeground(color);
                    // wxLogMessage(wxT("draw text: ") + (*wt).at(j).w_text + wxString::Format(wxT("%i"), (*wt).at(j).w_accumPixelWidth) + " " + wxString::Format(wxT("%i"), s.GetWidth()));
                    if ((*wt).at(j).w_text==wxT("\t"))
                    {
                        m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                        m_bTextAreaDC->DrawText(wxT(">"), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    }
                    // else if ((*wt).at(j).w_text==wxT(" "))
                    // {
                    //     m_bTextAreaDC->SetTextForeground(tc->GetBGColour().ChangeLightness(110));
                    //     m_bTextAreaDC->DrawText(wxT("."), (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    // }
                    else                    
                    {
                        m_bTextAreaDC->DrawText((*wt).at(j).w_text, (*wt).at(j).w_accumPixelWidth, baseH + (i * lineHeight)); 
                    }

                    lastAccumPixelSum = (*wt).at(j).w_accumPixelWidth + (*wt).at(j).w_pixelWidth;
                }

                // draw tail folding indicator.
                if (m_textView->HadFoldingTail_NP(i))
                {
                    // wxRect rectFoldingTail(lastAccumPixelSum, baseH + (i * lineHeight), 12, lineHeight);
                    // m_bTextAreaDC->DrawRoundedRectangle(rectFoldingTail, 2.2);
                    m_bTextAreaDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());
                    m_bTextAreaDC->DrawText("~", lastAccumPixelSum, baseH + (i * lineHeight));
                }

            }
        }


        // draw folding indicator
        if (m_areaI==SVID_FOLDING)
        {
            m_bFoldingIndDC->SetPen(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
            m_bFoldingIndDC->SetBrush(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetBGColour());
            m_bFoldingIndDC->SetTextForeground(m_theme->GetClass(wxString(wxT("_FOLDING")).ToStdString())->GetFGColour());

            for (int i=0; i<m_textView->GetNextPageTextListCount(); i++)
            {
                if (m_textView->HadFolding_NP(i))
                {
                    // wxRect rInd(1, baseH + (i * lineHeight), 10, lineHeight);
                    // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                    m_bFoldingIndDC->DrawText("+", 2, baseH + (i * lineHeight));
                }
                else if (m_textView->CanFolding_NP(i))
                {
                    // wxRect rInd(3, baseH + (i * lineHeight), 7, lineHeight);
                    // m_bFoldingIndDC->DrawRoundedRectangle (rInd, 1.2);  
                    m_bFoldingIndDC->DrawText("=", 2, baseH + (i * lineHeight));
                }
            }
        }

#ifndef NDEBUG
        wxLogMessage(wxT("svTextEditorCtrl::PrepareDCBuffer 10"));
#endif
    }

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
void svTextEditorCtrl::DoSmoothRefresh(const int p_smooth, char p_funcType)
{
    AdjustScrollbar();

    if (p_smooth==SVID_SMOOTH_NONE)
        PrepareDCBuffer(false);
    else
        PrepareDCBuffer(true);

#ifndef NDEBUG            
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
    // if (p_VScrollLineCnt>m_textView->GetLinesPerPage())
    //     p_VScrollLineCnt=m_textView->GetLinesPerPage();
    // else if (p_VScrollLineCnt<-m_textView->GetLinesPerPage())
    //     p_VScrollLineCnt=-m_textView->GetLinesPerPage();
    int p_VScrollPixelDistance = p_VScrollLineCnt * lineHeight;

    // 推算垂直捲原來的啟始位置
    int p_oldVScrollPixel = m_textView->GetPrevPageTextListCount() * lineHeight - (p_VScrollLineCnt * lineHeight);

    // wxLogMessage(wxString::Format(wxT("m_textView->GetOldHScrollPixel()=%i"), m_textView->GetOldHScrollPixel()));
    // wxLogMessage(wxString::Format(wxT("m_textView->GetHScrollPixel()=%i"), m_textView->GetHScrollPixel()));

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

            Refresh();  // YOU HAVE TO CALL REFRESH() THAN CALL UPDATE()!!!
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

}


/*
 * Clean Client Area.
 */
void svTextEditorCtrl::CleanBackground(const wxString &p_msg)
{
    if (m_bufferDC!=NULL) delete m_bufferDC;
    m_bufferDC = new wxMemoryDC();

    int w, h;
    w=h=0;

    this->GetClientSize(&w, &h);

    wxBitmap bitmapTA(w, h);
    m_bufferDC->SelectObject(bitmapTA);
    m_bufferDC->SetFont(m_font);
    m_bufferDC->SetBrush(wxBrush(*wxWHITE));
    m_bufferDC->SetPen(wxPen(*wxWHITE, 1));

    m_bufferDC->SetBackground(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
    m_bufferDC->Clear();

    wxColour color = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetFGColour();
    m_bufferDC->SetTextForeground(color.ChangeLightness(70));
    // m_bufferDC->DrawText(wxString::Format("w=%i h=%i", w, h), 0, 0); 
    // m_bufferDC->DrawText(wxString::Format("Preparing file...", w/2, h/2), w/2-70, h/2); 
    m_bufferDC->DrawText(p_msg, w/2-70, h/2); 

    Refresh();
    Update();
}

// if p_per>=0 && p_per<=100 then display progress bar.
// else display p_msg as message text.
void svTextEditorCtrl::DisplayLoadingBackground(const wxString &p_msg, const int p_per)
{
    if (m_bufferDC!=NULL) delete m_bufferDC;
    m_bufferDC = new wxMemoryDC();

    int w, h;
    w=h=0;

    this->GetClientSize(&w, &h);

    wxBitmap bitmapTA(w, h);
    m_bufferDC->SelectObject(bitmapTA);
    m_bufferDC->SetFont(m_font);
    m_bufferDC->SetBrush(wxBrush(*wxWHITE));
    m_bufferDC->SetPen(wxPen(*wxWHITE, 1));

    m_bufferDC->SetBackground(m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetBGColour());
    m_bufferDC->Clear();

    wxColour color = m_theme->GetClass(wxString(wxT("_DEFAULT")).ToStdString())->GetFGColour();
    m_bufferDC->SetTextForeground(color.ChangeLightness(70));

    if (p_per>=0 && p_per<=100)
    {
        int x, y;
        x=y=0;
        x = w / 3;
        if (x < 0) x = 0;
        y = (h-10)/2;
        if (y<0) y = 0;
        m_bufferDC->DrawText(p_msg, x, y+12); 
        m_bufferDC->SetBrush(wxBrush(color.ChangeLightness(70)));
        m_bufferDC->SetPen(wxPen(color.ChangeLightness(70), 1));
        m_bufferDC->DrawRoundedRectangle(x, y, x, 10, 2);
        m_bufferDC->SetBrush(wxBrush(color.ChangeLightness(40)));
        m_bufferDC->SetPen(wxPen(color.ChangeLightness(40), 1));
        m_bufferDC->DrawRoundedRectangle(x, y, (x*p_per/100), 10, 2);
    }
    else
    {
        // m_bufferDC->DrawText(wxString::Format("w=%i h=%i", w, h), 0, 0); 
        // m_bufferDC->DrawText(wxString::Format("Preparing file...", w/2, h/2), w/2-70, h/2); 
        m_bufferDC->DrawText(p_msg, w/2-70, h/2); 
    }

    Refresh();
    Update();
}



/* ----------------------------------------------------------------------- *
 *
 *                         Scroll Bar Processing.
 *       #define OS_SCROLLBAR for using wxWidgets scrollbar (OS style)
 *                  Or not defined for using customized style.
 *
 * ----------------------------------------------------------------------- */

#ifdef OS_SCROLLBAR

void svTextEditorCtrl::OnVScrollLineUp(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollLineUp"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(-1);
    m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_LINE_UP));
    // if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_LINE_UP, SVID_DUP_COUNT))
    //     DoSmoothRefresh(SVID_SMOOTH_NONE);
    // else
    //     DoSmoothRefresh();
    DoSmoothRefresh();
    this->SetFocus();
    event.Skip();
}

void svTextEditorCtrl::OnVScrollLineDown(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollLineDown"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(1);
    m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_LINE_DOWN));
    // if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_LINE_DOWN, SVID_DUP_COUNT))
    //     DoSmoothRefresh(SVID_SMOOTH_NONE);
    // else
    //     DoSmoothRefresh();
    DoSmoothRefresh();
    this->SetFocus();
    event.Skip();
}

void svTextEditorCtrl::OnVScrollPageUp(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollPageUp"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_UP));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_HIGH_DUP))
        //DoSmoothRefresh(SVID_SMOOTH_NONE);
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        //DoSmoothRefresh();
        DoSmoothRefresh(SVID_SMOOTH_HIGH);
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVScrollPageDown(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollPageDown"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage());
    m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_DOWN));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
        //DoSmoothRefresh(SVID_SMOOTH_NONE);
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        //DoSmoothRefresh();
        DoSmoothRefresh(SVID_SMOOTH_HIGH);
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVScrollTop(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollTop"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVScrollBottom(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollBottom"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVScrollThumbTrack(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollThumbTrack"));
#endif
    if (!m_mouseReady) return;

    int pos = event.GetPosition();
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnVScrollThumbTrack pos=%i", pos));
#endif
    m_textView->SetFirstLineIndex(pos);
    m_textView->Refresh();
    m_textView->UpdateCaretsInfo();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_THUMB_TRACK));

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());

    DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVScrollThumbRelease(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVScrollThumbRelease"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollLineUp(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollLineUp"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + -1 * m_font.GetPointSize();
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHScrollLineDown ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_LINE_UP));
    // if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_LINE_UP, SVID_DUP_COUNT))
    //     DoSmoothRefresh(SVID_SMOOTH_NONE);
    // else
    //     DoSmoothRefresh();
    DoSmoothRefresh();
    this->SetFocus();
    event.Skip();
}

void svTextEditorCtrl::OnHScrollLineDown(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollLineDown"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + m_font.GetPointSize();
    int maxHExtend = hScrollbar->GetRange()-(hScrollbar->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
    if (newPos > maxHExtend) newPos = maxHExtend;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHScrollLineDown ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_LINE_DOWN));
    // if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_LINE_DOWN, SVID_DUP_COUNT))
    //     DoSmoothRefresh(SVID_SMOOTH_NONE);
    // else
    //     DoSmoothRefresh();
    DoSmoothRefresh();
    this->SetFocus();
    event.Skip();
}

void svTextEditorCtrl::OnHScrollPageUp(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollPageUp"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + (-1) * hScrollbar->GetPageSize()*2/3;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHScrollLineDown old new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();

    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_UP));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_HIGH_DUP))
        //DoSmoothRefresh(SVID_SMOOTH_NONE);
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        //DoSmoothRefresh();
        DoSmoothRefresh(SVID_SMOOTH_HIGH);

    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollPageDown(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollPageDown"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + hScrollbar->GetPageSize()*1/3;
    int maxHExtend = hScrollbar->GetRange()-(hScrollbar->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
    if (newPos > maxHExtend) newPos = maxHExtend;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHScrollLineDown ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_DOWN));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollTop(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollTop"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollBottom(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollBottom"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollThumbTrack(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollThumbTrack"));
#endif
    if (!m_mouseReady) return;

    int newPos = event.GetPosition();
    int maxHExtend = hScrollbar->GetRange()-(hScrollbar->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
    if (newPos > maxHExtend) newPos = maxHExtend;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHScrollThumbTrack ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif    
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_THUMB_TRACK));
    DoSmoothRefresh(SVID_SMOOTH_NONE);
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHScrollThumbRelease(wxScrollEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHScrollThumbRelease"));
#endif
    if (!m_mouseReady) return;

    // m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    // m_textView->UpdateCaretsInfo();

    // vScrollbar->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    // DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

#else

/* -------------------------------------------------------------------------------- */

void svTextEditorCtrl::OnVsvScrollThumbTrack(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVsvScrollThumbTrack"));
#endif
    if (!m_mouseReady) return;

    int pos = event.GetPosition();
    int old_firstLine = m_textView->GetFirstUWLineNo();
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnVsvScrollThumbTrack pos=%i", pos));
#endif

    // // for smmoth scoll
    // if (old_firstLine>pos)
    // {
    //     m_textView->SetFirstLineIndex(pos+1);
    //     m_textView->MoveFirstLineIndex(-1);
    // }
    // else
    // {
    //     m_textView->SetFirstLineIndex(pos-1);
    //     m_textView->MoveFirstLineIndex(1);
    // }

    if (old_firstLine==pos)
    {
        event.Skip(); 
        return;
    }
    else if (old_firstLine>pos && old_firstLine-pos<=SVID_SCROLLTHUMB_SMOOTH)       // for smmoth scoll
    {
        // m_textView->SetFirstLineIndex(pos+1);
        m_textView->MoveFirstLineIndex(pos-old_firstLine);
    }
    else if (old_firstLine<pos && pos-old_firstLine<=SVID_SCROLLTHUMB_SMOOTH)      // for smmoth scoll
    {
        // m_textView->SetFirstLineIndex(pos-1);
        m_textView->MoveFirstLineIndex(pos-old_firstLine);
    }
    else
    {
        m_textView->SetFirstLineIndex(pos);  // no smooth scroll
    }

    m_textView->Refresh();
    m_textView->UpdateCaretsInfo();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_THUMB_TRACK));

    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());

    DoSmoothRefresh(SVID_SMOOTH_DEFAULT, SVID_SMOOTH_AVERAGE);
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVsvScrollPageUp(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVsvScrollPageUp"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage()*-1);
    m_textView->UpdateCaretsInfo();

    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_UP));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_HIGH_DUP))
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_UP, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnVsvScrollPageDown(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnVsvScrollPageDown"));
#endif
    if (!m_mouseReady) return;

    m_textView->MoveFirstLineIndex(m_textView->GetLinesPerPage());
    m_textView->UpdateCaretsInfo();

    m_vsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_V_SCROLL_PAGE_DOWN));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    else if (m_textView->DuplicatedAction(SVID_ACTION_V_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHsvScrollThumbTrack(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHsvScrollThumbTrack"));
#endif
    if (!m_mouseReady) return;

    int newPos = event.GetPosition();
    // int maxHExtend = m_hsb->GetRange()-(m_hsb->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
    // if (newPos > maxHExtend) newPos = maxHExtend;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHsvScrollThumbTrack ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif    
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_THUMB_TRACK));
    DoSmoothRefresh(SVID_SMOOTH_NONE);
    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHsvScrollPageUp(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHsvScrollPageUp"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + (-1) * m_hsb->GetPageSize()*2/3;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHsvScrollLineDown old new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();

    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_UP));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_HIGH_DUP))
        //DoSmoothRefresh(SVID_SMOOTH_NONE);
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_UP, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        //DoSmoothRefresh();
        DoSmoothRefresh(SVID_SMOOTH_HIGH);

    this->SetFocus();
    event.Skip();    
}

void svTextEditorCtrl::OnHsvScrollPageDown(svScrollBarEvent& event)
{
#ifndef NDEBUG
    wxLogMessage(wxT("OnHsvScrollPageDown"));
#endif
    if (!m_mouseReady) return;

    int newPos = m_textView->GetHScrollPixel() + m_hsb->GetPageSize()*1/3;
    // int maxHExtend = m_hsb->GetRange()-(m_hsb->GetPageSize()*2/3);    // 最多將文字最後一字水平捲動至畫面2/3處
    // if (newPos > maxHExtend) newPos = maxHExtend;
    if (newPos < 0) newPos = 0;
#ifndef NDEBUG
    wxLogMessage(wxString::Format("OnHsvScrollLineDown ole new Pos=%i %i", m_textView->GetHScrollPixel(), newPos));
#endif
    m_textView->SetHScrollPixel(newPos); 
    // m_textView->UpdateCaretsInfo();

    m_hsb->SetThumbPosition(m_textView->GetFirstUWLineNo());
    // m_textView->Refresh();
    m_textView->AppendActionRecord(svAction(SVID_ACTION_H_SCROLL_PAGE_DOWN));
    // 重覆按鍵情況下加速捲動
    if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_HIGH_DUP))
        DoSmoothRefresh(SVID_SMOOTH_NONE);
    else if (m_textView->DuplicatedAction(SVID_ACTION_H_SCROLL_PAGE_DOWN, SVID_LOW_DUP))
        DoSmoothRefresh(SVID_SMOOTH_LOW);
    else
        DoSmoothRefresh();
    this->SetFocus();
    event.Skip();    
}

#endif

void svTextEditorCtrl::ResetBufTextCarets(svFileDescOpened *p_fdo)
{
    m_bufText->SetCarets(p_fdo->m_carets);
    m_textView->SetFirstLineIndex(p_fdo->m_firstLineNo);
    m_textView->Refresh();
}

void svTextEditorCtrl::SetupFont(void)
{
    extern svPreference g_preference;
    m_font.SetFaceName(g_preference.GetFontFace());
    m_font.SetPointSize(g_preference.GetFontSize());
    m_font.SetStyle(wxFONTSTYLE_NORMAL);

    // prepare dc for cal space width and line number indicator area max width in pixel count.
    wxMemoryDC dc;
    wxSize s;
    wxBitmap bitmap(100, 100);
    dc.SelectObject(bitmap);
    dc.Clear();
    dc.SetFont(m_font);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(wxBrush(*wxWHITE));

    // Should be change to lazy bind. Preventing high CPU loading at a time.
    m_charHeight = m_font.GetPixelSize().GetHeight();
    s = dc.GetTextExtent(wxT(" "));
    // s = dc.GetTextExtent(wxT("A"));  // For 不等寬字, space width may less than average width.
    m_spaceWidth = s.GetWidth();

    if (m_textView)
    {
        m_textView->SetFont(m_font);
        m_textView->SetCharHeight(m_charHeight);
        m_textView->SetSpaceWidth(m_spaceWidth);
    }

}

