/*
   Copyright Notice in awvic.cpp
*/

#include "svTextView.h"
#include <wx/tokenzr.h>
#include <limits>

#include "stdwx.h"

#include "svPreference.h"

// **************
// svTextView
// **************

svTextView::svTextView():
m_bufText(NULL),
m_pageLinesCnt(0),
m_memoryX(0),
m_maxPixelWidth(0),
m_visiblePixelWidth(0),
m_HScrollPixel(0),
m_old_HScrollPixel(0),
m_needRefresh(true),
m_tabSize(4),
m_charHeight(0),
m_spaceWidth(0),
m_lineNoIndicatorAreaMaxWidth(0),
m_inCaretList(NULL),
m_outCaretList(NULL),
m_old_VScrollCnt(0),
m_VScrollCnt(0),
m_showFindKeywordInd(false),
m_prevFind(0)
{
    m_firstIndex.unwrap_idx = 0;
    m_firstIndex.wrap_idx = 0;
    m_old_firstIndex.unwrap_idx = 0;
    m_old_firstIndex.wrap_idx = 0;
    m_bufText = NULL;
}

svTextView::svTextView(svBufText* p_buftext, int p_linesPerTextArea, const wxFont& p_font, int p_maxPixelWidth, size_t p_tabSize, int p_charHeight, int p_spaceWidth)
{
    m_bufText = p_buftext;
    m_font = &p_font;
    m_pageLinesCnt = p_linesPerTextArea;
    m_maxPixelWidth = p_maxPixelWidth;
    m_visiblePixelWidth = m_maxPixelWidth;
    m_HScrollPixel = 0;
    m_old_HScrollPixel = 0;
    m_needRefresh = true;
    m_firstIndex.unwrap_idx = 0;
    m_firstIndex.wrap_idx = 0;
    m_old_firstIndex.unwrap_idx = 0;
    m_old_firstIndex.wrap_idx = 0;
    m_tabSize = p_tabSize;
    // m_charHeight = m_font->GetPixelSize().GetHeight();
    // m_spaceWidth = m_font->GetPixelSize().GetWidth();  // WRONG!! Will Get 0.
    m_charHeight = p_charHeight;
    m_spaceWidth = p_spaceWidth;
    m_lineNoIndicatorAreaMaxWidth = 2 * m_font->GetPixelSize().GetWidth();
    m_inCaretList = NULL;
    m_outCaretList = NULL;
    m_old_VScrollCnt = 0;
    m_VScrollCnt = 0;
    m_showFindKeywordInd = false;
    m_prevFind = 0;
    m_bufText = NULL;
}

svTextView::~svTextView()
{
    m_bufText = NULL;
    m_textList.clear();
    m_pptextList.clear();
    m_nptextList.clear();
    if (m_inCaretList)
    {
        m_inCaretList->clear();
        delete m_inCaretList;
    }
    if (m_outCaretList)
    {
        m_outCaretList->clear();
        delete m_outCaretList;
    }
}

void svTextView::Clear()
{
    m_bufText = NULL;
    m_textList.clear();
    m_pptextList.clear();
    m_nptextList.clear();
    m_pageLinesCnt =0;
    m_memoryX = 0; 
    m_maxPixelWidth = 0;
    m_visiblePixelWidth = 0;
    m_HScrollPixel = 0;
    m_old_HScrollPixel = 0;
    m_needRefresh = true;
    m_firstIndex.unwrap_idx = 0;
    m_firstIndex.wrap_idx = 0;
    m_old_firstIndex.unwrap_idx = 0;
    m_old_firstIndex.wrap_idx = 0;
    m_tabSize = 4;
    m_charHeight = 0;
    m_spaceWidth = 0;
    m_lineNoIndicatorAreaMaxWidth = 0;
    if (m_inCaretList)
    {
        m_inCaretList->clear();
        delete m_inCaretList;
        m_inCaretList = NULL;
    }
    if (m_outCaretList)
    {
        m_outCaretList->clear();
        delete m_outCaretList;
        m_outCaretList = NULL;
    }    
    m_old_VScrollCnt = 0;
    m_VScrollCnt = 0;
    m_showFindKeywordInd = false;
    m_prevFind = 0;
}

void svTextView::Reset(svBufText* p_buftext, int p_linesPerTextArea, const wxFont& p_font, int p_maxPixelWidth, size_t p_tabSize, int p_charHeight, int p_spaceWidth)
{
    m_bufText = p_buftext;
    m_pageLinesCnt = p_linesPerTextArea;
    m_font = &p_font;
    m_maxPixelWidth = p_maxPixelWidth;
    m_visiblePixelWidth = m_maxPixelWidth;
    m_HScrollPixel = 0;
    m_old_HScrollPixel = 0;
    m_needRefresh = true;
    m_firstIndex.unwrap_idx = 0;
    m_firstIndex.wrap_idx = 0;
    m_old_firstIndex.unwrap_idx = 0;
    m_old_firstIndex.wrap_idx = 0;
    m_tabSize = p_tabSize;
    m_charHeight = p_charHeight;
    m_spaceWidth = p_spaceWidth;
    m_lineNoIndicatorAreaMaxWidth = 2 * p_spaceWidth;
    // m_charHeight = m_font->GetPixelSize().GetHeight();
    // m_spaceWidth = m_font->GetPixelSize().GetWidth();   // WRONG!! Will Get 0;
    // m_lineNoIndicatorAreaMaxWidth = 2 * m_font->GetPixelSize().GetWidth();  // WRONG!
    if (m_inCaretList)
    {
        m_inCaretList->clear();
        delete m_inCaretList;
        m_inCaretList = NULL;
    }
    if (m_outCaretList)
    {
        m_outCaretList->clear();
        delete m_outCaretList;
        m_outCaretList = NULL;
    }
    m_old_VScrollCnt = 0;
    m_VScrollCnt = 0;
    m_showFindKeywordInd = false;
    m_prevFind = 0;

}

void svTextView::Refresh()
{
#ifndef NDEBUG
    wxLogMessage("svTextView::Refresh() start");
#endif
    
    if (!m_bufText)
    {
        wxLogMessage(wxT("Call svTextView::Refresh() error: m_bufText not setting yet."));
        return;
    }
    if (m_pageLinesCnt<=0)
    {
        wxLogMessage(wxT("Call svTextView::Refresh() error: m_pageLinesCnt not setting yet."));
        return;
    }
    if (!m_font)
    {
        wxLogMessage(wxT("Call svTextView::Refresh() error: m_font not setting yet."));
        return;
    }


    // prepare dc for cal space width and line number indicator area max width in pixel count.
    wxMemoryDC dc;
    wxSize s;
    wxBitmap bitmap(100, 100);
    dc.SelectObject(bitmap);
    dc.Clear();
    dc.SetFont(*m_font);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(wxBrush(*wxWHITE));


/*    m_charHeight = m_font->GetPixelSize().GetHeight();
    s = dc.GetTextExtent(wxT(" "));
    m_spaceWidth = s.GetWidth();
*/
    // Exception handle
    if (m_firstIndex.unwrap_idx >= m_bufText->LineCntUW())
    {
        // m_firstIndex.unwrap_idx = m_bufText->LineCntUW() - 1;
        m_firstIndex.unwrap_idx = m_bufText->GetLastVisibleLine();
        m_firstIndex.wrap_idx = 0;
    }

    m_textList.clear();

    int cnt = 0;   // How many lines processed.

    // 重新處理畫面上第一行所在的 m_bufText 行數：計算折行及文字風格等
    // reprocess the m_bufText line in the first line on the text area for line wrap and text style.
    // PrepareBufText(m_firstIndex.unwrap_idx);

    // 如果重算後的 wrap line 沒有原來的 wrap_idx 多，則將wrap_idx 設為可得的最大值
    // if the wrap_idx is bigger than reprocessed max availabe wrapped line count, reset it to the available max value.

    // if (!m_bufText->IsStyledWrapProcessedAt(m_firstIndex.unwrap_idx))
    // {
        // m_bufText->CreateKeywordTableAt(m_firstIndex.unwrap_idx); // may be duplicated. 可能重覆了，必要時再呼叫。
        m_bufText->ProcWrapAndStyleAt(m_firstIndex.unwrap_idx, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    // }

    size_t wlc = m_bufText->GetWrapLineCountAt(m_firstIndex.unwrap_idx);
    if (m_firstIndex.wrap_idx >= wlc)
    {
        m_firstIndex.wrap_idx = wlc - 1;
    }

    if (wlc == 0) // a line within only line break data.
    {
        bufTextIndex *t;
        t = new bufTextIndex;
        t->unwrap_idx = m_firstIndex.unwrap_idx;
        t->wrap_idx = 0;
        wrapLenDesc w;
        m_bufText->GetWrapLenOnIdxAt(t->unwrap_idx, t->wrap_idx, w);
        // t->wrap_desc.idx = w.idx;
        // t->wrap_desc.len = w.len;
        t->wrap_desc = w;
        t->pixelWidth = m_bufText->GetLine(t->unwrap_idx)->GetTextPixelLen(w.idx, w.len);
        m_textList.push_back(*t);
        delete t;
    }
    else
    {    
        for (size_t x=m_firstIndex.wrap_idx; x<wlc; x++)
        {
            bufTextIndex *t;
            t = new bufTextIndex;
            t->unwrap_idx = m_firstIndex.unwrap_idx;
            t->wrap_idx = x;
            wrapLenDesc w;
            m_bufText->GetWrapLenOnIdxAt(t->unwrap_idx, t->wrap_idx, w);
            // t->wrap_desc.idx = w.idx;
            // t->wrap_desc.len = w.len;
            t->wrap_desc = w;
            t->pixelWidth = m_bufText->GetLine(t->unwrap_idx)->GetTextPixelLen(w.idx, w.len);
            m_textList.push_back(*t);
            delete t;
            if (++cnt>m_pageLinesCnt) break;
        }
    }

    if (cnt <=m_pageLinesCnt)
    {
        for (size_t i=m_firstIndex.unwrap_idx+1; i < m_bufText->LineCntUW(); i++ )
        // for (size_t i=0; i<m_bufText->LineCntUW(); i++ )
        {
            if (!m_bufText->VisibleAt(i))
            {
                // Skip not visible line.
                continue;
            }
            // if (!m_bufText->IsStyledWrapProcessedAt(i))
            // {
                // m_bufText->CreateKeywordTableAt(m_firstIndex.unwrap_idx); // may be duplicated. 可能重覆了，必要時再呼叫。
                m_bufText->ProcWrapAndStyleAt(i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
            // }

            if (m_bufText->VisibleAt(i))  // skip invisible line.
            {
                // PrepareBufText(i);
                size_t wrapLineCnt = m_bufText->GetWrapLineCountAt(i);
                if (wrapLineCnt == 0) // a line within only line break data.
                {
                    bufTextIndex *t;
                    t = new bufTextIndex;
                    t->unwrap_idx = i;
                    t->wrap_idx = 0;
                    wrapLenDesc w;
                    m_bufText->GetWrapLenOnIdxAt(t->unwrap_idx, t->wrap_idx, w);
                    // t->wrap_desc.idx = w.idx;
                    // t->wrap_desc.len = w.len;
                    t->wrap_desc = w;
                    t->pixelWidth = m_bufText->GetLine(t->unwrap_idx)->GetTextPixelLen(w.idx, w.len);
                    m_textList.push_back(*t);
                    delete t;
                    if (++cnt>m_pageLinesCnt) break;
                }
                else
                {
                    for (size_t j=0; j<wrapLineCnt; j++)
                    {
                        bufTextIndex *t;
                        t = new bufTextIndex;
                        t->unwrap_idx = i;
                        t->wrap_idx = j;
                        wrapLenDesc w;
                        m_bufText->GetWrapLenOnIdxAt(t->unwrap_idx, t->wrap_idx, w);
                        // t->wrap_desc.idx = w.idx;
                        // t->wrap_desc.len = w.len;
                        t->wrap_desc = w;
                        t->pixelWidth = m_bufText->GetLine(t->unwrap_idx)->GetTextPixelLen(w.idx, w.len);
                        m_textList.push_back(*t);
                        delete t;
                        if (++cnt>m_pageLinesCnt) break;
                    }
                    if (cnt>m_pageLinesCnt) break;
                }
            }
        }
    }

    UpdateCaretsInfo();

    // Update m_pptextList
    m_pptextList.clear();

    if (m_textList.size()>0)
    {
        size_t o_unwrap_idx = m_textList.front().unwrap_idx;
        size_t o_wrap_idx = m_textList.front().wrap_idx;
        size_t n_unwrap_idx = 0;
        size_t n_wrap_idx = 0;
        int cnt = 0;
        for(int i=0; i<m_pageLinesCnt; i++)
        {
            cnt = 0;
            MoveLineIndex(o_unwrap_idx, o_wrap_idx, -1, n_unwrap_idx, n_wrap_idx, cnt);
            if (cnt) // cnt!=0
            {
                bufTextIndex ti;
                ti.unwrap_idx = n_unwrap_idx;
                ti.wrap_idx = n_wrap_idx;
                wrapLenDesc wd;
                m_bufText->GetLine(n_unwrap_idx)->GetWrapLenOnIdx(n_wrap_idx, wd);
                ti.wrap_desc = wd;
                m_pptextList.insert(m_pptextList.begin(), ti);
                o_unwrap_idx = n_unwrap_idx;
                o_wrap_idx = n_wrap_idx;
            }
        }
    }

    // Update m_nptextList
    m_nptextList.clear();

    if (m_textList.size()>0)
    {
        size_t o_unwrap_idx = m_textList.back().unwrap_idx;
        size_t o_wrap_idx = m_textList.back().wrap_idx;
        size_t n_unwrap_idx = 0;
        size_t n_wrap_idx = 0;
        int cnt = 0;
        for(int i=0; i<m_pageLinesCnt; i++)
        {
            cnt = 0;
            MoveLineIndex(o_unwrap_idx, o_wrap_idx, 1, n_unwrap_idx, n_wrap_idx, cnt);
            if (cnt) // cnt!=0
            {
                bufTextIndex ti;
                ti.unwrap_idx = n_unwrap_idx;
                ti.wrap_idx = n_wrap_idx;
                wrapLenDesc wd;
                m_bufText->GetLine(n_unwrap_idx)->GetWrapLenOnIdx(n_wrap_idx, wd);
                ti.wrap_desc = wd;
                m_nptextList.push_back(ti);
                o_unwrap_idx = n_unwrap_idx;
                o_wrap_idx = n_wrap_idx;
            }
        }
    }

    // recalculate line number indicator area max width.
    if (m_nptextList.size()>0)
    {
        size_t max_linNo = m_nptextList.back().unwrap_idx;
#ifdef __WXMSW__
        s = dc.GetTextExtent(wxString::Format(wxT("%Iu"), max_linNo));
#else
        s = dc.GetTextExtent(wxString::Format(wxT("%zu"), max_linNo));
#endif
        m_lineNoIndicatorAreaMaxWidth = s.GetWidth() + (2 * m_spaceWidth);
    }
    else if (m_textList.size()>0)
    {
        size_t max_linNo = m_textList.back().unwrap_idx;
#ifdef __WXMSW__
        s = dc.GetTextExtent(wxString::Format(wxT("%Iu"), max_linNo));
#else
        s = dc.GetTextExtent(wxString::Format(wxT("%zu"), max_linNo));
#endif
        m_lineNoIndicatorAreaMaxWidth = s.GetWidth() + (2 * m_spaceWidth);
    }
    else
    {
        m_lineNoIndicatorAreaMaxWidth = (2 * m_spaceWidth);
    }

    // 確保正確的計算應該是上面被註解掉的那一段
    // 下面這段在非等寬的字體可能是錯誤的。
    // The correct way is the above code been commented.
    // The codes following is wrong when the font's character width is not fixed.
    // if (m_textList.size()>0)
    // {
    //     size_t max_linNo = m_textList.back().unwrap_idx;
    //     wxString linNoStr = wxString::Format(wxT("%i"), max_linNo);
    //     m_lineNoIndicatorAreaMaxWidth = (linNoStr.Length()+2) * m_spaceWidth;
    // }
    // else
    // {
    //     m_lineNoIndicatorAreaMaxWidth = (2 * m_spaceWidth);
    // }

    m_needRefresh = false;

}

void svTextView::SetVisiblePixelWidth(int p_visiblePixelWidth)
{
    m_visiblePixelWidth = p_visiblePixelWidth;
}

vector<styledWrapText>* svTextView::GetStyledWrapText(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_textList.size()) return NULL;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    size_t w_idx = m_textList.at(idx).wrap_idx;

    return m_bufText->GetStyledWrapTextAt(uw_idx, w_idx);
}

// Previous page version
vector<styledWrapText>* svTextView::GetStyledWrapText_PP(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_pptextList.size()) return NULL;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;
    size_t w_idx = m_pptextList.at(idx).wrap_idx;

    return m_bufText->GetStyledWrapTextAt(uw_idx, w_idx);
}

// Next page version
vector<styledWrapText>* svTextView::GetStyledWrapText_NP(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_nptextList.size()) return NULL;

    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;
    size_t w_idx = m_nptextList.at(idx).wrap_idx;

    return m_bufText->GetStyledWrapTextAt(uw_idx, w_idx);
}

size_t svTextView::GetWrappedIndentPixelLen(size_t idx)
{
    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    size_t w_idx = m_textList.at(idx).wrap_idx;

    if (w_idx==0) return 0;

    return m_bufText->GetLine(uw_idx)->GetIndentPixelLen();
}


size_t svTextView::GetStyledWrapTextLineNoUW(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_textList.size()) return 0;

    return m_textList.at(idx).unwrap_idx + 1;
}

// Previous page version
size_t svTextView::GetStyledWrapTextLineNoUW_PP(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_pptextList.size()) return 0;

    return m_pptextList.at(idx).unwrap_idx + 1;
}

// Next page version
size_t svTextView::GetStyledWrapTextLineNoUW_NP(size_t idx)
{
    // if (m_needRefresh)
    //     Refresh();

    if (idx >= m_nptextList.size()) return 0;

    return m_nptextList.at(idx).unwrap_idx + 1;
}

size_t svTextView::GetLineNoIndicatorAreaMaxWidth()
{
    // if (m_needRefresh)
    //     Refresh();

    return m_lineNoIndicatorAreaMaxWidth;

}

void svTextView::SetPageLines(int p_pageLineCnt)
{
    m_pageLinesCnt = p_pageLineCnt;
    m_needRefresh = true;
}

// void svTextView::SetWrap(bool p_isWrap)
// {
//     m_wrap = p_isWrap;
//     m_needRefresh = true;
// }

void svTextView::SetMaxPixelWidth(int p_maxPixelWidth, bool p_isWrap)
{
    m_maxPixelWidth = p_maxPixelWidth;
    m_wrap = p_isWrap;
    m_needRefresh = true;
}

// 如以 call SetFirstLineIndex() 方式移動 svTextView 內容，不會有平滑捲動的效果
// 長距離的跳躍請 call 本函數，否則回應速度會較慢
void svTextView::SetFirstLineIndex(size_t p_firstLineIndex)
{
    if (p_firstLineIndex < m_bufText->LineCntUW())
    {
        m_firstIndex.unwrap_idx = p_firstLineIndex;
        m_firstIndex.wrap_idx = 0;

        m_needRefresh = true;
    }
}

/* ---------------------------------------------------------------------
 *
 * Paging Up Down & Line Up Down operation.
 *
 * --------------------------------------------------------------------- */

// Move line index. offset can be positive or negative.
// positive means move forward, otherwise move backword.
// 移動 svTextView 畫面上某一行的索引值
// dosn't check the validation of o_unwrap_idx, o_wrap_idx
// o_unwrap_idx, o_wrap_idx : original position. SHOULD BE VISIBLE.
// n_unwrap_idx, n_wrap_idx : new position.
// m_cnt : How many line moved.
void svTextView::MoveLineIndex(const size_t o_unwrap_idx, const size_t o_wrap_idx, const int p_offset, size_t &n_unwrap_idx, size_t &n_wrap_idx, int &m_cnt)
{
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::MoveLineIndex old: unwrap_idx=%i wrap_idx=%i, offset=%i"), (int)o_unwrap_idx, (int)o_wrap_idx, (int)p_offset));
// #endif  
    int cnt = 0;
    int offset = p_offset;

    size_t tmp_unwrap_idx = o_unwrap_idx;
    size_t tmp_wrap_idx = o_wrap_idx;

    if (offset >= 0)  // view move forward
    {
        if (!m_bufText->IsStyledWrapProcessedAt(o_unwrap_idx))
            m_bufText->ProcWrapAndStyleAt(o_unwrap_idx, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

        size_t wlc = m_bufText->GetWrapLineCountAt(o_unwrap_idx);

        if (wlc-1 > o_wrap_idx)
        {
            if (((int)wlc-1 - (int)o_wrap_idx) >= offset)
            {
                tmp_unwrap_idx = o_unwrap_idx;
                tmp_wrap_idx = o_wrap_idx + offset;
                cnt = offset;
            }
            else
            {
                cnt = wlc-1 - o_wrap_idx;
            }
        }

        if (cnt<offset)
        {
            for (size_t i=o_unwrap_idx+1; i < m_bufText->LineCntUW(); i++ )
            {
                if (m_bufText->VisibleAt(i))  // skip invisible line.
                {
                    if (!m_bufText->IsStyledWrapProcessedAt(i))
                        m_bufText->ProcWrapAndStyleAt(i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
                    size_t wrapLineCnt = m_bufText->GetWrapLineCountAt(i);
                    if (wrapLineCnt == 0) // a line within only line break data.
                    {
                        tmp_unwrap_idx = i;
                        tmp_wrap_idx = 0;
                        // ++cnt;
                        if (++cnt>=offset) break;
                    }
                    else
                    {
                        for (size_t j=0; j<wrapLineCnt; j++)
                        {
                            tmp_unwrap_idx = i;
                            tmp_wrap_idx = j;
                            // ++cnt;
                            if (++cnt>=offset) break;
                        }
                    }
                    if (cnt>=offset) break;
                }
            }            
        }

    }
    else  // offset < 0
    {
        offset = -offset;

        if (!m_bufText->IsStyledWrapProcessedAt(o_unwrap_idx))
            m_bufText->ProcWrapAndStyleAt(o_unwrap_idx, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

        // if (wlc-1 > o_wrap_idx)
        // {
            if ((int)o_wrap_idx >= offset)
            {
                tmp_unwrap_idx = o_unwrap_idx;
                tmp_wrap_idx = o_wrap_idx - offset;
                cnt = offset;
            }
            else
            {
                cnt = (int)o_wrap_idx;
            }
        // }

        if (cnt<offset)
        {
            for (int i=o_unwrap_idx-1; i >=0; i--)
            {
                if (m_bufText->VisibleAt(i))  // skip invisible line.
                {
                    if (!m_bufText->IsStyledWrapProcessedAt((size_t)i))
                        m_bufText->ProcWrapAndStyleAt((size_t)i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
                    size_t wrapLineCnt = m_bufText->GetWrapLineCountAt((size_t)i);
                    if (wrapLineCnt == 0) // a line within only line break data.
                    {
                        tmp_unwrap_idx = (size_t)i;
                        tmp_wrap_idx = 0;
                        if (++cnt>=offset) break;
                    }
                    else
                    {
                        for (int j=wrapLineCnt-1; j>=0; j--)
                        {
                            tmp_unwrap_idx = (size_t)i;
                            tmp_wrap_idx = (size_t)j;
                            if (++cnt>=offset) break;
                        }
                    }
                    if (cnt>=offset) break;
                }
            }          
        }

        cnt = -cnt;

    }

// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::MoveLineIndex old: unwrap_idx=%i wrap_idx=%i, new: unwrap_idx=%i wrap_idx=%i"), (int)o_unwrap_idx, (int)o_wrap_idx, (int)tmp_unwrap_idx, (int)tmp_wrap_idx));
// #endif        

    n_unwrap_idx = tmp_unwrap_idx;
    n_wrap_idx = tmp_wrap_idx;
    m_cnt = cnt;
    //m_needRefresh = true;

}


// Move first line index. offset can be positive or negative.
// positive means move forward, otherwise move backword.
// 移動 svTextView 畫面上第一行的索引值
// 如要垂直平滑捲動, 不要 call SetFirstLineIndex() 要 call MoveFirstLineIndex()
// 則其捲動行數才會紀錄下來，才會有平滑捲動的效果
int svTextView::MoveFirstLineIndex(int offset)
{
    size_t o_unwrap_idx = m_firstIndex.unwrap_idx;
    size_t o_wrap_idx = m_firstIndex.wrap_idx;
    int m_cnt = 0;
    size_t n_unwrap_idx = 0;
    size_t n_wrap_idx = 0;

    MoveLineIndex(o_unwrap_idx, o_wrap_idx, offset, n_unwrap_idx, n_wrap_idx, m_cnt);
    m_old_firstIndex.unwrap_idx = m_firstIndex.unwrap_idx;
    m_old_firstIndex.wrap_idx = m_firstIndex.wrap_idx;
    m_firstIndex.unwrap_idx = n_unwrap_idx;
    m_firstIndex.wrap_idx = n_wrap_idx;

    // Record how many lne(s) scrolled.
    // for smooth scroll.
    m_VScrollCnt = m_cnt;


#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("svTextView::MoveFirstLineIndex old: unwrap_idx=%i wrap_idx=%i, new: unwrap_idx=%i wrap_idx=%i"), (int)o_unwrap_idx, (int)o_wrap_idx, (int)n_unwrap_idx, (int)n_wrap_idx));
#endif        

    m_needRefresh = true;

    return m_cnt;
}


/* old version
// Move first line index. offset can be positive or negative.
// positive means move forward, otherwise move backword.
// 移動 svTextView 畫面上第一行的索引值
int svTextView::MoveFirstLineIndex(int offset)
{
    int cnt = 0;

    size_t tmp_unwrap_idx = m_firstIndex.unwrap_idx;
    size_t tmp_wrap_idx = m_firstIndex.wrap_idx;

    if (offset >= 0)  // view move forward
    {
        if (!m_bufText->IsStyledWrapProcessedAt(m_firstIndex.unwrap_idx))
            m_bufText->ProcWrapAndStyleAt(m_firstIndex.unwrap_idx, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);

        size_t wlc = m_bufText->GetWrapLineCountAt(m_firstIndex.unwrap_idx);

        if (wlc-1 > m_firstIndex.wrap_idx)
        {
            if (((int)wlc-1 - (int)m_firstIndex.wrap_idx) >= offset)
            {
                tmp_unwrap_idx = m_firstIndex.unwrap_idx;
                tmp_wrap_idx = m_firstIndex.wrap_idx + offset;
                cnt = offset;
            }
            else
            {
                cnt = wlc-1 - m_firstIndex.wrap_idx;
            }
        }

        if (cnt<offset)
        {
            for (size_t i=m_firstIndex.unwrap_idx+1; i < m_bufText->LineCntUW(); i++ )
            {
                if (m_bufText->VisibleAt(i))  // skip invisible line.
                {
                    if (!m_bufText->IsStyledWrapProcessedAt(i))
                        m_bufText->ProcWrapAndStyleAt(i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                    size_t wrapLineCnt = m_bufText->GetWrapLineCountAt(i);
                    if (wrapLineCnt == 0) // a line within only line break data.
                    {
                        tmp_unwrap_idx = i;
                        tmp_wrap_idx = 0;
                        // ++cnt;
                        if (++cnt>=offset) break;
                    }
                    else
                    {
                        for (size_t j=0; j<wrapLineCnt; j++)
                        {
                            tmp_unwrap_idx = i;
                            tmp_wrap_idx = j;
                            // ++cnt;
                            if (++cnt>=offset) break;
                        }
                    }
                    if (cnt>=offset) break;
                }
            }            
        }

    }
    else  // offset < 0
    {
        offset = -offset;

        if (!m_bufText->IsStyledWrapProcessedAt(m_firstIndex.unwrap_idx))
            m_bufText->ProcWrapAndStyleAt(m_firstIndex.unwrap_idx, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);

        // if (wlc-1 > m_firstIndex.wrap_idx)
        // {
            if ((int)m_firstIndex.wrap_idx >= offset)
            {
                tmp_unwrap_idx = m_firstIndex.unwrap_idx;
                tmp_wrap_idx = m_firstIndex.wrap_idx - offset;
                cnt = offset;
            }
            else
            {
                cnt = (int)m_firstIndex.wrap_idx;
            }
        // }

        if (cnt<offset)
        {
            for (int i=m_firstIndex.unwrap_idx-1; i >=0; i--)
            {
                if (m_bufText->VisibleAt(i))  // skip invisible line.
                {
                    if (!m_bufText->IsStyledWrapProcessedAt((size_t)i))
                        m_bufText->ProcWrapAndStyleAt((size_t)i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                    size_t wrapLineCnt = m_bufText->GetWrapLineCountAt((size_t)i);
                    if (wrapLineCnt == 0) // a line within only line break data.
                    {
                        tmp_unwrap_idx = (size_t)i;
                        tmp_wrap_idx = 0;
                        if (++cnt>=offset) break;
                    }
                    else
                    {
                        for (int j=wrapLineCnt-1; j>=0; j--)
                        {
                            tmp_unwrap_idx = (size_t)i;
                            tmp_wrap_idx = (size_t)j;
                            if (++cnt>=offset) break;
                        }
                    }
                    if (cnt>=offset) break;
                }
            }          
        }

        cnt = -cnt;

    }


#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("svTextView::MoveFirstLineIndex old: unwrap_idx=%i wrap_idx=%i, new: unwrap_idx=%i wrap_idx=%i"), m_firstIndex.unwrap_idx, m_firstIndex.wrap_idx, tmp_unwrap_idx, tmp_wrap_idx));
#endif        


    m_firstIndex.unwrap_idx = tmp_unwrap_idx;
    m_firstIndex.wrap_idx = tmp_wrap_idx;

    m_needRefresh = true;

    // return actually move count.
    return cnt;

}
*/

/*
 * caculate bufTextIndex value
 * 給定畫面差距行數及原始 bufTextIndex 位置，回傳新折行索引 bufTextIndex
 * Giving a speficed differencial (visible)line, and return it's destination bufTextIndex.
 * For example:
 *     original = (0, 0), diff = 8, 
 *     1111111111     <= original 
 *     1111111111
 *     111
 *     2222222222
 *     222222
 *     333
 *     44444444
 *     5555555555
 *     5555555555     <= return new bufTextIndex is (4, 1)
 *     5555555555
 *     555555555 
 */
/*int svTextView::GetDiffBufTextIndex(const bufTextIndex& p_oriIdx, int p_diff, bufTextIndex& p_newIdx)
{
    p_newIdx.unwrap_idx = p_oriIdx.unwrap_idx;
    p_newIdx.wrap_idx = p_oriIdx.wrap_idx;
    p_newIdx.wrap_desc.idx = p_oriIdx.wrap_desc.idx;
    p_newIdx.wrap_desc.len = p_oriIdx.wrap_desc.len;

    if (p_diff=0)
        return 0;

    int procLine = 0;

    int cnt=0;  // 已處理筆數。 How many diff line processed.
    if (p_diff>0)
    {
        int uwi = p_oriIdx.unwrap_idx;
        size_t wlc = m_bufText->GetWrapLineCountAt(p_oriIdx.unwrap_idx);
        bool reachLimit = false;
        if ((wlc-1)>p_oriIdx.wrap_idx)
        {
            cnt = wlc - p_oriIdx.wrap_idx - 1;
        }
        while(cnt<p_diff)
        {
            ++uwi;
            if (uwi<=(int)(m_bufText->LineCntUW()-1))
            {
                // if (!m_bufText->IsStyledWrapProcessedAt(i))
                    m_bufText->ProcWrapAndStyleAt((size_t)uwi, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                wlc = m_bufText->GetWrapLineCountAt((size_t)uwi);
                break;
            }
            else
            {
                reachLimit = true;
            }
            cnt += wlc;
        }

        if (reachLimit)  // reach end.
        {
            wrapLenDesc w;
            m_bufText->GetWrapLenOnIdxAt(m_bufText->LineCntUW()-1, wlc-1, w);
            p_newIdx.unwrap_idx = m_bufText->LineCntUW()-1;
            p_newIdx.wrap_idx = wlc-1;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = cnt;
        }
        else if (cnt>=p_diff)
        {
            wrapLenDesc w;
            m_bufText->GetWrapLenOnIdxAt(uwi, cnt-p_diff, w);
            p_newIdx.unwrap_idx = uwi;
            p_newIdx.wrap_idx = cnt-p_diff;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = p_diff;
        }
        else
        {
            wxLogMessage(wxT("Call svTextView::GetDiffBufTextIndex() error: nor reachLimit neither cnt>=p_diff."));
        }
    }
    else // p_diff <0
    {
        int uwi = p_oriIdx.unwrap_idx;
        size_t wlc = m_bufText->GetWrapLineCountAt(p_oriIdx.unwrap_idx);
        bool reachLimit = false;

        cnt = p_oriIdx.wrap_idx;

        while(cnt<-p_diff)
        {
            --uwi;
            if (uwi>=0)
            {
                // if (!m_bufText->IsStyledWrapProcessedAt(i))
                    m_bufText->ProcWrapAndStyleAt((size_t)uwi, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                wlc = m_bufText->GetWrapLineCountAt((size_t)uwi);
                break;
            }
            else
            {
                reachLimit = true;
            }
            cnt += wlc;
        }

        if (reachLimit)  // reach end.
        {
            wrapLenDesc w;
            m_bufText->GetWrapLenOnIdxAt(0, 0, w);
            p_newIdx.unwrap_idx = 0;
            p_newIdx.wrap_idx = 0;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = cnt;
        }
        else if (cnt>=-p_diff)
        {
            wrapLenDesc w;
            m_bufText->GetWrapLenOnIdxAt(uwi, wlc-(cnt-p_diff)-1, w);
            p_newIdx.unwrap_idx = uwi;
            p_newIdx.wrap_idx = wlc-(cnt-p_diff)-1;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = -p_diff;
        }
        else
        {
            wxLogMessage(wxT("Call svTextView::GetDiffBufTextIndex() error: nor reachLimit neither cnt>=-p_diff."));
        }
    }

    return procLine;
}
*/

/* ---------------------------------------------------------------------
 *
 * Operations about mouses.
 *
 * --------------------------------------------------------------------- 
 */

// 依據傳入的 pixel 位置，回傳該位置所代表的 row, col 值 
bool svTextView::PixelXY2TextRowColUW(int p_pixelX, int p_pixelY, int p_lineHeight, bufTextIndex& p_textIdx)
{
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW enter.")));
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: mouse x=%i y=%i"), p_pixelX, p_pixelY));
// #endif

    if (p_pixelX<0 || p_pixelY<0)
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: mouse x or y < 0")));
// #endif   
        return false;
    }


    int idx = p_pixelY / p_lineHeight;

// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: idx=%i"), idx));
// #endif   

    // over border.
    /*
    if (idx < 0 || idx >= (int)m_textList.size())
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: out of boundry=%i "), idx));
// #endif          
        return false;
    }*/
    
    if (idx < 0)
    {
        return false;
    }
    
    if (idx >= (int)m_textList.size()) // out of the last line.
    {
        idx = (int)m_textList.size() - 1;
    }

    size_t r = m_textList.at(idx).unwrap_idx;
    size_t c = m_textList.at(idx).wrap_idx;

// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: r=%i c=%i"), r, c));
// #endif   


    int tmp_pixelX = 0;
    int init_pixelX = 0;
    size_t tmp_textCol = 0;
    // Since m_textList.at(idx).unwrap_idx & wrap_idx record line wrap information.
    // We have to recal it to unwrap Row & Column informatio.
    // m_textList.at(idx) 紀錄的是已折行的資訊，我們要跟據這樣的資料回推尚未折行時的行、列值
    // 加總寬度再回推傳入的pixel X, Y值是對應到未折行處理的哪一個行、列

    wrapLenDesc wld;

    if (!m_bufText->GetWrapLenOnIdxAt(r, c, wld))
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: m_bufText->GetWrapLenOnIdxAt return false ")));
// #endif          
        return false;
    }

    // Calculate The pixel X vaule of the first character of the wrap line
    // 計算折行的初始位置
    if (c!=0)
    {
        if (!m_bufText->TextCol2PixelXAt(r, wld.idx, init_pixelX))
        {
// #ifndef NDEBUG
//             wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: m_bufText->TextCol2PixelXAtUW return false")));
// #endif
            return false;
        }
    }
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: init_pixelX=%i"), init_pixelX));
// #endif          


    // sub wrapped line indent.
    // 扣除折行的自動內縮值
    tmp_pixelX = init_pixelX + p_pixelX - GetWrappedIndentPixelLen(idx);
    // 加上畫面捲動值
    tmp_pixelX += m_HScrollPixel;
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: GetWrappedIndentPixelLen(idx)=%i"), GetWrappedIndentPixelLen(idx)));
//     wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: tmp_pixelX=%i"), tmp_pixelX));
// #endif      
    if (tmp_pixelX<init_pixelX)
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: init_pixelX + p_pixelX - GetWrappedIndentPixelLen(idx) < init_pixelX")));
// #endif        
        tmp_pixelX = init_pixelX;
    }

    if (m_bufText->PixelX2TextColAt(r, tmp_pixelX, tmp_textCol))
    {
        p_textIdx.unwrap_idx = r;
        p_textIdx.wrap_idx = tmp_textCol;
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: mouse x=%i y=%i, Text r=%i c=%i"), p_pixelX, p_pixelY, r, tmp_textCol));
// #endif         
        return true;
    }
    else
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: m_bufText->PixelX2TextColAtUW return false")));
// #endif        
        return false;
    }

}


// 依據傳入的 pixel 位置，回傳該位置所代表的 row值 
bool svTextView::PixelXY2TextRowUW(int p_pixelX, int p_pixelY, int p_lineHeight, size_t& p_row)
{
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW enter.")));
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: mouse x=%i y=%i"), p_pixelX, p_pixelY));
// #endif

    if (p_pixelX<0 || p_pixelY<0)
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: mouse x or y < 0")));
// #endif   
        return false;
    }


    int idx = p_pixelY / p_lineHeight;

// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW: idx=%i"), idx));
// #endif   

    // over border.
    if (idx < 0 || idx >= (int)m_textList.size())
    {
// #ifndef NDEBUG
//         wxLogMessage(wxString::Format(wxT("svTextView::PixelXY2TextRowColUW error: out of boundry=%i "), idx));
// #endif          
        return false; 
    }

    p_row = m_textList.at(idx).unwrap_idx;
    return true;

}

// 依據傳入的 (row, col) 位置，回傳該位置所代表的 pixel (x, y) 值 
bool svTextView::TextRowCol2PixelXYUW(int p_row, int p_col, int p_lineHeight, int &p_pixelX, int &p_pixelY)
{
    size_t srow, scol, erow, ecol;
    srow=scol=erow=ecol=0;
    if (GetVisibleTextPosRange(srow, scol, erow, ecol))
    {
        if ( (p_row>(int)srow&&p_row<(int)erow) ||
             (p_row==(int)srow&&p_col>=(int)srow) ||
             (p_row==(int)erow&&p_col<=(int)erow) )
        {
            
            int wl = m_bufText->GetLine(p_row)->InWhichWrappedLine(p_col);  // In whick wrap line.

            int idx=0;
            for (size_t i=0; i<m_textList.size(); i++)
            {

                // look up which index the (row, col) is at m_textList.
                if ( p_row==m_textList.at(idx).unwrap_idx && 
                     wl==(int)m_textList.at(idx).wrap_idx )
                {
                    break;
                }
                ++idx;
            }

            if (idx>=(int)m_textList.size())
            {
                // Exception!
                wxLogMessage(wxString::Format("Error: svTextView::TextRowCol2PixelXYUW cannot locate row=%i col=%i on m_textView", p_row, p_col));
                return false;
            }

            p_pixelY = (idx+1) * p_lineHeight;

            //size_t r = m_textList.at(idx).unwrap_idx;
            //size_t c = m_textList.at(idx).wrap_idx;

            int tmp_pixelX = 0;
            int init_pixelX = 0;
            size_t tmp_textCol = 0;
            // Since m_textList.at(idx).unwrap_idx & wrap_idx record line wrap information.
            // We have to recal it to unwrap Row & Column informatio.
            // m_textList.at(idx) 紀錄的是已折行的資訊，我們要跟據這樣的資料回推尚未折行時的行、列值
            // 加總寬度再回推傳入的pixel X, Y值是對應到未折行處理的哪一個行、列

            //wrapLenDesc wld;

            //if (!m_bufText->GetWrapLenOnIdxAt(r, c, wld))
            //{
                //return false;
            //}

            // Calculate The pixel X vaule of the first character of the wrap line
            // 計算折行的初始位置
            //if (c!=0)
            //{
                //if (!m_bufText->TextCol2PixelXAtW(r, wld.idx, init_pixelX))
                if (!m_bufText->TextCol2PixelXAtW(p_row, p_col, init_pixelX))
                {
                    return false;
                }
            //}

            // sub wrapped line indent.
            // 扣除折行的自動內縮值
            if (wl==0)
                tmp_pixelX = init_pixelX;
            else
                tmp_pixelX = init_pixelX + GetWrappedIndentPixelLen(idx);
            // 加上畫面捲動值
            tmp_pixelX -= m_HScrollPixel;
 
            p_pixelX = tmp_pixelX;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

}


/* ---------------------------------------------------------------------
 *
 * Caret related functions.
 *
 * --------------------------------------------------------------------- 
 */

// Get start and end Text(Row,Col) of the TextView
// 取得畫面上第一個及最後一個字元的座標(row, col)位置
// 2015/9/17 the last position including invisible line when last line on screen has folding information.
//           畫面上最後一個字元的座標(row, col)位置包含invisible line, 如果可見的最後一行有 folding information.
bool svTextView::GetVisibleTextPosRange(size_t &p_srow, size_t &p_scol, size_t &p_erow, size_t &p_ecol)
{
    int s=0;
    if ((s=(int)m_textList.size())>0)
    {

        int lastRowId = s > m_pageLinesCnt ? m_pageLinesCnt-1 : s-1;
        // Get the text positon information of the first and the last line on text view.
        svLineText *f = m_bufText->GetLine(m_textList.at(0).unwrap_idx);
        // svLineText *l = m_bufText->GetLine(m_textList.at(lastRowId).unwrap_idx);
        svLineText *l;

        wrapLenDesc ldf, ldl;

        f->GetWrapLenOnIdx(m_textList.at(0).wrap_idx, ldf);

        // m_textList.at(0).unwrap_idx is the start row on the Text View
        // ldf.idx is the start column on the Text View
        p_srow = m_textList.at(0).unwrap_idx;
        p_scol = ldf.idx;



        foldingInfo fi;
        if (m_bufText->GetLine(m_textList.back().unwrap_idx)->HadFolding(fi) &&
            m_bufText->GetLine(m_textList.back().unwrap_idx)->GetWrapLineCount()==m_textList.back().wrap_idx+1)
        {
            // The last line on text view has folding indicator.


            l = m_bufText->GetLine(m_textList.at(lastRowId).unwrap_idx+fi.range);
            l->GetWrapLenOnIdx(l->GetWrapLineCount()-1, ldl);

            // m_textList.at(s-1).unwrap_idx+fi.range is the last row on the Text View
            // ldl.idx + ldl.len + 1 is the last column on the Text View
            p_erow = m_textList.at(lastRowId).unwrap_idx + fi.range;
            p_ecol = ldl.idx + ldl.len;
            // 非該未折行的最後一個字元後的游標位置
            if (l->TextLen(SVID_NO_CRLF)!=ldl.idx+ldl.len) 
            {
                p_ecol--;
            }

        }
        else
        {
            l = m_bufText->GetLine(m_textList.at(lastRowId).unwrap_idx);
            l->GetWrapLenOnIdx(m_textList.at(lastRowId).wrap_idx, ldl);

            // m_textList.at(s-1).unwrap_idx is the last row on the Text View
            // ldl.idx + ldl.len + 1 is the last column on the Text View
            p_erow = m_textList.at(lastRowId).unwrap_idx;
            p_ecol = ldl.idx + ldl.len;
            // 非該未折行的最後一個字元後的游標位置
            if (l->TextLen(SVID_NO_CRLF)!=ldl.idx+ldl.len) 
            {
                p_ecol--;
            }
        }

    }
    else
    {
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("svTextView::GetVisibleTextPosRange error: s=%i m_pageLinesCnt=%i "), s, m_pageLinesCnt));
#endif        
    }

    return true;

}

//取得畫面內可見文字行的最大pixel寬度(最寬文字行的pixel寬度)
int svTextView::GetMaxLinePixelWidth(void)
{
    int maxWidth = 0;
    for(std::vector<bufTextIndex>::iterator it=m_textList.begin();
        it!=m_textList.end();
        ++it)
    {
        if (it->pixelWidth>maxWidth) maxWidth = it->pixelWidth;
    }
    return maxWidth;
}


// update m_inCaretList & m_outCaretList information
// 更新 m_inCaretList & m_outCaretList 訊息(畫面內及畫面外的caret) << 指的是垂直行數
bool svTextView::UpdateCaretsInfo(void)
{
    if (m_inCaretList)
    {
        m_inCaretList->clear();
        delete m_inCaretList;
    }
    if (m_outCaretList)
    {
        m_outCaretList->clear();
        delete m_outCaretList;
    }

    // size_t firstRow, lastRow;
    // size_t firstCol, lastCol;

    GetVisibleTextPosRange(m_firstRow, m_firstCol, m_lastRow, m_lastCol);

    // Because svBufText->GetCaretsOfRange or svCaretList->GetCaretsOfRange
    // could be delete vector<svCaret> * pointer. So we not passing m_inCaretList, m_outCaretList as parameter.
    // 直接傳 m_inCaretList, m_outCaretList 會造成 access violation. 所以先傳local變數，再assign給 m_inCaretList, m_outCaretList.
    // Why access violation? I guess that's because we access svTextView private member data.
    vector<svCaret> *p_inCaretList=NULL;  // Carets list on textView
    vector<svCaret> *p_outCaretList=NULL; // Carets list out of textView

    // acquire caret on the textView.
    bool r = m_bufText->GetCaretsOfRange(m_firstRow, m_firstCol, m_lastRow, m_lastCol, &p_inCaretList, &p_outCaretList);

    m_inCaretList = p_inCaretList;
    m_outCaretList = p_outCaretList;

    return r;

}

// Get visible Carets(Carets position in pixel on TextView) as a list.
// Call by scTextEditCtrl for cursor drawing.
vector<caretPosOnView> *svTextView::CalVisibleCaretsPos(void)
{

    vector<caretPosOnView> *p_caretPosList = new vector<caretPosOnView>();

    if (!m_textList.size())  // m_textList.size()==0
        return p_caretPosList;

    // 將 caret 轉為 對應的畫面上的哪一行(row)，哪個位置(pixel length)。
    for(std::vector<svCaret>::iterator it=m_inCaretList->begin();
        it!=m_inCaretList->end();
        ++it)
    {
        if (m_bufText->GetLine(it->GetRow())->Visible())
        {
            // carets position line is invisible.

            for(size_t idx=0; idx<m_textList.size(); idx++)
            {
                bufTextIndex *bit = &m_textList.at(idx);
                if (it->GetRow()==(int)bit->unwrap_idx)
                {
                    if ( // 除了該未折行的最後一個字元後的游標位置以外的游標位置
                         // (it->GetVisualCol()>=(int)bit->wrap_desc.idx && it->GetVisualCol()<(int)(bit->wrap_desc.idx+bit->wrap_desc.len)) ||
                         (it->GetVisualCol()>=(int)bit->StartCol() && it->GetVisualCol()<(int)(bit->StartCol()+bit->Len())) ||
                         // 該未折行的最後一個字元後的游標位置
                         // (it->GetVisualCol()==(int)(bit->wrap_desc.idx+bit->wrap_desc.len) && (m_bufText->TextLenAt(it->GetRow(), SVID_NO_CRLF)==bit->wrap_desc.idx+bit->wrap_desc.len)) 
                         (it->GetVisualCol()==(int)(bit->StartCol()+bit->Len()) && (m_bufText->TextLenAt(it->GetRow(), SVID_NO_CRLF)==bit->StartCol()+bit->Len())) 
                        )
                    {
                        //
                        caretPosOnView cp;
                        cp.row = idx;
                        svLineText *line = m_bufText->GetLine(it->GetRow());

                        // 計算 文字左邊界的像素值(已考慮折行)
                        int pixLen = 0; // 未折行的像素值
                        line->TextCol2PixelUW(it->GetVisualCol(), pixLen);

                        // 有無折行，如有要將折行的像素扣除
                        int wrapPixLen = 0; // 折行的像素值
                        line->TextCol2PixelUW(m_bufText->GetWrappedLineStartColOnPositionAt(it->GetRow(), it->GetVisualCol()), wrapPixLen);
                        cp.accumPixelWidth = pixLen - wrapPixLen;
                        // 讓折行內縮等於該行(未折時)ident相同。
                        if (wrapPixLen>0)   // 表示不是折行的第一行，折行的第二行起才要內縮
                            cp.accumPixelWidth += line->GetIndentPixelLen();
                        p_caretPosList->push_back(cp);
                        // continue;
                        break;
                    }
                }
            }

        }   // end of m_bufText->GetLine(it->GetRow()).Visible()
        else
        {
            // carets position line is invisible.
            // 當 caret 所在的行被隱藏時，caret 被顯示在前一個未隱藏行的最後一個字元後的 folding indicator 符號之後，約一個 space 的寬度。

            int prev_r=0;
            if (m_bufText->GetPrevVisibleLine(it->GetRow(), prev_r))
            {
                if ((it->GetRow()>=(int)m_textList.front().unwrap_idx &&
                     it->GetRow()<=(int)m_textList.back().unwrap_idx) ||
                    prev_r==(int)m_textList.back().unwrap_idx)   // 畫面上最後一行是 folding indicator 行 
                // invisible line in text view range.
                {

                    // 由下往上找到最接近的 visible line(才會找到最後一個折行)
                    // 將 caret 顯示在 最後一個折行 的最末端

                    for(size_t idx=m_textList.size()-1; idx>=0; idx--)
                    {

                        bufTextIndex *bit = &m_textList.at(idx);
                        if (prev_r==(int)bit->unwrap_idx)
                        {
                            // Sorry! I DO NOT LIKE NESTED CONDITION TOO.

                            caretPosOnView cp;
                            cp.row = idx;
                            svLineText *line = m_bufText->GetLine(prev_r);

                            // 計算 文字左邊界的像素值(已考慮折行)
                            int pixLen = 0; // 未折行的像素值
                            // line->TextCol2PixelUW(bit->wrap_desc.idx+bit->wrap_desc.len, pixLen);
                            line->TextCol2PixelUW(bit->StartCol()+bit->Len(), pixLen);

                            // 有無折行，如有要將折行的像素扣除
                            int wrapPixLen = 0; // 折行的像素值
                            // line->TextCol2PixelUW(m_bufText->GetWrappedLineStartColOnPositionAt(prev_r, bit->wrap_desc.idx+bit->wrap_desc.len), wrapPixLen);
                            line->TextCol2PixelUW(m_bufText->GetWrappedLineStartColOnPositionAt(prev_r, bit->StartCol()+bit->Len()), wrapPixLen);
                            cp.accumPixelWidth = pixLen - wrapPixLen;
                            // 讓折行內縮等於該行(未折時)ident相同。
                            if (wrapPixLen>0)   // 表示不是折行的第一行，折行的第二行起才要內縮
                                cp.accumPixelWidth += line->GetIndentPixelLen();
                            cp.accumPixelWidth += m_spaceWidth;
                            p_caretPosList->push_back(cp);
                            // continue;
                            break;

                        }
                    }

                }

            }

        }
    }

    // }

    return p_caretPosList;

}

// Get embrace symbol of visible Carets(in row, col) as a list.
// Call by scTextEditCtrl for embrace symbol drawing.
// The function's name is too long. Very bad.
vector<textRange> *svTextView::CalVisibleCaretsEmbraceSymbolPos(void)
{

    vector<textRange> *p_embracePosList = new vector<textRange>();

    if (!m_textList.size())  // m_textList.size()==0
        return p_embracePosList;

    // When there are more than 1 carets, don't display embrace symbol hilight. 
    // Because of performance consideration.    
    if (m_inCaretList->size()>1)    
        return p_embracePosList;

    vector<pairSymbol> embraceSymols = m_bufText->GetEmbraceSymbol();

    // 找出 caret 的 embrace symbol位置
    // 再轉為 對應的畫面上的哪一行(row)，哪個位置(pixel length)
    for(std::vector<svCaret>::iterator it=m_inCaretList->begin();
        it!=m_inCaretList->end();
        ++it)
    {
        bool s_found, e_found;
        int  start_row, start_col;
        int  end_row, end_col;

        pairSymbol s_foundSymbol, e_foundSymbol;

        s_found = e_found = false;
        start_row = start_col = end_row = end_col = 0;


        // m_bufText->FindEmbracePosInRange(it->GetRow(), it->GetVisualCol(), embraceSymols, m_firstRow, m_lastRow, start_row, start_col, end_row, end_col, s_found, e_found, s_foundSymbol, e_foundSymbol);
        m_bufText->FindEmbracePosInRange(it->GetRow(), it->GetVisualCol(), embraceSymols, 0, m_bufText->LineCntUW()-1, start_row, start_col, end_row, end_col, s_found, e_found, s_foundSymbol, e_foundSymbol);

        if (s_found && e_found && s_foundSymbol==e_foundSymbol)
        {
            p_embracePosList->push_back(textRange(start_row, start_col, start_row, start_col+s_foundSymbol.s_len));
            p_embracePosList->push_back(textRange(end_row, end_col, end_row, end_col+e_foundSymbol.e_len));
        }

    }

    return p_embracePosList;

}


// Return true if any Caret on the view.
// When No Caret on the view, p_caret will be setted as the nearest caret position(r, c)
// 當有任何的 Caret 在畫面上時，回傳 false 
// 否則傳回最接近畫面的 Caret 位置。(目前最接近是以未折行的行數來比較)
// 因為目前最接近是以未折行的行數來比較，所以若同一行有兩個以上的 Caret 且折行後位在不同的折行時，將 Caret 保持在 TextView 內可能會判段到不是最近的 Caret
bool svTextView::GetNearestCaret(svCaret& p_caret, bool& p_forward)
{
    // There are caret(s) in the TextView
    if (m_inCaretList->size()>0)
        return false;

    // There are no caret(s) out of the TextView
    if (m_outCaretList->size()==0)
        return false;

    int keep_idx=0;
    int shortest= std::numeric_limits<int>::max();

    int keep_col=-1;

    for(int i=0; i<(int)m_outCaretList->size(); ++i)
    {
        // Before TextView
        if (m_outCaretList->at(i).GetRow()<=(int)m_firstRow)
        {
            if ((int)m_firstRow-m_outCaretList->at(i).GetRow()<(int)shortest)
            {
                shortest = m_firstRow-m_outCaretList->at(i).GetRow();
                keep_idx = i;
                keep_col = m_outCaretList->at(i).GetVisualCol();
                p_forward = false;
            }
            else if ((int)m_firstRow-m_outCaretList->at(i).GetRow()==(int)shortest)
            {
                if (m_outCaretList->at(i).GetVisualCol()>keep_col)
                {
                    keep_idx = i;
                    keep_col = m_outCaretList->at(i).GetVisualCol();
                    p_forward = false;
                }
            }
        }
        else // After TextView
        {
            if (m_outCaretList->at(i).GetRow()-(int)m_lastRow<shortest)
            {
                shortest = m_outCaretList->at(i).GetRow()-m_lastRow;
                keep_idx = i;
                keep_col = m_outCaretList->at(i).GetVisualCol();
                p_forward = true;
            }
            else if (m_outCaretList->at(i).GetRow()-(int)m_lastRow==shortest)
            {
                if (m_outCaretList->at(i).GetVisualCol()<keep_col)
                {
                    keep_idx = i;
                    keep_col = m_outCaretList->at(i).GetVisualCol();
                    p_forward = true;
                }
            }
        }
    }

    // p_caret.SetPosition(m_outCaretList->at(keep_idx).GetRow(), m_outCaretList->at(keep_idx).GetCol(), m_outCaretList->at(keep_idx).GetVisualCol());
    p_caret = m_outCaretList->at(keep_idx);

    return true;

}

// 將 caret 保持在畫面中
void svTextView::KeepCaretOnView(void)
{
    // 1. 垂直處理
    svCaret p_caret;
    bool p_forward;
    if (GetNearestCaret(p_caret, p_forward))
    {
        if (p_forward) // 需向下
        {
            if ((int)(p_caret.GetRow()-m_lastRow)>GetLinesPerPage())  // 差太多行了，不捲動，直接跳到該行
                SetFirstLineIndex(p_caret.GetRow());
            else
                MoveFirstLineIndex(CalWrappedLineDistance(m_lastRow, m_lastCol, p_caret.GetRow(), p_caret.GetVisualCol()));

            Refresh();
        }
        else  // 需向上
        {
            if ((int)(m_firstRow-p_caret.GetRow())>GetLinesPerPage())  // 差太多行了，不捲動，直接跳到該行
                SetFirstLineIndex(p_caret.GetRow());
            else
                MoveFirstLineIndex(CalWrappedLineDistance(m_firstRow, m_firstCol, p_caret.GetRow(), p_caret.GetVisualCol()));
            Refresh();
        }
    }  

    // 2. 水平處理
    vector<caretPosOnView> *carets = CalVisibleCaretsPos();
    CalHScrollPixel(carets);
    carets->clear();
    delete carets;
}

// call vector<caretPosOnView> *CalVisibleCaretsPos(void) first 
// then call CalHScrollPixel passing the return vector from CalVisibleCaretsPos
// 計算左右捲動的像素值
int svTextView::CalHScrollPixel(vector<caretPosOnView> *p_caretPosList)
{
    int minHScrollPixel = 0;
    bool allOutOfVertialRange = true;
    int keepCaretPixel = 0;

    int left_margin_chars_cnt = 3;

    for(std::vector<caretPosOnView>::iterator it=p_caretPosList->begin();
        it!=p_caretPosList->end();
        ++it)
    {
        if ((int)it->accumPixelWidth>m_HScrollPixel && 
            (int)it->accumPixelWidth<m_HScrollPixel+m_visiblePixelWidth-left_margin_chars_cnt*m_spaceWidth)
        {
            allOutOfVertialRange = false;
        }
        else
        {
            if ((int)it->accumPixelWidth<=m_HScrollPixel)
            {
                // 往左方捲動
                int tmpPixel = m_HScrollPixel + - (int)it->accumPixelWidth ;
                if (tmpPixel > 0)
                {
                    if (!minHScrollPixel) // minHScrollPixel == 0
                    {
                        minHScrollPixel = tmpPixel;
                        keepCaretPixel = it->accumPixelWidth;
                    }
                    else
                    {
                        if (minHScrollPixel > tmpPixel)
                        {
                            minHScrollPixel = tmpPixel;
                            keepCaretPixel = it->accumPixelWidth;
                        }
                    }
                }
            }
            else if ((int)it->accumPixelWidth>=m_HScrollPixel+m_visiblePixelWidth-left_margin_chars_cnt*m_spaceWidth)
            {
                // 往右方捲動
                int tmpPixel = it->accumPixelWidth - m_HScrollPixel - m_visiblePixelWidth + left_margin_chars_cnt*m_spaceWidth;
                if (tmpPixel>0)
                {
                    if (!minHScrollPixel) // minHScrollPixel == 0
                    {
                        minHScrollPixel = tmpPixel;
                        keepCaretPixel = it->accumPixelWidth;
                    }
                    else
                    {
                        if (minHScrollPixel > tmpPixel)
                        {
                            minHScrollPixel = tmpPixel;
                            keepCaretPixel = it->accumPixelWidth;
                        }
                    }
                }
            }
        }
    }

    if (allOutOfVertialRange)  // 所有的caret都在畫面外時才計算左右捲動像素值
    {
        if (keepCaretPixel<=m_HScrollPixel)
        {
            // 往左方捲動
            m_old_HScrollPixel = m_HScrollPixel;
            m_HScrollPixel = m_HScrollPixel - minHScrollPixel;
        }
        else if (keepCaretPixel>=m_HScrollPixel+m_visiblePixelWidth-left_margin_chars_cnt*m_spaceWidth)
        {
            // 往右方捲動
            m_old_HScrollPixel = m_HScrollPixel;
            m_HScrollPixel = m_HScrollPixel + minHScrollPixel;
        }
    }

    return m_HScrollPixel;

}

// Return wrapped line distance by specified 2 position.
// Not including invisible line.
// 回傳svbufText任兩個位置的折行距離
// 傳入的位置指的是未折行的Row, Col訊息
// 不計看不見的行數
int svTextView::CalWrappedLineDistance(size_t p_startRow, size_t p_startCol, size_t p_endRow, size_t p_endCol)
{

    // true means end position greater than start position.
    bool p_forward = (p_startRow<p_endRow || (p_startRow==p_endRow && p_startCol<p_endCol));

    size_t p_sr, p_sc, p_er, p_ec;

    if (p_forward)
    {
        p_sr = p_startRow;
        p_sc = p_startCol;
        p_er = p_endRow;
        p_ec = p_endCol;
    }
    else
    {
        p_sr = p_endRow;
        p_sc = p_endCol;
        p_er = p_startRow;
        p_ec = p_startCol;
    }

    // If start row or end row is in invisible line, return 0.
    if (!m_bufText->VisibleAt(p_sr) || !m_bufText->VisibleAt(p_er))
    {
        return 0;
    }

    int sum = 0;
    for (int i=(int)p_sr; i<=(int)p_er; i++)
    {
        if (!m_bufText->VisibleAt(i))  // skip invisible line.
            continue;

        if (!m_bufText->IsStyledWrapProcessedAt(i))
            m_bufText->ProcWrapAndStyleAt((size_t)i, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
        if (i==p_sr && i==p_er) // first line and the last line is the same line
        {
            int c = m_bufText->InWhichWrappedLineAt((size_t)i, p_sc);
            int d = m_bufText->InWhichWrappedLineAt((size_t)i, p_ec);
            sum += d-c;
        } 
        else if (i==p_sr) // first line
        {
            int c = m_bufText->InWhichWrappedLineAt((size_t)i, p_sc);
            sum += m_bufText->GetWrapLineCountAt((size_t)i) - c;
        }
        else if (i==p_er) // last line
        {
            sum += m_bufText->InWhichWrappedLineAt((size_t)i, p_ec);
        }
        else // middle line
        {
            sum += m_bufText->GetWrapLineCountAt((size_t)i);
        }
    }

    if (!p_forward)
        sum = -sum;

    return sum;

}

// caret move left
void svTextView::CaretsLeft(void)
{
    m_bufText->CaretsLeft();
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_LEFT));
    
}

// caret move left to the word's head position. 
void svTextView::CaretsLeftHead(void)
{
    m_bufText->CaretsLeftHead(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_LEFT));
    
}

// caret move right
void svTextView::CaretsRight(void)
{
    m_bufText->CaretsRight();
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_RIGHT));
}

// caret move right to the word's end position.
void svTextView::CaretsRightEnd(void)
{
    m_bufText->CaretsRightEnd(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_RIGHT));
}

// caret move up
void svTextView::CaretsUp(void)
{
    // for abstracting svBufText class.
    // 1. Get all caret position an transfer into bufTextIndex format. (unwrap_idx, wrap_idx)
    // 2. Caculate new bufTextIndex format position. (unwrap_idx, wrap_idx)
    // 3. Set caret position in svBufText in (Row, Col) format.

    // Althought we pass m_tabSize, m_spaceWidth, m_font and m_maxPixelWidth as 
    // parameters to svBufText.CaretsUp. It's actually shoud not be modified on
    // different call. svTextView::CaretsDown follow the same rules.
    // m_tabSize, m_spaceWidth, m_font and m_maxPixelWidth 雖然是參數，但是這些參數
    // 不能改變，意即不同的svTextView必需使用相同的設定值。
    // 意即，一個svBufText可以有多個svTextView，但是m_tabSize, m_spaceWidth, m_font and m_maxPixelWidth
    // 的設定是相同的。
    // svTextView::CaretsDown也是相同的規則。
    m_bufText->CaretsUp(m_tabSize, m_spaceWidth, m_font, m_maxPixelWidth, m_wrap);
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_UP));
}

// caret move up
void svTextView::CaretsDown(void)
{
    m_bufText->CaretsDown(m_tabSize, m_spaceWidth, m_font, m_maxPixelWidth, m_wrap);
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_DOWN));
}

// caret move one page down
void svTextView::PageDown(void)
{
    int cnt = MoveFirstLineIndex(m_pageLinesCnt-1);
    for(int i=0; i<cnt; ++i)
    {
        m_bufText->CaretsDown(m_tabSize, m_spaceWidth, m_font, m_maxPixelWidth, m_wrap);
    }
    m_showFindKeywordInd = false;
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_PAGE_DOWN));
}

// caret move one page up
void svTextView::PageUp(void)
{
    int cnt = MoveFirstLineIndex(-m_pageLinesCnt+1);
    for(int i=0; i<-cnt; ++i)
    {
        m_bufText->CaretsUp(m_tabSize, m_spaceWidth, m_font, m_maxPixelWidth, m_wrap);
    }
    m_showFindKeywordInd = false;
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_PAGE_UP));
}


// caret move to the head of line(wrapped line)
void svTextView::CaretsLineHead(void)
{
    m_bufText->CaretsLineHead_W();
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_HEAD_OF_LINE));
}


// caret move to the end of line(wrapped line)
void svTextView::CaretsLineEnd(void)
{
    m_bufText->CaretsLineEnd_W();
    m_showFindKeywordInd = false;
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_END_OF_LINE));
}

void svTextView::ClearCaretSelect(void)
{
    m_bufText->ClearCaretSelect();
}

void svTextView::SetCaretSelect(void)
{
    m_bufText->RemoveHiddenLineCarets();
    m_bufText->RemoveDuplication();
    m_bufText->SetCaretSelect();
}

bool svTextView::GetCaretSelectPixel(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_textList.size()) return false;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    // size_t w_idx_s = m_textList.at(idx).wrap_desc.idx;
    // size_t w_idx_e = m_textList.at(idx).wrap_desc.idx + m_textList.at(idx).wrap_desc.len;
    size_t w_idx_s = m_textList.at(idx).StartCol();
    size_t w_idx_e = m_textList.at(idx).StartCol() + m_textList.at(idx).Len();

    return m_bufText->GetCaretSelectPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

bool svTextView::GetCaretSelectPixel_PP(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_pptextList.size()) return false;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;
    // size_t w_idx_s = m_pptextList.at(idx).wrap_desc.idx;
    // size_t w_idx_e = m_pptextList.at(idx).wrap_desc.idx + m_pptextList.at(idx).wrap_desc.len;
    size_t w_idx_s = m_pptextList.at(idx).StartCol();
    size_t w_idx_e = m_pptextList.at(idx).StartCol() + m_pptextList.at(idx).Len();

    return m_bufText->GetCaretSelectPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

bool svTextView::GetCaretSelectPixel_NP(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_nptextList.size()) return false;

    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;
    // size_t w_idx_s = m_nptextList.at(idx).wrap_desc.idx;
    // size_t w_idx_e = m_nptextList.at(idx).wrap_desc.idx + m_nptextList.at(idx).wrap_desc.len;
    size_t w_idx_s = m_nptextList.at(idx).StartCol();
    size_t w_idx_e = m_nptextList.at(idx).StartCol() + m_nptextList.at(idx).Len();

    return m_bufText->GetCaretSelectPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

// The same logic with GetCaretSelectPixel, but with textRange data structure.
bool svTextView::GetTextRangePixel(size_t idx, vector<int> &p_pixelList, vector<textRange> p_range)
{
    if (idx >= m_textList.size()) return false;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    // size_t w_idx_s = m_textList.at(idx).wrap_desc.idx;
    // size_t w_idx_e = m_textList.at(idx).wrap_desc.idx + m_textList.at(idx).wrap_desc.len;
    size_t w_idx_s = m_textList.at(idx).StartCol();
    size_t w_idx_e = m_textList.at(idx).StartCol() + m_textList.at(idx).Len();

    return m_bufText->GetTextRangePixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList, p_range);
}

void svTextView::ResetCaretPosition(const size_t p_row, const size_t p_col)
{
    m_bufText->ResetCaretPosition(p_row, p_col);
    Refresh();
}

void svTextView::AppendCaretPosition(const size_t p_row, const size_t p_col)
{
    m_bufText->AppendCaretPosition(p_row, p_col);
    Refresh();
}

void svTextView::LastCaretMoveTo(const size_t p_row, const size_t p_col)
{
    m_bufText->LastCaretMoveTo(p_row, p_col);
    Refresh();
}

// return the pixel location (x, y) for the last caret
bool svTextView::GetLastCaretPixelXY(int &p_x, int &p_y, int p_lineHeight)
{
    int row, col;
    row = col = 0;

    m_bufText->GetLastCaret()->GetPosition(row, col);

    if (TextRowCol2PixelXYUW(row, col, p_lineHeight, p_x, p_y))
    {
        return true;
    }
    else
    {
        return false;
    }

}

// return the pixel location (x, y) for the last caret position's word
bool svTextView::GetLastCaretWordPixelXY(int &p_x, int &p_y, int p_lineHeight)
{
    int row, col;
    row = col = 0;

    m_bufText->GetLastCaret()->GetPosition(row, col);
    
    int wpos, wlen;
    wpos=wlen=0;
    if (m_bufText->GetLine(row)->GetKeywordPosition(col, wpos, wlen))
    {
        if (TextRowCol2PixelXYUW(row, wpos, p_lineHeight, p_x, p_y))
        {
            return true;
        }
    }
    
    return false;

}

bool svTextView::GetTextIndentIndicatorCharLen(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_textList.size()) return false;

    extern svPreference g_preference;
    
    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    size_t w_idx = m_textList.at(idx).wrap_idx;

    int spaceCnt = m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW();
    spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
    
    if (m_bufText->GetLine(uw_idx)->TextIsEmptyOrBlank())
    {
        // a blank or empty line, reference to the previous non blank line.
        // If the performance is an issue, rewrite(restructure) this section.
        bool found = false;
        int prvIdx = uw_idx-1;
        while(prvIdx>=0)
        {
            if (!m_bufText->GetLine(prvIdx)->TextIsEmptyOrBlank())
            {
                found = true;
                break;
            }
            --prvIdx;
        }
        
        if (found)
        {
            spaceCnt = (int)m_bufText->GetLine(prvIdx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(prvIdx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
            spaceCnt += g_preference.GetTabSize();
        }
    }
    else
    {
        if (w_idx>0)
        {
            // a wrapped line.    
            //spaceCnt += (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
        }
    }
    
    int ind=0;
    p_pixelList.clear();
    while (spaceCnt>ind)
    {
        p_pixelList.push_back(ind);
        ind += g_preference.GetTabSize();
    }

    if (p_pixelList.size())
        return true;
    else
        return false;
}

bool svTextView::GetTextIndentIndicatorCharLen_PP(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_pptextList.size()) return false;

    extern svPreference g_preference;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;
    size_t w_idx = m_pptextList.at(idx).wrap_idx;

    int spaceCnt = m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW();
    spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
    
    if (m_bufText->GetLine(uw_idx)->TextIsEmptyOrBlank())
    {
        // a blank or empty line, reference to the previous non blank line.
        // If the performance is an issue, rewrite(restructure) this section.
        bool found = false;
        int prvIdx = uw_idx-1;
        while(prvIdx>=0)
        {
            if (!m_bufText->GetLine(prvIdx)->TextIsEmptyOrBlank())
            {
                found = true;
                break;
            }
            --prvIdx;
        }
        
        if (found)
        {
            spaceCnt = (int)m_bufText->GetLine(prvIdx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(prvIdx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
            spaceCnt += g_preference.GetTabSize();
        }
    }
    else
    {
        if (w_idx>0)
        {
            // a wrapped line.    
            //spaceCnt += (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
        }
    }
    
    int ind=0;
    p_pixelList.clear();
    while (spaceCnt>ind)
    {
        p_pixelList.push_back(ind);
        ind += g_preference.GetTabSize();
    }


    if (p_pixelList.size())
        return true;
    else
        return false;
}

bool svTextView::GetTextIndentIndicatorCharLen_NP(size_t idx, vector<int> &p_pixelList)
{
    if (idx >= m_nptextList.size()) return false;

    extern svPreference g_preference;
    
    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;
    size_t w_idx = m_nptextList.at(idx).wrap_idx;

    int spaceCnt = m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW();
    spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
    
    if (m_bufText->GetLine(uw_idx)->TextIsEmptyOrBlank())
    {
        // a blank or empty line, reference to the previous non blank line.
        // If the performance is an issue, rewrite(restructure) this section.
        bool found = false;
        int prvIdx = uw_idx-1;
        while(prvIdx>=0)
        {
            if (!m_bufText->GetLine(prvIdx)->TextIsEmptyOrBlank())
            {
                found = true;
                break;
            }
            --prvIdx;
        }
        
        if (found)
        {
            spaceCnt = (int)m_bufText->GetLine(prvIdx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(prvIdx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
            spaceCnt += g_preference.GetTabSize();
        }
    }
    else
    {
    if (w_idx>0)
        {
            // a wrapped line.    
            //spaceCnt += (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = (int)m_bufText->GetLine(uw_idx)->GetIndentCharLen();
            spaceCnt = m_bufText->GetLine(uw_idx)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
        }
    }
    
    int ind=0;
    p_pixelList.clear();
    while (spaceCnt>ind)
    {
        p_pixelList.push_back(ind);
        ind += g_preference.GetTabSize();
    }

    if (p_pixelList.size())
        return true;
    else
        return false;
}

/****************************************************************************
 *  Editing related functions.
 ****************************************************************************/

// EditingInsertChar is for insert wxString without newline characters
void svTextView::EditingInsertChar(const wxString &p_str)
{
    bool cont = false;
    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        m_bufText->EditingSelectedTextDelete();
    }
    else
    {
        cont = m_bufText->CheckContinousUndoOperation(SVID_UNDO_INSERT);
        cont = cont && m_actionRec.LastActionIs(SVID_ACTION_INSERT);
        if (!cont)
        {
            m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        }
    }

    m_bufText->EditingInsertChar(p_str, cont, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingInsertChar() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();

    m_bufText->UpdateAvailableHint();
    m_actionRec.Append(svAction(SVID_ACTION_INSERT));
}


// EditingInsertHintChar is for insert wxString without newline characters from typing hint
void svTextView::EditingInsertHintChar(const wxString &p_str, int p_curWordLen, int p_curWordOffset)
{
    bool cont = false;
    m_bufText->ClearCaretSelect();

    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    for (int i=0; i<p_curWordOffset; i++)
    {
        m_bufText->CaretsRight();
    }

    for (int i=0; i<p_curWordLen; i++)
    {
        m_bufText->EditingTextBackDelete(cont);
        cont = true;
    }

    m_bufText->EditingInsertChar(p_str, false, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingInsertChar() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();

    m_bufText->UpdateAvailableHint();
    m_actionRec.Append(svAction(SVID_ACTION_INSERT_HINT));
}


void svTextView::EditingSplitLine(void)
{
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->EditingSelectedTextDelete();
    }

    m_bufText->EditingSplitLine(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // m_bufText->CalHScrollPixel();
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingSplitLine() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_SPLIT));
}

void svTextView::EditingTextDelete(void)
{
    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        m_bufText->EditingSelectedTextDelete();
    }
    else
    {
        bool cont = m_bufText->CheckContinousUndoOperation(SVID_UNDO_DELETE);
        if (!cont)
        {
            m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        }
        m_bufText->EditingTextDelete(cont);
    }
    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_DELETE));
}

void svTextView::EditingTextBackDelete(void)
{
    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        m_bufText->EditingSelectedTextDelete();
    }
    else
    {
        bool cont = m_bufText->CheckContinousUndoOperation(SVID_UNDO_BACKDEL);
        if (!cont)
        {
            m_bufText->InitialNewUndoActions();  // initialize a new undo action list.
        }
        m_bufText->EditingTextBackDelete(cont);
    }
    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_BACKDEL));
}

void svTextView::EditingTextCopySelected(void)
{
    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->EditingTextCopySelected();
    }
    else
    {
        // Do nothing.
    }
    // m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;

    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_COPY));
}

void svTextView::EditingTextPaste(void)
{
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->EditingSelectedTextDelete();
    }
    m_bufText->EditingPaste(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_PASTE));
}

void svTextView::EditingTextCut(void)
{
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    if (m_bufText->CaretsHasSelect())
    {
        m_bufText->EditingTextCopySelected();
        m_bufText->EditingSelectedTextDelete();
    }

    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CUT));
}

void svTextView::EditingTextLineComment(void)
{
    wxString lineComment;

    if (!m_bufText->GetLineCommentSymbol(lineComment))  // No line comments symbol found. cannot be line comment.
        return;

    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    m_bufText->EditingLineComment(lineComment, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_LINE_COMMENT));
}

void svTextView::EditingTextBlockComment(void)
{
    wxString startComment, endComment;

    if (!m_bufText->GetBlockCommentSymbol(startComment, endComment))  // No block symbol found. cannot be block comments.
        return;
    if (!m_bufText->CaretsHasSelect()) // carets have no select, cannot be block comments.
        return;

    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    m_bufText->EditingBlockComment(startComment, endComment, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);

    m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_BLOCK_COMMENT));
}

// EditingInsertChar is for insert wxString without newline characters
void svTextView::EditingTextIndent(void)
{
    bool cont = false;
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    m_bufText->EditingIndent(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    // m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingInsertChar() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();

    m_bufText->UpdateAvailableHint();
    m_actionRec.Append(svAction(SVID_ACTION_INDENT));
}

// EditingInsertChar is for insert wxString without newline characters
void svTextView::EditingTextOutdent(void)
{
    bool cont = false;
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    m_bufText->EditingOutdent(m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap);
    // m_bufText->ClearCaretSelect();
    m_showFindKeywordInd = false;
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingInsertChar() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();

    m_bufText->UpdateAvailableHint();
    m_actionRec.Append(svAction(SVID_ACTION_INDENT));
}

void svTextView::EditingTextResetCarets(void)
{
    m_bufText->KeepFirstCaret();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_CARET_RESET));
}

void svTextView::EditingTextDuplicateLine(void)
{
    m_bufText->InitialNewUndoActions();  // initialize a new undo action list.

    // if (m_bufText->CaretsHasSelect())
    // {
    //     m_bufText->ClearCaretSelect();
    // }
    // else
    // {
    //     // Do nothing.
    // }

    m_bufText->ClearCaretSelect();
    m_bufText->EditingDuplicateLine();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_DUP_LINE));
}

void svTextView::UndoEditing(void)
{

    m_bufText->UndoEditing();
    m_showFindKeywordInd = false;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_UNDO));
}

/* ------------------------------------------------------------------------
 *  Finding related functions.
 * ------------------------------------------------------------------------ */
void svTextView::FindNext(void)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(m_bufText->GetLastFindReplaceOption());
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();

    int dstLine = 0;
    int dstCol = 0;
    if (m_bufText->CaretMoveToMatch(SVID_NEXT_MATCH, dstLine, dstCol, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);
    }

    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();     // 如果行數跳躍很多時，這行的處理會很慢，要修改  
    m_actionRec.Append(svAction(SVID_ACTION_FIND_NEXT));
}

void svTextView::FindPrev(void)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(m_bufText->GetLastFindReplaceOption());
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();

    
    int dstLine = 0;
    int dstCol = 0;
    if (m_bufText->CaretMoveToMatch(SVID_PREV_MATCH, dstLine, dstCol, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);
    }

    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();     // 如果行數跳躍很多時，這行的處理會很慢，要修改  
    m_actionRec.Append(svAction(SVID_ACTION_FIND_PREV));
}

void svTextView::FindAll(void)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(m_bufText->GetLastFindReplaceOption());
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();

    int dstLine = 0;
    int dstCol = 0;
    if (m_bufText->CaretMoveToAllMatch(dstLine, dstCol, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);
    }

    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();     // 如果行數跳躍很多時，這行的處理會很慢，要修改  
    m_actionRec.Append(svAction(SVID_ACTION_FIND_PREV));
}

void svTextView::ReplaceNext(const svFindReplaceOption &p_frOption)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(p_frOption);
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->InitialNewUndoActions();
    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();
    
    int dstLine = 0;
    if (m_bufText->Replace(SVID_NEXT_MATCH, p_frOption, dstLine, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);

    }
    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();      // 如果行數跳躍很多時，這行的處理會很慢，要修改
    m_actionRec.Append(svAction(SVID_ACTION_FIND_CURRENT_WORD_NEXT));
}

void svTextView::ReplacePrev(const svFindReplaceOption &p_frOption)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(p_frOption);
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->InitialNewUndoActions();
    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();
    
    int dstLine = 0;
    if (m_bufText->Replace(SVID_PREV_MATCH, p_frOption, dstLine, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);

    }
    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();      // 如果行數跳躍很多時，這行的處理會很慢，要修改
    m_actionRec.Append(svAction(SVID_ACTION_FIND_CURRENT_WORD_NEXT));
}

void svTextView::ReplaceAll(const svFindReplaceOption &p_frOption)
{
    if (m_bufText->FileChangedSinceLastSearch())
    {
        FindMatchLocations(p_frOption);
    }

    if (!m_bufText->HasMatchedWord())
    {
        return;
    }

    m_bufText->InitialNewUndoActions();
    m_bufText->ClearCaretSelect();
    m_bufText->KeepFirstCaret();
    
    int dstLine = 0;
    if (m_bufText->ReplaceAll(p_frOption, dstLine, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth, m_wrap))
    {
        // m_prevFROption = p_frOption;
        // 加快換頁速度
        if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
            ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
            SetFirstLineIndex(dstLine-2);

    }
    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();      // 如果行數跳躍很多時，這行的處理會很慢，要修改
    m_actionRec.Append(svAction(SVID_ACTION_FIND_CURRENT_WORD_NEXT));
}


bool svTextView::GetFindKeywordPixel(size_t idx, vector<int> &p_pixelList)
{
    if (!m_showFindKeywordInd) return false;
    if (idx >= m_textList.size()) return false;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    size_t w_idx_s = m_textList.at(idx).StartCol();
    size_t w_idx_e = m_textList.at(idx).StartCol() + m_textList.at(idx).Len();

    return m_bufText->GetFindMatchPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

bool svTextView::GetFindKeywordPixel_PP(size_t idx, vector<int> &p_pixelList)
{
    if (!m_showFindKeywordInd) return false;
    if (idx >= m_pptextList.size()) return false;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;
    size_t w_idx_s = m_pptextList.at(idx).StartCol();
    size_t w_idx_e = m_pptextList.at(idx).StartCol() + m_pptextList.at(idx).Len();

    return m_bufText->GetFindMatchPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

bool svTextView::GetFindKeywordPixel_NP(size_t idx, vector<int> &p_pixelList)
{
    if (!m_showFindKeywordInd) return false;
    if (idx >= m_nptextList.size()) return false;

    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;
    size_t w_idx_s = m_nptextList.at(idx).StartCol();
    size_t w_idx_e = m_nptextList.at(idx).StartCol() + m_nptextList.at(idx).Len();

    return m_bufText->GetFindMatchPixelAtW(uw_idx, w_idx_s, w_idx_e, m_spaceWidth, p_pixelList);
}

bool svTextView::FindMatchLocations(const svFindReplaceOption &p_frOption)
{
    // 不清除 caret 選擇區間，也不只留下第一個 caret
    // 因為 find & replace 有 inSelection 選項，只 find & replace caret selection 文字。
    // m_bufText->ClearCaretSelect();
    // m_bufText->KeepFirstCaret();

    bool result=false;
    int srow, scol;
    srow=scol=0;
    if (m_textList.size()>0)
    {
        srow = m_textList.at(0).unwrap_idx;
        scol = m_textList.at(0).wrap_idx;
    }

    int dstLine = 0;
    vector<svInt2Pair> resList;
    if (m_bufText->FindMatchLocations(p_frOption))
    {
        result = true;
        // m_prevFind = SVID_FIND_REGEX;
        // m_prevFROption = p_frOption;

        // if (resList.size()>0)
        // {
        //     dstLine = resList.at(0).num1;
        //     // 加快換頁速度
        //     if (((int)m_firstRow>dstLine && (int)m_firstRow-dstLine>GetCurPageTextListCount()*2) ||
        //         ((int)m_firstRow<dstLine && dstLine-(int)m_firstRow>GetCurPageTextListCount()*2) ) 
        //         SetFirstLineIndex(dstLine-2);
        // }
    }
    m_showFindKeywordInd = true;
    
    // Before call UpdateCaretsInfo, please update the m_textList information.
    // Because EditingTextBackDelete() may change the text, so Refresh to recalate the m_textList data.
    Refresh();
    UpdateCaretsInfo();
    KeepCaretOnView();
    m_actionRec.Append(svAction(SVID_ACTION_FIND_REGEX));
    return result;
}


// Move current page, let p_lineNo line in the middle of screen.
void svTextView::GotoLine(const int p_lineNo)
{
    if (p_lineNo>=(int)m_bufText->LineCntUW()||
        p_lineNo<0)
        return;

    int dstLine = p_lineNo - (m_pageLinesCnt/2);
    if (dstLine<0) dstLine = 0;

    int curFirstLineNo = (int)m_firstIndex.unwrap_idx;
    int diff = dstLine - curFirstLineNo;

    m_bufText->KeepFirstCaret();
    m_bufText->ClearCaretSelect();
    m_bufText->LastCaretMoveTo(p_lineNo, 0);

    if (diff==0)
    {
        // return;
    }
    else if (diff<0)
    {
        if (diff>m_pageLinesCnt*-2)
        {
            MoveFirstLineIndex(diff);
        }
        else
        {
            if (dstLine+m_pageLinesCnt<(int)m_bufText->LineCntUW())
            {
                SetFirstLineIndex(dstLine+m_pageLinesCnt);
                MoveFirstLineIndex(-m_pageLinesCnt);
            }
            else
            {
                SetFirstLineIndex(p_lineNo);
            }
        }

    }
    else // diff > 0
    {
        if (diff<m_pageLinesCnt*2)
        {
            MoveFirstLineIndex(diff);
        }
        else
        {
            if (dstLine-m_pageLinesCnt>0)
            {
                SetFirstLineIndex(dstLine-m_pageLinesCnt);
                MoveFirstLineIndex(m_pageLinesCnt);
            }
            else
            {
                SetFirstLineIndex(p_lineNo);
            }
        }
    }

    Refresh();
    UpdateCaretsInfo();
    // KeepCaretOnView();
}

/*---------------------------------------------------------------------------
 *  Folding related functions.
 *---------------------------------------------------------------------------*/
void svTextView::DoFolding(const size_t p_unwrap_idx)
{
    foldingInfo fi;

    if (m_bufText->GetLine(p_unwrap_idx)->HadFolding(fi))
    {
        // unfolding.
        m_bufText->UnfoldingAt(p_unwrap_idx);
    }
    else
    {
        // Folding
    
        if (       m_bufText->IsPythonSyntax() && 
               (   p_unwrap_idx+1<m_bufText->LineCntUW() && 
                 ( m_bufText->GetLine(p_unwrap_idx)->FindFirstNonSpaceColUW() <
                   m_bufText->GetLine(p_unwrap_idx+1)->FindFirstNonSpaceColUW() 
                 ) &&
                 (m_bufText->GetLine(p_unwrap_idx+1)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(p_unwrap_idx+1)->FindFirstNonSpaceColUW())
               )
           )
        {
            // for python indent folding.
            // Didn't consideration about the python folding symbol :
            int indentSpace = m_bufText->GetLine(p_unwrap_idx)->FindFirstNonSpaceColUW();
            
            int r=0;
            for(r=(int)p_unwrap_idx+1; r<(int)m_bufText->LineCntUW(); r++)
            {
                // looking for the end indent
                
                if (m_bufText->GetLine(r)->FindFirstNonSpaceColUW()<=indentSpace)
                {
                    m_bufText->FoldingAt(p_unwrap_idx, r);
                    break;
                }
            }
            
        }
        else
        {
            // regular folding by symbol like { }  () etc.
            // & python ''' ''' folding if python indent folding is not available.
            int pos = 0;
            int end_cnt = 0;
            pairSymbol ps;
            vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
            if (m_bufText->GetLine(p_unwrap_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps))
            {
                int r=0;
                int start_count = 0;
                for(r=(int)p_unwrap_idx+1; r<(int)m_bufText->LineCntUW(); r++)
                {
                    // 找對應的 {} 
                    if (m_bufText->GetLine(r)->KeywordContainSymbolEnd(v_ps, -1, pos, start_count, ps))
                    {
                        m_bufText->FoldingAt(p_unwrap_idx, r);
                        break;
                    }
                }
            }
        }
    
    
    }

    Refresh();
}

bool svTextView::HadFolding_PP(size_t idx)
{
    if (idx >= m_pptextList.size()) return false;

    // only first line of wrapped line show folding indicator.
    if (m_pptextList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::HadFolding(size_t idx)
{
    if (idx >= m_textList.size()) return false;

    // only first line of wrapped line show folding indicator.
    if (m_textList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::HadFolding_NP(size_t idx)
{
    if (idx >= m_nptextList.size()) return false;

    // only first line of wrapped line show folding indicator.
    if (m_nptextList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::HadFoldingTail_PP(size_t idx)
{
    if (idx >= m_pptextList.size()) return false;

    // only last line of wrapped line show folding tail indicator.
    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;
    size_t w_idx = m_pptextList.at(idx).wrap_idx;

    if (!(m_bufText->GetLine(uw_idx)->IsLastWrappedLine(w_idx)))
        return false;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::HadFoldingTail(size_t idx)
{
    if (idx >= m_textList.size()) return false;

    // only last line of wrapped line show folding tail indicator.
    size_t uw_idx = m_textList.at(idx).unwrap_idx;
    size_t w_idx = m_textList.at(idx).wrap_idx;

    if (!(m_bufText->GetLine(uw_idx)->IsLastWrappedLine(w_idx)))
        return false;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::HadFoldingTail_NP(size_t idx)
{
    if (idx >= m_nptextList.size()) return false;

    // only last line of wrapped line show folding tail indicator.
    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;
    size_t w_idx = m_nptextList.at(idx).wrap_idx;

    if (!(m_bufText->GetLine(uw_idx)->IsLastWrappedLine(w_idx)))
        return false;

    foldingInfo fi;

    return m_bufText->GetLine(uw_idx)->HadFolding(fi);
}

bool svTextView::CanFolding_PP(size_t idx)
{
    if (idx >= m_pptextList.size()) return false;

    // only first line of wrapped line show folding indicator.
    if (m_pptextList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_pptextList.at(idx).unwrap_idx;

    int pos;
    pairSymbol ps;
    int end_cnt=0;

    if (m_bufText->IsPythonSyntax())
    {
        if (   uw_idx+1<m_bufText->LineCntUW() &&   // next line exist.
             ( m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW() <
               m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()   // indent space is not the same
             ) &&
             (m_bufText->GetLine(uw_idx+1)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()) && // not a blank line
             (m_bufText->GetLine(uw_idx)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW())      // not a blank line 
           )
        {
            // for python indent folding.
            // Didn't consideration about the python folding symbol :
            return true;
        }
        else
        {
            // for python '''   '''  block comment folding.
            vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
            return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
        }
    }
    else
    {
        // regular folding by symbol like { }  () etc.
        vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
        return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
    }

    //vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
    //return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
}

bool svTextView::CanFolding(size_t idx)
{
    if (idx >= m_textList.size()) return false;

    // only first line of wrapped line show folding indicator.
    // if (!(m_textList.at(idx).wrap_idx))
    if (m_textList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_textList.at(idx).unwrap_idx;

    int pos;
    pairSymbol ps;
    int end_cnt=0;

    if (m_bufText->IsPythonSyntax())
    {
        if (   uw_idx+1<m_bufText->LineCntUW() &&   // next line exist.
             ( m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW() <
               m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()   // indent space is not the same
             ) &&
             (m_bufText->GetLine(uw_idx+1)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()) && // not a blank line
             (m_bufText->GetLine(uw_idx)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW())      // not a blank line 
           )
        {
            // for python indent folding.
            // Didn't consideration about the python folding symbol :
            return true;
        }
        else
        {
            // for python '''   '''  block comment folding.
            vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
            return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
        }
    }
    else
    {
        // regular folding by symbol like { }  () etc.
        vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
        return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
    }
}

bool svTextView::CanFolding_NP(size_t idx)
{
    if (idx >= m_nptextList.size()) return false;

    // only first line of wrapped line show folding indicator.
    if (m_nptextList.at(idx).wrap_idx!=0)
        return false;

    size_t uw_idx = m_nptextList.at(idx).unwrap_idx;

    int pos;
    pairSymbol ps;
    int end_cnt=0;

    if (m_bufText->IsPythonSyntax())
    {
        if (   uw_idx+1<m_bufText->LineCntUW() &&   // next line exist.
             ( m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW() <
               m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()   // indent space is not the same
             ) &&
             (m_bufText->GetLine(uw_idx+1)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx+1)->FindFirstNonSpaceColUW()) && // not a blank line
             (m_bufText->GetLine(uw_idx)->TextLen(SVID_NO_CRLF)!=m_bufText->GetLine(uw_idx)->FindFirstNonSpaceColUW())      // not a blank line 
           )
        {
            // for python indent folding.
            // Didn't consideration about the python folding symbol :
            return true;
        }
        else
        {
            // for python '''   '''  block comment folding.
            vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
            return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
        }
    }
    else
    {
        // regular folding by symbol like { }  () etc.
        vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
        return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
    }

    //vector<pairSymbol> v_ps = m_bufText->GetFoldingSymbol();
    //return m_bufText->GetLine(uw_idx)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps);
}
