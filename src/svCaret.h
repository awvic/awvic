/*
   Copyright Notice in awvic.cpp
*/

#ifndef _SVCARET_H
#define _SVCARET_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

// #include <cstdint>
#include <stdint.h>
#include <vector>

using namespace std;

enum
{
    SVID_NO_SELECT=-1,
    SVID_CARET_ON_HEAD=0,
    SVID_CARET_ON_TAIL=1
};

class svCaret
{
public:
    svCaret();
    svCaret(int p_row, int p_col);
    svCaret(int p_row, int p_visual_col, int p_keep_space, int p_select_row, int p_select_col, int p_select_keep_space);
    ~svCaret();

    inline
    void SetRow(int p_row)
    {
        m_row = p_row;
    }

    // SetPosition doesn't set m_keep_space and m_select_*
    inline
    void SetPosition(int p_row, int p_col)
    {
        m_row = p_row;
        m_visual_col = p_col;
    }

    // // ResetPosition alse set m_keep_space
    // inline
    // void ResetPosition(int p_row, int p_col)
    // {
    //     m_row = p_row;
    //     m_visual_col = m_keep_space = p_col;
    // }
  
    inline
    int GetRow() const
    {
        return m_row;
    }

    inline
    int GetVisualCol() const
    {
        return m_visual_col;
    }

    inline 
    void SetKeepSpace(int p_keep_space)
    {
        m_keep_space = p_keep_space;
    }

    inline 
    int GetKeepSpace() const
    {
        return m_keep_space;
    }

    svCaret(const svCaret& obj)  // copy constructor
    {
        m_row = obj.m_row;
        m_visual_col = obj.m_visual_col;
        m_keep_space = obj.m_keep_space;
        m_select_row = obj.m_select_row;
        m_select_col = obj.m_select_col;
        m_select_keep_space = obj.m_select_keep_space;
    }

    svCaret& operator=(const svCaret& obj)
    {
        m_row = obj.m_row;
        m_visual_col = obj.m_visual_col;
        m_keep_space = obj.m_keep_space;
        m_select_row = obj.m_select_row;
        m_select_col = obj.m_select_col;
        m_select_keep_space = obj.m_select_keep_space;
        return *this;
    }

    inline
    bool operator==(const svCaret &c)
    {

        // if (this->GetRow()==c.GetRow() &&
        //     this->GetVisualCol()==c.GetVisualCol())
        if (m_row==c.m_row && m_visual_col==c.m_visual_col)
            return true;
        else
            return false;
    }

    inline
    bool operator<(const svCaret &c)
    {
        if ( this->GetRow()<c.GetRow() ||
            (this->GetRow()==c.GetRow() &&
             this->GetVisualCol()<c.GetVisualCol())
           ) 
            return true;
        else
            return false;
    }

    inline
    bool operator>(const svCaret &c)
    {
        if ( this->GetRow()>c.GetRow() ||
            (this->GetRow()==c.GetRow() &&
             this->GetVisualCol()>c.GetVisualCol())
           ) 
            return true;
        else
            return false;
    }


    void GetPosition(int &p_row, int &p_visual_col) const;
    void SetSelect(void);
    inline
    void SetSelect(const int p_row, const int p_col, const int p_select_keep_space)
    {
        m_select_row = p_row;
        m_select_col = p_col;
        m_select_keep_space = p_select_keep_space;
    }
    void ClearSelect(void);
    void GetSelectPos(int &p_select_srow, int &p_select_scol, int &p_select_erow, int &p_select_ecol) const;
    inline
    int GetSelectKeepSpace(void)
    {
        return m_select_keep_space;
    }

    inline
    bool HasSelect(void) const
    {
        if (m_select_row>=0 && (m_select_row!=m_row || m_select_col!=m_visual_col))
            return true;
        else
            return false;
    }

    inline
    int GetSelectRow(void)
    {
        return m_select_row;
    }

    inline
    int GetSelectCol(void)
    {
        return m_select_col;
    }



    char SelectType(void);
    bool IsOverlaped(const svCaret p_target);
    void MergeOverlap(svCaret &p_target);

    // inline
    // bool IsFirst(void)
    // {
    //     return m_is_first;
    // }

    // inline
    // void SetFirst(const bool p_first)
    // {
    //     m_is_first = p_first;
    // }

    // inline
    // bool IsLast(void)
    // {
    //     return m_is_last;
    // }

    // inline
    // void SetLast(const bool p_last)
    // {
    //     m_is_last = p_last;
    // }

private:
    // m_row, m_visual_col 是未折行的位置
    // m_keep_space 是紀錄折行時的column等同的space數量
    int m_row;
    // int m_keep_col; Remove m_keep_col 20150423 useless, replace by m_keep_space
    /*
     * m_keep_col 是用以紀錄 caret 在第幾個字元
     * m_visual_col 則是 caret 要顯示在第幾個字元
     * m_visual_col 在一般的情況下等於 m_keep_col
     * 但是當 caret 由較長的行向上(下)移動至較短行時，
     * m_keep_col 的長度可能會較該行的長度為長，
     * 這時的m_visual_col 會存放實際可顯示的最大長度
     * 這是兩者不相同的情境。
     *
     * 或者當折行內縮時，有可能這時的m_visual_col 會
     * 小於 m_keep_col
     *
     * 當編輯時，請以 m_row, m_visual_col 這組資料為主
     *
     */ 
    int m_visual_col;
    int m_keep_space;  // m_keep_col 等於幾個space(s) for tab and wrapped lin indent.

    int m_select_row;        // 游標標記時的啟始 row 值
    int m_select_col;        // 游標標記時的啟始 col 值
    int m_select_keep_space; // 游標標記時的 m_keep_space

    // bool m_is_first;
    // bool m_is_last;
};

inline
bool operator==(const svCaret &c1, const svCaret &c2)
{
    if (c1.GetRow()==c2.GetRow() &&
        c1.GetVisualCol()==c2.GetVisualCol())
        return true;
    else
        return false;
}

inline
bool operator<(const svCaret &c1, const svCaret &c2)
{
    if ( c1.GetRow()<c2.GetRow() ||
        (c1.GetRow()==c2.GetRow() &&
         c1.GetVisualCol()<c2.GetVisualCol())
       ) 
        return true;
    else
        return false;
}

inline
bool operator>(const svCaret &c1, const svCaret &c2)
{
    if ( c1.GetRow()>c2.GetRow() ||
        (c1.GetRow()==c2.GetRow() &&
         c1.GetVisualCol()>c2.GetVisualCol())
       ) 
        return true;
    else
        return false;
}


// svCaret stored in svCaretList in order of it's location.
class svCaretList
{
    friend class svBufText;

public:
    svCaretList();
    // svCaretList(svBufText *p_bufText);
    ~svCaretList();

    inline
    unsigned int Size()
    {
        return m_caretsList.size();
    }

    void Append(const svCaret &p_caret);
    void KeepFirstAndClearElse(void);
    void KeepLastAndClearElse(void);
    svCaret *At(const int idx);
    svCaret *GetLastCaret(void);
    // vector<svCaret> *GetCaretsInRange(const size_t p_sr, const size_t p_sc, const size_t p_er, const size_t p_ec);
    bool GetCaretsOfRange(const int32_t p_sr, const int32_t p_sc, const int32_t p_er, const int32_t p_ec, vector<svCaret> **p_inCaretList, vector<svCaret> **p_outCaretList);
    void RemoveDuplication(void);
    void DumpCarets(void);
    bool MergeOverlap(void);
    bool HasSelect(void);
    inline
    void Clear(void)
    {
        m_caretsList.clear();
        m_first_index = 0;
        m_last_index = 0;
    }
    bool IsOverlaped(const svCaret p_target);

    inline 
    int GetFirstIndex(void) const
    {
        return m_first_index;
    }

    inline 
    int GetLastIndex(void) const
    {
        return m_last_index;
    }

    inline 
    void SetFirstIndex(const int p_first_index)
    {
        m_first_index = p_first_index;
    }

    inline 
    void SetLastIndex(const int p_last_index)
    {
        m_last_index = p_last_index;
    }


private:
    vector<svCaret> m_caretsList;
    int m_first_index;
    int m_last_index;
    // svBufText *m_bufText;

};

typedef struct carets_info
{
    vector<svCaret> caretsList;
    int first_index;
    int last_index;

    ~carets_info()
    {
        caretsList.clear();
    }
} carets_info;

#endif
