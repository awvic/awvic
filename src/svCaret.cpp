/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include "svCaret.h"

#include "stdwx.h"
/*
 * svCaret Class
 */
svCaret::svCaret():
m_row(0),
m_visual_col(0),
m_keep_space(0),
m_select_row(-1),
m_select_col(-1),
m_select_keep_space(0)/*,
m_is_first(false),
m_is_last(false)*/
{
}

svCaret::svCaret(int p_row, int p_col):
m_row(p_row),
m_visual_col(p_col),
m_keep_space(p_col),
m_select_row(-1),
m_select_col(-1),
m_select_keep_space(0)/*,
m_is_first(false),
m_is_last(false)*/
{
}

svCaret::svCaret(int p_row, int p_visual_col, int p_keep_space, int p_select_row, int p_select_col, int p_select_keep_space):
m_row(p_row),
m_visual_col(p_visual_col),
m_keep_space(p_keep_space),
m_select_row(p_select_row),
m_select_col(p_select_col),
m_select_keep_space(p_select_keep_space)
{
}

svCaret::~svCaret()
{
}

void svCaret::GetPosition(int &p_row, int &p_visual_col) const
{
    p_row = m_row;
    p_visual_col = m_visual_col;
}

// 如果 m_select_row 已經有值便不會再重設
void svCaret::SetSelect(void)
{
    if (m_select_row==SVID_NO_SELECT)
    {
        m_select_row = m_row;
        m_select_col = m_visual_col;
        m_select_keep_space = m_keep_space;
    }
}

void svCaret::ClearSelect(void)
{
    m_select_row = -1;
    m_select_col = -1;
    m_select_keep_space = 0;
}

// 取得選取的caret位置區間
// 回傳值已經排序，小者在前，大者在後
void svCaret::GetSelectPos(int &p_select_srow, int &p_select_scol, int &p_select_erow, int &p_select_ecol) const
{

    if (m_select_row < m_row)
    {
        p_select_srow = m_select_row;
        p_select_scol = m_select_col;
        p_select_erow = m_row;
        p_select_ecol = m_visual_col;

    }
    else if (m_select_row == m_row)
    {
        if (m_select_col<m_visual_col)
        {
            p_select_srow = m_select_row;
            p_select_scol = m_select_col;
            p_select_erow = m_row;
            p_select_ecol = m_visual_col;
        }
        else
        {
            p_select_srow = m_row;
            p_select_scol = m_visual_col;
            p_select_erow = m_select_row;
            p_select_ecol = m_select_col;
        }

    } if (m_select_row > m_row)
    {
        p_select_srow = m_row;
        p_select_scol = m_visual_col;
        p_select_erow = m_select_row;
        p_select_ecol = m_select_col;
    }

}

char svCaret::SelectType(void)
{
    if (m_select_row!=SVID_NO_SELECT)
    {
        if ( m_select_row<m_row || 
            (m_select_row==m_row && m_select_col<m_visual_col))
        {
            return SVID_CARET_ON_TAIL;
        }
        else
        {
            return SVID_CARET_ON_HEAD;
        }
    }
    else
    {
        return SVID_NO_SELECT;
    }
}

// 檢查兩個游標是否重疉
bool svCaret::IsOverlaped(const svCaret p_target)
{
    int bsr, bsc, ber, bec;
    int tsr, tsc, ter, tec;

    bsr=bsc=ber=bec=tsr=tsc=ter=tec=0;

    // if (this->HasSelect() && p_target.HasSelect())
    if (this->HasSelect())
    {
        GetSelectPos(bsr, bsc, ber, bec);
    }
    else
    {
        GetPosition(bsr, bsc);
        ber = bsr;
        bec = bsc;
    }
    if (p_target.HasSelect())
    {
        p_target.GetSelectPos(tsr, tsc, ter, tec);
    }
    else
    {
        p_target.GetPosition(tsr, tsc);
        ter = tsr;
        tec = tsc;
    }

    double bs = bsr * 10000000000 + bsc;
    double be = ber * 10000000000 + bec;
    double ts = tsr * 10000000000 + tsc;
    double te = ter * 10000000000 + tec;

    if ((bs>=ts&&bs<=te) ||
        (be>=ts&&be<=te) ||
        (bs<ts&&be>te))
    {
        return true;
    }

    return false;

}


// Merge two carets data
void svCaret::MergeOverlap(svCaret &p_target)
{
    char b_select_type = this->SelectType();
    char t_select_type = p_target.SelectType();

    int bsr, bsc, ber, bec;
    int tsr, tsc, ter, tec;

    bsr=bsc=ber=bec=tsr=tsc=ter=tec=0;

    this->GetSelectPos(bsr, bsc, ber, bec);
    p_target.GetSelectPos(tsr, tsc, ter, tec);

    double bs = bsr * 10000000000 + bsc;
    double be = ber * 10000000000 + bec;
    double ts = tsr * 10000000000 + tsc;
    double te = ter * 10000000000 + tec;

    if (bs<=ts&&be>=te)
    {
        // type 1: base大包target小
        // do nothing. p_target 消失
    }
    else if (ts<=bs&&te>=be)
    {
        // type 2: base小target大
        // 將 base 變為 p_target
        *this = p_target;
    }
    else if (ts<=bs&&te>=be)
    {
        // type 2: base小target大
        // 將 base 變為 p_target
        *this = p_target;
    }
    else if (be>=ts&&be<=te)
    {
        // type 3: base 在 target 之前
        // 將 target 加入 base
        if (b_select_type==SVID_CARET_ON_HEAD)
        {
            this->SetSelect(ter, tec, p_target.GetSelectKeepSpace());
        }
        else // SVID_CARET_ON_TAIL
        {
            // *this = p_target;
            // this->SetSelect(bsr, bsc, this->GetSelectKeepSpace());
            this->SetPosition(ter, tec);
        }
    }
    else if (bs>=ts&&bs<=te)
    {
        // type 4: base 在 target 之後
        // 將 target 加入 base
        if (b_select_type==SVID_CARET_ON_HEAD)
        {
            // *this = p_target;
            // this->SetSelect(ber, bec, this->GetSelectKeepSpace());
            this->SetPosition(tsr, tsc);
        }
        else // SVID_CARET_ON_TAIL  理論上這個判斷式不會為true
        {
            this->SetSelect(tsr, tsc, p_target.GetSelectKeepSpace());
        }
    }
    else
    {
        // unknown type: write log
        wxLogMessage(wxString::Format("svCaret::MergeOverlap unkown type base:(%i,%i-%i,%i) target:(%i,%i-%i,%i)", bsr, bsc, ber, bec, tsr, tsc, ter, tec));
    }
}

/*
 * svCaretList Class
 */
svCaretList::svCaretList()
{
    // m_bufText = NULL;
    m_caretsList.push_back(svCaret());
    m_first_index = 0;
    m_last_index = 0;
}

// svCaretList::svCaretList(svBufText *p_bufText)
// {
//     m_bufText = p_bufText;
//     m_caretsList.push_back(svCaret());
//     m_caretsList.push_back(svCaret(1,0));
// }

svCaretList::~svCaretList()
{
    // m_bufText = NULL;
    m_caretsList.clear();
}

void svCaretList::Append(const svCaret &p_caret)
{
    // m_caretsList.push_back(p_caret);
    bool inserted = false;
    for (int i=m_caretsList.size()-1; i>=0; i--)
    {
        if (m_caretsList.at(i)<p_caret)
        {
            m_caretsList.insert(m_caretsList.begin()+i+1, p_caret);
            m_last_index = i + 1;
            if (m_first_index>=i+1)
            {
                ++m_first_index;
            }
            inserted = true;
            break;
        }
    }

    if (!inserted) // Insert at head of vector
    {
        m_caretsList.insert(m_caretsList.begin(), p_caret);
        m_last_index = 0;
        m_first_index = 0;
        inserted = true;
    }
}

// Clear all caret in caretList except the first one.
void svCaretList::KeepFirstAndClearElse(void)
{
    svCaret c = m_caretsList.at(m_first_index);
    m_caretsList.clear();
    m_caretsList.push_back(c);
    m_first_index = 0;
    m_last_index = 0;
}

// Clear all caret in caretList except the last one.
void svCaretList::KeepLastAndClearElse(void)
{
    svCaret c = m_caretsList.at(m_last_index);
    m_caretsList.clear();
    m_caretsList.push_back(c);
    m_first_index = 0;
    m_last_index = 0;
}

svCaret *svCaretList::At(const int idx)
{
    svCaret *ptr;
    ptr = &m_caretsList.at(idx);
    return ptr;
}

svCaret *svCaretList::GetLastCaret(void)
{
    svCaret *ptr;
    ptr = &m_caretsList.at(m_last_index);
    return ptr;
}

// Return svCaret list for carets in specified position range and out of the range.
// vector<svCaret> *p_inCaretList : caret list in the specified range.
// vector<svCaret> *p_outCaretList : caret list out of the specified range.
//vector<svCaret> *svCaretList::GetCaretsOfRange(const int32_t p_sr, const int32_t p_sc, const int32_t p_er, const int32_t p_ec)
bool svCaretList::GetCaretsOfRange(const int32_t p_sr, const int32_t p_sc, const int32_t p_er, const int32_t p_ec, vector<svCaret> **p_inCaretList, vector<svCaret> **p_outCaretList)
{
    if (*p_inCaretList)
    {
        (*p_inCaretList)->clear();
        delete (*p_inCaretList);
    }
    if (*p_outCaretList)
    {
        (*p_outCaretList)->clear();
        delete (*p_outCaretList);
    }

    // vector<svCaret> *p_inCaretList = new vector<svCaret>;
    // vector<svCaret> *p_outCaretList = new vector<svCaret>;
    (*p_inCaretList) = new vector<svCaret>;
    (*p_outCaretList) = new vector<svCaret>;

    // size_t firstRow, lastRow;
    // size_t firstCol, lastCol;
    int32_t firstRow, lastRow;
    int32_t firstCol, lastCol;

    firstRow = p_sr;
    firstCol = p_sc;
    lastRow = p_er;
    lastCol = p_ec;

    for(std::vector<svCaret>::iterator it=m_caretsList.begin();
        it!=m_caretsList.end();
        ++it)
    {
        if (firstRow==lastRow)
        {
            if (it->GetRow()==(int)firstRow &&
                it->GetVisualCol()>=(int)firstCol && it->GetVisualCol()<=(int)lastCol)
            {
                // In range
                (*p_inCaretList)->push_back(*it);
                continue;
            }
        }
        else
        {
            if ((it->GetRow()==(int)firstRow && it->GetVisualCol()>=(int)firstCol) || 
                (it->GetRow()>(int)firstRow && it->GetRow()<(int)lastRow)   ||
                (it->GetRow()==(int)lastRow && it->GetVisualCol()<=(int)lastCol))
            {
                // In range
                (*p_inCaretList)->push_back(*it);
                continue;
            }
        }

        // Out of range
        (*p_outCaretList)->push_back(*it);

    }

    // if (p_type == SVID_IN_RANGE)
    // {
    //     if (p_outCaretList)
    //     {
    //         p_outCaretList->clear();
    //         delete p_outCaretList;
    //     }
    //     return p_inCaretList;
    // }
    // else
    // {
    //     if (p_inCaretList)
    //     {
    //         p_inCaretList->clear();
    //         delete p_inCaretList;
    //     }
    //     return p_outCaretList;
    // }

    return true;

}

// Remove duplicate location carets.
// Using the stupidest way now.
void svCaretList::RemoveDuplication(void)
{
    for (int i=(int)m_caretsList.size()-1; i>0; i--)
    {
        for(int j=i-1; j>=0; j--)
        {
            if (m_caretsList.at(i) == m_caretsList.at(j))
            {
                m_caretsList.erase(m_caretsList.begin()+i);
                if (m_first_index==i)
                {
                    m_first_index=j;
                }
                if (m_last_index==i)
                {
                    m_last_index=j;
                }
                break;  // node i no longer exist.
            }
        }
    }
}

// Remove duplicate location carets.
// Using the stupidest way now.




// Print Carets information
void svCaretList::DumpCarets(void)
{
    wxString str;
    for (int i=0; i<(int)m_caretsList.size()-1; i++)
    {
        str += wxString::Format("%i=(%i, %i) ", i, m_caretsList.at(i).GetRow(), m_caretsList.at(i).GetVisualCol());
    }
    wxLogMessage(str);
}

// Merge selected text overlaped carets.
// Using the stupidest way now.
// 目前只將其他 caret 與最後一個 caret 合併
// return true if merge happened.
bool svCaretList::MergeOverlap(void)
{
    if (m_caretsList.size()<=1) return false;

    int removedIdx, idx;
    removedIdx = -1;
    idx = -1;

    bool merge = false;

    std::vector<svCaret>::iterator it_last=m_caretsList.begin()+m_last_index;
    std::vector<svCaret>::iterator it1 = m_caretsList.begin();

    while (it1 != m_caretsList.end())
    {
        ++idx;

        it_last=m_caretsList.begin()+m_last_index;
        if (it1==it_last)
        // svCaret a = *it1;
        // svCaret b = *it_last;
        // if (*it1==*it_last)
        {
            ++it1;
            continue;
        }

        if (it_last->IsOverlaped(*it1))
        {
            it_last->MergeOverlap(*it1);
            it1 = m_caretsList.erase(it1);
            removedIdx = idx;
            merge = true;

            if (removedIdx<=m_first_index)
            {
                --m_first_index;
                if (m_first_index<0) m_first_index=0;
            }
            if (removedIdx<=m_last_index)
            {
                --m_last_index;
                if (m_last_index<0) m_last_index=0;
            }
            --idx;  // for current item has been removed.
        }
        else
        {
            ++it1;
        }
    }

    assert(m_first_index<(int)m_caretsList.size() && "m_first_index>=carets size()");
    assert(m_last_index<(int)m_caretsList.size() && "m_last_index>=carets size()");

    return merge;
}

bool svCaretList::HasSelect(void)
{
    for(std::vector<svCaret>::iterator it=m_caretsList.begin();
        it!=m_caretsList.end();
        ++it)
    {
        if (it->HasSelect())
            return true;
    }

    return false;
}

// 檢查給定的游標是否與現有的游標重疉
bool svCaretList::IsOverlaped(const svCaret p_target)
{
    for(std::vector<svCaret>::iterator it=m_caretsList.begin();
        it!=m_caretsList.end();
        ++it)
    {
        if (it->IsOverlaped(p_target))
            return true;
    }

    return false;
}

/*

// 單個caret往左移
// ir, iv_c 是未折行的caret位置
// nr, nv_c 是回傳的新位置
// keep_space 是回傳的保持位置
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 ir, iv_c 的合理性
void svCaret::SingleCaretMoveLeft(const int ir, const int iv_c, int &nr, int &nv_c, int &keep_space)
{
    int r, v_c;
    nr = r = ir; nv_c = v_c = iv_c;
    keep_space = 0;

    if (VisibleAt(r)) // caret in a visible line.
    {
        int len = (int)m_buftext.at(r).TextLen(SVID_NO_CRLF);
        if (!v_c) // c==0
        {
            int new_r = GetPrevVisibleUWLine(r);
            if (new_r>=0) // r>0
            {
                nr = new_r;
                nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
            }
        }
        else
        {
            if (v_c>len)
            {
                nr = r;
                nv_c = len-1;
            }
            else
            {
                nr = r;
                nv_c = v_c-1;
            }
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(nr, nv_c, wrappedLine, k_wrappedCol);                    
        keep_space = m_buftext.at(nr).Col2SpaceW(wrappedLine, k_wrappedCol);

    }
    else // caret in a invisible line.
    {
        int new_r = GetPrevVisibleUWLine(r);
        if (new_r>=0) // Move to the previous visible line.
        {
            nr = new_r;
            nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
        }
        else // If no previous visible, move to the next visible line.
        {
            new_r = GetNextVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = 0;
            }
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(nr, nv_c, wrappedLine, k_wrappedCol);                    
        keep_space = m_buftext.at(nr).Col2SpaceW(wrappedLine, k_wrappedCol);

    }


    // processing if caret in an invisible line occasionally.
    // Exception catching.
    if (!VisibleAt(r))
    {
        int new_r = GetPrevVisibleUWLine(r);
        if (new_r>=0) // Move to the previous visible line.
        {
            nr = new_r;
            nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
        }
        else // If no previous visible, move to the next visible line.
        {
            new_r = GetNextVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = 0;
            }
        }
    }

}

// 單個caret往右移
// ir, iv_c 是未折行的caret位置
// nr, nv_c 是回傳的新位置
// keep_space 是回傳的保持位置
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 ir, iv_c 的合理性
void svCaret::SingleCaretMoveRight(const int ir, const int iv_c, int &nr, int &nv_c, int &keep_space)
{
    int r, v_c;
    nr = r = ir; nv_c = v_c = iv_c;
    keep_space = 0;

    if (VisibleAt(r)) // caret in a visible line.
    {        
        int len = (int)m_buftext.at(r).TextLen(SVID_NO_CRLF);
        if (v_c>=len)  // In the end of the line
        {
            int new_r = GetNextVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = 0;
            }
        }
        else
        {
            nr = r;
            nv_c = v_c+1;
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(nr, nv_c, wrappedLine, k_wrappedCol);                    
        keep_space = m_buftext.at(nr).Col2SpaceW(wrappedLine, k_wrappedCol);

    }
    else // caret in a invisible line.
    {
        int new_r = GetNextVisibleUWLine(r);
        if (new_r>=0) // Move to the next visible line
        {
            nr = new_r;
            nv_c = 0;
        }
        else // If no next visible then Move to the previous visible line
        {
            new_r = GetPrevVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
            }                
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(nr, nv_c, wrappedLine, k_wrappedCol);                    
        keep_space = m_buftext.at(nr).Col2SpaceW(wrappedLine, k_wrappedCol);

    }


    // processing if caret in an invisible line occasionally.
    // Exception catching.
    if (!VisibleAt(r))
    {
        int new_r = GetNextVisibleUWLine(r);
        if (new_r>=0) // Move to the next visible line
        {
            nr = new_r;
            nv_c = 0;
        }
        else // If no next visible then Move to the previous visible line
        {
            new_r = GetPrevVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
            }                
        }
    }
}

// Move Caret Up
// 單個caret往上移
// ir, iv_c 是未折行的caret位置
// nr, nv_c 是回傳的新位置
// caret 上下移動時不影響 keepSpace 值
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 ir, iv_c 的合理性
void svCaret::SingleCaretMoveUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                                  const int ir, const int iv_c, const int keep_space, int &nr, int &nv_c)
{
    int r, v_c;
    nr = r = ir; nv_c = v_c = iv_c;

#ifndef NDEBUG
    wxLogMessage(wxString::Format(wxT("Call svCaret::Carets log before: r=%i, vc=%i"), r, v_c));
#endif

    int len = (int)m_buftext.at(r).TextLen(SVID_NO_CRLF);
    if (!IsStyledWrapProcessedAt(r))
        ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);

    // 計算在第幾個折行及該折行內的第幾個column
    int wrappedLine = 0;
    int v_wrappedCol = 0;

    InWhichWrappedLineColAt(r, v_c, wrappedLine, v_wrappedCol);
    if (wrappedLine<0) wrappedLine = 0;

    int spaceCount = 0;        // 等值的space數量

    // That means we move from a long line into a short line.
    // Then move from the short line into a long line.
    // We have to keep the column value so that it will move to 
    // correct position when we move from short to long line.
    // if ((k_c>=(int)(m_buftext.at(r).m_wrapLenList.at(wrappedLine).idx + m_buftext.at(r).m_wrapLenList.at(wrappedLine).len)) ||
    //     (k_c>v_c) )

    // 取回紀錄的等價的space值
    // spaceCount = it->GetKeepSpace();
    spaceCount = keep_space;

    int wrapCount = m_buftext.at(r).m_wrapLenList.size();

    if (wrapCount==0)
    {
        // Error. wrapCount should > 0.
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svCaret::CaretsUp log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
    }
    else if (wrapCount==1 || wrappedLine==0)
    {
        // We have to move to the last wrapped line of the previous unwrapped line.
        // 移動到前一行的最後一個折行
        if (r>0)
        {
            --r;
            if (!IsStyledWrapProcessedAt(r))
                ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
            int wrapCount2 = m_buftext.at(r).m_wrapLenList.size();

            // 回推space值等值的col位置
            nr = r;                
            nv_c = m_buftext.at(nr).Space2ColW(wrapCount2-1, spaceCount);
        }            
    }
    else  // Move to the previous wrapped line. 移到前一個折行
    {
        // 回推space值等值的col位置
        nr = r;
        nv_c = m_buftext.at(nr).Space2ColW(wrappedLine-1, spaceCount);
    }

    // processing if caret in an invisible line occasionally.
    // Exception catching.
    if (!VisibleAt(r))
    {
        int new_r = GetPrevVisibleUWLine(r);
        if (new_r>=0) // Move to the previous visible line.
        {
            nr = new_r;
            nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
        }
        else // If no previous visible, move to the next visible line.
        {
            new_r = GetNextVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = 0;
            }
        }
    }
}

// Move Caret Down
// 單個caret往下移
// ir, iv_c 是未折行的caret位置
// nr, nv_c 是回傳的新位置
// caret 上下移動時不影響 keepSpace 值
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 ir, iv_c 的合理性
void svCaret::SingleCaretMoveDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                                    const int ir, const int iv_c, const int keep_space, int &nr, int &nv_c)
{

    int r, v_c;
    nr = r = ir; nv_c = v_c = iv_c;

#ifndef NDEBUG
    wxLogMessage(wxString::Format(wxT("Call svCaret::CaretsDown log before: r=%i, vc=%i"), r, v_c));
#endif

    int len = (int)m_buftext.at(r).TextLen(SVID_NO_CRLF);
    if (!IsStyledWrapProcessedAt(r))
        ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);

    // 計算在第幾個折行及該折行內的第幾個column
    int wrappedLine = 0;
    int v_wrappedCol = 0;

    InWhichWrappedLineColAt(r, v_c, wrappedLine, v_wrappedCol);
    if (wrappedLine<0) wrappedLine = 0;


    int spaceCount = 0;        // 等值的space數量

    // That means we move from a long line into a short line.
    // Then move from the short line into a long line.
    // We have to keep the column value so that it will move to 
    // correct position when we move from short to long line.
    // if ((k_c>=(int)(m_buftext.at(r).m_wrapLenList.at(wrappedLine).idx + m_buftext.at(r).m_wrapLenList.at(wrappedLine).len)) ||
    //     (k_c>v_c) )
    // if (k_c>=(int)(m_buftext.at(r).m_wrapLenList.at(wrappedLine).idx + m_buftext.at(r).m_wrapLenList.at(wrappedLine).len))

    // 取回紀錄的等價的space值
    // spaceCount = it->GetKeepSpace();
    spaceCount = keep_space;

    int wrapCount = m_buftext.at(r).m_wrapLenList.size();

    if (wrapCount==0)
    {
        // Error. wrapCount should > 0.
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svCaret::CaretsDown log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
    }
    else if (wrapCount==1 || wrappedLine>=wrapCount-1)
    {
        // We have to move to the last wrapped line of the previous unwrapped line.
        // 移動到下一行的第一個折行
        if (r<(int)m_buftext.size()-1)
        {
            ++r;
            if (!IsStyledWrapProcessedAt(r))
                ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
            // 回推space值等值的col位置
            nr = r;
            nv_c = m_buftext.at(nr).Space2ColW(0, spaceCount);
        }  
    }
    else   // Move to the  wrapped line. 移到下一個折行
    {
        // 移動到下一個折行
        // 回推space值等值的col位置
        nr = r;
        nv_c = m_buftext.at(nr).Space2ColW(wrappedLine+1, spaceCount);
    }



    // processing if caret in an invisible line occasionally.
    // Exception catching.
    if (!VisibleAt(r))
    {
        int new_r = GetNextVisibleUWLine(r);
        if (new_r>=0) // Move to the next visible line
        {
            nr = new_r;
            nv_c = 0;
        }
        else // If no next visible then Move to the previous visible line
        {
            new_r = GetPrevVisibleUWLine(r);
            if (new_r>=0)
            {
                nr = new_r;
                nv_c = (int)m_buftext.at(nr).TextLen(SVID_NO_CRLF);
            }                
        }
    }

}






void svCaret::CaretsLeft(void)
{
    // After CaretsLeft Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int nr, nv_c;
        int keep_space=0;
        r=v_c=0;
        nr=nv_c=0;
        it->GetPosition(r, v_c);
        SingleCaretMoveLeft(r, v_c, nr, nv_c, keep_space);
        it->SetPosition(nr, nv_c);
        it->SetKeepSpace(keep_space);
    }
}

void svCaret::CaretsRight(void)
{
    // After CaretsRight Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int nr, nv_c;
        int keep_space=0;
        r=v_c=0;
        nr=nv_c=0;
        it->GetPosition(r, v_c);
        SingleCaretMoveRight(r, v_c, nr, nv_c, keep_space);
        it->SetPosition(nr, nv_c);
        it->SetKeepSpace(keep_space);
    }
}

// Move Caret Up
void svCaret::CaretsUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int nr, nv_c;
        r=v_c=0;
        nr=nv_c=0;
        it->GetPosition(r, v_c);
        SingleCaretMoveUp(p_tabSize, p_spaceWidth, p_font, p_maxPixelWidth, r, v_c, it->GetKeepSpace(), nr, nv_c);
        it->SetPosition(nr, nv_c);
    }
}

// Move Caret Down
void svCaret::CaretsDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int nr, nv_c;
        r=v_c=0;
        nr=nv_c=0;
        it->GetPosition(r, v_c);
        SingleCaretMoveDown(p_tabSize, p_spaceWidth, p_font, p_maxPixelWidth, r, v_c, it->GetKeepSpace(), nr, nv_c);
        it->SetPosition(nr, nv_c);
    }
}
*/