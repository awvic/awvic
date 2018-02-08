/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVTEXTEDITORCTRL_H
#define _SVTEXTEDITORCTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <memory>
#include <wx/caret.h>
#include <wx/textfile.h>
#include <wx/dcbuffer.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#include "svBufText.h"
#include "svCommand.h"
#include "svListOfIntList.h"
#include "svTextView.h"
#include "svTheme.h"
#include "svScrollBar.h"
#include "svCommandLineCtrl.h"
#include "svBaseType.h"
#include "svTypeHintCtrl.h"
#include "svListBoxCtrl.h"

using namespace std;

#define PI 3.14159265

#define SVID_VERY_LOW_FPS     3   // 3       /* smooth scroll frames */
#define SVID_LOW_FPS     4   // 3       /* smooth scroll frames */
#define SVID_MEDIUM_FPS 16  // 30      /* smooth scroll frames */
#define SVID_HIGH_FPS    30  // 60      /* smooth scroll frames */

#define SVID_LOW_DUP   4  // 計算重覆動作的次數，超過這個次數表示使用者一直重覆某個動作(目前只考慮page up page down)，則作 low smooth scroll
#define SVID_HIGH_DUP  7  // 計算重覆動作的次數，超過這個次數表示使用者一直重覆某個動作(目前只考慮page up page down)，則不作 smooth scroll

#define SVID_SCROLLTHUMB_SMOOTH   2  // 計算垂直scrollbar捲動幾行內作平滑捲動


#define SVID_FOLDING_AREA_WIDTH   12  // Folding indicator area width.
// #define OS_SCROLLBAR   // TURN ON for using OS scroll bar.

#define SVID_MOUSE_MOTION_TIMER_INTERVAL  100
#define SVID_MOUSE_WHEEL_SPEED  40

class svMainFrame;
class svMouseMotionTimer;

enum
{
    SVID_SMOOTH_NONE=0,
    SVID_SMOOTH_LOW,
    SVID_SMOOTH_MEDIUM,
    SVID_SMOOTH_HIGH,
    SVID_SMOOTH_DEFAULT
};

enum
{
    SVID_SMOOTH_SIN=0,
    SVID_SMOOTH_AVERAGE
};


// Define a customed event
class svTextCtrlEvent : public wxNotifyEvent
{
private:
    bool m_modified;

public:
    svTextCtrlEvent(wxEventType commandType = wxEVT_NULL, int id = 0):
    wxNotifyEvent(commandType, id)
    {
        m_modified = false;
    }
    svTextCtrlEvent(const svTextCtrlEvent& event):
    wxNotifyEvent(event){}
    virtual wxEvent *Clone() const
    { 
        return new svTextCtrlEvent(*this); 
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


    DECLARE_DYNAMIC_CLASS(svTextCtrlEvent);
};

typedef void (wxEvtHandler::*svTextCtrlEventFunction)
(svTextCtrlEvent&);

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_EVENT, 811)
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_MODIFIED_CHANGED, 812)
DECLARE_EVENT_TYPE(svEVT_TEXTCTRL_MSG, 813)
END_DECLARE_EVENT_TYPES()

#define EVT_SVTEXTCTRL_EVENT(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_EVENT, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svTextCtrlEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVTEXTCTRL_MODIFIED_CHANGED(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_MODIFIED_CHANGED, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svTextCtrlEventFunction) & fn, \
    (wxObject *) NULL ),

#define EVT_SVTEXTCTRL_MSG(id, fn) DECLARE_EVENT_TABLE_ENTRY( \
    svEVT_TEXTCTRL_MSG, id, -1, (wxObjectEventFunction) \
    (wxEventFunction) (svTextCtrlEventFunction) & fn, \
    (wxObject *) NULL ),

enum
{
    SVID_VSCROLLBAR=wxID_HIGHEST+1,
    SVID_HSCROLLBAR,

    SVID_SVVSB,
    SVID_SVHSB,

    SVID_TXTAREA,
    SVID_LINNUM,
    SVID_FOLDING,
    SVID_COLNUM,
    SVID_VSCB,
    SVID_HSCB,
    SVID_OUTOFBORDER,

    SVID_DEL_NO_LINE,
    SVID_DEL_1_LINE,
    SVID_DEL_COUPLE_LINES,

    SVID_CTM_CUT,
    SVID_CTM_COPY,
    SVID_CTM_PASTE,

    SVID_CMD_LINE,

    SVID_LB01

};

// edit mode list;
enum
{
    SVID_TEXT_MODE=0x1000,  // 0001 0000 0000 0000
    SVID_VI_MODE=0x2000,    // 0002 0000 0000 0000
    SVID_HEX_MODE=0x3000    // 0003 0000 0000 0000
};

enum
{
    SVID_TEXT_NO_SCB=0,
    SVID_TEXT_VSCB=1,
    SVID_TEXT_HSCB=2,
    SVID_TEXT_BOTH_SCB=3
};

enum
{
    SVID_CTRL_DOWN=1,
    SVID_SHIFT_DOWN=2,
    SVID_ALT_DOWN=4
};

// The following declaration proves how STUPID I am!

typedef struct {
    int Width, Height;   // Text Area Width & Height.
    int X, Y;            // Text Area Start Position(X&Y) from DC drawing area. 
    int OffsetX;         // Horizontal Scrollbar offset.
} TextAreaInfo;

typedef struct {
    int Width;
    int Height;   // Client Drawing Area Width & Height.
} ClientDrawingAreaInfo;

typedef struct {
    int Width;
    int Height;   // Client Drawing Area Width & Height.
} WindowsClientInfo;

// typedef struct {
//     int Width;
//     int Height;                 // Vertical Scroll Bar Width & Height. 
//     int X;
//     int Y;                      // Vertical Scroll Bar Start Position(X&Y). 
// } VScrollbarInfo;

// typedef struct {
//     int Width;
//     int Height;                 // Horizontal Scroll Bar Width & Height.
//     int X;
//     int Y;                      // Horizontal Scroll Bar Start Position(X&Y).
// } HScrollbarInfo;

typedef struct {
    int Width;
    int Height;                 // Column Number Indicator Area Width & Height.
    int X;
    int Y;                      // Column Number Indicator Area Start Position(X&Y).
} ColumnNumberIndicatorAreaInfo;

typedef struct {
    int Width;
    int Height;                 // Line Number Indicator Area Width & Height.
    int X;
    int Y;                      // Line Number Indicator Area Start Position(X&Y).
    int BorderWidth;            // Line Number Indicator Area Border Width.
} LineNumberIndicatorAreaInfo;

typedef struct {
    int Width;
    int Height;                 // Folder Indicator Area Width & Height.
    int X;
    int Y;                      // Folder Indicator Area Start Position(X&Y).
    int BorderWidth;            // Folder Indicator Area Border Width.
} FolderIndicatorAreaInfo;

typedef struct {
    int Width;
    int Height;                 // Border Width & Height.
} BorderAreaInfo;

typedef struct {
    int Width;                  // Area width.
    int Height;                 // Area height.
    int X;                      // Area location X.
    int Y;                      // Area location Y.
    int BorderWidth;            // Area border width.
} AreaInfo;


class svTextEditorCtrl : public wxWindow
{
public:
    svTextEditorCtrl(svMainFrame *mainFrame, wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name);
    ~svTextEditorCtrl();
    void OnErase(wxEraseEvent& evet);
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void DoOnSize(void);
    void OnChar(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnSetFocus(wxFocusEvent& event);

    void OnMouseEnterSB(wxMouseEvent& event);

    bool InitNewFile(const wxString& filename);
    bool ReadTextFile(const wxString& filename);
    bool Reset(void);    
    bool SaveFile(void);
    bool SaveFile(const wxString& filename);    

    void AdjustScrollbar(void);

    void SetFileDesc(const svFileDesc &p_fd);
    inline
    svFileDesc GetFileDesc(void)
    {
        return m_fileDesc;
    }
    wxString GetDisplayName(void);
    wxString GetFullPathName(void);

    wxString GetTextFileCharSet(void);    

    void ResetSyntaxHilight(const wxString& filename);

    svCommand* EmergeTextCommand(const wxKeyEvent& event);
    svCommand* EmergeTextCommand2(const wxKeyEvent& event);
    void ProcessTextCommand(svCommand* cmd);
    void DoTextNavi(svCommand* cmd);

    void DoTextInsert(svCommand* cmd);
    void DoTextInsertHint(const wxString p_str);
    void DoTextSplitLine(svCommand* cmd);
    void DoTextDelete(svCommand* cmd);
    void DoTextBackDelete(svCommand* cmd);
    void DoTextCopy(svCommand* cmd);
    void DoTextPaste(svCommand* cmd);
    void DoTextCut(svCommand* cmd);
    void DoTextDuplicateLine(svCommand* cmd);
    void DoTextLineComment(svCommand* cmd);
    void DoTextBlockComment(svCommand* cmd);
    void DoTextIndent(svCommand* cmd);
    void DoTextOutdent(svCommand* cmd);
    void DoTextResetCarets(svCommand* cmd);
    void DoTextUndo(svCommand* cmd);

    void DoFindMatchLocations(const svFindReplaceOption &p_frOption);

    void DoFindCurrentWord(void);
    void DoFindNext(void);
    void DoFindPrev(void);
    void DoFindAll(void);

    void DoReplaceNext(const svFindReplaceOption &p_frOption);
    void DoReplacePrev(const svFindReplaceOption &p_frOption);
    void DoReplaceAll(const svFindReplaceOption &p_frOption);

    inline
    svFindReplaceOption GetBufferFindReplaceOption(void)
    {
        return m_bufText->GetLastFindReplaceOption();
    }

    void DoShowHideCommandLine(char p_commandType);

    void DoResizeChildArea(void);
    void DoResizeLineNoIndicatorArea(void);


    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseRightUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    int  MouseXY2Area(int p_mouseX, int p_mouseY, bool p_setCursor=true);

    void PrepareDCBuffer(bool p_smooth);
    void DoSmoothRefresh(const int p_smooth=SVID_SMOOTH_DEFAULT, char p_funcType=SVID_SMOOTH_SIN);
    void CleanBackground(const wxString &p_msg);
    void DisplayLoadingBackground(const wxString &p_msg, const int p_per);

#ifdef OS_SCROLLBAR
    // Vertical Scroll Bar
    void OnVScrollLineUp(wxScrollEvent& event);
    void OnVScrollLineDown(wxScrollEvent& event);
    void OnVScrollPageUp(wxScrollEvent& event);
    void OnVScrollPageDown(wxScrollEvent& event);
    void OnVScrollTop(wxScrollEvent& event);
    void OnVScrollBottom(wxScrollEvent& event);
    void OnVScrollThumbTrack(wxScrollEvent& event);
    void OnVScrollThumbRelease(wxScrollEvent& event);

    // Hertical Scroll Bar
    void OnHScrollLineUp(wxScrollEvent& event);
    void OnHScrollLineDown(wxScrollEvent& event);
    void OnHScrollPageUp(wxScrollEvent& event);
    void OnHScrollPageDown(wxScrollEvent& event);
    void OnHScrollTop(wxScrollEvent& event);
    void OnHScrollBottom(wxScrollEvent& event);
    void OnHScrollThumbTrack(wxScrollEvent& event);
    void OnHScrollThumbRelease(wxScrollEvent& event);

#else
    // Vertical Scroll Bar
    void OnVsvScrollThumbTrack(svScrollBarEvent& event);
    void OnVsvScrollPageUp(svScrollBarEvent& event);
    void OnVsvScrollPageDown(svScrollBarEvent& event);
    void OnHsvScrollThumbTrack(svScrollBarEvent& event);
    void OnHsvScrollPageUp(svScrollBarEvent& event);
    void OnHsvScrollPageDown(svScrollBarEvent& event);
#endif

    inline
    void GotoLine(const int p_line)
    {
        m_textView->GotoLine(p_line);
        DoSmoothRefresh();
    }

    inline
    bool BufferIsModified(void)
    {
        return m_bufText->IsModified();
    }

    void ResetBufTextCarets(svFileDescOpened *p_fdo);


    void CheckTextModified(void);

    void SetupFont(void);

    inline
    bool TextViewIsSet(void)
    {
        if (m_textView)
            return (m_textView->HasBuffer());
        else
            return false;
    }

    inline
    void ShowTypeHint(vector<wxString> &p_hintList)
    {
        m_typeHintCtrl->SetHints(p_hintList);
        m_typeHintCtrl->Show();
    }
    inline
    void HideTypeHint(void)
    {
        m_typeHintCtrl->Hide();
        m_listBoxCtrl->Hide();
    }

    inline
    bool GetDefinitionLineNo(vector<svIntText> &p_defLineNo)
    {
        if (m_bufText)
            return m_bufText->GetDefinitionLineNo(p_defLineNo);
        else
            return false;
    }


private:

    svMainFrame *m_mainFrame;

#ifdef OS_SCROLLBAR
    wxScrollBar* hScrollbar;  // horizontal scrollbar. 
    wxScrollBar* vScrollbar;  // vertical scrollbar.
#else
    svScrollBar *m_vsb;
    svScrollBar *m_hsb;
#endif

    svCommandLineCtrl* m_commandLineCtrl;  // Command Line Control.
    bool m_showCommandLine;       // Display command line or not.

    svBufText* m_bufText;     // Editing text buffer.

    WindowsClientInfo m_wci;

    
    int m_linesPerTxtArea;    // How many lines in a display area;


    int m_hChar;              // Character(text) height.
    int m_rowInterval;        // line interval height.
    static const int m_rowIntPct = 20; // line interval height ratio

    ClientDrawingAreaInfo m_cdai;

    int m_scb_type;             // 0:no scrollbar  1:horizontal scrollbar 2:vertical scrollbar 3:both scrollbar

    // VScrollbarInfo m_vsbi;
    // HScrollbarInfo m_hsbi;
    ScrollbarInfo m_vsbi;
    ScrollbarInfo m_hsbi;


    // VScrollbarInfo m_svvsbi;
    // HScrollbarInfo m_svhsbi;
    ScrollbarInfo m_svvsbi;
    ScrollbarInfo m_svhsbi;


    TextAreaInfo m_tai;


    ColumnNumberIndicatorAreaInfo m_cniai;

    LineNumberIndicatorAreaInfo m_lniai;

    AreaInfo m_fai;                // folding area information.

    BorderAreaInfo m_border;


    long m_txtMaxW;             // Text max width in pixel.
    bool m_setCaretOnTxtArea;   // Set caret int show area (horizontal). vertical pls call DoTextCaretOnTextArea()


    int m_selSX, m_selSY;       // Selected string start position.
    int m_selEX, m_selEY;       // Selected string end position.


    int m_mouseX, m_mouseY;     // Mouse X, Y.

    int m_areaI;                // indicator for mouse in which area. 


    bool m_mouseLeftIsDown;
    int  m_wheelScrollLines;



    svFileDesc m_fileDesc;      // full path & display file name.
    bool m_modified;            // indicator for modified or not.
                                // 保留的是前一個狀態
                                // 最新的 modified 狀態要參考 m_bufText->IsModified()
                                // 用以比對當 m_modified != m_bufText->IsModified() 時 觸發 svEVT_TEXTCTRL_MODIFIED_CHANGED event 


    bool m_showDBGMSG;



    int  m_mode;                // editing mode.




    int m_dbgcnt;



    // To be deprecated and move to omStyledText.
    // programming language & style type
    wxString m_pgmLang;         // programming language of editing file.
    wxString m_style;           // style name.



    // message text #1 #2 #3
    wxString m_msg1;
    wxString m_msg2;
    wxString m_msg3;



    wxMenu* m_contextMenu;       // context menu for right mouse click.


    int m_tabWidth;              // tab width (should be varity later). 1 tab equal to how many space(s)

    svTextView* m_textView;      // buffer record screen(display area) text string which replace [tab] with proper space width.


    bool m_mouseReady;           // indicator for ready to process mouse signal or not.



    size_t m_maxColumn, m_lineWrap; // line wrap control




    wxFont m_font;
    int m_charHeight;
    int m_spaceWidth;

    svTheme* m_theme;


    bufTextIndex debug_textIdx;  // for debug.

    // A Window cannot have more than 2 wxCarets. Only one caret will be shown.
    // wxCaret is not copyable. use unique_ptr with move to push_back to a vector.
    // 考慮修改 wxCaret 讓一個 caret 可以同時顯示多個矩形，目前暫時用dc.DrawRectangle方式處理。
    // vector<unique_ptr<wxCaret> > m_wxCaretList;


    // for smooth scroll.
    wxMemoryDC *m_bTextAreaDC;         // DC for Text Area.
    wxMemoryDC *m_bLineNumIndDC;       // DC for Line number indicator 
    wxMemoryDC *m_bFoldingIndDC;       // DC for Folding indicator 
    wxMemoryDC *m_bColNumIndDC;        // DC for Column number indicator

    wxMemoryDC *m_bufferDC;            // buffer DC for onPaint called.

    int m_bTextAreaDC_x;                    // The relational Text Area DC coordinator x to be displayed.
    int m_bTextAreaDC_y;                    // The relational Text Area DC coordinator y to be displayed.

    svMouseMotionTimer *m_mouseMotionTimer;

    svFindReplaceOption m_lastSearchOption; // last search option. << BAD practice of comment.

    svTypeHintCtrl *m_typeHintCtrl;         // Typing hint control.

    // svTypeHintCtrl *m_listBoxCtrl;
    svListBoxCtrl *m_listBoxCtrl;

private:
    DECLARE_EVENT_TABLE()

};


/*enum
{
    SVTEXT_MODIFIED=1,
    SVTEXT_NEW=2,
    SVTEXT_OPEN=4
};*/

class svMouseMotionTimer : public wxTimer
{
private:
    svTextEditorCtrl *m_textCtrl;

public:
    svMouseMotionTimer(svTextEditorCtrl *p_textCtrl)
    {
        m_textCtrl = p_textCtrl;
    }

    virtual void Notify() override
    {
        wxMouseEvent e;
        wxPoint p = wxGetMousePosition();
        p = m_textCtrl->ScreenToClient(p);
        e.SetX(p.x);
        e.SetY(p.y);
        e.SetLeftDown(true);
        m_textCtrl->OnMouseMotion(e);
#ifndef NDEBUG
        wxLogMessage(wxString::Format("svMouseMotionTimer::Notify() %i %i", e.m_x, e.m_y));
#endif
    }
};


#endif
