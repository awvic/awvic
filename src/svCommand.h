/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVCOMMAND_H
#define _SVCOMMAND_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

enum
{
    // text mode command
    CMD_TXT_INSERT=0x1001,
    CMD_TXT_INS_UNI,
    CMD_TXT_SPLIT,
    CMD_TXT_DELETE,              // x
    CMD_TXT_BACKDEL,             // x
    CMD_TXT_CUT,                 // x d
    CMD_TXT_COPY,                // y
    CMD_TXT_PASTE,               // p 
    CMD_TXT_DUP_LINE,            //
    CMD_TXT_UNDO,                // u 
    CMD_TXT_LINE_HEAD,           // ^
    CMD_TXT_LINE_END,            // $
    CMD_TXT_UP,                  // j
    CMD_TXT_DOWN,                // k
    CMD_TXT_LEFT,                // h 
    CMD_TXT_LEFT_HEAD,           // h 
    CMD_TXT_RIGHT,               // l
    CMD_TXT_RIGHT_END,           // l
    CMD_TXT_PAGEUP,              // ctrl+f
    CMD_TXT_PAGEDOWN,            // ctrk+b
    CMD_TXT_TOP_SCREEN,          // H
    CMD_TXT_MIDDLE_SCREEN,       // M
    CMD_TXT_BOTTOM_SCREEN,       // M
    CMD_TXT_TOP,                 // gg
    CMD_TXT_BOTTOM,              // G
    CMD_TXT_CUR_TOP_SCREEN,      // zt
    CMD_TXT_CUR_BOTTOM_SCREEN,   // zb
    CMD_TXT_CUR_MIDDLE_SCREEN,   // zz
    CMD_TXT_SEL_START,
    CMD_TXT_SEL_END,
    CMD_TXT_POPUP_MENU,
    CMD_TXT_SEARCH,
    CMD_TXT_REPLACE,
    CMD_TXT_SAVE,
    CMD_TXT_SAVEAS,
    CMD_TXT_CLOSE,
    CMD_TXT_INS_TAB,
    CMD_TXT_LINE_COMMENT,
    CMD_TXT_BLOCK_COMMENT,
    CMD_TXT_INDENT,
    CMD_TXT_OUTDENT,
    CMD_TXT_RESET_CARETS,
    CMD_TXT_HIDE_HINT,
    CMD_TXT_HIDE_COMMAND_LINE,
    CMD_TXT_HIDE_FIND,
    CMD_TXT_FIND_CURRENT_WORD,
    CMD_TXT_FIND_CURRENT_WORD_ALL,
    CMD_TXT_FIND_NEXT_WORD,
    CMD_TXT_FIND_PREV_WORD,
    CMD_TXT_SIDEBAR,
    CMD_TXT_DEBUG01,
    CMD_TXT_DEBUG02,
    CMD_SHOW_COMMAND_LINE,
    CMD_SHOW_COMMAND_LINE_GOTO, 
    CMD_SHOW_COMMAND_LINE_GOTO_DEFINITION, 

    // VI mode command
    CMD_VI_INSERT=0x2001,
    CMD_VI_DELETE,              // x
    CMD_VI_CUT,                 // x d
    CMD_VI_COPY,                // y
    CMD_VI_PASTE,               // p 
    CMD_VI_HEAD_LINE,           // ^
    CMD_VI_END_LINE,            // $
    CMD_VI_UP,                  // j
    CMD_VI_DOWN,                // k
    CMD_VI_LEFT,                // h 
    CMD_VI_RIGHT,               // l
    CMD_VI_PAGEUP,              // ctrl+f
    CMD_VI_PAGEDOWN,            // ctrk+b
    CMD_VI_TOP_SCREEN,          // H
    CMD_VI_MIDDLE_SCREEN,         // M
    CMD_VI_BOTTOM_SCREEN,         // M
    CMD_VI_TOP_FILE,            // gg
    CMD_VI_BOTTOM_FILE,         // G
    CMD_VI_CUR_TOP_SCREEN,      // zt
    CMD_VI_CUR_BOTTOM_SCREEN,   // zb
    CMD_VI_CUR_MIDDLE_SCREEN,   // zz
    CMD_VI_SEL_START,
    CMD_VI_SEL_END,
    CMD_VI_POPUP_MENU,
    CMD_VI_SEARCH,
    CMD_VI_REPLACE,
    CMD_VI_SAVE,
    CMD_VI_SAVEAS,
    CMD_VI_CLOSE,

    // HEX mode command
    CMD_HEX_INSERT=0x3001,
    CMD_HEX_DELETE,              // x
    CMD_HEX_CUT,                 // x d
    CMD_HEX_COPY,                // y
    CMD_HEX_PASTE,               // p 
    CMD_HEX_HEAD_LINE,           // ^
    CMD_HEX_END_LINE,            // $
    CMD_HEX_UP,                  // j
    CMD_HEX_DOWN,                // k
    CMD_HEX_LEFT,                // h 
    CMD_HEX_RIGHT,               // l
    CMD_HEX_PAGEUP,              // ctrl+f
    CMD_HEX_PAGEDOWN,            // ctrk+b
    CMD_HEX_TOP_SCREEN,          // H
    CMD_HEX_DOWN_SCREEN,         // M
    CMD_HEX_TOP_FILE,            // gg
    CMD_HEX_BOTTOM_FILE,         // G
    CMD_HEX_CUR_TOP_SCREEN,      // zt
    CMD_HEX_CUR_BOTTOM_SCREEN,   // zb
    CMD_HEX_CUR_MIDDLE_SCREEN,   // zz
    CMD_HEX_SEL_START,
    CMD_HEX_SEL_END,
    CMD_HEX_POPUP_MENU,
    CMD_HEX_SEARCH,
    CMD_HEX_REPLACE,
    CMD_HEX_SAVE,
    CMD_HEX_SAVEAS,
    CMD_HEX_CLOSE,

    // others command
    CMD_UNDEFINED=0x4001,
    CMD_UNKNOW
};

enum
{
    SVID_CMD_KEY=0,
    SVID_CMD_MOUSE
};

class svCommand
{
private:
    int m_cmd;
    int m_cmdSrc;

    // key 
    int m_keyCode;
    wxChar m_unicodeKey;
    int m_modifiers;
    bool m_altDown;
    bool m_cmdDown;
    bool m_controlDown;
    bool m_metaDown;
    bool m_shiftDown;

    // mouse
    bool m_mouseLeftDown;
    bool m_mouseMiddleDown;
    bool m_mouseRightDown;
    long m_mouseX;
    long m_mouseY;
    int m_mouseWheelRotation;
    int m_mouseWheelDelta;

    // text ctrl coordinate
    int m_row;
    int m_col;

public:
    svCommand(int cmd, const wxKeyEvent& event);
    svCommand(int cmd, const wxMouseEvent& event);
    ~svCommand();
    int Name(void);
    void SetCaretPosition(int p_row, int p_col);

    bool AltDown(void);
    bool CmdDown(void);
    bool ControlDown(void);
    bool MetaDown(void);
    bool ShiftDown(void);

    int GetKeyCode(void);
    wxChar GetUnicodeKey(void);
};

#endif
