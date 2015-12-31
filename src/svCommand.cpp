/*
   Copyright Notice in awvic.cpp
*/

#include "svCommand.h"

#include "stdwx.h"

svCommand::svCommand(int cmd, const wxKeyEvent& event):
m_cmd(cmd), 
m_cmdSrc(SVID_CMD_KEY), 
m_keyCode(event.GetKeyCode()), 
m_unicodeKey(event.GetUnicodeKey()),
m_altDown(event.AltDown()), 
m_cmdDown(event.CmdDown()), 
m_controlDown(event.ControlDown()),
m_metaDown(event.MetaDown()), 
m_shiftDown(event.ShiftDown())
{
}

svCommand::svCommand(int cmd, const wxMouseEvent& event):
m_cmd(cmd), 
m_cmdSrc(SVID_CMD_MOUSE), 
m_mouseLeftDown(event.LeftDown()), 
m_mouseMiddleDown(event.MiddleDown()), 
m_mouseRightDown(event.RightDown()), 
m_mouseX(event.GetX()), 
m_mouseY(event.GetY()),
m_mouseWheelRotation(event.GetWheelRotation()), 
m_mouseWheelDelta(event.GetWheelDelta())
{
}

svCommand::~svCommand()
{
}

int svCommand::Name(void)
{
    return m_cmd;
}

void svCommand::SetCaretPosition(int p_row, int p_col)
{
    m_row = p_row;
    m_col = p_col;
}

bool svCommand::AltDown(void)
{
    return m_altDown;
}

bool svCommand::CmdDown(void)
{
    return m_cmdDown;
}

bool svCommand::ControlDown(void)
{
    return m_controlDown;
}

bool svCommand::MetaDown(void)
{
    return m_metaDown;
}

bool svCommand::ShiftDown(void)
{
    return m_shiftDown;
}

int svCommand::GetKeyCode(void)
{
    return m_keyCode;
}

wxChar svCommand::GetUnicodeKey(void)
{
    return m_unicodeKey;
}

