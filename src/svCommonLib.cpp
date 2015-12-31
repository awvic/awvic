/*
   Copyright Notice in awvic.cpp
*/

#include <wx/tokenzr.h>
#include "svCommonLib.h"

#include "stdwx.h"
// #include "unicode/ucnv.h"     /* C   Converter API    */

svCommonLib::svCommonLib()
{
}

svCommonLib::~svCommonLib()
{

}

// /*
//  * ICU Convert from wxString to UChar*
//  * We only cast it, and don't know if it will works in every situation.
//  */ 
// UErrorCode svCommonLib::ConvertwxStrToUChar(const wxString &p_str, UChar *p_ubuf, int &p_ubufLen)
// {
// // #ifndef NDEBUG
// //     wxLogMessage("svCommonLib::ConvertwxStrToUChar start");
// // #endif

//     const char *source;
//     const char *sourceLimit;
//     UChar *uBuf;
//     UChar *target;
//     UChar *targetLimit;
//     int32_t uBufSize = 0;
//     UConverter *conv = NULL;
//     UErrorCode status = U_ZERO_ERROR;
//     uint32_t total=0;

//     const char p_encoding = "UTF-8";

//     wxCharBuffer b = p_str.ToUTF8();
//     int m_bufferLen = strlen(b.data());

//     char* m_buffer;
//     m_buffer = (char *) malloc(sizeof(char) * (m_bufferLen+1));
//     strncpy(m_buffer, (const char*)p_str.mb_str(wxConvUTF8), m_bufferLen);

//     conv = ucnv_open( p_encoding, &status);
//     assert(U_SUCCESS(status));

//     // ************************* start convert *****************************

//     uBufSize = (m_bufferLen/ucnv_getMinCharSize(conv));
//     // printf("input bytes %d / min chars %d = %d UChars\n",
//     //         isize, ucnv_getMinCharSize(conv), uBufSize);
//     uBuf = (UChar*)malloc(uBufSize * sizeof(UChar));

//     assert(uBuf!=NULL);

//     target = uBuf;
//     targetLimit = uBuf + uBufSize;
//     source = (char*)m_buffer;
//     sourceLimit = (char*)(m_buffer + (m_bufferLen));

//     ucnv_toUnicode( conv, &target, targetLimit,
//             &source, sourceLimit, NULL,
//             true,           /* pass 'flush' when eof */
//             /* is true (when no more data will come) */
//             &status);

//     // ucnv_fromUChars : The ICU function which convert UChar into other encoding string.

//     if (status == U_BUFFER_OVERFLOW_ERROR)
//     {
//         // simply ran out of space - we'll reset the target ptr the next
//         // time through the loop.                
//         status = U_ZERO_ERROR;
//     }
//     else
//     {
//         // Check other errors here.
//         assert(U_SUCCESS(status));
//         // Break out of the loop (by force)
//     }

//     total = target - uBuf;
//     p_ubufLen = total;

//     ucnv_close(conv);

//     free(m_buffer);

// // #ifndef NDEBUG
// //     wxLogMessage("svCommonLib::ConvertwxStrToUChar end");
// // #endif

//     return U_ZERO_ERROR;
// }

