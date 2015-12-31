/*
   Copyright Notice in awvic.cpp
*/
 
#include "svLineText.h"
#include <wx/tokenzr.h>
#include <wx/dcmemory.h>
#include "svCommonLib.h"
#include "svBufText.h"
// #include "svRELib.h"

#include "stdwx.h"

#define CONV_BUFFERSIZE 1024 // 1kb

svLineText::svLineText()
{
    m_parent = NULL;
    // m_text = wxT("");  // Maybe a stupid and slow way.
    m_text = NULL;
    // m_text = NULL;

    m_buffer = NULL;
    m_bufferLen = 0;
    m_bufferCRLFType = SVID_NEWLINE_NONE;
    m_bufferCRLFLen = 0;
    m_converted = false;

    m_ICUbuffer = NULL;
    m_ICUbufferLen = 0;
    m_ICUconverted = false;

    m_updated = true;
    m_wrapped = false;
    m_styled = false;
    m_wrapStyled = false;

    m_visible = true;

    m_indentPixelLen = 0;
    m_indentCharLen = 0;

}

/*
svLineText::svLineText(const wxString& txt)
{
    m_text = txt;
    m_updated = true;
    m_buffer = NULL;

    m_updated=true;
    m_wrapped=false;
    m_styled=false;
    m_wrapStyled=false;

}
*/

svLineText::svLineText(svBufText *p_parent, unsigned char* p_buf, size_t p_bufLen, int p_crlfType, size_t p_crlfLen)
{
    m_parent = p_parent;
    // m_text = wxT("");  // Maybe a stupid and slow way.
    m_text = NULL;
    // m_text = NULL;

    // m_buffer = p_buf;
    m_buffer = (unsigned char *)malloc(p_bufLen*sizeof(unsigned char));
    memcpy(m_buffer, p_buf, p_bufLen);
    m_bufferLen = p_bufLen;
    m_bufferCRLFType = p_crlfType;
    m_bufferCRLFLen = p_crlfLen;
    m_converted = false;

    m_ICUbuffer = NULL;
    m_ICUbufferLen = 0;
    m_ICUconverted = false;

    m_updated = true;
    m_wrapped = false;
    m_styled = false;
    m_wrapStyled = false;

    m_visible = true;

    m_indentPixelLen = 0;
    m_indentCharLen = 0;

}

// Because of class svLineText within a pointer dynamic allocate memory.
// We have to write a copy constructor to avoid strange bugs when push_back to a vector.
svLineText::svLineText(const svLineText& obj)
{
    m_parent = obj.m_parent;
    if (obj.m_buffer)
    {
        // if (m_buffer) free(m_buffer); crash!!
        m_buffer = (unsigned char *)malloc(obj.m_bufferLen*sizeof(unsigned char));
        memcpy(m_buffer, obj.m_buffer, obj.m_bufferLen);
    }
    else
    {
        m_buffer = NULL;
    }
    m_bufferLen = obj.m_bufferLen;
    m_bufferCRLFType = obj.m_bufferCRLFType;
    m_bufferCRLFLen = obj.m_bufferCRLFLen;
    m_converted = obj.m_converted;

    if (obj.m_ICUbuffer)
    {
        // if (m_ICUbuffer) free(m_ICUbuffer); crash!!
        m_ICUbuffer = (UChar *)malloc(obj.m_ICUbufferLen*sizeof(UChar));
        memcpy(m_ICUbuffer, obj.m_ICUbuffer, obj.m_ICUbufferLen*sizeof(UChar));
    }
    else
    {
        m_ICUbuffer = NULL;
    }
    m_ICUbufferLen = obj.m_ICUbufferLen;
    m_ICUconverted = obj.m_ICUconverted;

    if (obj.m_text)
    {
        m_text = new wxString(*obj.m_text);
    }
    else
    {
        m_text = NULL;
    }

    m_charWidList.Clear();
    m_charWidList = obj.m_charWidList;    // using default copy constructor

    m_charPixelWidList.Clear();
    m_charPixelWidList = obj.m_charPixelWidList; // using default copy constructor

    m_styleList.Clear();
    m_styleList = obj.m_styleList; // using default copy constructor

    m_wrapLenList.clear();
    m_wrapLenList = obj.m_wrapLenList;

    for (std::vector<vector<styledWrapText> >::iterator it=m_listOfStyledWrapTextList.begin();
         it!=m_listOfStyledWrapTextList.end();
         ++it)
    {
        it->clear();
    }
    m_listOfStyledWrapTextList = obj.m_listOfStyledWrapTextList;
 
    m_indentPixelLen = obj.m_indentPixelLen;
    m_indentCharLen = obj.m_indentCharLen;

    m_updated=obj.m_updated;
    m_wrapped=obj.m_wrapped;
    m_styled=obj.m_styled;
    m_wrapStyled=obj.m_wrapStyled;
    m_visible = obj.m_visible;

    m_keywordList.clear();
    m_keywordList = obj.m_keywordList;

    m_blockTagList.clear();
    m_blockTagList = obj.m_blockTagList;

    m_hintList.clear();
    m_hintList = obj.m_hintList;

    m_folding.clear();
    m_folding = obj.m_folding;
}

// Because of class svLineText within a pointer dynamic allocate memory.
// We have to override assign operator to avoid strange bugs when push_back to a vector.
// operator= 要記得 free 或 delete pointer 之前所分配候記憶體(copy construture 卻不用!!)
// 當 class 是以 pointer + new 的方式定義時，operator= 是有被呼叫的
// 例： svLineText *l = new svLineText();
// 實際上是呼叫 svLineText::svLineText() constructure 再呼叫 svLineText& svLineText::operator=(const svLineText& obj) 才得到 l 的資料 <== windows 下 Visual C++ 的行為，linux 未確認。
svLineText& svLineText::operator=(const svLineText& obj)
{
    m_parent = obj.m_parent;
    if (m_buffer) free(m_buffer);
    if (obj.m_buffer)
    {
        m_buffer = (unsigned char *)malloc(obj.m_bufferLen*sizeof(unsigned char));
        memcpy(m_buffer, obj.m_buffer, obj.m_bufferLen);
    }
    else
    {
        m_buffer = NULL;
    }
    m_bufferLen = obj.m_bufferLen;
    m_bufferCRLFType = obj.m_bufferCRLFType;
    m_bufferCRLFLen = obj.m_bufferCRLFLen;
    m_converted = obj.m_converted;

    if (m_ICUbuffer) free(m_ICUbuffer);
    if (obj.m_ICUbuffer)
    {
        m_ICUbuffer = (UChar *)malloc(obj.m_ICUbufferLen*sizeof(UChar));
        memcpy(m_ICUbuffer, obj.m_ICUbuffer, obj.m_ICUbufferLen*sizeof(UChar));
    }
    else
    {
        m_ICUbuffer = NULL;
    }
    m_ICUbufferLen = obj.m_ICUbufferLen;
    m_ICUconverted = obj.m_ICUconverted;

    if (m_text) delete(m_text);
    if (obj.m_text)
    {
        m_text = new wxString(*obj.m_text);
    }
    else
    {
        m_text = NULL;
    }

    m_charWidList.Clear();
    m_charWidList = obj.m_charWidList;    // using default copy constructor

    m_charPixelWidList.Clear();
    m_charPixelWidList = obj.m_charPixelWidList; // using default copy constructor

    m_styleList.Clear();
    m_styleList = obj.m_styleList; // using default copy constructor

    m_wrapLenList.clear();
    m_wrapLenList = obj.m_wrapLenList;

    for (std::vector<vector<styledWrapText> >::iterator it=m_listOfStyledWrapTextList.begin();
         it!=m_listOfStyledWrapTextList.end();
         ++it)
    {
        it->clear();
    }
    m_listOfStyledWrapTextList = obj.m_listOfStyledWrapTextList;

    m_indentPixelLen = obj.m_indentPixelLen;
    m_indentCharLen = obj.m_indentCharLen;
 
    m_updated=obj.m_updated;
    m_wrapped=obj.m_wrapped;
    m_styled=obj.m_styled;
    m_wrapStyled=obj.m_wrapStyled;
    m_visible = obj.m_visible;

    m_keywordList.clear();
    m_keywordList = obj.m_keywordList;

    m_blockTagList.clear();
    m_blockTagList = obj.m_blockTagList;

    m_hintList.clear();
    m_hintList = obj.m_hintList;

    m_folding.clear();
    m_folding = obj.m_folding;

    return *this;
}

// 僅複製屬性
// Only copy some attribute.
void svLineText::CopyAttribute(const svLineText& obj)
{
    m_parent = obj.m_parent;
    m_bufferLen = obj.m_bufferLen;
    m_bufferCRLFType = obj.m_bufferCRLFType;
    m_bufferCRLFLen = obj.m_bufferCRLFLen;
    m_converted = obj.m_converted;

    m_ICUbufferLen = obj.m_ICUbufferLen;
    m_ICUconverted = obj.m_ICUconverted;

    m_indentPixelLen = obj.m_indentPixelLen;
    m_indentCharLen = obj.m_indentCharLen;

    m_updated=obj.m_updated;
    m_wrapped=obj.m_wrapped;
    m_styled=obj.m_styled;
    m_wrapStyled=obj.m_wrapStyled;
    m_visible = obj.m_visible;
}

svLineText::~svLineText()
{
    if (m_buffer)
    { 
        free(m_buffer);
        m_buffer = NULL;
        m_bufferLen = 0;
        m_bufferCRLFLen = 0;
    }

    if (m_ICUbuffer)
    { 
        free(m_ICUbuffer);
        m_ICUbuffer = NULL;
        m_ICUbufferLen = 0;
    }

    if (m_text)
    {
        delete m_text;
        m_text = NULL;
    }

    m_charWidList.Clear();
    m_charPixelWidList.Clear();
    m_styleList.Clear();

    m_wrapLenList.clear();
    for(size_t i=0; i<m_listOfStyledWrapTextList.size(); i++)
    {
        m_listOfStyledWrapTextList.at(i).clear();
    }
    m_listOfStyledWrapTextList.clear();
    m_keywordList.clear();
    m_blockTagList.clear();
    m_hintList.clear();

}

void svLineText::SetBuffer(unsigned char* p_buf, size_t p_bufLen, int p_crlfType, size_t p_crlfLen)
{
    if (m_text) delete m_text;
    m_text = NULL;

    if (m_buffer) free(m_buffer);
    m_buffer = (unsigned char *)malloc(p_bufLen*sizeof(unsigned char));
    memcpy(m_buffer, p_buf, p_bufLen);
    m_bufferLen = p_bufLen;
    m_bufferCRLFType = p_crlfType;
    m_bufferCRLFLen = p_crlfLen;
    // m_bufferCRLFLen = m_bufferCRLFLen;
    m_converted = false;

    TextUpdated();

    // m_visible = true;

    m_indentPixelLen = 0;
    m_indentCharLen = 0;    

    m_charWidList.Clear();
    m_charPixelWidList.Clear();
    m_styleList.Clear();

    m_wrapLenList.clear();
    for(size_t i=0; i<m_listOfStyledWrapTextList.size(); i++)
    {
        m_listOfStyledWrapTextList.at(i).clear();
    }
    m_listOfStyledWrapTextList.clear();
    m_keywordList.clear();
    m_blockTagList.clear();
    m_hintList.clear();
}

bool svLineText::ConvertBuffer2String(const wxString& p_charSet)
{
// #ifndef NDEBUG
//     wxLogMessage("svLineText::ConvertBuffer2String start");
// #endif
    
    if (!m_converted)
    {
        if (m_buffer)
        {
            if (m_bufferLen-m_bufferCRLFLen>0)  // It should be >= 0.
            {
                if (m_text) delete m_text;
                m_text = new wxString(m_buffer, wxCSConv(p_charSet), m_bufferLen-m_bufferCRLFLen);
            }
            else
            {
                if (m_text) delete m_text;
                m_text = new wxString(wxT(""));
            }

            // wxString hexString;
            // for (int i=0; i<(int)m_bufferLen; i++)
            // {
            //     hexString += wxString::Format("%lx ", m_buffer[i]);
            // }
            // m_text = new wxString(hexString);
        }
        else   // m_buffer is NULL means a empty line.
        {
            if (m_text) delete m_text;
            m_text = new wxString(wxT(""));
        }
    }
    m_converted = true;

// #ifndef NDEBUG
//     wxLogMessage("svLineText::ConvertBuffer2String end");
// #endif

    return true;
}

/*
 * ICU Convert from char* to UChar* for encoding=>m_CScharSet
 * m_buffer is unsigned char* and ICU convert need char*
 * We only cast it, and don't know if it will works in every situation.
 */ 
UErrorCode svLineText::ConvertBufferICU(const char *p_encoding)
{
// #ifndef NDEBUG
//     wxLogMessage("svLineText::ConvertBufferICU start");
// #endif
    //static char inBuf[CONV_BUFFERSIZE];
    const char *source;
    const char *sourceLimit;
    UChar *uBuf;
    UChar *target;
    UChar *targetLimit;
    int32_t uBufSize = 0;
    UConverter *conv = NULL;
    UErrorCode status = U_ZERO_ERROR;
    uint32_t total=0;


    conv = ucnv_open( p_encoding, &status);
    assert(U_SUCCESS(status));

    // ************************* start convert *****************************

    uBufSize = (m_bufferLen/ucnv_getMinCharSize(conv));
    // printf("input bytes %d / min chars %d = %d UChars\n",
    //         isize, ucnv_getMinCharSize(conv), uBufSize);
    uBuf = (UChar*)malloc(uBufSize * sizeof(UChar));

    assert(uBuf!=NULL);

    target = uBuf;
    targetLimit = uBuf + uBufSize;
    source = (char*)m_buffer;
    sourceLimit = (char*)(m_buffer + (m_bufferLen));

    ucnv_toUnicode( conv, &target, targetLimit,
            &source, sourceLimit, NULL,
            true,           /* pass 'flush' when eof */
            /* is true (when no more data will come) */
            &status);

    // ucnv_fromUChars : The ICU function which convert UChar into other encoding string.

    if (status == U_BUFFER_OVERFLOW_ERROR)
    {
        // simply ran out of space - we'll reset the target ptr the next
        // time through the loop.                
        status = U_ZERO_ERROR;
    }
    else
    {
        // Check other errors here.
        assert(U_SUCCESS(status));
        // Break out of the loop (by force)
    }

    total = target - uBuf;
    m_ICUbufferLen = total;

    ucnv_close(conv);

    if (m_ICUbuffer) free(m_ICUbuffer);
    m_ICUbuffer = uBuf;
    m_ICUconverted = true;

// #ifndef NDEBUG
//     wxLogMessage("svLineText::ConvertBufferICU end");
// #endif

    return U_ZERO_ERROR;
}

// 重設該行的 換行符號 類別
// 呼叫本函式後，如有需要請呼叫 
//              GetLine(n)->ConvertString2Buffer(m_charSet);
//              GetLine(n)->ConvertBufferICU(m_CScharSet);
//              CreateKeywordTableAt(n);
//              以更新 m_buffer 及 m_ICUbuffer 及同步 m_bufferLen, m_ICUbufferLen
void svLineText::SetNewLineType(const int p_CRLFType, const size_t p_unitSize)
{

    int ori_bufferCRLFType = m_bufferCRLFType;
    size_t ori_bufferCRLFLen = m_bufferCRLFLen;

    m_bufferCRLFType = p_CRLFType;

    switch (m_bufferCRLFType)
    {
        case SVID_NEWLINE_CRLF:
            m_bufferCRLFLen = p_unitSize * 2;
            break;
        case SVID_NEWLINE_CR:
            m_bufferCRLFLen = p_unitSize * 1;
            break;
        case SVID_NEWLINE_LF:
            m_bufferCRLFLen = p_unitSize * 1;
            break;
        case SVID_NEWLINE_NONE:
            m_bufferCRLFLen = 0;
            break;
        default:
            m_bufferCRLFLen = p_unitSize * 1;
    }

}

// Convert wxString into Char* under it's original encoding.
// 要注意不是所有轉碼皆能成功
// 目前測 Big5 UTF-8 utf-16le utf-16be utf-32le utf-32be 可輸入中文字並轉為 m_bufffer 成功
bool svLineText::ConvertString2Buffer(const wxString& p_charSet)
{
    // SVID_NEWLINE_CRLF=0,
    // SVID_NEWLINE_CR=1,
    // SVID_NEWLINE_LF=2,
    // SVID_NEWLINE_NONE=3

    unsigned char newline_crlf_1bytes[] = "\x0D\x0A";      // \r\n = cr lf
    unsigned char newline_cr_1bytes[] = "\x0D";            // \r = cr
    unsigned char newline_lf_1bytes[] = "\x0A";            // \n = lf

    unsigned char newline_crlf_2bytes_be[] = "\x00\x0D\x00\x0A";  // big endian \r\n = cr lf
    unsigned char newline_cr_2bytes_be[] = "\x00\x0D";            // big endian \r = cr
    unsigned char newline_lf_2bytes_be[] = "\x00\x0A";            // big endian \n = lf

    unsigned char newline_crlf_2bytes_le[] = "\x0D\x00\x0A\x00";  // little endian \r\n = cr lf
    unsigned char newline_cr_2bytes_le[] = "\x0D\x00";            // little endian \r = cr
    unsigned char newline_lf_2bytes_le[] = "\x0A\x00";            // little endian \n = lf

    unsigned char newline_crlf_4bytes_be[] = "\x00\x00\x00\x0D\x00\x00\x00\x0A";  // big endian \r\n = cr lf
    unsigned char newline_cr_4bytes_be[] = "\x00\x00\x00\x0D";                    // big endian \r = cr
    unsigned char newline_lf_4bytes_be[] = "\x00\x00\x00\x0A";                    // big endian \n = lf

    unsigned char newline_crlf_4bytes_le[] = "\x0D\x00\x00\x00\x0A\x00\x00\x00";  // little endian \r\n = cr lf
    unsigned char newline_cr_4bytes_le[] = "\x0D\x00\x00\x00";                    // little endian \r = cr
    unsigned char newline_lf_4bytes_le[] = "\x0A\x00\x00\x00";                    // little endian \n = lf

    unsigned char *newline_crlf;     // \r\n = cr lf
    unsigned char *newline_cr;       // \r = cr
    unsigned char *newline_lf;       // \n = lf



    int endian = 0;
    int unit_size = 0;
    if (p_charSet == wxT("UTF-32BE"))
    {
        unit_size = 4;
        endian=SVID_BIG_ENDIAN;
    }
    else if (p_charSet == wxT("UTF-32LE"))
    {
        unit_size = 4;
        endian=SVID_LITTLE_ENDIAN;
    }
    else if (p_charSet == wxT("UTF-16BE"))
    {
        unit_size = 2;
        endian=SVID_BIG_ENDIAN;
    }
    else if (p_charSet == wxT("UTF-16LE"))
    {
        unit_size = 2;
        endian=SVID_LITTLE_ENDIAN;
    }
    else  // UTF8 ASCII etc.
    {
        unit_size = 1;
    }

    if (unit_size==1)
    {
        newline_crlf = newline_crlf_1bytes;
        newline_cr = newline_cr_1bytes;
        newline_lf = newline_lf_1bytes;
    }
    else if (unit_size==2 && endian==SVID_BIG_ENDIAN)
    {
        newline_crlf = newline_crlf_2bytes_be;
        newline_cr = newline_cr_2bytes_be;
        newline_lf = newline_lf_2bytes_be;
    }
    else if (unit_size==2 && endian==SVID_LITTLE_ENDIAN)
    {
        newline_crlf = newline_crlf_2bytes_le;
        newline_cr = newline_cr_2bytes_le;
        newline_lf = newline_lf_2bytes_le;
    }
    else if (unit_size==4 && endian==SVID_BIG_ENDIAN)
    {
        newline_crlf = newline_crlf_4bytes_be;
        newline_cr = newline_cr_4bytes_be;
        newline_lf = newline_lf_4bytes_be;
    }
    else if (unit_size==4 && endian==SVID_LITTLE_ENDIAN)
    {
        newline_crlf = newline_crlf_4bytes_le;
        newline_cr = newline_cr_4bytes_le;
        newline_lf = newline_lf_4bytes_le;
    }
    else
    {
        unit_size=1;
        newline_crlf = newline_crlf_1bytes;
        newline_cr = newline_cr_1bytes;
        newline_lf = newline_lf_1bytes;
    }


    if (m_buffer)
    {
        free(m_buffer);
        m_buffer = NULL;
    }

    // int p_bufLen = m_text->mb_str(wxConvUTF8).length(); // not including CR LF
    int p_bufLen = m_text->mb_str(wxCSConv(p_charSet)).length(); // not including CR LF

    if (m_bufferCRLFType==SVID_NEWLINE_CRLF)
    {
        // m_bufferLen = m_text->mb_str(wxConvUTF8).length() + newlineLen;
        m_bufferLen = m_text->mb_str(wxCSConv(p_charSet)).length() + (2*unit_size);
        m_buffer = (unsigned char *)malloc(m_bufferLen*sizeof(unsigned char));
        // memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxConvUTF8).data(), p_bufLen);
        memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxCSConv(p_charSet)).data(), p_bufLen);
        memcpy(m_buffer+p_bufLen, newline_crlf, (2*unit_size));
    }
    else if (m_bufferCRLFType==SVID_NEWLINE_CR)
    {
        // m_bufferLen = m_text->mb_str(wxConvUTF8).length() + 1;
        m_bufferLen = m_text->mb_str(wxCSConv(p_charSet)).length() + unit_size;
        m_buffer = (unsigned char *)malloc(m_bufferLen*sizeof(unsigned char));
        // memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxConvUTF8).data(), p_bufLen);
        memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxCSConv(p_charSet)).data(), p_bufLen);
        memcpy(m_buffer+p_bufLen, newline_cr, unit_size);
    }
    else if (m_bufferCRLFType == SVID_NEWLINE_LF)
    {
        // m_bufferLen = m_text->mb_str(wxConvUTF8).length() + 1;
        m_bufferLen = m_text->mb_str(wxCSConv(p_charSet)).length() + unit_size;
        m_buffer = (unsigned char *)malloc(m_bufferLen*sizeof(unsigned char));
        // memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxConvUTF8).data(), p_bufLen);
        memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxCSConv(p_charSet)).data(), p_bufLen);
        memcpy(m_buffer+p_bufLen, newline_lf, unit_size);
    }
    else
    {
        // m_bufferLen = m_text->mb_str(wxConvUTF8).length();
        m_bufferLen = m_text->mb_str(wxCSConv(p_charSet)).length();
        m_buffer = (unsigned char *)malloc(m_bufferLen*sizeof(unsigned char));
        // memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxConvUTF8).data(), p_bufLen);
        memcpy(m_buffer, (unsigned char *) m_text->mb_str(wxCSConv(p_charSet)).data(), p_bufLen);
    }

    m_converted = false;

    return true;
}


// Convert wxString into Char* under it's original encoding.
// 要注意不是所有轉碼皆能成功，所以當轉碼失敗時，回傳"?"
wxString svLineText::ConvertString2Buffer(const wxString &p_text, const wxString& p_charSet)
{
    if (!p_text.mb_str(wxCSConv(p_charSet)).length())
        return wxString("?");  // 轉碼失敗
    else
        return p_text;  // 轉碼成功

}

// 將 m_buffer, m_ICUbuffer, m_text 統一起來(三國一統)
void svLineText::UnifyBuffers(const wxString& p_charSet, const char *p_encoding, char p_convert_type)
{
    if (p_convert_type==SVID_CONV_FROM_TEXT)
    {
        ConvertString2Buffer(p_charSet);
        ConvertBufferICU(p_encoding);
    }
    else
    {
        ConvertBufferICU(p_encoding);
        ConvertBuffer2String(p_charSet);
    }
    m_updated = false;
}

/* ---------------------------------------------------------------------------- */

// 檢查 m_text 是否為 null
// bool svLineText::CheckTextUW(const wxString &p_charSet)
// {
//     if (!m_text)
//     {
//         ConvertBuffer2String(p_charSet);
//         return false;
//     }
//     else
//         return true;
// }

// Maybe Get the dirty data. Good luck!
// m_text 在一開始是只會產生畫面上的文字(含前後一頁) << 為了執行效率
// 所以非畫面上的行內的文字(m_text) 可能尚未產生而為 null，
// 所以多加了一個判斷，當是 null 時重轉一次文字，但有可能是 dirty data, please be careful.
wxString svLineText::GetTextUW(void)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    return *m_text;
}

void svLineText::SetTextUW(const wxString& txt)
{
    if (m_text) delete m_text;
    m_text = new wxString(txt);

    TextUpdated();
}

//  GetTextBeforeUW(3)
//  0 1 2 3 4 5 6 7 8
//  T H I S A T E S T
//  X X X
// 有可能是 dirty data, please be careful.
wxString svLineText::GetTextBeforeUW(int pos)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    return m_text->Left(pos);
}

//  GetTextAfterUW(3)
//  0 1 2 3 4 5 6 7 8
//  T H I S A T E S T
//        X X X X X X
// 有可能是 dirty data, please be careful.
wxString svLineText::GetTextAfterUW(int pos)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    int len = m_text->Length();
    return m_text->Right(len-pos);
}

// p_scol the start col of the string.
// p_len the length to be substring and return.
// 有可能是 dirty data, please be careful.
wxString svLineText::GetTextUW(int p_scol, int p_len)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    // int len = m_text->Length();
    return m_text->Mid(p_scol, p_len);
}

// DeleteLineTextAfter(n, 2)
//  0 1 2 3 4 5 6 7 8
//      x x x x x x x
wxString svLineText::DeleteTextAfterUW(int x)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    wxString txt;
    txt = m_text->Right(m_text->Length()-x);
    wxString* tmp_txt = new wxString(m_text->Left(x));
    delete m_text;
    m_text = tmp_txt;

    TextUpdated();

    return txt;
}

// DeleteLineTextBefore(n, 2)
//  0 1 2 3 4 5 6 7 8
//  x x x
wxString svLineText::DeleteTextBeforeUW(int x)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());

    wxString txt;
    txt = m_text->Left(x);
    // (*m_text) = m_text->Right(m_text->Length()-x);
    wxString* tmp_txt = new wxString(m_text->Right(m_text->Length()-x));
    delete m_text;
    m_text = tmp_txt;

    TextUpdated();

    return txt;
}


// DeleteLineText(n, 2, 6)
//  0 1 2 3 4 5 6 7 8
//      x x x x 
wxString svLineText::DeleteTextUW(int sx, int ex)
{
    if (!m_text)
        ConvertBuffer2String(m_parent->GetOriginalCharSet());
    
    wxString txt;
    txt = m_text->Mid(sx, ex-sx);
    // (*m_text) = m_text->Left(sx) + m_text->Right(m_text->Length()-ex);
    wxString* tmp_txt = new wxString(m_text->Left(sx) + m_text->Right(m_text->Length()-ex));
    delete m_text;
    m_text = tmp_txt;

    TextUpdated();

    return txt;
}

wxString svLineText::InsertTextUW(int sx, const wxString& txt)
{
    SetTextUW(GetTextBeforeUW(sx) + txt + GetTextAfterUW(sx));

    TextUpdated();

    return *m_text;
}

/*
 * 折行(line wrap)處理
 * 跟據 字型尺寸、tab的長度計算一行字串的長度 in pixel
 * 再跟據 p_screenWidth (最大寬度)將一行字串分為數個折行訊息並存在 m_wrapLenList
 * 每個字元的長度也是在此 function 計算並存到 m_charWidList
 */
void svLineText::ProcWrapLine(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    if (!m_updated) return;

    m_charWidList.Clear();

    // tab process: cal how much space 1 tab present
    // 計算一個tab要用多少個space表示
    // 同時計算第一個非space、非tab的字元位置。
    size_t sum = 0;

    bool firstNonSpace = false;
    size_t spaceCnt = 0;

    wxStringTokenizer st(*m_text, wxT("\t"), wxTOKEN_RET_EMPTY_ALL);
    while(st.HasMoreTokens())
    {
        wxString tkn = st.GetNextToken();
        wxString del = st.GetLastDelimiter();
        if (!tkn.IsEmpty()) // non [tab] character
        {
            size_t len = tkn.Length();
            for(size_t l=0; l<len; l++)
            {
                // m_charWidList.push_back(1);
                m_charWidList.Append(1);

                if (!firstNonSpace)
                {
                    if(tkn[l].GetValue()==wxT(' ') || tkn[l].GetValue()==wxT('\t'))
                    {
                        ++spaceCnt;
                    }
                    else
                    {
                        firstNonSpace = true;
                    }
                }
            }
            sum += len;
        }
        if (!del.IsEmpty() && del==wxT("\t")) // [tab]
        {
            size_t rmd = sum;
            while(rmd > p_tabWidth)
            {
                rmd -= p_tabWidth;
            }
            if (rmd < p_tabWidth)
            rmd = p_tabWidth - rmd;        
            sum += rmd;
            // m_charWidList.push_back(rmd);
            m_charWidList.Append(rmd);

            if (!firstNonSpace)
            {
                if(del==wxT(" ") || del==wxT("\t"))
                {
                    ++spaceCnt;
                }
                else
                {
                    firstNonSpace = true;
                }
            }            
        }
    }

    // character width by pixel process

    // m_charPixelWidList.clear();
    m_charPixelWidList.Clear();

    wxMemoryDC dc;
    wxSize s;
    wxBitmap bitmap(100, 100);
    dc.SelectObject(bitmap);
    dc.SetFont(p_font);
    //dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(wxBrush(*wxWHITE));
    dc.SetPen(wxPen(*wxBLACK, 1));

    for (size_t i=0; i<m_text->Length(); i++)
    {
        s = dc.GetTextExtent(m_text->Mid(i, 1));
        if (m_text->Mid(i, 1) == wxT("\t"))
        {
            // m_charPixelWidList.push_back(m_charWidList.at(i) * p_spacePixWidth);
            m_charPixelWidList.Append(m_charWidList.Get(i) * p_spacePixWidth);
        }
        else
            // m_charPixelWidList.push_back(s.GetWidth());
            m_charPixelWidList.Append(s.GetWidth());
    }


    // 計算該未折行的 indent Pixel Length
    m_indentPixelLen = 0;
    m_indentCharLen = 0;
    m_indentPixelLen = m_charPixelWidList.Sum(spaceCnt);
    m_indentCharLen = spaceCnt;


    // cal wrap information.
    // 開始計算折行資訊

    // For avoid bug when the indent pixel length greater than the p_screenWith.
    // Set p_screenWidth to SIZE_MAX when m_indentCharLen is too large. => that mean almost no wrapped.
    if (m_indentPixelLen>p_screenWidth)
    {
        // p_screenWidth = SIZE_MAX;
        m_indentPixelLen = m_indentCharLen = 0;
    }

    m_wrapLenList.clear();
    size_t sidx = 0;
    sum = 0;
    bool first = true;

    // if (m_charWidList.Size() > 0)
    //     sum = m_charWidList.Get(0);

    // for (size_t i=1; i<(size_t)m_charWidList.Size(); i++)
    // {
    //     // sum += m_charWidList.at(i);
    //     sum += m_charWidList.Get(i);
    //     if (sum>p_screenWidth)
    //     {
    //         // save sidx, i - sidx
    //         wrapLenDesc *w = new wrapLenDesc;
    //         w->idx = sidx;
    //         w->len = i - sidx;
    //         m_wrapLenList.push_back(*w);
    //         sidx = i;
    //         // sum = m_charWidList.at(i);
    //         sum = m_charWidList.Get(i);
    //     }
    // }

    // // if (sidx < m_charWidList.size())
    // if (sidx < (size_t)m_charWidList.Size())
    // {
    //     // save sdix, m_charWidList.Size - sidx
    //     wrapLenDesc *w = new wrapLenDesc;
    //     w->idx = sidx;
    //     // w->len = m_charWidList.size()-sidx;
    //     w->len = m_charWidList.Size()-sidx;
    //     m_wrapLenList.push_back(*w);
    // }

    if (m_charPixelWidList.Size() > 0)
    {

        if (p_isWrap)  // preference: wrap is true
        {
            sum = m_charPixelWidList.Get(0);
            
            for (size_t i=1; i<(size_t)m_charPixelWidList.Size(); i++)
            {
                // sum += m_charPixelWidList.at(i);
                sum += m_charPixelWidList.Get(i);
                if (sum>p_screenWidth)
                {
                    // save sidx, i - sidx
                    wrapLenDesc *w = new wrapLenDesc;
                    w->idx = sidx;
                    w->len = i - sidx;
                    m_wrapLenList.push_back(*w);
                    delete w;  w=NULL;
                    sidx = i;
                    // sum = m_charPixelWidList.Get(i);
                    sum = m_indentPixelLen + m_charPixelWidList.Get(i);
                }
            }

            // if (sidx < m_charPixelWidList.size())
            if (sidx < (size_t)m_charPixelWidList.Size())
            {
                // save sdix, m_charPixelWidList.Size - sidx
                wrapLenDesc *w = new wrapLenDesc;
                w->idx = sidx;
                // w->len = m_charPixelWidList.size()-sidx;
                w->len = m_charPixelWidList.Size()-sidx;
                m_wrapLenList.push_back(*w);
                delete w; w=NULL;
            }

        }
        else   // No Wrap
        {
            // text line only new line character.
            wrapLenDesc *w = new wrapLenDesc;
            w->idx = 0;
            w->len = m_charPixelWidList.Size();
            m_wrapLenList.push_back(*w);
            delete w; w=NULL;
        }
    }
    else
    {
        // text line only new line character.
        wrapLenDesc *w = new wrapLenDesc;
        w->idx = 0;
        // w->len = m_charPixelWidList.size()-sidx;
        w->len = 0;
        m_wrapLenList.push_back(*w);
        delete w; w=NULL;
    }

    m_wrapped = true;

}

size_t svLineText::GetWrapLineCount()
{
    return m_wrapLenList.size();
}

bool svLineText::GetWrapLenOnIdx(size_t p_idx, wrapLenDesc& p_wrapLenDesc)
{
    if (p_idx<0 || p_idx >= m_wrapLenList.size())
        return false;

    p_wrapLenDesc.idx = m_wrapLenList.at(p_idx).idx;
    p_wrapLenDesc.len = m_wrapLenList.at(p_idx).len;

    return true;

}

// 計算在第幾個折行內
// Giving a index and return it in which wrapped line.
int svLineText::InWhichWrappedLine(size_t p_idx)
{
    int wrappedNo = 0;
    int sum = 0;
    std::vector<wrapLenDesc>::iterator wit;
    for(wit=m_wrapLenList.begin();
        wit!=m_wrapLenList.end();
        ++wit)
    {
        if (p_idx>=(int)wit->idx && p_idx<(int)(wit->idx+wit->len))
        {
            return wrappedNo;
        }
        ++wrappedNo;
        sum = wit->idx+wit->len;
    }
    // In the end of the last line.
    if (p_idx==sum)
    {
        return wrappedNo-1;
    }
    // if (wit==m_wrapLenList.end())
    // {
    //     wrappedNo = -1;
    // }
    return -1;
}

// 計算在哪個折行，哪個欄位
// Giving a index and return it in which wrapped line, col.
bool svLineText::InWhichWrappedLineCol(size_t p_idx, int &p_wrapline, int &p_wrapcol)
{
    int sum = 0;
    p_wrapcol = p_idx;
    p_wrapline = 0;
    int last_len = 0;

    std::vector<wrapLenDesc>::iterator wit;
    for(wit=m_wrapLenList.begin();
        wit!=m_wrapLenList.end();
        ++wit)
    {
        if (p_idx>=(int)wit->idx && p_idx<(int)(wit->idx+wit->len))
        {
            return true;
        }
        ++p_wrapline;
        sum = wit->idx+wit->len;
        p_wrapcol -= wit->len;
        last_len = wit->len;
    }
    
    // In the end of the last line.
    if (p_idx==sum)
    {
        --p_wrapline;
        p_wrapcol = last_len;
        return true;
    }

    return false;
}

// 傳入哪個wrapped line，傳回它的啟始位置(characters column)
// Giving a wrapped line index and return it's start col position.
int svLineText::GetWrappedLineStartColOnIdx(size_t p_idx)
{
    if (p_idx >= GetWrapLineCount())
        return -1;

    return m_wrapLenList.at(p_idx).idx;
}

// 傳入哪個未折行的 character column number ，傳回它所在折行的啟始位置(characters column)
// Giving a column of unwrapped line and return it's wrapped line start col position.
int svLineText::GetWrappedLineStartColOnPosition(size_t p_idx)
{
    int wrappedLine = InWhichWrappedLine(p_idx);

    if (wrappedLine < 0) return -1;
    return GetWrappedLineStartColOnIdx(wrappedLine);
}

//  GetPixelLen(3, 4)
//  0 1 2 3 4 5 6 7 8 9 0
//        x x x x 
// 傳入哪個起啟字完位置及長度，回傳其pixel長度
int svLineText::GetTextPixelLen(size_t p_idx, size_t p_len)
{
    return m_charPixelWidList.SumByLen(p_idx, p_len);
}

/*
 * Syntax hilight 處理
 * 將一行字串跟據保留字等訊息分割為多個token
 * 再比對 svTextStyle 後將 styleName 等訊息存在 m_styleList
 */
/*void svLineText::ProcStyledText(svTextStyle* txtStyle)
{

    m_styleList.Clear();
    int acc_x = 0;

    // tab as default delimeter.
    wxStringTokenizer st(*m_text, txtStyle->GetDelimiter() + wxT("\t"), wxTOKEN_RET_EMPTY_ALL);
    while(st.HasMoreTokens())
    {
        wxString tkn = st.GetNextToken();
        wxString del = st.GetLastDelimiter();
        if (!tkn.IsEmpty())
        {
            txtStyleDesc* d = new txtStyleDesc;
            d->startPos = acc_x;
            d->len = tkn.Length();
            d->styleName = txtStyle->GetTextStyle(tkn)->GetName();
            m_styleList.Append(*d);
            acc_x+=d->len;
            delete d; d=NULL;
        }
        if (!del.IsEmpty())  
        {
            txtStyleDesc* d = new txtStyleDesc;
            d->startPos = acc_x;
            d->len = del.Length();
            d->styleName = txtStyle->GetTextStyle(del)->GetName();
            m_styleList.Append(*d);
            acc_x+=d->len;
            delete d; d=NULL;
        }
    }

    m_styled = true;
}*/

/*
 * Syntax hilight 處理
 * 將一行字串跟據keyword或pattern的regular expression處理後的資訊
 * 再將 styleName 等訊息存在 m_styleList
 */
void svLineText::ProcStyledText(vector<re_rule> *p_synReRules)
{

    m_styleList.Clear();
 
    int32_t keep_x = 0; // keep last processed end position.

    for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
    {
        // not REGEX matched text, Setting as default style.
        if(it->k_pos>(size_t)keep_x)
        {
           txtStyleDesc* d = new txtStyleDesc;
           d->startPos = keep_x;
           d->len = it->k_pos-keep_x;
           d->styleName = wxT("_DEFAULT");
           m_styleList.Append(*d);
           keep_x=d->startPos+d->len;
           delete d; d=NULL;
        }

        txtStyleDesc* dd = new txtStyleDesc;
        //dd->styleName = wxT("default");
        // Get ReRule name. the ReRule name is the same name in theme file.
        bool matched = false;
        for(std::vector<re_rule>::iterator rit=p_synReRules->begin(); rit!=p_synReRules->end(); ++rit)
        {
            // 因為是 vector<re_rule> iterator sequential match
            // 所以如果 block tag rules 不是先出現，則該 keyword 的 styleName 會被設為原本 matched 的那個規則。
            if (it->k_blocked_pid==rit->pid)      // 如果是位在 block tab 之間
            {
                dd->styleName = wxString(rit->name);
                matched = true;
                break;
            }
            else if (it->k_pid==rit->pid)
            {
                dd->styleName = wxString(rit->name);
                matched = true;
                break;
            }
        }
        if (!matched)
        {
            dd->styleName = wxT("_DEFAULT");
        }
        dd->startPos = it->k_pos;
        dd->len = it->k_len;
        m_styleList.Append(*dd);
        keep_x=it->k_pos+it->k_len;
        delete dd; dd=NULL; 

    }

/*    int32_t textLen = u_countChar32(m_ICUbuffer, u_strlen(m_ICUbuffer));
    if (m_bufferCRLFType==SVID_NEWLINE_CRLF)
    {
        textLen -= 2;
    }
    else if (m_bufferCRLFType==SVID_NEWLINE_CR||m_bufferCRLFType==SVID_NEWLINE_LF)
    {
        textLen -= 1;
    }*/
    int32_t textLen = (int32_t)TextLen(SVID_NO_CRLF);

    if (keep_x < textLen)
    {
        txtStyleDesc* d = new txtStyleDesc;
        d->startPos = keep_x;
        d->len = textLen-keep_x;
        d->styleName = wxT("_DEFAULT");
        m_styleList.Append(*d);
        keep_x=d->startPos+d->len;
        delete d; d=NULL; 
    }

    m_styled = true;
}


/*
 * 文字折行後可能會折行在 Syntax hilight 的 token 內
 * 這個 funtion 將被斷開的 token 分列為兩個(或多個token)
 * 最終的訊息儲存在 m_listOfStyledWrapTextList
 */
void svLineText::ProcStyledWrapLineText()
{

    for(size_t i=0; i<m_listOfStyledWrapTextList.size(); i++)
    {
        m_listOfStyledWrapTextList.at(i).clear();
    }
    m_listOfStyledWrapTextList.clear();

    for (size_t i=0; i<m_wrapLenList.size(); i++)
    {
        size_t spos = m_wrapLenList.at(i).idx;
        size_t slen = m_wrapLenList.at(i).len;
        vector<styledWrapText> *sl = new vector<styledWrapText>;
        size_t accumPixel = 0;

        for (int j=0; j<m_styleList.Size(); j++)
        {
            txtStyleDesc d = m_styleList.Get(j);
            if (d.startPos < spos && (d.startPos+d.len) <= spos)
            {
                // skip
            }
            else if (d.startPos < spos && (d.startPos+d.len) > spos && (d.startPos+d.len) <= spos+slen)
            {
                // split and insert bigger half
                styledWrapText *s = new styledWrapText;
                s->w_text = m_text->Mid(spos, d.len-(spos-d.startPos));
                s->w_styleName = d.styleName;
                s->w_pixelWidth = m_charPixelWidList.Sum(spos, spos + d.len-(spos-d.startPos));
                s->w_accumPixelWidth = accumPixel;
                sl->push_back(*s);
                accumPixel += s->w_pixelWidth;
                delete s; s=NULL;

                // txtStyleDesc *nd = new txtStyleDesc;
                // nd->startPos = spos;
                // nd->len = d.len-(spos-d.startPos);
                // nd->styleName = d.styleName;
                // sl->Append(*nd);
            }
            else if (d.startPos >= spos && (d.startPos+d.len) <= spos+slen)
            {
                // insert whole
                styledWrapText *s = new styledWrapText;
                s->w_text = m_text->Mid(d.startPos, d.len);
                s->w_styleName = d.styleName;
                s->w_pixelWidth = m_charPixelWidList.Sum(d.startPos, d.startPos + d.len);
                s->w_accumPixelWidth = accumPixel;
                sl->push_back(*s);
                accumPixel += s->w_pixelWidth;
                delete s; s=NULL;

                // txtStyleDesc *nd = new txtStyleDesc;
                // nd->startPos = d.startPos;
                // nd->len = d.len;
                // nd->styleName = d.styleName;
                // sl->Append(*nd);
            }
            else if (d.startPos >= spos && d.startPos < spos+slen && (d.startPos+d.len) > spos+slen)
            {
                // insert small half
                styledWrapText *s = new styledWrapText;
                s->w_text = m_text->Mid(d.startPos, d.len - ((d.startPos+d.len) - (spos+slen)));
                s->w_styleName = d.styleName;
                s->w_pixelWidth = m_charPixelWidList.Sum(d.startPos, d.startPos + d.len - ((d.startPos+d.len) - (spos+slen)));
                s->w_accumPixelWidth = accumPixel;
                sl->push_back(*s);
                accumPixel += s->w_pixelWidth;
                delete s; s=NULL;

                // txtStyleDesc *nd = new txtStyleDesc;
                // nd->startPos = d.startPos;
                // nd->len = d.len - ((d.startPos+d.len) - (spos+slen));
                // nd->styleName = d.styleName;
                // sl->Append(*nd);
            }
            else if (d.startPos < spos && (d.startPos+d.len) >= spos+slen)
            {
                // insert middle
                styledWrapText *s = new styledWrapText;
                s->w_text = m_text->Mid(spos, slen);
                s->w_styleName = d.styleName;
                s->w_pixelWidth = m_charPixelWidList.Sum(spos, spos + slen);
                s->w_accumPixelWidth = accumPixel;
                sl->push_back(*s);
                accumPixel += s->w_pixelWidth;
                delete s; s=NULL;

                // txtStyleDesc *nd = new txtStyleDesc;
                // nd->startPos = spos;
                // nd->len = slen;
                // nd->styleName = d.styleName;
                // sl->Append(*nd);
            }
            else if (d.startPos >= spos+slen)
                break;
        }

        // add m_indentPixelLen into accumPixel from the second wrapped line of the line
        // 將內縮像素長度加入第二個折行起的資料
        if (m_listOfStyledWrapTextList.size()>0)
        {
            for (std::vector<styledWrapText>::iterator it=sl->begin();
                 it!=sl->end();
                 ++it)
            {
                it->w_accumPixelWidth += m_indentPixelLen;
            }
        }

        m_listOfStyledWrapTextList.push_back(*sl);
        delete sl; sl=NULL;
        accumPixel = 0;

    }

    m_wrapStyled = true;

}

vector<vector<styledWrapText> >* svLineText::GetStyledWrapLineText()
{
    return &(m_listOfStyledWrapTextList);
}
    
vector<styledWrapText>* svLineText::GetStyledWrapLineText(size_t idx)
{
    if (idx >= m_wrapLenList.size()) return NULL;

    return &(m_listOfStyledWrapTextList.at(idx));

}

unsigned char svLineText::GetStyledWrapLineType(size_t idx)
{
    if (idx == m_listOfStyledWrapTextList.size())
        return SVID_LINE_END;
    else
        return SVID_LINE_WRAP;
}

bool svLineText::IsStyledWrapProcessed()
{
    return m_wrapped & m_styled & m_wrapStyled;
}

bool svLineText::PixelX2TextColUW(int p_pixelX, size_t& p_textCol)
{
    if (p_pixelX < 0) return false;

    p_textCol = m_charPixelWidList.DeSum(p_pixelX, true);

    return true;
}

// 不考慮 wrap line 的像素值
bool svLineText::TextCol2PixelUW(size_t p_textCol, int& p_pixelX)
{
    // if (p_textCol < 0 || p_textCol >= m_text->Length()) 2015/03/20
    if (p_textCol < 0 || p_textCol > m_text->Length())
        return false;

    p_pixelX = m_charPixelWidList.Sum(p_textCol);

    return true;
}

// 不考慮 wrap line 的像素值
bool svLineText::TextCol2PixelUW(size_t p_sCol, size_t p_eCol, int& p_pixelX)
{
    // if (p_textCol < 0 || p_textCol >= m_text->Length()) 2015/03/20
    if ( (p_sCol < 0 || p_sCol > m_text->Length()) ||
         (p_eCol < 0 || p_eCol > m_text->Length()) )
        return false;

    p_pixelX = m_charPixelWidList.Sum(p_eCol) - m_charPixelWidList.Sum(p_sCol);

    return true;
}

// 已考慮 wrap line 的像素值(折行內縮)
// p_sCol p_eCol 必須在同一折行否則計算結果會錯誤
// 為何需要兩個 col 值呢?
//
// 原字串 ABCDEFGHIJKLMNOPQR
//   位置 012345678901234567
//
// 折行後 ABCDEFGHI
//       012345678
//       JKLMNOPQR
//       901234567
// 則位置 9 可以是第一行的最後一個位置或第二行的第一個位置
//
// 所以 p_sCol 的功用為確定 p_eCol 是在哪一個折行，以便計算出正確的值
bool svLineText::TextCol2PixelW(size_t p_sCol, size_t p_eCol, int &p_pixelStart, int &p_pixelEnd)
{
    if ( (p_sCol < 0 || p_sCol > m_text->Length()) ||
         (p_eCol < 0 || p_eCol > m_text->Length()) )
        return false;

    int wsc = GetWrappedLineStartColOnPosition(p_sCol);  // 該折行第一個 col 值   

    p_pixelStart = m_charPixelWidList.Sum(p_sCol) - m_charPixelWidList.Sum(wsc);
    p_pixelEnd = m_charPixelWidList.Sum(p_eCol) - m_charPixelWidList.Sum(wsc);

    if (wsc>0)   // 0表示所在位置為第一行，>0表示所在位置折行
    {
        p_pixelStart += m_indentPixelLen;
        p_pixelEnd += m_indentPixelLen;
    }

    return true;
}

/*
 *
 * keyword processing functions.
 *
 */
bool svLineText::CreateKeywordTable(vector<re_rule> *p_synReRules)
{
    m_keywordList.clear();
    m_blockTagList.clear();
    m_hintList.clear();
    for (std::vector<re_rule>::iterator it=p_synReRules->begin(); it != p_synReRules->end(); ++it)
    {
        // CreateKeywordTablePhrase02(&*it);
        switch(it->type)
        {
            case 0:
                REMatchPattern_0(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup, it->cm1, it->cm2);
                // svRELib::ICU_RE_Pattern_0(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup, it->cm1, it->cm2);
                break;
            case 1:
                REMatchPattern_1(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup);
                // svRELib::ICU_RE_Pattern_1(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup);
                break;
            case 2:
                REMatchKeyword_2(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup);
                // svRELib::ICU_RE_Pattern_2(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup);
                break;
            case 3:
                REMatchKeyword_3(m_ICUbuffer, m_ICUbufferLen, it->re1, it->pid, it->regroup);
                break;                
        }
    }


    return true;
}

// bool svLineText::CreateKeywordTablePhrase02(const re_rule *p_re_rule)
// {
//     // 01. search not match range of text.
//     // 02. pattern match processing for the not match range text.

//     vector<keywordUnit> tmp_list;
//     size_t keep_x = 0;
//     for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
//     {
//         if (it->k_pos!=keep_x)
//         {
//             // insert a NULL keyword unit.
//             keywordUnit ku;
//             ku.k_text = NULL;
//             ku.k_pid = 0;
//             ku.k_pos = keep_x;
//             ku.k_len = it->k_pos-keep_x;
//             tmp_list.push_back(ku);
//         }
//         keep_x = it->k_pos+it->k_len;
//     }

//     if (TextLen(SVID_NO_CRLF)-keep_x>0)
//     {
//         // insert a NULL keyword unit.
//         keywordUnit ku;
//         ku.k_text = NULL;
//         ku.k_pid = 0;
//         ku.k_pos = keep_x;
//         ku.k_len = TextLen(SVID_NO_CRLF)-keep_x;
//         tmp_list.push_back(ku);
//     }



//     for(std::vector<keywordUnit>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it)
//     {
//         // m_ICUbuffer + it->k_pos 是不正確的，應該要算Unicode Code length 而非 UChar length.
//         switch(p_re_rule->type)
//         {
//             case 0:
//                 REMatchPattern_0(m_ICUbuffer+it->k_pos, it->k_len, p_re_rule->re1, p_re_rule->pid, p_re_rule->regroup, p_re_rule->cm1, p_re_rule->cm2);
//                 break;
//             case 1:
//                 REMatchPattern_1(m_ICUbuffer+it->k_pos, it->k_len, p_re_rule->re1, p_re_rule->pid, p_re_rule->regroup);
//                 break;
//             case 2:
//                 wxString ErrMsg = wxString::Format(wxT("k_pos=%i k_len=%i"), it->k_pos, it->k_len);
//                 wxLogMessage(ErrMsg);
//                 REMatchKeyword_2(m_ICUbuffer+it->k_pos, it->k_len, p_re_rule->re1, p_re_rule->pid, p_re_rule->regroup);
//                 break;
//         }
//     }

//     tmp_list.clear();

//     return true;

// }

// check if it's an overlapped keyword
bool svLineText::OverlapedKeyword(const keywordUnit *p_ku)
{
    if (m_keywordList.size()==0)
    {
        return false; // Should not happened.
    }
    else
    {
        for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
        {
            if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
            {
                return true;
            }
        }
    }
    return false;
}

// m_keywordList shoud be order by k_pos.
// Keyword should not be overlaped.
// keywordUnit 是對該行的 syntax hilight 分析，所以不能重疉，意即 每一個字只能出現在一個 keywordUnit 內
bool svLineText::InsertKeywordUnit(const keywordUnit* p_ku)
{
    if (m_keywordList.size()==0)
    {
        m_keywordList.push_back(*p_ku);
    }
    else
    {
        // 此處改為先出現的 keyword 覆蓋後出現的 keyword
        for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
        {
            if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // overlaped match keyword or pattern
            {
                return false;
            }
            else if ((p_ku->k_pos+p_ku->k_len>it->k_pos) && p_ku->k_pos+p_ku->k_len<=(it->k_pos+it->k_len)) // overlaped match keyword or pattern
            {
                remove_from_block_list(&*it);  // removing it from block list if it exist in that.
                *it = *p_ku;
                return true;
            }
            else if ((p_ku->k_pos<it->k_pos) && p_ku->k_pos+p_ku->k_len>(it->k_pos+it->k_len)) // overlaped match keyword or pattern. embrace original one. replace it.
            {
                remove_from_block_list(&*it);  // removing it from block list if it exist in that.
                *it = *p_ku;
                return true;
            }
            else if (p_ku->k_pos <= it->k_pos) // insert keyword.
            {
                m_keywordList.insert(it, *p_ku);
                return true;
            }
        }
        m_keywordList.push_back(*p_ku);
    }
    return true;
}

bool svLineText::remove_from_block_list(const keywordUnit *p_ku)
{

    for(std::vector<keywordUnit>::iterator it = m_blockTagList.begin();
        it != m_blockTagList.end();
        ++it)
    {
        if ( (p_ku->k_pos==it->k_pos) && 
             (p_ku->k_len==it->k_len) ) // && 
             // (p_ku->k_pid==it->k_pid) )
        {
            m_blockTagList.erase(it);
            return true;
        }
    }
    return false;

}

// m_hintList shoud be order by k_pos
bool svLineText::InsertHintUnit(const keywordUnit* p_ku)
{
    if (m_hintList.size()==0)
    {
        m_hintList.push_back(*p_ku);
    }
    else
    {
        for(std::vector<keywordUnit>::iterator it = m_hintList.begin(); it != m_hintList.end(); ++it)
        {
            if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
            {
                return false;
            }
            else if (p_ku->k_pos <= it->k_pos) // = should not happened.
            {
                m_hintList.insert(it, *p_ku);
                return true;
            }
        }
        m_hintList.push_back(*p_ku);
    }
    return true;
}

// m_keywordList shoud be order by k_pos and may not overlapped
bool svLineText::InsertBlockTagKeywordUnit(const keywordUnit* p_ku)
{
    if (m_blockTagList.size()==0)
    {
        m_blockTagList.push_back(*p_ku);
    }
    else
    {
        for(std::vector<keywordUnit>::iterator it = m_blockTagList.begin(); it != m_blockTagList.end(); ++it)
        {
            if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
            {
                return false;
            }
            else if (p_ku->k_pos <= it->k_pos) // = should not happened.
            {
                m_blockTagList.insert(it, *p_ku);
                return true;
            }
        }
        m_blockTagList.push_back(*p_ku);
    }
    return true;
}

// /*
//  * change keywordUnit's pid in specified range.
//  * 將某個區間內的keywordUnit的pid改變其pid值(block processing時呼叫本函數)
//  */
// bool svLineText::UpdateBlockKeyword(const size_t p_start_col, const size_t p_end_col, const int p_pid)
// {
//     /*
//      * 01. scan existed m_keywordList and record blanked keyword
//      * 02. restore blanked into m_keywordList
//      * 03. change keywordUnit's pid to p_pid if it's in the specified range.
//      * 01. 掃描現有 m_keywordList 並紀錄下keywordUnit間空白的區塊
//      * 02. 將步驟1空白的區塊當作keywordUnit寫回m_keywordList
//      * 03. 再將m_keywordList內在指定區間的keywordUnit的備pid設為p_pid
//      *
//      * 說明：這個函數是在作block處理時呼叫使用
//      * 因為block的ReRule只紀綠下開始及結束tag的位置，其間的文字並未加入m_keywordList內
//      * 所以需要再將開始及結束tag間的文字也加入m_keywordList內並將其pid設為一致
//      * 因為開始及結束tag間的文字可能符合其他的regular expression規則而已被加入m_keywordList
//      * 所以需要將未加入的部份先找出來，再回寫m_keywordList
//      * 再修改pid，以完成block處理的效果
//      */
//     vector<keywordUnit> tmp_list;
//     size_t keep_x = 0;
//     for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
//     {
//         if (it->k_pos!=keep_x && 
//             (keep_x>=p_start_col&&keep_x<=p_end_col))
//         {
//             // insert a NULL keyword unit.
//             keywordUnit ku;
//             ku.k_text = NULL;
//             ku.k_pid = p_pid;
//             ku.k_pos = keep_x;
//             ku.k_len = it->k_pos-keep_x;
//             ku.k_blocked_pid = SVID_NONE_BLOCKED;
//             tmp_list.push_back(ku);
//         }
//         keep_x = it->k_pos+it->k_len;
//     }

//     if ((int)p_end_col-(int)keep_x>0)  // unsigned minus unsigned always unsigned(always>0)
//     {
//         // insert a NULL keyword unit.
//         keywordUnit ku;
//         ku.k_text = NULL;
//         ku.k_pid = p_pid;
//         ku.k_pos = keep_x;
//         ku.k_len = p_end_col-keep_x;
//         ku.k_blocked_pid = SVID_NONE_BLOCKED;
//         tmp_list.push_back(ku);
//     }

//     for(std::vector<keywordUnit>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it)
//     {
//         InsertKeywordUnit(&*it);
//     }

//     tmp_list.clear();

//     for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
//     {
//         if ((it->k_pos>=p_start_col) && (it->k_pos<=p_end_col))
//         {
//             it->k_pid = p_pid;
//         }
//     }

//     return true;

// }

/*
 * change keywordUnit's pid in specified range.
 * 將某個區間內的keywordUnit的k_bloced_pid清空(SVID_NONE_BLOCKED)
 */
void svLineText::ClearBlockKeywordInRange(const size_t p_start_col, const size_t p_end_col, const int p_pid)
{
    /*
     * 01. change keywordUnit's pid to p_pid if it's in the specified range.
     * 01. 再將m_keywordList內在指定區間的keywordUnit的備k_blocked_pid設為SVID_NONE_BLOCKED
     *
     * 說明：這個函數是在作block處理時呼叫使用
     */
    for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
    {
        if ((it->k_pos>=p_start_col) && (it->k_pos<=p_end_col))
        {
            if (it->k_blocked_pid==p_pid)   // only clear the specified pid(the same)
                it->k_blocked_pid = SVID_NONE_BLOCKED;
        }
    }
}



/*
 * change keywordUnit's pid in specified range.
 * 將某個區間內的keywordUnit的k_bloced_pid改變其pid值(block processing時呼叫本函數)
 */
bool svLineText::UpdateBlockKeywordInRange(const size_t p_start_col, const size_t p_end_col, const int p_pid)
{
    /*
     * 01. scan existed m_keywordList and record blanked keyword
     * 02. restore blanked into m_keywordList
     * 03. change keywordUnit's pid to p_pid if it's in the specified range.
     * 01. 掃描現有 m_keywordList 並紀錄下keywordUnit間空白的區塊
     * 02. 將步驟1空白的區塊當作keywordUnit寫回m_keywordList
     * 03. 再將m_keywordList內在指定區間的keywordUnit的備pid設為p_pid
     *
     * 說明：這個函數是在作block處理時呼叫使用
     * 因為block的ReRule只紀綠下開始及結束tag的位置，其間的文字並未加入m_keywordList內
     * 所以需要再將開始及結束tag間的文字也加入m_keywordList內並將其pid設為一致
     * 因為開始及結束tag間的文字可能符合其他的regular expression規則而已被加入m_keywordList
     * 所以需要將未加入的部份先找出來，再回寫m_keywordList
     * 再修改pid，以完成block處理的效果
     */
    vector<keywordUnit> tmp_list;
    size_t keep_x = 0;
    for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
    {
        if (it->k_pos!=keep_x && 
            (keep_x>=p_start_col&&keep_x<=p_end_col))
        {
            // insert a NULL keyword unit.
            keywordUnit ku;
            ku.k_text = NULL;
            // ku.k_pid = p_pid;
            ku.k_pid = SVID_DEFAULT_PID;
            ku.k_pos = keep_x;
            ku.k_len = it->k_pos-keep_x;
            ku.k_blocked_pid = SVID_NONE_BLOCKED;
            tmp_list.push_back(ku);
        }
        keep_x = it->k_pos+it->k_len;
    }

    if ((int)p_end_col-(int)keep_x>0)  // unsigned minus unsigned always unsigned(always>0)
    {
        // insert a NULL keyword unit.
        keywordUnit ku;
        ku.k_text = NULL;
        // ku.k_pid = p_pid;
        ku.k_pid = SVID_DEFAULT_PID;
        ku.k_pos = keep_x;
        ku.k_len = p_end_col-keep_x;
        ku.k_blocked_pid = SVID_NONE_BLOCKED;
        tmp_list.push_back(ku);
    }

    for(std::vector<keywordUnit>::iterator it = tmp_list.begin(); it != tmp_list.end(); ++it)
    {
        InsertKeywordUnit(&*it);
    }

    tmp_list.clear();

    for(std::vector<keywordUnit>::iterator it = m_keywordList.begin(); it != m_keywordList.end(); ++it)
    {
        if ((it->k_pos>=p_start_col) && (it->k_pos<=p_end_col))
        {
            if (it->k_blocked_pid==SVID_NONE_BLOCKED)   // 考慮重覆的狀況
                it->k_blocked_pid = p_pid;
            // it->k_pid = p_pid;
        }
    }

    return true;

}

//
// Regular Expression matching specified pattern.
// for /* and */
//
// !! THE SAME PROCESSING RULES WITH svBufText::REMatchPattern_0 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svLineText::REMatchPattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end)
{

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    UChar *ustart = NULL;
    UChar *uend   = NULL;
    int32_t startLen = 0;
    int32_t endLen = 0;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    ustart = (UChar*) malloc (sizeof(UChar)*(strlen(start)+1));
    u_charsToUChars(start, ustart, strlen(start)+1);
    startLen = strlen(start);

    uend = (UChar*) malloc (sizeof(UChar)*(strlen(end)+1));
    u_charsToUChars(end, uend, strlen(end)+1);
    endLen = strlen(end);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize); coold
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE0 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)
        if (p_regroup<=gCnt)
        { 
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE0 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                // fprintf(stdout, "Regex Match(g=%d):", i);
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                ku.k_blocked_pid = SVID_NONE_BLOCKED;
                bool inserted = InsertKeywordUnit(&ku);
                // ku.k_text = NULL;

                // Insert into start/end tag into block tag vector.
                if (inserted)
                {
                    keywordUnit tku;
                    // tku.k_text = dest;
                    tku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                    u_memcpy(tku.k_text, dest, (sPos-s));
                    if (u_strncmp(dest, ustart, startLen)==0)
                    {
                        // start tag.
                        tku.k_pid = SVID_BLOCK_START;
                    }
                    else // if not /* then */
                    {
                        // end tag.
                        tku.k_pid = SVID_BLOCK_END;
                    }
                    tku.k_pos = s;
                    tku.k_len = sPos - s;
                    tku.k_blocked_pid = SVID_NONE_BLOCKED;
                    InsertBlockTagKeywordUnit(&tku);
                    // tku.k_text = NULL;
                }
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE0 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE0 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-07 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (ustart) free(ustart);
    if (uend) free(uend);

    return U_ZERO_ERROR;
}


/*
 * Regular Expression matching specified pattern.
 */
// !! THE SAME PROCESSING RULES WITH svBufText::REMatchPattern_1 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svLineText::REMatchPattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup)
{

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    const char* tab = "\t";
    UChar *utab = NULL;

    char const* space = " ";
    UChar *uspace = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    utab = (UChar*) malloc (sizeof(UChar)*(strlen(tab)+1));
    u_charsToUChars(tab, utab, strlen(tab)+1);
    int tablen = strlen(tab);

    uspace = (UChar*) malloc (sizeof(UChar)*(strlen(space)+1));
    u_charsToUChars(space, uspace, strlen(space)+1);
    int spacelen = strlen(space);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize); coold
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE1 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);        
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        //for (int32_t i=0; i<1; i++)
        // we only process the regroup specified.
        if (p_regroup<=gCnt)
        { 
            for(int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);        
                }
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE1 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg);
                }
                // fprintf(stdout, "Regex Match(g=%d):", i);
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);

                // pattern 1 have to consider \t

                int32_t old_x = 0;
                // int32_t textLen = u_countChar32(dest,  u_strlen(dest));
                int32_t textLen = sPos - s;
                for(int32_t i=0; i<textLen; i++)
                {
                    // break /t and space for symbol(tab space) display.
                    if (u_strncmp(dest+i, utab, 1)==0)
                    {
                        // break data
                        if (i!=old_x)
                        {
                            UChar* dst = NULL;
                            dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                            u_strncpy(dst, dest+old_x, i-old_x);
                            keywordUnit ku;
                            // ku.k_text = dst;
                            ku.k_text = (UChar *)malloc(sizeof(UChar) * (i-old_x));
                            u_memcpy(ku.k_text, dst, (i-old_x));
                            ku.k_pid = p_pid;      
                            ku.k_pos = s+old_x;
                            ku.k_len = i-old_x;
                            ku.k_blocked_pid = SVID_NONE_BLOCKED;
                            InsertKeywordUnit(&ku);
                            // ku.k_text = NULL;
                            free(dst);
                        }

                        // break /t
                        {
                        keywordUnit xku;
                        // xku.k_text = utab;
                        xku.k_text = (UChar *)malloc(sizeof(UChar) * tablen);
                        u_memcpy(xku.k_text, utab, tablen);
                        xku.k_pid = p_pid;      
                        xku.k_pos = s+i;
                        xku.k_len = 1;
                        xku.k_blocked_pid = SVID_NONE_BLOCKED;
                        InsertKeywordUnit(&xku);
                        // xku.k_text = NULL;
                        }

                        old_x = i+1;
                    }
                    else if (u_strncmp(dest+i, uspace, 1)==0)
                    {
                        // break data
                        if (i!=old_x)
                        {
                            UChar* dst = NULL;
                            dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                            u_strncpy(dst, dest+old_x, i-old_x);
                            keywordUnit ku;
                            // ku.k_text = dst;
                            ku.k_text = (UChar *)malloc(sizeof(UChar) * (i-old_x));
                            u_memcpy(ku.k_text, dst, (i-old_x));
                            ku.k_pid = p_pid;      
                            ku.k_pos = s+old_x;
                            ku.k_len = i-old_x;
                            ku.k_blocked_pid = SVID_NONE_BLOCKED;
                            InsertKeywordUnit(&ku);
                            // ku.k_text = NULL;
                            free(dst);
                        }

                        // break space
                        {
                        keywordUnit xku;
                        // xku.k_text = uspace;
                        xku.k_text = (UChar *)malloc(sizeof(UChar) * spacelen);
                        u_memcpy(xku.k_text, uspace, spacelen);
                        xku.k_pid = p_pid;      
                        xku.k_pos = s+i;
                        xku.k_len = 1;
                        xku.k_blocked_pid = SVID_NONE_BLOCKED;
                        InsertKeywordUnit(&xku);
                        // xku.k_text = NULL;
                        }

                        old_x = i+1;
                    }                       
                }

                // 句尾的處理
                // 要考慮是否包含 /r/n 的情況
                // regular expression rule 要有相對應的設定
                if (old_x!=textLen)
                {
                    int32_t i = textLen;
                    UChar* dst = NULL;
                    dst = (UChar *) malloc(sizeof(UChar)*(i-old_x));
                    u_strncpy(dst, dest+old_x, i-old_x);
                    keywordUnit ku;
                    // ku.k_text = dst;
                    ku.k_text = (UChar *)malloc(sizeof(UChar)*(i-old_x));
                    u_memcpy(ku.k_text, dst, (i-old_x));
                    ku.k_pid = p_pid;      
                    ku.k_pos = s+old_x;
                    ku.k_len = i-old_x;
                    ku.k_blocked_pid = SVID_NONE_BLOCKED;
                    InsertKeywordUnit(&ku);
                    // ku.k_text = NULL;
                    free(dst);
                }

                free(dest);
            }
        }
        else  // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE1 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE1 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }

    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-07 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (utab) free(utab);
    if (uspace) free(uspace);

    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// !! THE SAME PROCESSING RULES WITH svBufText::REMatchPattern_2 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svLineText::REMatchKeyword_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup)
{

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize);
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE2 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)   // We only care group 0 which is the complete match.
        if (p_regroup<=gCnt)
        { 
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                ku.k_blocked_pid = SVID_NONE_BLOCKED;
                InsertKeywordUnit(&ku);
                // ku.k_text = NULL;
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE2 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE2 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);  
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-07 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);

    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// !! THE SAME PROCESSING RULES WITH svBufText::REMatchPattern_3 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svLineText::REMatchKeyword_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup)
{

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    // fprintf(stdout, "RE Pattern:%s\n", pattern);
    upattern = (UChar*) malloc (sizeof(UChar)*(strlen(pattern)+1));
    u_charsToUChars(pattern, upattern, strlen(pattern)+1);

    // URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = uregex_open(upattern, -1, 0, &pe, &status); 
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-01 error(uregex_open):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize);
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-02 error(uregex_setText):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE3 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
        
        // for (int32_t i=0; i<gCnt; i++)
        // for (int32_t i=0; i<1; i++)   // We only care group 0 which is the complete match.
        if (p_regroup<=gCnt)
        { 
            for (int32_t i=p_regroup; i<=p_regroup; i++)
            {
                int32_t s = uregex_start(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-04 error(uregex_start):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                sPos = uregex_end(re, i, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-05 error(uregex_end):%i"), status);
                    wxLogMessage(ErrMsg);  
                }   
                UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
                uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                ku.k_blocked_pid = SVID_NONE_BLOCKED;
                // InsertKeywordUnit(&ku);
                InsertHintUnit(&ku);
                // ku.k_text = NULL;
                free(dest);
            }
        }
        else   // p_regroup>=gCnt
        {
            int32_t s = uregex_start(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE3 Seq-07 error(uregex_start):%i"), status);
                wxLogMessage(ErrMsg);  
            }   
            sPos = uregex_end(re, 0, &status);
            if (U_FAILURE(status))
            {
                // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
                wxString ErrMsg = wxString::Format(wxT("RE3 Seq-08 error(uregex_end):%i"), status);
                wxLogMessage(ErrMsg);
            }              
        }
    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-07 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);

    return U_ZERO_ERROR;
}


/*
 *
 * hint dictionary processing functions.
 *
 */
void svLineText::CreateHintDictionary(svDictionary &p_dict)
{
    if (!m_hintList.size()) return;

    for (std::vector<keywordUnit>::iterator it = m_hintList.begin();
         it != m_hintList.end();
         ++it)
    {
        p_dict.Add(it->k_text, it->k_len);
    }
}


void svLineText::SubHintDictionary(svDictionary &p_dict)
{
    if (!m_hintList.size()) return;

    for (std::vector<keywordUnit>::iterator it = m_hintList.begin();
         it != m_hintList.end();
         ++it)
    {
        p_dict.Sub(it->k_text, it->k_len);
    }
}

// return hint keyword py a giving position
// PLEASE free(p_text) AFTER CALL THIS FUNCTION!
// p_offset 該 hint 字串與 caret 位置的差距值, 0 表示 caret 位在 hint 最後一個字元
//
//  Example: caret between O and R
//
//    K E Y W O|R D
//
//    p_size = 7
//    p_offset = 2
bool svLineText::GetPositionHint(const int p_pos, UChar **p_text, int &p_size, int &p_offset)
{
    if (!m_hintList.size()) return false;

    for (std::vector<keywordUnit>::iterator it = m_hintList.begin();
         it != m_hintList.end();
         ++it)
    {
        if (p_pos>=(int)it->k_pos && p_pos<=(int)(it->k_pos+it->k_len))
        {
            p_size = it->k_len;
            p_offset = it->k_pos+it->k_len-p_pos;
            if (*p_text) free(*p_text);
            *p_text = (UChar *)malloc(sizeof(UChar) * it->k_len);
            u_memcpy(*p_text, it->k_text, it->k_len);
            return true;
        }
    }

    return false;
}

// return keyword start position and length by a giving position
//
//  Example: caret between O and R
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
//    W h e r e   i s   K E Y W O|R D .
//
//    p_cpos = 14
//    p_wpos = 9
//    p_wlen = 7
bool svLineText::GetKeywordPosition(const int p_cpos, int &p_wpos, int &p_wlen)
{
    for (std::vector<keywordUnit>::iterator it = m_hintList.begin();
         it != m_hintList.end();
         ++it)
    {
        if (p_cpos>=(int)it->k_pos && p_cpos<=(int)(it->k_pos+it->k_len))
        {
            p_wlen = it->k_len;
            p_wpos = it->k_pos;
            return true;
        }
    }

    return false;
}

// return keyword start position and length by a giving position and direction.
// direction = SVID_PREVIOUS return previous keyword information if p_cpos in the beginner of a Keyword.
// direction = SVID_NEXT return next keyword information if p_cpos in the end of a Keyword.
//
//  Example: caret between O and R
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
//    W h e r e   i s   K E Y W O|R D .
//
//    p_cpos = 14
//    p_direction = SVID_BACKWARD or SVID_FORWARD
//    p_wpos = 9
//    p_wlen = 7
//
//    p_cpos = 9
//    p_direction = SVID_BACKWARD
//    p_wpos = 6
//    p_wlen = 2

//    p_cpos = 8
//    p_direction = SVID_FORWARD
//    p_wpos = 9
//    p_wlen = 7

// return true means data founded, else means boundry meet.
bool svLineText::GetKeywordPosition(const int p_cpos, char p_direction, char p_type, int &p_wpos)
{
    vector<int> headPosList;
    vector<int> endPosList;

    // Storing position into vectors.
    for (std::vector<keywordUnit>::iterator it = m_hintList.begin();
         it != m_hintList.end();
         ++it)
    {
        headPosList.push_back(it->k_pos);
        endPosList.push_back(it->k_pos+it->k_len);
    }
    
    // blank line.
    if (headPosList.size()==0)
        headPosList.push_back(0);
    if (endPosList.size()==0)
        endPosList.push_back(TextLen(SVID_NO_CRLF));

   
    if (endPosList.back()<(int)TextLen(SVID_NO_CRLF))
    {
        endPosList.push_back(TextLen(SVID_NO_CRLF));
    }

    if (p_direction==SVID_FORWARD)
    {
        if (p_type==SVID_HEAD_OF_KEYWORD)
        {
            for (std::vector<int>::iterator it = headPosList.begin();
                 it != headPosList.end();
                 ++it)
            {
                if (*it>p_cpos)
                {
                    p_wpos = *it;
                    return true;
                }
            }
        }
        else if (p_type=SVID_TAIL_OF_KEYWORD)
        {
            for (std::vector<int>::iterator it = endPosList.begin();
                 it != endPosList.end();
                 ++it)
            {
                if (*it>p_cpos)
                {
                    p_wpos = *it;
                    return true;
                }
            }
        }
    }
    else if (p_direction==SVID_BACKWARD)
    {
        if (p_type==SVID_HEAD_OF_KEYWORD)
        {
            for (std::vector<int>::reverse_iterator it = headPosList.rbegin();
                 it != headPosList.rend();
                 ++it)
            {
                if (*it<p_cpos)
                {
                    p_wpos = *it;
                    return true;
                }
            }
        }
        else if (p_type=SVID_TAIL_OF_KEYWORD)
        {
            for (std::vector<int>::reverse_iterator it = endPosList.rbegin();
                 it != endPosList.rend();
                 ++it)
            {
                if (*it<p_cpos)
                {
                    p_wpos = *it;
                    return true;
                }
            }
        }
    }
    
    return false;
}

// return the text length by character counting. Including or not including CRLF.
size_t svLineText::TextLen(short int p_type)
{
    size_t len = 0;
    // 沒有\0結尾可能會出錯，要指定正確的長度
    // len = (size_t)u_countChar32(m_ICUbuffer, u_strlen(m_ICUbuffer));
    len = (size_t)u_countChar32(m_ICUbuffer, m_ICUbufferLen);

    if (p_type==SVID_NO_CRLF)
    {
        if (m_bufferCRLFType==SVID_NEWLINE_CRLF)
        {
            len -= 2;
        }
        else if (m_bufferCRLFType==SVID_NEWLINE_CR || 
                 m_bufferCRLFType==SVID_NEWLINE_LF)
        {
            len -= 1;
        }
    }

    return len;

}

/*bool svLineText::Visible(void)
{
    return m_visible;
}

void svLineText::Visible(const bool p_visible)
{
    m_visible = p_visible;
}
*/

// Giving a column return it's equilavant spaces number.
// Already consider the wrapped line indent and tab processing.
// 傳入第幾個折行，第幾個欄位(折行欄位)，回傳該欄位所在等同的space值
// 已考慮折行內縮及tab處理
int svLineText::Col2SpaceW(const int p_wrapline, const int p_col)
{

    // Assumming the ProcWrapAndStyle were already processed.
    // if (!IsStyledWrapProcessed())
    //     ProcWrapAndStyle(p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
    int wrapCount = m_wrapLenList.size();

    if (p_wrapline>=wrapCount || p_wrapline < 0) 
        return 0;


    size_t start = m_wrapLenList.at(p_wrapline).idx;
    size_t len = 0;
    int sum = 0;

    if (p_col > (int)m_wrapLenList.at(p_wrapline).len)
    {
        // end = m_wrapLenList.at(p_wrapline).idx + m_wrapLenList.at(p_wrapline).len;
        len = m_wrapLenList.at(p_wrapline).len;
    }
    else if (p_col < 0)
    {
        // end = m_wrapLenList.at(p_wrapline).idx;      
        len = 0;
    }
    else
    {
        // end = m_wrapLenList.at(p_wrapline).idx + p_col;
        len = p_col;
    }

    sum = m_charWidList.SumByLen(start, len);
    if (p_wrapline>0)
    {
        // sum += m_indentCharLen;
        sum += m_charWidList.SumByLen(0, m_indentCharLen);
    }

    return sum;
   
}

// Giving spaces value return it's equilavant column number.
// Already consider the wrapped line indent and tab processing.
// the return value is the unwrapped line column.
// 這個函數是在作畫面欄位與字串位置的轉換
// 因為如tab等的字元在畫面上可能是等於4個字元的寬度
// 以及折行內縮可能該字串在畫面上顯示時向右移動
// 所以考慮給定在第幾個折行、幾個space寬度值，傳回在該行字串的文字欄位(算字元)
// 傳入第幾個折行，等值的space，回傳所對應的欄位值
// 已考慮折行內縮及tab處理
// 回傳值是未折行的文字欄位值(字元數)
// 考慮折行問題折行的最後一行才會有可能回傳等同於該折行長度的值
int svLineText::Space2ColW(const int p_wrapline, const int p_space)
{

    // Assumming the ProcWrapAndStyle were already processed.
    // if (!IsStyledWrapProcessed())
    //     ProcWrapAndStyle(p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
    int wrapCount = m_wrapLenList.size();

    if (p_wrapline>=wrapCount || p_wrapline < 0) 
        return 0;

    size_t start = m_wrapLenList.at(p_wrapline).idx;
    size_t Maxlen = m_wrapLenList.at(p_wrapline).idx + m_wrapLenList.at(p_wrapline).len;
    int sum = p_space;

    if (p_wrapline>0)
    {
        // 扣除折行內縮值
        // sum -= m_indentCharLen;
        sum -= m_charWidList.SumByLen(0, m_indentCharLen);
    }

    // return m_charWidList.DeSum(sum, start, false);
    //return m_charWidList.DeSumLeft(sum, start);
    unsigned int col = m_charWidList.DeSumLeft(sum, start, Maxlen);

    if (p_wrapline==wrapCount-1)
    {
        // last wrapped line
    }
    else
    {
        // not last wrapped line
        // caret should not in the end of the line.
        // 只有折行的最後一行以可以回傳大於、等於該折行的字串長度
        // 因為游標不可停在折行的最後一個字元之後(因該停在下一個折行的第一個位置)
        if (col>=Maxlen)
        {
            col = Maxlen - 1;
        }
    }

    return col;
  
}

// ----------------------------------------------------------------------
// Find related functions
// ----------------------------------------------------------------------
// Find UChar string from m_hintList 
bool svLineText::FindNextKeywordUCharFrom(const UCharText &p_keyword, const int p_scol, int &p_fcol)
{
    for (std::vector<keywordUnit>::iterator it=m_hintList.begin();
         it!=m_hintList.end();
         ++it)
    {
        // if (it->k_pos+it->k_len<p_scol)
        if ((int)it->k_pos<p_scol)
            continue;

        if (it->k_len==p_keyword.len && u_memcmp(it->k_text, p_keyword.uctext, it->k_len)==0)
        {
            p_fcol = it->k_pos;
            return true;
        }
    }
    return false;
}

// Find UChar string from m_hintList
bool svLineText::FindPrevKeywordUCharFrom(const UCharText &p_keyword, const int p_scol, int &p_fcol)
{
    for (std::vector<keywordUnit>::reverse_iterator rit=m_hintList.rbegin();
         rit!=m_hintList.rend();
         ++rit)
    {
        if (p_scol!=SVID_END_OF_LINE)
            if ((int)(rit->k_pos+rit->k_len)>=p_scol)
                continue;

        if (rit->k_len==p_keyword.len && u_memcmp(rit->k_text, p_keyword.uctext, rit->k_len)==0)
        {
            p_fcol = rit->k_pos;
            return true;
        }
    }
    return false;
}

// Find wxString string from m_text
bool svLineText::FindNextKeywordwxStrFrom(const wxString &p_keyword, const wxString& p_charSet, const int p_scol, int &p_fcol, bool p_case)
{
    // int found = m_text->Right(p_scol).Find(p_keyword);
    // m_text 可能是 null (畫面未及之處)
    if (!m_text)
        ConvertBuffer2String(p_charSet);

    int found = wxNOT_FOUND;
    if (p_case)
    {
        found = (int)m_text->find(p_keyword, (size_t)p_scol);
    }
    else
    {
        found = (int)m_text->Lower().find(p_keyword.Lower(), (size_t)p_scol);
    }

    if (found!=wxNOT_FOUND)
    {
        p_fcol = found;
        return true;
    }
    return false;
}

// Find wxString string from m_text
bool svLineText::FindPrevKeywordwxStrFrom(const wxString &p_keyword, const wxString& p_charSet, const int p_scol, int &p_fcol, bool p_case)
{

    // m_text 可能是 null (畫面未及之處)
    if (!m_text)
        ConvertBuffer2String(p_charSet);

    int len = 0;

    if (p_scol == SVID_END_OF_LINE)
        len = m_text->Length();
    else
        len = p_scol;

    int found = wxNOT_FOUND;
    if (p_case)
    {
        found = (int)m_text->Left(len).rfind(p_keyword, len);
    }
    else
    {
        found = (int)m_text->Lower().Left(len).rfind(p_keyword.Lower(), len);
    }

    if (found!=wxNOT_FOUND)
    {
        p_fcol = found;
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------------------
 *
 * Folding related functions
 *
 *----------------------------------------------------------------------------------------*/

bool svLineText::AddFolding(const foldingInfo &p_fi)
{
    // only one item will be stored
    m_folding.clear();
    m_folding.push_back(p_fi);
    return true;
}

bool svLineText::RemoveFolding(foldingInfo &p_fi)
{
    if (!m_folding.size())
        return false;

    p_fi = m_folding.at(0);
    m_folding.clear();
    return true;
}

bool svLineText::HadFolding(foldingInfo &p_fi)
{
    if (m_folding.size())
    {
        p_fi = m_folding.at(0);
        return true;
    }
    else
        return false;
}


/*
 * looking for the folding start symbal.
 * from right to left.
 * 
 * { { }  <= for this condition, the first { will be return.
 * 
 * p_start_col < 0 means from the end of the line, else from the position of p_start_col.
 *
 * p_e_cnt 因為可能由 svBufText 計算，所以要將前幾行的 end symbol count 當參數傳入以便正確計算
 *
 */
bool svLineText::KeywordContainSymbolStart(vector<pairSymbol> &p_ps, const int p_scol, int &p_pos, int &p_e_cnt, pairSymbol &p_pair)
{
    for(std::vector<pairSymbol>::iterator it=p_ps.begin();
        it!=p_ps.end();
        ++it)
    {
        int end_count = 0;
        for (std::vector<keywordUnit>::reverse_iterator it2=m_keywordList.rbegin(); 
             it2!=m_keywordList.rend(); 
             ++it2)
        {
            if (it2->k_text!=NULL)
            // it2->k_text == NULL 是keyword以外的字串
            {
                if ((p_scol<0 || (int)it2->k_pos<=p_scol) &&
                    (it->s_len==it2->k_len && u_memcmp(it->s_text, it2->k_text, it->s_len)==0) )
                {
                    if (!p_e_cnt)
                    {
                        p_pos = it2->k_pos;
                        p_pair = *it;
                        return true;
                    }
                    else
                    {
                        --p_e_cnt;
                    }
                }
                else if ((p_scol<0 || (int)it2->k_pos<p_scol) &&
                         (it->e_len==it2->k_len && u_memcmp(it->e_text, it2->k_text, it->e_len)==0) )
                {
                    ++p_e_cnt;
                }
            }
        }
    }
    return false;
}

/*
 * looking for the folding end symbal.
 * from left to right.
 * 
 * p_start_col < 0 means from the start of the line, else from the position of p_start_col.
 *
 * p_s_cnt 因為可能由 svBufText 計算，所以要將前幾行的 start symbol count 當參數傳入以便正確計算
 *
 */
bool svLineText::KeywordContainSymbolEnd(vector<pairSymbol> &p_ps, const int p_scol, int &p_pos, int &p_s_cnt, pairSymbol &p_pair)
{
    for(std::vector<pairSymbol>::iterator it=p_ps.begin();
        it!=p_ps.end();
        ++it)
    {
        for (std::vector<keywordUnit>::iterator it2=m_keywordList.begin(); 
             it2!=m_keywordList.end(); 
             ++it2)
        {
            if (it2->k_text!=NULL)
            // it2->k_text == NULL 是keyword以外的字串
            {
                if ((p_scol<0 || (int)it2->k_pos>=p_scol) &&
                    (it->e_len==it2->k_len && u_memcmp(it->e_text, it2->k_text, it->e_len)==0) )
                {
                    if (!p_s_cnt)
                    {
                        p_pos = it2->k_pos;
                        p_pair = *it;
                        return true;
                    }
                    else
                    {
                        --p_s_cnt;
                    }
                }
                else if ((p_scol<0 || (int)it2->k_pos>p_scol) &&
                         (it->s_len==it2->k_len && u_memcmp(it->s_text, it2->k_text, it->s_len)==0) )
                {
                    ++p_s_cnt;
                }
            }
        }
    }
    return false;
}

// return true if keyword contain any keyword pid equal to p_pid and return the first one meeted.
bool svLineText::KeywordContainPID(vector<int> &p_pid, keywordUnit &p_keyword)
{
    for (std::vector<keywordUnit>::iterator it=m_keywordList.begin(); 
         it!=m_keywordList.end(); 
         ++it)
    {
        if (it->k_text!=NULL)
        // it2->k_text == NULL 是keyword以外的字串
        {
            for (vector<int>::iterator it2=p_pid.begin();
                 it2!=p_pid.end();
                 ++it2)
            {
                if (it->k_pid==*it2)
                {
                    p_keyword = *it;
                    return true;
                }
            }
        }
    }
    return false;
}

// return the first column position where non space appeared.
int svLineText::FindFirstNonSpaceColUW(void)
{
    wxString trimLeftText = GetTextUW();
    trimLeftText.Trim(false);
    return GetTextUW().Length() - trimLeftText.Length();
}

// 將扣除 p_spaceCnt 個空白字元的字串(啟始位置)回傳.
// 一個 tab = n 個 spaces.
// 若空白字元數小於 p_spaceCnt 個，則最多只將空白扣除
int svLineText::FindFirstNonSpaceColUW(int p_spaceCnt, int p_tabSpace)
{
    wxString trimLeftText = GetTextUW();
    int maxSpaceCnt = FindFirstNonSpaceColUW();
    int sum=0;
    int idx=0;

    while (sum<p_spaceCnt && idx<maxSpaceCnt)
    {
        if (trimLeftText.Mid(idx, 1)=='\t')
        {
            sum += p_tabSpace;
        }
        else if (trimLeftText.Mid(idx, 1)==' ')
        {
            sum += 1;
        }
        ++idx;
    }

    return idx;

}