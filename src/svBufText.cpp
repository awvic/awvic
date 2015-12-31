/*
   Copyright Notice in awvic.cpp
*/

#include "svBufText.h"
#include <wx/textfile.h>
#include <vector>
#include <algorithm>    // std::sort

#include <wx/tokenzr.h>
#include <wx/clipbrd.h>

#include "unicode/utypes.h"
#include "unicode/ucsdet.h"
#include "unicode/ucnv.h"     /* C ICU Converter API    */

#include "stdwx.h"

#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
#include <string.h>

#include "svCommonLib.h"
// #include "svRELib.h"
#include "svTextEditorCtrl.h"
#include "svPreference.h"

// for performance efficiency evaluation.
// #include <chrono>
// typedef std::chrono::high_resolution_clock Clock;

#define BUFFER_SIZE 8192  // 8kb
#define CONV_BUFFERSIZE 1024 * 1024 // 1Mb
#define DETECTBUFFERSIZE 1024* 1024 // 1024kb

//using namespace std;
using std::vector;

svBufText::svBufText()
{
    svLineText *l = new svLineText();

#ifdef SINGLE_BUFFER
    m_buftext.push_back(*l);
#else
    vector<svLineText> firstSec;
    firstSec.push_back(*l);
    m_buftextSection.push_back(firstSec);
#endif
    delete l;

    m_charSet = wxString("UTF-8");
    m_CScharSet = NULL;
    char const * utf = "UTF-8";
    m_CScharSet = (char *) malloc (strlen(utf)+1);
    strcpy(m_CScharSet, utf);
    m_filename = wxT("");

    wxPlatformInfo pi;
    pi = wxPlatformInfo::Get();

    // m_EOL_wxStr 指的是寫入檔案時換行符號
    // 內部處理時換行符號一律以 \n 處理
    if (pi.IsOk())
    {
        wxString os = pi.GetOperatingSystemFamilyName(pi.GetOperatingSystemId());
        if (os==wxT("Unix"))
        {
            // set to UNIX type.
            m_EOL_wxStr = wxT("\n");       
            m_EOL_type = SVID_NEWLINE_LF;
        }
        else if (os==wxT("Macintosh"))
        {
            // set to Macintosh type.
            m_EOL_wxStr = wxT("\r");       
            m_EOL_type = SVID_NEWLINE_CR;
        }
        else if (os==wxT("Windows") || os==wxT("DOS") || os==wxT("OS/2"))
        {
            // set to DOS, Windows family type.
            m_EOL_wxStr = wxT("\r\n");     
            m_EOL_type = SVID_NEWLINE_CRLF;
        }
        else
        {
            // default set to UNIX type.
            m_EOL_wxStr = wxT("\n");       
            m_EOL_type = SVID_NEWLINE_LF;
        }
    }
    else
    {
        // default set to UNIX type.
        m_EOL_wxStr = wxT("\n");  
        m_EOL_type = SVID_NEWLINE_LF;
    }

    m_unitSize = 1;   // UTF-8 default

    m_folding_type = SVID_C_FOLDING;   // C style folding.

    // If Open file
    m_savedSeqCode = svSeqCodeGenerator::OLD_FILE_INIT_SEQ_CODE;
    m_modified = false;
    // else New File
    // m_savedSeqCode = svSeqCodeGenerator::NEW_FILE_INIT_SEQ_CODE;
    // m_modified = true;

    m_bufUCharAll = NULL;
    // m_bufUCharAllDirty = true;
    // m_dirtyAfterSearch = true;
    m_bufUCharAllLen = 0;

    m_synDefPid.clear();


    // m_currentWord2Find = NULL;
}

svBufText::~svBufText()
{
#ifdef SINGLE_BUFFER
    m_buftext.clear();
#else
    for(int i=0; i<(int)m_buftextSection.size(); ++i)
    {
        m_buftextSection.at(i).clear();
    }
    m_buftextSection.clear();
#endif

    if (m_CScharSet) free(m_CScharSet);
    m_synReRules.clear();
    m_availableHint.clear();

    if (m_bufUCharAll) free(m_bufUCharAll);
    m_newlineLocations.clear();
    m_synDefPid.clear();

    // if (m_currentWord2Find) free(m_currentWord2Find);
}

char* svBufText::DetectFileEncoding(const char *filename)
{

    char *encodeName = NULL;

    static char buffer[DETECTBUFFERSIZE];

    FILE *ifp;
    char const *mode = "rb";

    ifp = fopen( filename, mode);

    if (ifp == NULL)
    {
        // fprintf(stderr, "---------------------------\nCan't open input file %s\n", filename);
// #ifdef __WXMSW__        
//         wxLogMessage(wxT("svBufText::DetectFileEncoding error: Can't open file ") + wxString::FromAscii(filename));
// #else
//         wxLogMessage(wxT("svBufText::DetectFileEncoding error: Can't open file ") + wxString(filename, wxConvUTF8));
// #endif        
        wxLogMessage(wxT("svBufText::DetectFileEncoding error: Can't open file ") + svCommonLib::CharFilename2wxStr(filename));
        return NULL;
    }
    
    /* detect file text encoding */
    int32_t inputLength, matchCount = 0;
    inputLength = fread(buffer, sizeof(char), DETECTBUFFERSIZE, ifp);
    

    if (!inputLength)  // inputLength == 0
    {
        /*
         * It's a blank file. Assign file encoding as UTF-8.
         * Notic: If a text file is encoding UTF-8 with bom, even it has no data, 
         * it's file length will no be 0 and encoding can be detected.
         * encodeName == NULL
         *
         * 檔案大小為0時無法偵測編碼，將之預設為UTF-8
         * UTF-8 有BOM的檔案即使沒有內容，檔案大小也不會為0，編碼可被偵測，不會進入此段。
         */
        char const *UTF8 = "UTF-8";
        encodeName = (char*)malloc(sizeof(char)*(strlen(UTF8)+1));
        strcpy(encodeName, UTF8);
    }
    else
    {
        // Duplicate input into buffer when input file is small to rise the detect rate.
        if (inputLength<DETECTBUFFERSIZE)
        {
            int32_t remLen = DETECTBUFFERSIZE - inputLength;
            while(remLen>0)
            {
                memcpy(&buffer[DETECTBUFFERSIZE-remLen], buffer, inputLength>remLen?remLen:inputLength);
                remLen -= inputLength;
            }
        }
        inputLength = DETECTBUFFERSIZE;

        UCharsetDetector* csd;
        const UCharsetMatch **csm;
        UErrorCode status = U_ZERO_ERROR;

        fclose(ifp);

        csd = ucsdet_open(&status);
        ucsdet_setText(csd, buffer, inputLength, &status);

        csm = ucsdet_detectAll(csd, &matchCount, &status);

        // printf("------------------------\n%s :\n", filename);
        for(int match = 0; match < matchCount; match += 1) {
            const char *name = ucsdet_getName(csm[match], &status);
            const char *lang = ucsdet_getLanguage(csm[match], &status);
            int32_t confidence = ucsdet_getConfidence(csm[match], &status);

            if (lang == NULL || strlen(lang) == 0) {
                lang = "**";
            }

            // printf("%s (%s) %d\n", name, lang, confidence);

            if (match==0)
            {
                encodeName = (char*)malloc(sizeof(char)*(strlen(name)+1));
                memcpy(encodeName, name, strlen(name)+1);
                // //encodeName[strlen(encodeName)] = '\0';
                // fprintf(stdout, "first encoding: %s \n", encodeName);
            }
        }

        ucsdet_close(csd);

    }

    // for avoiding ICU converting bug.
    if (!encodeName || strncmp(encodeName, "IBM", 3)==0)
    {
        if (encodeName) free(encodeName);
        char const *ASCII = "ISO_8859-1";
        encodeName = (char*)malloc(sizeof(char)*(strlen(ASCII)+1));
        strcpy(encodeName, ASCII);
    }

    return encodeName;

}

/*
 * Initilize new file for text file and initialize svLineText for each line.
 */
bool svBufText::InitNewFile(const wxString& filename, svTextEditorCtrl *editor)
{
    const char *test1 = (const char *)filename.mb_str();
    const char *test2 = (const char *)filename.ToUTF8();

    /*
     * In windows filename is encoding in CP950.
     * In Morden Unix(Linux, OSX) filename is encoding in UTF8.
     * So Wi have different filename converting from wxString.
     */

    const char *cfilename = svCommonLib::wxStrFilename2Char(filename);


    // New file default encoding to UTF-8
    if (m_CScharSet) free(m_CScharSet);
    m_CScharSet = (char *) strdup("UTF-8");
    m_charSet = wxString::FromAscii(m_CScharSet);


    // UTF-32LE UTF-32BE ==> 4bytes per unit
    // UTF-16LE UTF-16BE ==> 2bytes per unit
    // else    ==> 1byte  per unit
    // m_buftext.clear();
    clear_buffer_sections();

    if (m_charSet == wxT("UTF-32BE"))
    {
        // break_file_by_crlf(cfilename, 4, SVID_BIG_ENDIAN);
        m_unitSize = 4;
    }
    else if (m_charSet == wxT("UTF-32LE"))
    {
        // break_file_by_crlf(cfilename, 4, SVID_LITTLE_ENDIAN);
        m_unitSize = 4;
    }
    else if (m_charSet == wxT("UTF-16BE"))
    {
        // break_file_by_crlf(cfilename, 2, SVID_BIG_ENDIAN);
        m_unitSize = 2;
    }
    else if (m_charSet == wxT("UTF-16LE"))
    {
        // break_file_by_crlf(cfilename, 2, SVID_LITTLE_ENDIAN);
        m_unitSize = 2;
    }
    else  // UTF8 ASCII etc.
    {
        // break_file_by_crlf(cfilename, 1, SVID_BIG_ENDIAN);
        svLineText *pText = new svLineText(this, NULL, 0, SVID_NEWLINE_NONE, 0);
        append_new_line(pText);
        delete pText;
        m_unitSize = 1;
    }

    m_filename = filename;

    if (editor){
        editor->DisplayLoadingBackground("Converting buffer...", 15);
    }

    // Convert buffer to ICU Unicode buffer (UTF-16)
    // ºô½Ð svLineText::CreatKeywordTable Ç°Ò»¶¨ÒªÓÐICUBufferµÄÙYÁÏ
    // ConvertBufferICUAll();

    if (editor){
        editor->DisplayLoadingBackground("Processing wording...", 30);
    }

    // CreateKeywordTableAll();
    // CreateKeywordTableOnLoading(editor);

    if (editor){
        editor->DisplayLoadingBackground("Creating hint words...", 85);
    }

    // CreateHintDictionaryAll();

    if (editor){
        editor->DisplayLoadingBackground("Done!", 100);
    }

    m_savedSeqCode = svSeqCodeGenerator::NEW_FILE_INIT_SEQ_CODE;
    m_modified = true;

    if (m_bufUCharAll) free(m_bufUCharAll);
    m_bufUCharAll = NULL;
    // m_bufUCharAllDirty = true;
    // m_dirtyAfterSearch = true;
    m_bufUCharAllLen = 0;
    m_newlineLocations.clear();

    return true;
}


/*
 * Reading from text file and write to svLineText for each line.
 * Create keyword table initialy.
 */
// bool svBufText::ReadTextFile(const wxString& filename)
bool svBufText::ReadTextFile(const wxString& filename, svTextEditorCtrl *editor)
// bool svBufText::ReadTextFile(const wxString& filename, void *ed, svTextEditorCtrl &editor)
{
    const char *test1 = (const char *)filename.mb_str();
    const char *test2 = (const char *)filename.ToUTF8();

    /*
     * In windows filename is encoding in CP950.
     * In Morden Unix(Linux, OSX) filename is encoding in UTF8.
     * So Wi have different filename converting from wxString.
     */
// #ifdef __WXMSW__      
//     const char *cfilename = (const char *)filename.mb_str();
// #else
//     const char *cfilename = (const char *)filename.ToUTF8();
// #endif

    const char *cfilename = svCommonLib::wxStrFilename2Char(filename);

    // printf("readtextfile:%s\n", (const char*)filename.mb_str());
    // printf("readtextfile:%s\n", (const char*)filename.mb_str(wxConvUTF8));

    // Detect file encoding and save to m_CScharSet and m_charSet
    if (m_CScharSet) free(m_CScharSet);
    m_CScharSet = DetectFileEncoding(cfilename);
    m_charSet = wxString::FromAscii(m_CScharSet);


    // UTF-32LE UTF-32BE ==> 4bytes per unit
    // UTF-16LE UTF-16BE ==> 2bytes per unit
    // else    ==> 1byte  per unit
    // m_buftext.clear();
    clear_buffer_sections();

    if (m_charSet == wxT("UTF-32BE"))
    {
        break_file_by_crlf(cfilename, 4, SVID_BIG_ENDIAN);
        m_unitSize = 4;
    }
    else if (m_charSet == wxT("UTF-32LE"))
    {
        break_file_by_crlf(cfilename, 4, SVID_LITTLE_ENDIAN);
        m_unitSize = 4;
    }
    else if (m_charSet == wxT("UTF-16BE"))
    {
        break_file_by_crlf(cfilename, 2, SVID_BIG_ENDIAN);
        m_unitSize = 2;
    }
    else if (m_charSet == wxT("UTF-16LE"))
    {
        break_file_by_crlf(cfilename, 2, SVID_LITTLE_ENDIAN);
        m_unitSize = 2;
    }
    else  // UTF8 ASCII etc.
    {
        break_file_by_crlf(cfilename, 1, SVID_BIG_ENDIAN);
        m_unitSize = 1;
    }

    m_filename = filename;

    if (editor){
        editor->DisplayLoadingBackground("Converting buffer...", 15);
    }

    // Convert buffer to ICU Unicode buffer (UTF-16)
    // ºô½Ð svLineText::CreatKeywordTable Ç°Ò»¶¨ÒªÓÐICUBufferµÄÙYÁÏ
    ConvertBufferICUAll();

    if (editor){
        editor->DisplayLoadingBackground("Processing wording...", 30);
    }

    // CreateKeywordTableAll();
    CreateKeywordTableOnLoading(editor);

    if (editor){
        editor->DisplayLoadingBackground("Creating hint words...", 85);
    }

    CreateHintDictionaryAll();

    if (editor){
        editor->DisplayLoadingBackground("Done!", 100);
    }

    m_savedSeqCode = svSeqCodeGenerator::OLD_FILE_INIT_SEQ_CODE;
    m_modified = false;

    if (m_bufUCharAll) free(m_bufUCharAll);
    m_bufUCharAll = NULL;
    // m_bufUCharAllDirty = true;
    // m_dirtyAfterSearch = true;
    m_bufUCharAllLen = 0;
    m_newlineLocations.clear();

    return true;
}


// read file and break it into pieces accord to crlf/cr/lf.
// unit_size is the basic size of cr or lf according the encoding of file.
// ascii, utf-8 unit_size(in byte) should be 1,
// utf-16 should be 2, and utf32 should be 4.

// read files encoding like windows cp950 ascii utf-8 and process crlf.
// 這個程序用以處理文字檔內的newline符號
// \r\n for DOS WINDOWS
// \r for MAC
// \n for UNIX like
// 處理方式說明如下：
// 每次讀取 buf_size 長度 binary 資料
// 依unit_size(1 or 2 or 4 bytes)逐一比對是否符合 newline 符號
// 如比對符合 newline 符號，則將前一個比對符合的位置到這個比對符合的位置的資料存入另一個 bianry buffer 內供其他程式使用。
//
// unit_size 是考慮 ascii、utf8、utf16、utf32儲存一個字元所用的長度不同，\r \n在不同編碼方式檔案內儲存的資料也不同。
//
// 2014/11/26 效能測試
// read_file_1bytes讀取一個60MB的文字檔 buf_size=1024*1024 處理所花的時間約900-1500 miliseconds.
// read_file_1bytes讀取一個60MB的文字檔 buf_size=10 處理所花的時間約2000 miliseconds.
// 
void svBufText::break_file_by_crlf(const char *filename, int unit_size, int endian)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::break_file_by_crlf start");
// #endif

    FILE *ifp;
    // char *mode = "rb";
    char const mode[] = "rb";

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

    ifp = fopen( filename, mode);

    if (ifp == NULL)
    {
        //fprintf(stderr, "Can't open input file %s\n", filename);
        wxLogMessage(wxT("svBufText::break_file_by_crlf error: Can't open file ") + wxString::FromAscii(filename));
        return;
    }

    unsigned char *ch=NULL;
    int read_count = 0;
    int total_read = 0;
    int crlf_count = 0;
    int cr_count = 0;
    int lf_count = 0;
    int buf_size = unit_size * 1024 * 1024;
    unsigned char* keep_buf = NULL;
    int keep_len = 0;
    size_t copy_len = 0;

    while (!feof(ifp))
    {
        ch = (unsigned char *)malloc(buf_size*sizeof(unsigned char));
        int read_len = (int)fread(ch, sizeof(unsigned char), buf_size, ifp);
        read_count++;
        total_read += read_len;
        if (read_len>0)
        {
            int sp = 0;
            unsigned char *line = NULL;
            for (int i=0; i<read_len; i+=unit_size)
            {
                if (!memcmp(ch+i, newline_crlf, unit_size*2)) // \r\n = crlf 
                {
                    crlf_count++;
                    i+=unit_size;
                    if (sp==0 && keep_buf) // Fist time process after a reading, if keep_buf exist, append it in the front buffer.
                    {
                        copy_len = (size_t)(i-sp+unit_size); // include cr lf 
                        line = (unsigned char *) malloc( (copy_len+keep_len)*sizeof(unsigned char));
                        memcpy(line, keep_buf, (size_t)keep_len);
                        memcpy(line+keep_len, ch+sp, (size_t)(copy_len));

                        // We should consider implement a move constructor with vector to impoving efficiency.
                        svLineText *pText = new svLineText(this, line, copy_len+keep_len, SVID_NEWLINE_CRLF, 2*unit_size);
                        append_new_line(pText);
                        delete pText;
                        free(keep_buf);
                        keep_buf = NULL;
                    }
                    else
                    {
                        copy_len = (size_t)(i-sp+unit_size); // include cr lf 
                        line = (unsigned char *) malloc( (copy_len)*sizeof(unsigned char));
                        memcpy(line, ch+sp, (size_t)(copy_len));
                        svLineText *pText = new svLineText(this, line, copy_len, SVID_NEWLINE_CRLF, 2*unit_size);
                        append_new_line(pText);
                        delete pText;
                    }
                    free(line);
                    line = NULL;
                    sp = i+unit_size;
                }
                else
                {
                    if (!memcmp(ch+i, newline_cr, unit_size))  // \r = cr 
                    {
                        // \r found.
                        cr_count++;
                        if (sp==0 && keep_buf)
                        {
                            copy_len = (size_t)(i-sp+unit_size); // include cr 
                            line = (unsigned char *) malloc( (copy_len+keep_len)*sizeof(unsigned char));
                            memcpy(line, keep_buf, (size_t)keep_len);
                            memcpy(line+keep_len, ch+sp, (size_t)(copy_len));
                            svLineText *pText = new svLineText(this, line, copy_len+keep_len, SVID_NEWLINE_CR, unit_size);
                            append_new_line(pText);
                            delete pText;
                            free(keep_buf);
                            keep_buf = NULL;
                        }
                        else
                        {
                            if (i == buf_size-unit_size) // the last byte(s) unit of a reading. Take care of \r\n.
                            {
                                // process \r\n been cut while reading.
                                // fseek a byte(s) unit and read. if it's a \n mean we have to append \n to the previous \r.
                                // 每一次的讀取的最後一個 byte(s) unit 剛好是 \r, 考慮下一個 byte(s) unit 如果是 \n 時要將他們組合在一起。
                                unsigned char* tmp_buf = (unsigned char *) malloc(sizeof(unsigned char));
                                int tmp_rl = (int)fread(tmp_buf, sizeof(unsigned char), sizeof(unsigned char), ifp);
                                if (tmp_rl>0 && !memcmp(tmp_buf, newline_lf, unit_size))
                                {
                                    copy_len = (size_t)(i-sp+unit_size+unit_size); // include cr lf
                                    line = (unsigned char *) malloc((copy_len)*sizeof(unsigned char));
                                    memcpy(line, ch+sp, (size_t)(copy_len-unit_size));
                                    memcpy(line+i-sp+unit_size, newline_lf, unit_size);
                                    svLineText *pText = new svLineText(this, line, copy_len, SVID_NEWLINE_CRLF, 2*unit_size);
                                    append_new_line(pText);
                                    delete pText;
                                    tmp_buf = NULL;
                                }
                                else
                                {
                                    fseek(ifp, -1 * unit_size, SEEK_CUR);
                                    copy_len = (size_t)(i-sp+unit_size); // include cr 
                                    line = (unsigned char *) malloc((copy_len)*sizeof(unsigned char));
                                    memcpy(line, ch+sp, (size_t)(copy_len));
                                    svLineText *pText = new svLineText(this, line, copy_len, SVID_NEWLINE_CR, unit_size);
                                    append_new_line(pText);
                                    delete pText;
                                }
                                free(tmp_buf);
                            }
                            else
                            {
                                copy_len = (size_t)(i-sp+unit_size); // include cr 
                                line = (unsigned char *) malloc((copy_len)*sizeof(unsigned char));
                                memcpy(line, ch+sp, (size_t)(copy_len));
                                svLineText *pText = new svLineText(this, line, copy_len, SVID_NEWLINE_CR, unit_size);
                                append_new_line(pText);
                                delete pText;
                            }
                        }
                        free(line);
                        line = NULL;
                        sp = i+unit_size;
                    }
                    else
                    {
                        if (!memcmp(ch+i, newline_lf, unit_size))   // \n = lf 
                        {
                            lf_count++;
                            if (sp==0 && keep_buf)
                            {
                                copy_len = (size_t)(i-sp+unit_size); // include lf
                                line = (unsigned char *) malloc( (copy_len+keep_len)*sizeof(unsigned char));
                                memcpy(line, keep_buf, (size_t)keep_len);
                                memcpy(line+keep_len, ch+sp, (size_t)(copy_len));
                                svLineText *pText = new svLineText(this, line, copy_len+keep_len, SVID_NEWLINE_LF, unit_size);
                                append_new_line(pText);
                                delete pText;
                                free(keep_buf);
                                keep_buf = NULL;
                            }
                            else
                            {
                                copy_len = (size_t)(i-sp+unit_size); // include lf
                                line = (unsigned char *) malloc( (copy_len)*sizeof(unsigned char));
                                memcpy(line, ch+sp, (size_t)(copy_len));
                                svLineText *pText = new svLineText(this, line, copy_len, SVID_NEWLINE_LF, unit_size);
                                append_new_line(pText);
                                delete pText;
                            }
                            free(line);
                            line = NULL;
                            sp = i+unit_size;
                        }
                    }
                }
                //wxLogMessage(wxString::Format(wxT("CRLF=%i CR=%i LF=%i read=%i"), crlf_count, cr_count, lf_count, read_count));
            }
            // 每一次讀取檔案後，判段 newline 字元後剩下的資料，留待下一次的讀取一併處理。
            if (sp < read_len) // In the end of read buffer, keep it to be process in next read.
            {
                if (keep_buf) // Previois keep_buf already exist, appending data.
                {
                    unsigned char* old_keep_buf;
                    old_keep_buf = keep_buf;
                    int old_keep_len = keep_len;

                    keep_len += read_len - sp;
                    keep_buf = (unsigned char*) malloc(keep_len*(sizeof(unsigned char)));

                    memcpy(keep_buf, old_keep_buf, (size_t)old_keep_len);
                    memcpy(keep_buf+old_keep_len, ch+sp, (size_t)read_len-sp);

                    free(old_keep_buf);
                    old_keep_buf = NULL;
                }
                else
                {
                    keep_len = read_len - sp;
                    keep_buf = (unsigned char*) malloc(keep_len*(sizeof(unsigned char)));
                    memcpy(keep_buf, ch+sp, (size_t)keep_len);
                }
            }
        }
        if (ch) free(ch);
        if (int err_no = ferror(ifp))
        {
            // printf("%i error while read %s.\n", err_no, filename);
            wxLogMessage(wxString::Format(wxT("%i error wile reading %s"), err_no, wxString::FromAscii(filename)));
            break;
        }
        // wxLogMessage(wxString::Format(wxT("CRLF=%i CR=%i LF=%i read=%i"), crlf_count, cr_count, lf_count, read_count));
    }

    if (keep_buf) // In the end. reminder buffer without crlf|cr|lf.
    {
        svLineText *pText = new svLineText(this, keep_buf, keep_len, SVID_NEWLINE_NONE, 0);
        append_new_line(pText);
        delete pText;
        free(keep_buf);
        keep_buf = NULL;
    }
    else
    {
        if (crlf_count+cr_count+lf_count>0)
        {
            // add an empty string in the end of file.
            // Thre previous line has an newline in the end.
            svLineText *pText = new svLineText(this, NULL, 0, SVID_NEWLINE_NONE, 0);
            append_new_line(pText);
            delete pText;
        }
    }

    if (read_count==1 && LineCntUW()==0)  // 0 byte file.
    {
        svLineText *pText = new svLineText(this, NULL, 0, SVID_NEWLINE_NONE, 0);
        append_new_line(pText);
        delete pText;
    }

    fclose(ifp);
    // printf("%i time(s) and %i byte(s) read.\n", read_count, total_read);
    // printf("%i \\n\\r newline(s) found.\n", crlf_count);
    // printf("%i \\r newline(s) found.\n", cr_count);
    // printf("%i \\n newline(s) found.\n", lf_count);

    // 跟據 crlf cr lf計數最大者定該檔案的 end of line 符號
    if (crlf_count >= cr_count && crlf_count>= lf_count)
    {
        m_EOL_wxStr = wxT("\r\n");
        m_EOL_type = SVID_NEWLINE_CRLF;
    }
    else if (cr_count >= crlf_count && cr_count>= lf_count)
    {
        m_EOL_wxStr = wxT("\r");
        m_EOL_type = SVID_NEWLINE_CR;
    }
    else if (lf_count >= crlf_count && lf_count>= cr_count)
    {
        m_EOL_wxStr = wxT("\n"); 
        m_EOL_type = SVID_NEWLINE_LF;
    }

// #ifndef NDEBUG
//     wxLogMessage("svBufText::break_file_by_crlf end");
// #endif

}

void svBufText::append_new_line(svLineText const *p_line)
{
#ifdef SINGLE_BUFFER
    m_buftext.push_back(*p_line);
#else
    int idx = m_buftextSection.size() - 1;
    if (idx>=0)
    {
        // every section has a up limit number. If full, append a new section.
        if (m_buftextSection.at(idx).size()<BUF_SECTION_LEN)
        {
            m_buftextSection.at(idx).push_back(*p_line);
        }
        else
        {
            vector<svLineText> nextSec;
            nextSec.push_back(*p_line);
            m_buftextSection.push_back(nextSec);
            // m_buftextSection.at(idx+1).push_back(*p_line);
        }
    }
    else // empty m_buftextSection
    {
        vector<svLineText> firstSec;
        firstSec.push_back(*p_line);
        m_buftextSection.push_back(firstSec);
        // m_buftextSection.at(0).push_back(*p_line);
    }
#endif
}

void svBufText::insert_new_line(int uw_idx, svLineText const *p_line)
{
#ifdef SINGLE_BUFFER
    //list<svLineText>::iterator it = m_buftext.begin();
    //advance(it, uw_idx);
    //m_buftext.insert(it, *p_line);
    m_buftext.insert(m_buftext.begin()+uw_idx, *p_line);
#else
    int sumLen = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
        if (sumLen>uw_idx)
        {
            int index = it->size()-(sumLen-uw_idx);
            // wxLogMessage(wxString::Format("svBufText::insert_new_line=%i", index));
            it->insert(it->begin() + index, *p_line);    
            // it->insert(it->begin() + it->size()-(sumLen-uw_idx), *p_line);
            break;
        }
    }

    // boundry check
    if (uw_idx == sumLen)  // append a line
    {
        m_buftextSection.at(m_buftextSection.size()-1).push_back(*p_line);
    }
    else if (uw_idx > sumLen) // out of index 
    {
        // exception
        wxLogMessage(wxString::Format("svBufText::insert_new_line exception index(%i) out of range(%i).", uw_idx, sumLen));
    }
#endif
}

// insert lines for performance improvement.
void svBufText::insert_new_lines(int uw_idx, vector<svLineText> &p_newlines)
{
#ifdef SINGLE_BUFFER
    //list<svLineText>::iterator it = m_buftext.begin();
    //advance(it, uw_idx);
    //m_buftext.insert(it, *p_line);
    m_buftext.insert(m_buftext.begin()+uw_idx, p_newlines.begin(), p_newlines.end());
#else
    int sumLen = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
        if (sumLen>uw_idx)
        {
            int index = it->size()-(sumLen-uw_idx);
            // wxLogMessage(wxString::Format("svBufText::insert_new_line=%i", index));
            it->insert(it->begin() + index, p_newlines.begin(), p_newlines.end());    
            // it->insert(it->begin() + it->size()-(sumLen-uw_idx), *p_line);
            break;
        }
    }

    // boundry check
    if (uw_idx == sumLen)  // append a line
    {
        for (vector<svLineText>::iterator it=p_newlines.begin();
             it!=p_newlines.end();
             ++it)
        {
            m_buftextSection.at(m_buftextSection.size()-1).push_back(*it);
        }
    }
    else if (uw_idx > sumLen) // out of index 
    {
        // exception
        wxLogMessage(wxString::Format("svBufText::insert_new_line exception index(%i) out of range(%i).", uw_idx, sumLen));
    }
#endif
}

void svBufText::delete_line(int uw_idx)
{
#ifdef SINGLE_BUFFER
    //list<svLineText>::iterator it = m_buftext.begin();
    //advance(it, uw_idx);
    //m_buftext.erase(it);
    m_buftext.erase(m_buftext.begin()+uw_idx);
#else

    int sumLen = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
        if (sumLen>uw_idx)
        {
            int index = it->size()-(sumLen-uw_idx);
            // wxLogMessage(wxString::Format("svBufText::delete_line=%i", index));
            it->erase(it->begin() + index);
            // it->erase(it->begin() + it->size()-(sumLen-uw_idx));
            break;
        }
    }

#endif
}


void svBufText::delete_line_range(int uw_idx, int p_cnt)
{
#ifdef SINGLE_BUFFER
    //list<svLineText>::iterator it1 = m_buftext.begin();
    //advance(it1, uw_idx);
    //list<svLineText>::iterator it2 = m_buftext.begin();
    //advance(it2, uw_idx+p_cnt);
    //m_buftext.erase(it1, it2);
    m_buftext.erase(m_buftext.begin()+uw_idx, m_buftext.begin()+uw_idx+p_cnt);
#else

    int sumLen = 0;
    int len = p_cnt;
    int rmd = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
        if (sumLen>uw_idx)
        {
            int index = it->size()-(sumLen-uw_idx);
            rmd = len - it->size() - index;
            // wxLogMessage(wxString::Format("svBufText::delete_line=%i", index));
            if (rmd<=0)
            {
                it->erase(it->begin() + index, it->begin() + index + len);
                break;
            }
            else
            {
                it->erase(it->begin() + index, it->end());
                len = rmd;
            }
            // it->erase(it->begin() + it->size()-(sumLen-uw_idx));
            // break;
        }
    }

#endif
}

// // 這個函式在例外處理上仍有問題
// int svBufText::DetectFileEncoding(const char *filename)
// {

//     char *encodeName = NULL;

//     static char buffer[BUFFER_SIZE];

//     FILE *ifp;
//     char *mode = "rb";

//     ifp = fopen( filename, mode);

//     if (ifp == NULL)
//     {
//         // fprintf(stderr, "---------------------------\nCan't open input file %s\n", filename);
//         wxLogMessage(wxT("svBufText::DetectFileEncoding error: Can't open file ") + wxString::FromAscii(filename));
//         return -1;
//     }
    
//     /* detect file text encoding */
//     int32_t inputLength, matchCount = 0;
//     inputLength = fread(buffer, sizeof(char), BUFFER_SIZE, ifp);
    
//     // Duplicate input into buffer when input file is small to rising the detect rate.
//     if (inputLength<BUFFER_SIZE)
//     {
//         int32_t remLen = BUFFER_SIZE - inputLength;
//         while(remLen>0)
//         {
//             memcpy(&buffer[BUFFER_SIZE-remLen], buffer, inputLength>remLen?remLen:inputLength);
//             remLen -= inputLength;
//         }
//     }
//     inputLength = BUFFER_SIZE;

//     UCharsetDetector* csd;
//     const UCharsetMatch **csm;
//     UErrorCode status = U_ZERO_ERROR;

//     fclose(ifp);

//     csd = ucsdet_open(&status);
//     ucsdet_setText(csd, buffer, inputLength, &status);

//     csm = ucsdet_detectAll(csd, &matchCount, &status);

//     // printf("------------------------\n%s :\n", filename);
//     // Get and print all results.
//     /* for(int match = 0; match < matchCount; match += 1) {
//         const char *name = ucsdet_getName(csm[match], &status);
//         const char *lang = ucsdet_getLanguage(csm[match], &status);
//         int32_t confidence = ucsdet_getConfidence(csm[match], &status);

//         if (lang == NULL || strlen(lang) == 0) {
//             lang = "**";
//         }

//         // printf("%s (%s) %d\n", name, lang, confidence);

//         if (match==0)
//         {
//             encodeName = (char*)malloc(sizeof(char)*(strlen(name)+1));
//             memcpy(encodeName, name, strlen(name)+1);
//             //encodeName[strlen(encodeName)] = '\0';
//             fprintf(stdout, "first encoding: %s \n", encodeName);
//         }
//     } */

//     // Get the First result.
//     const char *name = ucsdet_getName(csm[0], &status);
//     m_charSet = wxString::FromAscii(name);    
//     m_CScharSet = (char*)malloc(sizeof(char)*(strlen(name)+1));
//     strcpy(m_CScharSet, name);

//     ucsdet_close(csd);

//     return 0;

// }


// bool svBufText::ReadTextFileICU(const wxString& filename)
// {

//     // DetectFileEncoding(filename.mb_str());

//     // UChar *ucharBuf = NULL;
//     // int32_t usize = 0;

//     // // ConvFileICU(filename.mb_str(), &ucharBuf, &usize);

//     // // process ucharBuf ...

//     // // line break process by ICU regular expression


//     return true;

// }


// 這個函式在例外處理上仍有問題
// 有 memory leak!
UErrorCode svBufText::ConvFileICU(const char *ifname, UChar **obuf, int32_t *osize)
{
    // fprintf(stdout, "\n===============================================\n"
    //        "Convert data from file into icu unicode string.\n");

    FILE *f;
    int32_t count;
    static char inBuf[CONV_BUFFERSIZE];
    char* destBuffer = NULL;
    const char *source;
    const char *sourceLimit;
    UChar *uBuf = NULL;
    UChar *target;
    UChar *targetLimit;
    int32_t uBufSize = 0;
    UConverter *conv = NULL;
    UErrorCode status = U_ZERO_ERROR;
    uint32_t inbytes=0, total=0;


    // fprintf(stdout, "Convert from '%s'(%s) to '%s\n'", ifname, encoding, ofname); 

    f = fopen(ifname, "rb");
    if (!f)
    {
        // fprintf(stderr, "Could't open file '%s'.\n", ifname);
// #ifdef __WXMSW__      
//         wxLogMessage(wxT("svBufText::ConvertFromFileICU error: Can't open file ") + wxString::FromAscii(ifname));
// #else
//         wxLogMessage(wxT("svBufText::ConvertFromFileICU error: Can't open file ") + wxString(ifname, wxConvUTF8));
// #endif        
        wxLogMessage(wxT("svBufText::ConvertFromFileICU error: Can't open file ") + svCommonLib::CharFilename2wxStr(ifname));

        return U_FILE_ACCESS_ERROR;
    }

    // ************************* read file into buffer *****************************

    conv = ucnv_open( m_CScharSet, &status);
    assert(U_SUCCESS(status));

    while((!feof(f)) &&
            ((count=fread(inBuf, 1, CONV_BUFFERSIZE, f)) > 0) )
    {
        inbytes += count;

        // Convert bytes to unicode
        source = inBuf;
        sourceLimit = inBuf + count;

        if (!destBuffer)
        {
            // first read.
            destBuffer = (char*)malloc(inbytes*sizeof(char));
            memcpy(destBuffer, inBuf, count);
        }
        else
        {
            // not the first read.
            char *oldDestBuffer = destBuffer;
            destBuffer = (char*)malloc(inbytes*sizeof(char));
            memcpy(destBuffer, oldDestBuffer, inbytes-count);
            memcpy(destBuffer+(inbytes-count), inBuf, count);
            free(oldDestBuffer);
        }
    }

    fclose(f);

    // printf("\n");

    // ************************* start convert *****************************

    uBufSize = (inbytes/ucnv_getMinCharSize(conv));
    // printf("input bytes %d / min chars %d = %d UChars\n",
    //         inbytes, ucnv_getMinCharSize(conv), uBufSize);
    uBuf = (UChar*)malloc(uBufSize * sizeof(UChar));

    assert(uBuf!=NULL);

    target = uBuf;
    targetLimit = uBuf + uBufSize;
    source = destBuffer;
    sourceLimit = destBuffer + (inbytes);

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
    

    // out = fopen(ofname, "w+b");
    // if (!out)
    // {
    //     fprintf(stderr, "Could't open file '%s'.\n", ofname);
    //     return U_FILE_ACCESS_ERROR;
    // }
    // total = target - uBuf;
    // *osize = total;

    // size_t wsize = fwrite(uBuf, sizeof(uBuf[0]), total, out);
    // fprintf(stdout, "%d bytes in, %d UChars out, %d bytes write.\n", inbytes, total, wsize*sizeof(UChar));

    ucnv_close(conv);
    // fclose(out);

    if (destBuffer) free(destBuffer);
    // if (obuf) free(obuf);
    *obuf = uBuf;
    *osize = uBufSize;
    // if (uBuf) free(uBuf);

    return U_ZERO_ERROR;

}

/*
 * ICU Convert from char* to UChar* for encoding=>m_CScharSet
 */ 
UErrorCode svBufText::ConvBufferICU(const char *ibuf, int32_t isize, UChar **obuf, int32_t *osize)
{

    // static char inBuf[CONV_BUFFERSIZE];
    // char* inBuf;
    const char *source;
    const char *sourceLimit;
    UChar *uBuf;
    UChar *target;
    UChar *targetLimit;
    int32_t uBufSize = 0;
    UConverter *conv = NULL;
    UErrorCode status = U_ZERO_ERROR;
    uint32_t total=0;


    conv = ucnv_open( m_CScharSet, &status);
    assert(U_SUCCESS(status));

    // uBufSize = (CONV_BUFFERSIZE/ucnv_getMinCharSize(conv));
    // // printf("input bytes %d / min chars %d = %d UChars\n",
    // //         BUFFERSIZE, ucnv_getMinCharSize(conv), uBufSize);
    // uBuf = (UChar*)malloc(uBufSize * sizeof(UChar));
    // assert(uBuf!=NULL);


    // ************************* start convert *****************************

    uBufSize = (isize/ucnv_getMinCharSize(conv));
    // printf("input bytes %d / min chars %d = %d UChars\n",
    //         isize, ucnv_getMinCharSize(conv), uBufSize);
    uBuf = (UChar*)malloc(uBufSize * sizeof(UChar));

    assert(uBuf!=NULL);

    target = uBuf;
    targetLimit = uBuf + uBufSize;
    source = ibuf;
    sourceLimit = ibuf + (isize);

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
    *osize = total;

    ucnv_close(conv);

    *obuf = uBuf;

    return U_ZERO_ERROR;
}


wxString svBufText::GetFileName(void)
{
    return m_filename;
}

wxString svBufText::GetTextAt(int uw_idx)
{
    return GetLine(uw_idx)->GetTextUW();
}

int svBufText::GetTextLengthAt(int uw_idx)
{
    return GetLine(uw_idx)->GetTextUW().Length();
}

bool svBufText::SetLineTextAt(int uw_idx, const wxString& txt)
{
    if (uw_idx<0 || uw_idx>=(int)LineCntUW()) return false;
    // m_buftext[uw_idx].SetLineTextAt(txt);
    GetLine(uw_idx)->SetTextUW(txt);
    return true;
}

bool svBufText::InsertAfterLineAt(int uw_idx, const wxString& txt)
{
    int totalLinesCnt = (int)LineCntUW();
    if (uw_idx<0 || uw_idx>=totalLinesCnt) 
        return false;

    svLineText *l = new svLineText();

    // including eol attribute 所以新增行的換行符號是參考被複製行的換行符號
    l->CopyAttribute(*GetLine(uw_idx));
    l->Visible(true);  // always visible when it's a new line

    if (uw_idx==totalLinesCnt-1) // 最後一行
    {
        // 最後一行原來是沒有換行符號
        // 因為插入下一行，所以要補上換行符號，否則存檔後會與上一行黏在一起
        GetLine(uw_idx)->SetNewLineType(m_EOL_type, m_unitSize);
        // 要重新更新 m_buffer m_ICUbuffer 等資料
        GetLine(uw_idx)->ConvertString2Buffer(m_charSet);
        GetLine(uw_idx)->ConvertBufferICU(m_CScharSet);
        CreateKeywordTableAt(uw_idx);        
    }
    else
    {
        // 如果不是最後一行
        // 則以 該檔案的最多的換行符號 或 作業系統的預設值 設定換行符號
        l->SetNewLineType(m_EOL_type, m_unitSize);
    }

    l->SetTextUW(txt);
    insert_new_line(uw_idx + 1, l);
    delete l;

    return true;
}

bool svBufText::InsertBeforeLineAt(int uw_idx, const wxString& txt)
{
    int totalLinesCnt = (int)LineCntUW();
    if (uw_idx<0 || uw_idx>=totalLinesCnt) 
        return false;

    svLineText *l = new svLineText();

    // including eol attribute 所以新增行的換行符號是參考被複製行的換行符號
    l->CopyAttribute(*GetLine(uw_idx));
    l->Visible(true);  // always visible when it's a new line

    // 以 該檔案的最多的換行符號 或 作業系統的預設值 設定換行符號
    l->SetNewLineType(m_EOL_type, m_unitSize);

    l->SetTextUW(txt);
    insert_new_line(uw_idx, l);
    delete l;

    return true;
}

// 在指定行的指定位置插入字串
// p_insertLineCnt 插入的文字行數(以 eol 符號分隔)
// p_lastLineLen 插入的文字行的最後一行的文字長度
// 由 wxClipboard 讀入的文字換行符號似乎一律都是 \n (僅管寫入時是 \n\r 或其他) 

// 換行符號一律以 \n 處理，如以\n\r wxStringTokenizer時處理會多一個空白行
// 傳入字串請以程式內部處理的資料(如clipboard 或 caret select delete )，不會影響編輯檔案的原換行符號
// 編輯檔案的新行的換行符號原則上參考其上一行
void svBufText::InsertTextAt(int uw_idx, int sx, const wxString& txt, int &p_insertLineCnt, int &p_lastLineLen)
{
    p_insertLineCnt = 0;
    p_lastLineLen = 0;

    // if txt is a newline symbol
    if (txt.Cmp("\n")==0)
    // if (txt.Cmp(m_EOL_wxStr)==0)
    {
        SplitLineAt(uw_idx, sx);
        p_insertLineCnt = 2;
        p_lastLineLen = 0;
        return;
    }

    //split it first
    // wxStringTokenizer tkz(txt, m_EOL_wxStr, wxTOKEN_RET_EMPTY_ALL);
    wxStringTokenizer tkz(txt, "\n", wxTOKEN_RET_EMPTY_ALL);
    p_insertLineCnt = tkz.CountTokens();
    if (p_insertLineCnt==1)  // only 1 line text
    {
        wxString token = tkz.GetNextToken();
        SetLineTextAt(uw_idx, GetTextBeforeAt(uw_idx, sx) + token + GetTextAfterAt(uw_idx, sx));
        p_lastLineLen = token.Length();
    }
    else if (p_insertLineCnt==2) // 2 lines text
    {
        wxString token1 = tkz.GetNextToken();
        wxString token2 = tkz.GetNextToken();
        InsertAfterLineAt(uw_idx, token2 + GetTextAfterAt(uw_idx, sx));
        SetLineTextAt(uw_idx, GetTextBeforeAt(uw_idx, sx) + token1);
        p_lastLineLen = token2.Length();
    }
    else if (p_insertLineCnt>2) // more than 2 lines text
    {
        wxString txtAftCaret = GetTextAfterAt(uw_idx, sx);
        wxString txtBefCaret = GetTextBeforeAt(uw_idx, sx);
        wxString token1 = tkz.GetNextToken();
        SetLineTextAt(uw_idx, GetTextBeforeAt(uw_idx, sx) + token1);
        
        
/*        while ( tkz.HasMoreTokens() )
        {
            wxString token = tkz.GetNextToken();
            if (tkz.HasMoreTokens())
            {
                InsertAfterLineAt(uw_idx, token);
            }
            else  // last line
            {
                InsertAfterLineAt(uw_idx, token + txtAftCaret);
                p_lastLineLen = token.Length();
            }
            uw_idx++;
        }
*/      

        // Code for performance improvement.
        // Inserting lines into a vector of lines by line is time consumming.
        // Inserting vector of lines into a vector of lines is better on time sonsumming.
        svLineText *referenceLine = GetLine(uw_idx);  
        vector<svLineText> newLines;
        while ( tkz.HasMoreTokens() )
        {
            wxString token = tkz.GetNextToken();
            if (tkz.HasMoreTokens())
            {
                svLineText *l = new svLineText();
                l->CopyAttribute(*referenceLine);
                l->Visible(true);  // always visible when it's a new line
                l->SetTextUW(token);
                newLines.push_back(*l);
                delete l;
            }
            else  // last line
            {
                svLineText *l = new svLineText();
                l->CopyAttribute(*referenceLine);
                l->Visible(true);  // always visible when it's a new line
                l->SetTextUW(token + txtAftCaret);
                newLines.push_back(*l);
                p_lastLineLen = token.Length();
                delete l;
            }
        }
        insert_new_lines(uw_idx+1, newLines);
    }
    
}

wxString svBufText::GetTextBeforeAt(int uw_idx, int pos)
{
    // if (uw_idx<0 || uw_idx>=(int)m_buftext.size()) return;
    return GetLine(uw_idx)->GetTextBeforeUW(pos);
}

wxString svBufText::GetTextAfterAt(int uw_idx, int pos)
{
    // if (uw_idx<0 || uw_idx>=(int)m_buftext.size()) return;
    return GetLine(uw_idx)->GetTextAfterUW(pos);
}

wxString svBufText::GetTextAt(int uw_idx, int p_scol, int p_len)
{
    // if (uw_idx<0 || uw_idx>=(int)m_buftext.size()) return;
    return GetLine(uw_idx)->GetTextUW(p_scol, p_len);
}

// 內部處理使用 newline symblo is set to \n
wxString svBufText::GetTextRange(int p_s_row, int p_s_col, int p_e_row, int p_e_col)
{
    wxString getText;

    if (p_s_row==p_e_row)
    {
        getText = GetTextAt(p_s_row, p_s_col, p_e_col-p_s_col);
        return getText;
    }

    // get last line
    // getText = m_EOL_wxStr + GetTextBeforeAt(p_e_row, p_e_col);
    getText = "\n" + GetTextBeforeAt(p_e_row, p_e_col);

    // get middle lines
    // get between p_s_row+1 to p_e_row-1
    for (int i=p_e_row-1; i>=p_s_row+1; i--)
    {
        // getText = m_EOL_wxStr + GetTextAt(i) + getText;
        getText = "\n" + GetTextAt(i) + getText;
    }

    // get first line
    getText = GetTextAfterAt(p_s_row, p_s_col) + getText;

    return getText;
}

svLineText *svBufText::GetLine(int uw_idx)
{
#ifdef SINGLE_BUFFER
    //list<svLineText>::iterator it=m_buftext.begin();
    //advance(it, uw_idx);
    //return &(*it);
    return &m_buftext.at(uw_idx);
#else

    int sumLen = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
        if (sumLen>uw_idx)
        {
            int index = it->size()-(sumLen-uw_idx);
            // wxLogMessage(wxString::Format("svBufText::GetLine index=%i %i", uw_idx, index));
            return &(it->at(index));
        }
    }

    // exception: out of index
    wxLogMessage(wxString::Format("svBufText::GetLine Exception index=%i %i", uw_idx, LineCntUW()));
    return NULL;

#endif
}

size_t svBufText::TextLenAt(size_t uw_idx, short int p_type)
{
    return GetLine(uw_idx)->TextLen(p_type);
}

size_t svBufText::LineCntUW(void)
{
#ifdef SINGLE_BUFFER
    return m_buftext.size();
#else
    size_t sumLen = 0;
    for(std::vector<vector<svLineText> >::iterator it=m_buftextSection.begin();
        it!=m_buftextSection.end();
        ++it)
    {
        sumLen += it->size();
    }   
    return sumLen; 
#endif
}

wxString svBufText::DeleteLineAt(int uw_idx)
{
    wxString txt = GetLine(uw_idx)->GetTextUW();

    // 2015.10.05 copy folding information to the prev line.
    foldingInfo fi;
    if (uw_idx>0 && GetLine(uw_idx)->HadFolding(fi))
        GetLine(uw_idx-1)->SetFoldingInfo(GetLine(uw_idx)->GetFoldingInfo());

    delete_line(uw_idx);
    return txt;
}

// 刪除由  uw_idx 開始的文字行，共 p_len 行
// 回傳值僅供內部使用 因為 newline 固定為 \n
wxString svBufText::DeleteLineRangeAt(int uw_idx, int p_len)
{
    if (p_len<=0) return wxString("");
    if (p_len==1) return DeleteLineAt(uw_idx);
    if (p_len<1||(p_len+uw_idx)>(int)LineCntUW())
    {
        // Exception
        wxLogMessage(wxString::Format("svBufText::DeleteLineRangeAt Error : out of range %i+%i of %i", (int)uw_idx, (int)p_len, (int)LineCntUW()));
        return wxString("");        
    }  

    wxString txt;
    for (int i=0; i<p_len; i++)
    {
        if (i!=p_len-1)
        {
            // txt += GetLine(uw_idx+i)->GetTextUW() + m_EOL_wxStr;
            txt += GetLine(uw_idx+i)->GetTextUW() + "\n";
        }
        else
        {
            txt += GetLine(uw_idx+i)->GetTextUW();
        }
    }
    delete_line_range(uw_idx, p_len);
    return txt;
}

// DeleteLineTextAfter(n, 2)
//  0 1 2 3 4 5 6 7 8
//      x x x x x x x
wxString svBufText::DeleteTextAfterAt(int uw_idx, int x)
{
    wxString txt;
    txt = GetLine(uw_idx)->DeleteTextAfterUW(x);
    return txt;
}

// DeleteLineTextBefore(n, 2)
//  0 1 2 3 4 5 6 7 8
//  x x x
wxString svBufText::DeleteTextBeforeAt(int uw_idx, int x)
{
    wxString txt;
    txt = GetLine(uw_idx)->DeleteTextBeforeUW(x);
    return txt;
}

// DeleteLineText(n, 2, 6)
//  0 1 2 3 4 5 6 7 8
//      x x x x 
wxString svBufText::DeleteTextAt(int uw_idx, int sx, int ex)
{
    wxString txt;
    // txt = GetLine(uw_idx).DeleteTextAt(sx, ex-sx);
    txt = GetLine(uw_idx)->DeleteTextUW(sx, ex);
    return txt;
}

// 換行符號一律以 \n 處理，如以\n\r處理會多一個空白行
// 回傳的字串只用以程式內部處理，不會影響編輯檔案的原換行符號
// 編輯檔案的新行的換行符號原則上參考其上一行
wxString svBufText::DeleteTextRange(int p_s_row, int p_s_col, int p_e_row, int p_e_col)
{
    wxString delText;

    if (p_s_row==p_e_row)
    {
        delText = DeleteTextAt(p_s_row, p_s_col, p_e_col);
        return delText;
    }

    // delete p_e_row line
    delText = "\n" + DeleteTextBeforeAt(p_e_row, p_e_col);
    // delText = m_EOL_wxStr + DeleteTextBeforeAt(p_e_row, p_e_col);

    // delete between p_s_row+1 to p_e_row-1
    if (p_e_row-p_s_row-1>0)
    {
        delText = "\n" + DeleteLineRangeAt(p_s_row+1, p_e_row-p_s_row-1) + delText;
        // delText = m_EOL_wxStr + DeleteLineRangeAt(p_s_row+1, p_e_row-p_s_row-1) + delText;
    }

    // delete p_s_row
    delText = DeleteTextAfterAt(p_s_row, p_s_col) + delText;

    // glue the 1st and the last delete text line.
    if (p_s_row+1 < (int)LineCntUW())
    {
        SetLineTextAt(p_s_row, GetTextAt(p_s_row)+GetTextAt(p_s_row+1));
        DeleteLineAt(p_s_row+1);
    }
    return delText;
}

void svBufText::SplitLineAt(int uw_idx, int sx)
{
    if (GetLine(uw_idx)->GetTextUW().Length()==0 || 
            GetLine(uw_idx)->GetTextUW().Length()==sx)  // in the head or end of the text line
    {
        //InsertAfterLineAt(uw_idx, wxString(wxT("\n")));
        // InsertAfterLineAt(uw_idx, wxString(m_EOL));
        InsertAfterLineAt(uw_idx, wxString(""));
    }
    else   // int the middle of a text line
    {
        InsertAfterLineAt(uw_idx, GetTextAfterAt(uw_idx, sx));
        SetLineTextAt(uw_idx, GetTextBeforeAt(uw_idx, sx));
    }

    // When split a line, the folding infomation should be keep on the new line.
    GetLine(uw_idx+1)->SetFoldingInfo(GetLine(uw_idx)->GetFoldingInfo());
    GetLine(uw_idx)->RemoveFoldingInfo();
    
}

bool svBufText::JoinNextLineAt(int uw_idx)
{
    if (uw_idx+1>=(int)LineCntUW()) // out of boundry
        return false;

    SetLineTextAt(uw_idx, GetTextAt(uw_idx) + GetTextAt(uw_idx+1));
    // 2015.09.17 copy folding information from been joined line.
    // GetLine(uw_idx)->SetFoldingInfo(GetLine(uw_idx+1)->GetFoldingInfo());
    DeleteLineAt(uw_idx+1);

    return true;
}

wxString svBufText::DuplicateLineAt(int uw_idx)
{

    wxString txt = GetTextAt(uw_idx);
    InsertAfterLineAt(uw_idx, txt);

    // When duplicate a line, the folding infomation should be keep on the new line.
    GetLine(uw_idx+1)->SetFoldingInfo(GetLine(uw_idx)->GetFoldingInfo());
    GetLine(uw_idx)->RemoveFoldingInfo();

    return txt;
}

char svBufText::LineCommentLineAt(int uw_idx, const wxString &p_lineCom)
{
    wxString txt = GetTextAt(uw_idx);
    int col = GetLine(uw_idx)->FindFirstNonSpaceColUW();
    
    if (col==GetLine(uw_idx)->TextLen(SVID_NO_CRLF))
    {
        // It's a blank line. nothing changed.
        return SVID_LINE_NOT_CHANGED;
    }
    else if (txt.Mid(col, p_lineCom.Length())==p_lineCom)
    {
        SetLineTextAt(uw_idx, txt.Left(col) + txt.Right(txt.Length()-col-p_lineCom.Length()));
        return SVID_LINE_UNCOMMENTED;
    }
    else
    {
        SetLineTextAt(uw_idx, txt.Left(col) + p_lineCom + txt.Right(txt.Length()-col));
        return SVID_LINE_COMMENTED;
    }

}

char svBufText::BlockCommentAt(int p_sr, int p_sc, int p_er, int p_ec, const wxString &p_startCom, const wxString &p_endCom)
{
    wxString stxt = GetTextAt(p_sr);
    wxString etxt = GetTextAt(p_er);

    if (p_sr<p_er)
    {

        if ( (stxt.Mid(p_sc, p_startCom.Length())==p_startCom) &&
             (p_ec>=(int)p_endCom.Length()) && 
             (etxt.Mid(p_ec-p_endCom.Length(), p_endCom.Length())==p_endCom) )
        {
            // The start and end position is /* and */
            SetLineTextAt(p_sr, stxt.Left(p_sc) + stxt.Right(stxt.Length()-p_sc-p_startCom.Length()));
            SetLineTextAt(p_er, etxt.Left(p_ec-p_endCom.Length()) + etxt.Right(etxt.Length()-p_ec));
            return SVID_BLOCK_UNCOMMENTED_MULTI_LINE;
        }
        else
        {
            // The start and end position is not /* and */, block comment it!
            SetLineTextAt(p_sr, stxt.Left(p_sc) + p_startCom + stxt.Right(stxt.Length()-p_sc));
            SetLineTextAt(p_er, etxt.Left(p_ec) + p_endCom + etxt.Right(etxt.Length()-p_ec));
            return SVID_BLOCK_COMMENTED_MULTI_LINE;
        }
    }
    else if (p_sr==p_er)
    {
        if (p_sc<p_ec)
        {
            if ( (stxt.Mid(p_sc, p_startCom.Length())==p_startCom) &&
                 (p_ec>=(int)p_endCom.Length()) && 
                 (etxt.Mid(p_ec-p_endCom.Length(), p_endCom.Length())==p_endCom) )
            {
                // The start and end position is /* and */
                SetLineTextAt(p_sr, stxt.Left(p_sc) + 
                              stxt.Mid(p_sc+p_startCom.Length(), p_ec-p_sc-p_startCom.Length()-p_endCom.Length()) +
                              stxt.Right(stxt.Length()-p_ec)
                              );
                return SVID_BLOCK_UNCOMMENTED_SINGLE_LINE;
            }
            else
            {
                // The start and end position is not /* and */, block comment it!
                SetLineTextAt(p_sr, stxt.Left(p_sc) + 
                              p_startCom + 
                              stxt.Mid(p_sc, p_ec-p_sc) +
                              p_endCom + 
                              stxt.Right(stxt.Length()-p_ec));
                return SVID_BLOCK_COMMENTED_SINGLE_LINE;
            }            
        }
        else
        {
            wxLogMessage(wxString::Format("svBufText::BlockCommentAt exception block comment start from row %i col %i to %i", p_sr, p_sc, p_ec));
            return 0;
        }
    }
    else
    {
        wxLogMessage(wxString::Format("svBufText::BlockCommentAt exception block comment start from row %i to %i", p_sr, p_er));
        return 0;
    }

}

void svBufText::SetEOLType(const char p_type)
{
    m_EOL_type = p_type;
}

char svBufText::GetEOLType(void)
{
    return m_EOL_type;
}

// bool svBufText::SearchUW(const wxString& token, svSearchPozList* posList)
// {
//     for (int i=0; i<(int)m_buftext.size(); i++)
//     {
//         if (GetLine(i).GetTextUW().Length()==0) continue;
//         wxString text = GetLine(i).GetTextUW();
//         int c = text.Find(token); 
//         int shift=0;
//         while(c!=wxNOT_FOUND)
//         {
//             svPozLen* pos = new svPozLen(i, shift+c, token.Length()); 
//             posList->Append(pos);
//             delete pos;
//             text = text.Right(text.Length()-token.Length()-c);
//             shift += c + token.Length();
//             c = text.Find(token);
//         }
//     }

//     if (posList->Size()>0)
//     {
//         posList->SetToken(token);
//         return true;
//     }
//     else
//     return false;

// }

// bool svBufText::ReplaceUW(int idx, int sx, const wxString& oldtxt, const wxString& newtxt)
// {
//     if (idx<0 || idx>=(int)m_buftext.size()) return false;

//     if (GetLine(idx).GetTextUW().Mid(sx, oldtxt.Length()) != oldtxt)
//     return false;

//     // m_buftext[idx] = m_buftext[idx].GetTextAt().Left(sx) + newtxt + m_buftext[idx].GetTextAt().Right(m_buftext[idx].GetTextAt().Length()-sx-oldtxt.Length());
//     GetLine(idx).SetTextUW(GetLine(idx).GetTextUW().Left(sx) + newtxt + GetLine(idx).GetTextUW().Right(GetLine(idx).GetTextUW().Length()-sx-oldtxt.Length()));
//     return true;
// }

vector<vector<styledWrapText> >* svBufText::GetStyledWrapTextAt(size_t uw_idx)
{
    return GetLine(uw_idx)->GetStyledWrapLineText();
}

vector<styledWrapText>* svBufText::GetStyledWrapTextAt(size_t uw_idx, size_t w_idx)
{
    return GetLine(uw_idx)->GetStyledWrapLineText(w_idx);
}

size_t svBufText::GetWrapLineCountAt(size_t uw_idx)
{
    return GetLine(uw_idx)->GetWrapLineCount();
}

// void svBufText::ProcWrapLineAt(size_t uw_idx, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth)
// {
//     return GetLine(uw_idx)->ProcWrapLine(p_tabWidth, p_spacePixWidth, p_font, p_screenWidth);
// }

bool svBufText::GetWrapLenOnIdxAt(size_t uw_idx, size_t p_idx, wrapLenDesc& p_wrapLenDesc)
{
    return GetLine(uw_idx)->GetWrapLenOnIdx(p_idx, p_wrapLenDesc);
}

int svBufText::InWhichWrappedLineAt(size_t uw_idx, size_t p_idx)
{
    return GetLine(uw_idx)->InWhichWrappedLine(p_idx);
}

bool svBufText::InWhichWrappedLineColAt(size_t uw_idx, size_t p_idx, int &p_wrapline, int &p_wrapcol)
{
    return GetLine(uw_idx)->InWhichWrappedLineCol(p_idx, p_wrapline, p_wrapcol);
}

int svBufText::GetWrappedLineStartColOnIdxAt(size_t uw_idx, size_t p_idx)
{
    return GetLine(uw_idx)->GetWrappedLineStartColOnIdx(p_idx);
}

int svBufText::GetWrappedLineStartColOnPositionAt(size_t uw_idx, size_t p_idx)
{
    return GetLine(uw_idx)->GetWrappedLineStartColOnPosition(p_idx);
}


// void svBufText::ProcStyledWrapLineTextAt(size_t uw_idx)
// {
//     return GetLine(uw_idx)->ProcStyledWrapLineText();
// }

// void svBufText::ProcStyledTextAt(size_t uw_idx)
// {
//     // return GetLine(uw_idx).ProcStyledText(txtStyle);
//     return GetLine(uw_idx)->ProcStyledText(&m_synReRules);
// }

void svBufText::ProcWrapAndStyleAt(size_t uw_idx, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::ProcWrapAndStyleAt start");
// #endif

    // 將該行的 buffer 轉為 wxString 字串
    // ConvertBuffer2StringAt(uw_idx);
    GetLine(uw_idx)->ConvertBuffer2String(m_charSet);

    // CreateKeywordTableAt(uw_idx);  // 有重覆，應判斷條件後呼叫。

    // 計算折行資訊
    // ProcWrapLineAt(uw_idx, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth);
    GetLine(uw_idx)->ProcWrapLine(p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);

    // 計算 syntax hiligh 資訊
    // ProcStyledTextAt(uw_idx);
    GetLine(uw_idx)->ProcStyledText(&m_synReRules);

    // 彙整折行及syntax hiligh 資訊
    // ProcStyledWrapLineTextAt(uw_idx);
    GetLine(uw_idx)->ProcStyledWrapLineText();

// #ifndef NDEBUG
//     wxLogMessage("svBufText::ProcWrapAndStyleAt end");
// #endif    

}

void svBufText::ProcWrapAndStyleAll(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    for (size_t i=0; i < LineCntUW(); i++ )
    {
        // ConvertBuffer2StringAt(i);
        GetLine(i)->ConvertBuffer2String(m_charSet);
        // ProcWrapLineAt(i, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth);
        GetLine(i)->ProcWrapLine(p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);
        // ProcStyledTextAt(i);
        GetLine(i)->ProcStyledText(&m_synReRules);
        // ProcStyledWrapLineTextAt(i);
        GetLine(i)->ProcStyledWrapLineText();
    }
}


// ------------------------------------------------------------------------------------- //
// Wrap version member functions.
// ------------------------------------------------------------------------------------- //

// Line wrapped process was designed to be lazy. Available wrapped lines not equal to all wrapped lines.
// Sequential access.
size_t svBufText::GetAvailableLineCnt_W(void)
{
    size_t wl_count = 0;

    for (size_t i=0; i<LineCntUW(); i++)
    {
        wl_count += GetLine(i)->GetWrapLineCount();
    }
    return wl_count;
}

// Sequential access.
vector<styledWrapText>* svBufText::GetStyledWrapTextAt_W(size_t idx)
{
    size_t wl_count = 0;

    for (size_t i=0; i<LineCntUW(); i++)
    {
        for (size_t j=0; j<GetLine(i)->GetWrapLineCount(); j++)
        {
            if (wl_count == idx)
            {
                vector<styledWrapText>* s = (vector<styledWrapText> *) GetLine(i)->GetStyledWrapLineText();
                return s;
            }
            wl_count++;
        }
    }

    return NULL;
}

bool svBufText::PixelX2TextColAt(size_t uw_idx, int p_pixelX, size_t& p_textCol)
{
    return GetLine(uw_idx)->PixelX2TextColUW(p_pixelX, p_textCol);
}

bool svBufText::TextCol2PixelXAt(size_t uw_idx, size_t p_textCol, int& p_pixelX)
{
    return GetLine(uw_idx)->TextCol2PixelUW(p_textCol, p_pixelX);
}

// already consider the wrap situation.
// pixel x already minus wrap line.
bool svBufText::TextCol2PixelXAtW(size_t uw_idx, size_t p_textCol, int& p_pixelX)
{
    bool flag = GetLine(uw_idx)->TextCol2PixelUW(p_textCol, p_pixelX);
    if (flag)
    {
        int w = GetLine(uw_idx)->GetWrappedLineStartColOnPosition(p_textCol);
        int startPosPixel=0;
        flag = GetLine(uw_idx)->TextCol2PixelUW(w, startPosPixel);
        if (flag)
            p_pixelX = p_pixelX - startPosPixel;
    }
    return flag;
}

// bool svBufText::TextRow2PixelXAt(size_t uw_idx, int& p_pixelY)
// {
//     return GetLine(uw_idx)->TextRow2PixelUW(p_pixelY);
// }

// Convert data from memory buffer to wxString.
// bool svBufText::ConvertBuffer2StringAt(size_t uw_idx)
// {
//     if (uw_idx>=LineCntUW())
//     {
// // #ifndef NDEBUG
//         wxLogMessage(wxString::Format("svBufText::ConvertBuffer2StringAt(%i) out of index(%i).", uw_idx, LineCntUW()));
//         return false;
// // #endif
//     }
//     return GetLine(uw_idx)->ConvertBuffer2String(m_charSet);
// }

/*
 * -------------------------------------------------------------------------
 * To be implement.
 * -------------------------------------------------------------------------
 */
bool svBufText::DetectSyntaxType(void)
{
    return true;
}

// Read regular expression rules from ***.svsyn file.
bool svBufText::LoadSyntaxDesc(const char *p_filename)
{

    m_synReRules.clear();
    m_synDefPid.clear();

    UChar *ubcs = NULL;
    UChar *ubce = NULL;
    UChar *ulinecomment = NULL;
    UChar *ukeyword = NULL;
    UChar *uoperator = NULL;
    UChar *upreprocessor = NULL;

    std::ifstream f(p_filename);
    std::string config_doc;

    try
    {

        if (!f.fail())
        {
            f.seekg(0, std::ios::end);
            int size = f.tellg();
            config_doc.reserve(size);
            f.seekg(0, std::ios::beg);

            config_doc.assign((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
            }
        else
        {
            //std::cout << "Exception  opening/reading file:" << e.what();
            //std::cout << "Exception  opening/reading file:" << m_filename;
            wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc Error - Exception opening file:" + m_filename));
            wxLogMessage(ErrMsg);
            return false;            
        }

    }
    // catch (std::ios_base::failure& e)
    catch (std::exception& e)
    {
        //std::cout << "Exception  opening/reading file:" << e.what();
        wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc - Exception opening/reading file:" + m_filename + wxT(" ") + wxString(e.what())));
        wxLogMessage(ErrMsg);
        return false;
    }

    f.close();

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    bool parsingSuccessful = reader.parse( config_doc, root );
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        // std::cout  << "Failed to parse configuration\n"
        //            << reader.getFormattedErrorMessages();
        wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc - Failed to parse configuration: " + wxString(reader.getFormattedErrorMessages())));
        wxLogMessage(ErrMsg);
        return false;
    }

    // Get the value of the member of root named 'encoding', return 'UTF-8' if there is no
    // such member.
    std::string encoding = root.get("encoding", "UTF-8" ).asString();
    // Get the value of the member of root named 'encoding', return a 'null' value if
    // there is no such member.
    const Json::Value jlanguage = root["language"];
    m_synName = jlanguage.asString();


    /* for goto definition rules pid */

    /* for embrace and folding rules */
    
    const Json::Value def_rules = root["definition_pid"];
    for ( unsigned int index = 0; index < def_rules.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value rule_val  = def_rules[index];

        try
        {
            int def_pid = rule_val["pid"].asInt();
            m_synDefPid.push_back(def_pid);
        }
        catch(const std::exception &e )
        {
            wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc - Parsing Json error:" + wxString(e.what())));
            wxLogMessage(ErrMsg);
        }
    }

    // m_synDefPid = root.get("definition_pid", -1).asInt();  // default -1 for no definition pid setting.

    /* for syntax rules */

    const Json::Value rules = root["syntax_rule"];
    for ( unsigned int index = 0; index < rules.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value rule_val  = rules[index];
        re_rule rd;

        try
        {
            int rtype = rule_val["type"].asInt();

            rd.name = strdup(rule_val["name"].asString().c_str());
            rd.re1 = strdup(rule_val["re"].asString().c_str());
            rd.re2 = NULL;
            if (rtype==0)  // block comments
            {
                rd.cm1 = strdup(rule_val["comment"]["start"].asString().c_str());
                rd.cm2 = strdup(rule_val["comment"]["end"].asString().c_str());
            }
            else if (rtype==1) // line comments
            {
                rd.cm1 = strdup(rule_val["comment"]["comment1"].asString().c_str());
                rd.cm2 = strdup(rule_val["comment"]["comment2"].asString().c_str());
            }
            else  // else
            {
                rd.cm1 = NULL;
                rd.cm2 = NULL;
            }
            rd.type = rule_val["type"].asInt();
            rd.pid = rule_val["pid"].asUInt();
            rd.regroup = (int32_t)rule_val["regroup"].asInt();
            if (rd.type<=100)
            {
                m_synReRules.push_back(rd);
            }
            else
            {
                // other rules.
                // for c/c++ foldering and embrace symbol testing.
                // if (rd.type==101)
                // {
                //     m_foldingSymbol.push_back(pairSymbol("{", "}"));
                //     m_embraceSymbol.push_back(pairSymbol("{", "}"));
                //     m_embraceSymbol.push_back(pairSymbol("(", ")"));
                // }
            }
        }
        catch(const std::exception &e )
        {
            // std::cout << "Unhandled exception:" << e.what() << "\n";
            wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc - Parsing Json error:" + wxString(e.what())));
            wxLogMessage(ErrMsg);
        }

    }


    /* for embrace and folding rules */

    const Json::Value e_rules = root["embrace_rule"];
    for ( unsigned int index = 0; index < e_rules.size(); ++index )  // Iterates over the sequence elements.
    {
        const Json::Value rule_val  = e_rules[index];
        embrace_rule er;

        try
        {
            int rtype = rule_val["type"].asInt();

            char *s_start = NULL;
            char *s_end = NULL;
            s_start = strdup(rule_val["start"].asString().c_str());
            s_end = strdup(rule_val["end"].asString().c_str());
            if (rtype==102)  // folding rule
            {
                m_folding_type = rule_val["folding_type"].asInt();
            }
            er.type = rule_val["type"].asInt();
            er.pid = rule_val["pid"].asUInt();

            if (rtype==101)
            {
                m_embraceSymbol.push_back(pairSymbol(s_start, s_end));
            }
            else if (rtype==102)
            {
                m_foldingSymbol.push_back(pairSymbol(s_start, s_end));
            }

            if (s_start) free(s_start);
            if (s_end) free(s_end);
        }
        catch(const std::exception &e )
        {
            wxString ErrMsg = wxString::Format(wxT("svBufText::LoadSyntaxDesc - Parsing Json error:" + wxString(e.what())));
            wxLogMessage(ErrMsg);
        }
    }

    return true;
}

/*
 * 
 * 
 * 
 */

bool svBufText::ConvertBufferICUAll(void)
{
    for (size_t i=0; i < LineCntUW(); i++ )
    {
        GetLine(i)->ConvertBufferICU(m_CScharSet);
    }

    return true;
}

bool svBufText::CreateKeywordTableAll(void)
{
    for (size_t i=0; i < LineCntUW(); i++ )
    {
        GetLine(i)->CreateKeywordTable(&m_synReRules);
    }

    CheckBlockTag();

    return true;
}

bool svBufText::CreateKeywordTableAt(size_t uw_idx)
{
    // 先將該行原有的 hint 自 m_hintDict 內移除
    GetLine(uw_idx)->SubHintDictionary(m_hintDict);

    // 重新計算新的 keyword 資訊(包含 hint 資訊)
    GetLine(uw_idx)->CreateKeywordTable(&m_synReRules);

    // 將新資料的 hint 加回 m_hintDict
    GetLine(uw_idx)->CreateHintDictionary(m_hintDict);

    return true;
}

// ------------------------------------------------------------------------------------- //
// Create keyword table functions.
// ------------------------------------------------------------------------------------- //
void svBufText::CreateHintDictionaryAll(void)
{
    for (size_t i=0; i < LineCntUW(); i++ )
    {
        GetLine(i)->CreateHintDictionary(m_hintDict);
    }
}


/*
 * for block syntax tag processing. 
 * 處理 block 形式的語法規則
 */  
bool svBufText::CheckBlockTag(void)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::CheckBlockTag start");
// #endif    
    // 01. get type 0 rule from m_synReRules;
    // 02. start from start tag and search for next end tag
    // 03. repeat 02 till end of block tag list.
    for(std::vector<re_rule>::iterator it=m_synReRules.begin();it!=m_synReRules.end(); ++it)
    {
        if (it->type==SVID_BLOCK_RE_RULE)  // block rule(s).
        {
            UChar *ustag = NULL;
            UChar *uetag   = NULL;
            int32_t stagLen = 0;
            int32_t etagLen = 0;
            int pid = it->pid;          // pattern id

            ustag = (UChar*) malloc (sizeof(UChar)*(strlen(it->cm1)+1));
            u_charsToUChars(it->cm1, ustag, strlen(it->cm1)+1);
            stagLen = strlen(it->cm1);

            uetag = (UChar*) malloc (sizeof(UChar)*(strlen(it->cm2)+1));
            u_charsToUChars(it->cm2, uetag, strlen(it->cm2)+1);
            etagLen = strlen(it->cm2);

            size_t start_row, start_col;
            size_t result_row, result_col;

            start_row = start_col = 0;
            result_row = result_col = 0;

            size_t col = start_col; // 0 default. col is the search start poing position. Add string string length to it.

            while (GetNextBlockTag(ustag, stagLen, start_row, col, &result_row, &result_col))
            {
                ClearBlockKeywordUnitPID(start_row, start_col, result_row, result_col, pid);
                start_row = result_row;
                start_col = result_col;
                col = result_col + stagLen;
                if (GetNextBlockTag(uetag, etagLen, start_row, col, &result_row, &result_col))
                {
                    // process syntax between (start_row, start_col) to (result_row, result_col)
                    ChangeBlockKeywordUnitPID(start_row, start_col, result_row, result_col, pid, stagLen, etagLen);
                    start_row = result_row;
                    start_col = result_col;
                    col = result_col + etagLen;
                }
            }

            // 清除剩餘者
            result_row = LineCntUW() - 1;
            result_col = GetLine(result_row)->TextLen(SVID_NO_CRLF);
            ClearBlockKeywordUnitPID(start_row, start_col, result_row, result_col, pid);

            if (ustag) free(ustag);
            if (uetag) free(uetag);

        }
    }

// #ifndef NDEBUG
//     wxLogMessage("svBufText::CheckBlockTag end");
// #endif    

    return true;

}

// locate the next p_tag in m_blockTagList
bool svBufText::GetNextBlockTag(const UChar *p_tag, const int32_t p_tagLen, const size_t p_row, const size_t p_col, size_t *r_row, size_t *r_col)
{
    for(size_t i=p_row; i<LineCntUW(); i++)
    {
        // svBufText is friend class of svLineText for access m_blockTagList.
        for(std::vector<keywordUnit>::iterator it=GetLine(i)->m_blockTagList.begin(); it!=GetLine(i)->m_blockTagList.end(); ++it)
        {
            if (u_strncmp(it->k_text, p_tag, p_tagLen)==0)
            {
                if ((i>p_row)||(i==p_row && it->k_pos>=p_col))  // block_tag's location greater than p_row, p_col
                {
                    // if (!GetLine(i)->OverlapedKeyword(&*it))    // prevent block tag between string tag.
                    // {
                        *r_row = i;
                        *r_col = it->k_pos;
                        return true;
                    // }
                }
            }
        }
    }
    return false;
}

/*
 * Clear keywordUnit's k_blocked_pid in specified range.
 * This is for block syntax processing.
 * 清除變某個區間內所有keywordUnit的 k_blocked_pid
 */
void svBufText::ClearBlockKeywordUnitPID(const size_t p_start_row, const size_t p_start_col, const size_t p_end_row, const size_t p_end_col, const int p_pid)
{

    if (p_start_row==p_end_row)
    {
        size_t i = p_start_row;
        GetLine(i)->ClearBlockKeywordInRange(p_start_col, p_end_col, p_pid);
    }
    else
    {
        for(size_t i=p_start_row; i<=p_end_row; i++)
        {
            if (i==p_start_row)
            {
                GetLine(i)->ClearBlockKeywordInRange(p_start_col, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
            }
            else if (i==p_end_col)
            {
                GetLine(i)->ClearBlockKeywordInRange(0, p_end_col, p_pid);
            }
            else
            {
                GetLine(i)->ClearBlockKeywordInRange(0, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
            }
        }
    }
}

/*
 * Change keywordUnit's pid in specified range.
 * This is for block syntax processing.
 * 改變某個區間內所有keywordUnit的 pid
 * 因為 block ReRule 只有找出開始及結束的tag
 * 還必須將開始tag及結束tag間的所有tag(keywordUnit)的pid改為跟block ReRule相同
 * 才會有block的效果
 */
bool svBufText::ChangeBlockKeywordUnitPID(const size_t p_start_row, const size_t p_start_col, const size_t p_end_row, const size_t p_end_col, const int p_pid, const int32_t p_stagLen, const int32_t p_etagLen)
{

    if (p_start_row==p_end_row)
    {
        size_t i = p_start_row;
        // GetLine(i)->UpdateBlockKeyword(p_start_col, p_end_col+p_etagLen, p_pid);
        GetLine(i)->UpdateBlockKeywordInRange(p_start_col, p_end_col+p_etagLen, p_pid);
    }
    else
    {
        for(size_t i=p_start_row; i<=p_end_row; i++)
        {
            if (i==p_start_row)
            {
                // GetLine(i)->UpdateBlockKeyword(p_start_col, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
                GetLine(i)->UpdateBlockKeywordInRange(p_start_col, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
            }
            else if (i==p_end_row)
            {
                // GetLine(i)->UpdateBlockKeyword(0, p_end_col+p_etagLen, p_pid);
                GetLine(i)->UpdateBlockKeywordInRange(0, p_end_col+p_etagLen, p_pid);
            }
            else
            {
                // GetLine(i)->UpdateBlockKeyword(0, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
                GetLine(i)->UpdateBlockKeywordInRange(0, GetLine(i)->TextLen(SVID_NO_CRLF), p_pid);
            }
        }
    }
    return true;    
}



/* 
 * ===========================================================================
 *  Regular expression functions for CreateKeywordTableOnLoading()
 * ===========================================================================
 */

/*
 * Create keyword table for the first time on file loading.
 * Because CreateKeywordTableAll could be slow when file is big(600K for 6secs on i7).
 * We try to reduce time consumming when regular expression processing.
 * *NOTICE* This function only execute on file loading.
 *
 * passing a pointer to svTextEditorCtrl if you want to display loading progress on the screen.
 */
bool svBufText::CreateKeywordTableOnLoading(svTextEditorCtrl *editor)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::CreateKeywordTableOnLoading start");
// #endif
    /*
     * 01 Read file into ICU buffer.
     * 02 Each rule in m_synReRules create a keywordList and save to p_KWLList.
     * 03 crlf as keywordUnit List and save to p_crlfList.
     * 04 break p_KWLList by p_crlfList and save keywordList to each line(svLineText).
     * 05 block tag processing.

     * 01 將檔案讀入並轉換為 ICU buffer.
     * 02 為 m_synReRules 的每個規則建立一個 keywordList 再存到 p_KWLList ( 儲存 keywordUnit vector 的 vector).
     * 03 為 crlf 建立一個 keywordUnit List 並存到 p_crlfList.
     * 04 依照 p_crlfList 內crlf的位置，逐一將 p_KWLList 內 keywordUnit List 資料寫入至各行的 keywordUnit List.
     * 05 block tag 跨行符號處理.

     * svBufText::REMatchPattern_0 REMatchPattern_1 REMatchPattern_2
     * 這3個函數處理方式是與svLineText::REMatchPattern_0 REMatchPattern_1 REMatchPattern_2 相同的
     * 只有 InsertKeywordUnit InsertBlockTagKeywordUnit 略有小差異
     */

    UChar *uBuf = NULL;
    int32_t usize = 0;

// #ifdef __WXMSW__      
//     const char *cfilename = (const char *)m_filename.mb_str();
// #else
//     const char *cfilename = (const char *)m_filename.ToUTF8();
// #endif
    const char *cfilename = svCommonLib::wxStrFilename2Char(m_filename);

    // Read file and convert it into uBuf.
    ConvFileICU(cfilename, &uBuf, &usize);

    if (editor){
        editor->DisplayLoadingBackground("Regular Expression...", 35);
    }

    vector<vector<keywordUnit> > p_KWLList, p_BKWLList, p_HWLList; // Vector of Vector of keyword.
    vector<keywordUnit> p_crlfList;

    int s = (int)m_synReRules.size();
    int delta = 0;

    for (std::vector<re_rule>::iterator it=m_synReRules.begin(); it != m_synReRules.end(); ++it)
    {
        vector<keywordUnit> keywordList;
        vector<keywordUnit> blockList;
        vector<keywordUnit> hintList;
        switch(it->type)
        {
            case 0:
                REMatchPattern_0(uBuf, usize, it->re1, it->pid, it->regroup, it->cm1, it->cm2, &keywordList, &blockList);
                // svRELib::ICU_RE_Pattern_0(uBuf, usize, it->re1, it->pid, it->regroup, it->cm1, it->cm2, &keywordList, &blockList);
                break;
            case 1:
                REMatchPattern_1(uBuf, usize, it->re1, it->pid, it->regroup, &keywordList);
                // svRELib::ICU_RE_Pattern_1(uBuf, usize, it->re1, it->pid, it->regroup, &keywordList);
                break;
            case 2:
                REMatchKeyword_2(uBuf, usize, it->re1, it->pid, it->regroup, &keywordList);
                // svRELib::ICU_RE_Pattern_2(uBuf, usize, it->re1, it->pid, it->regroup, &keywordList);
                break;
            case 3:
                REMatchKeyword_3(uBuf, usize, it->re1, it->pid, it->regroup, &hintList);
                // svRELib::ICU_RE_Pattern_3(uBuf, usize, it->re1, it->pid, it->regroup, &hintList);
                break;
        }
        if (keywordList.size()>0)
            p_KWLList.push_back(keywordList);
        if (blockList.size()>0)
            p_BKWLList.push_back(blockList);
        if (hintList.size()>0)
            p_HWLList.push_back(hintList);

        delta+=(70-35)/s;
        if (editor)
        {
            editor->DisplayLoadingBackground("Counting words...", 35+delta);
        }        
    }

    if (editor)
    {
        editor->DisplayLoadingBackground("Breaking lines...", 70);
    }

    // create crlf position list
    REMatchKeyword_2(uBuf, usize, "(\r\n|\n|\r)", 0, 0, &p_crlfList);

    if (editor)
    {
        editor->DisplayLoadingBackground("Storing keywords...", 72);
    }

    // process uBuf
    CreateLineKeywordTable(&p_KWLList, &p_BKWLList, &p_HWLList, &p_crlfList);

    if (editor)
    {
        editor->DisplayLoadingBackground("Storing to every lines...", 80);
    }

    for (std::vector<vector<keywordUnit> >::iterator it=p_KWLList.begin();
         it != p_KWLList.end();
         ++it)
    {
        it->clear();
    }
    p_KWLList.clear();

    for (std::vector<vector<keywordUnit> >::iterator it=p_BKWLList.begin();
         it != p_BKWLList.end();
         ++it)
    {
        it->clear();
    }
    p_BKWLList.clear();

    for (std::vector<vector<keywordUnit> >::iterator it=p_HWLList.begin();
         it != p_HWLList.end();
         ++it)
    {
        it->clear();
    }
    p_HWLList.clear();

    p_crlfList.clear();

    // Set UChar ubffer and set dirty flag to false.
    if (uBuf) free(uBuf);
    // m_bufUCharAll = uBuf;   
    // m_bufUCharAllDirty = false;
    // m_bufUCharAllLen = usize;


    if (editor)
    {
        editor->DisplayLoadingBackground("Processing block tags...", 82);
    }

    CheckBlockTag();

// #ifndef NDEBUG
//     wxLogMessage("svBufText::CreateKeywordTableOnLoading end");
// #endif

    return true;

}

// m_keywordList shoud be order by k_pos
bool svBufText::InsertKeywordUnit(vector<keywordUnit>* p_keywordList, const keywordUnit* p_ku)
{
    // if (p_keywordList->size()==0)
    // {
        p_keywordList->push_back(*p_ku);
    // }
    // else
    // {
    //     for(std::vector<keywordUnit>::iterator it = p_keywordList->begin(); it != p_keywordList->end(); ++it)
    //     {
    //         if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
    //         {
    //             return false;
    //         }
    //         else if (p_ku->k_pos <= it->k_pos) // = should not happened.
    //         {
    //             p_keywordList->insert(it, *p_ku);
    //             return true;
    //         }
    //     }
    //     p_keywordList->push_back(*p_ku);
    // }
    return true;
}

// m_keywordList shoud be order by k_pos
// bool svBufText::InsertKeywordUnit2(vector<keywordUnit>* p_keywordList, const keywordUnit& p_ku)
// {

//     p_keywordList->push_back(p_ku);

//     return true;
// }

// p_hintList shoud be order by k_pos
bool svBufText::InsertHintUnit(vector<keywordUnit>* p_hintList, const keywordUnit* p_ku)
{
    // if (p_hintList->size()==0)
    // {
        p_hintList->push_back(*p_ku);
    // }
    // else
    // {
    //     for(std::vector<keywordUnit>::iterator it = p_hintList->begin(); it != p_hintList->end(); ++it)
    //     {
    //         if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
    //         {
    //             return false;
    //         }
    //         else if (p_ku->k_pos <= it->k_pos) // = should not happened.
    //         {
    //             p_hintList->insert(it, *p_ku);
    //             return true;
    //         }
    //     }
    //     p_hintList->push_back(*p_ku);
    // }
    return true;
}

// p_hintList shoud be order by k_pos
// bool svBufText::InsertHintUnit2(vector<keywordUnit>* p_hintList, const keywordUnit& p_ku)
// {
//     p_hintList->push_back(p_ku);

//     return true;
// }


// m_keywordList shoud be order by k_pos
bool svBufText::InsertBlockTagKeywordUnit(vector<keywordUnit>* p_blockTagList, const keywordUnit* p_ku)
{
    // if (p_blockTagList->size()==0)
    // {
        p_blockTagList->push_back(*p_ku);
    // }
    // else
    // {
    //     for(std::vector<keywordUnit>::iterator it = p_blockTagList->begin(); it != p_blockTagList->end(); ++it)
    //     {
    //         if ((p_ku->k_pos>=it->k_pos) && p_ku->k_pos<(it->k_pos+it->k_len)) // dup match keyword or pattern
    //         {
    //             return false;
    //         }
    //         else if (p_ku->k_pos <= it->k_pos) // = should not happened.
    //         {
    //             p_blockTagList->insert(it, *p_ku);
    //             return true;
    //         }
    //     }
    //     p_blockTagList->push_back(*p_ku);
    // }
    return true;
}

// Insert keywordUnit into svLineText line by line
// 將 keywordUnit 寫入每行的 keywordUnit List
// 逐一比對 crlf(newline)的位置，以算出是哪一行，再將 keywordUnit 寫入該行的 keywordUnit List
bool svBufText::CreateLineKeywordTable(vector<vector<keywordUnit> >* p_KWLList, vector<vector<keywordUnit> >* p_BKWLList, vector<vector<keywordUnit> >* p_HWLList, vector<keywordUnit>* p_crlfList)
{
    int lineCnt = LineCntUW();  // max line num.

    // BLOCK KEYWORD MUST INSERTED BEFORE KEYWORD.
    // THE ORDER IS MATTER. 

    // block keyword
    for(std::vector<vector<keywordUnit> >::iterator it_KL=p_BKWLList->begin();
        it_KL!=p_BKWLList->end();
        it_KL++)
    {
        // sequential match 
        std::vector<keywordUnit>::iterator it_k=it_KL->begin();
        std::vector<keywordUnit>::iterator it_crlf=p_crlfList->begin();

        int32_t lineNo = 0;
        int32_t keep_pos = 0;


        while(it_k!=(it_KL->end()))
        {
            if ( it_crlf==p_crlfList->end() ||
                 it_k->k_pos < it_crlf->k_pos+it_crlf->k_len )
            {
                it_k->k_pos -= keep_pos;
                GetLine(lineNo)->InsertBlockTagKeywordUnit(&*it_k);
                it_k++;
            }
            else // it_k->k_pos>=it_crlf->k_pos
            {
                if (it_crlf!=p_crlfList->end())
                {
                    keep_pos = it_crlf->k_pos+it_crlf->k_len;
                    lineNo++;
                    it_crlf++;
                    if (lineNo>=lineCnt)
                    {
                        // Somehow bug just happend.
                        // only casually happen on release build, but not on debug build (Windows). Wonder why.
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #04 p_crlf.size()=%i", p_crlfList->size()));
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #04 svLineText is null index %i of %i", lineNo, lineCnt));
                        break;
                    }
                }
            }
        }
    }

    // keyword
    for(std::vector<vector<keywordUnit> >::iterator it_KL=p_KWLList->begin();
        it_KL!=p_KWLList->end();
        it_KL++)
    {
        // sequential match 
        std::vector<keywordUnit>::iterator it_k=it_KL->begin();
        std::vector<keywordUnit>::iterator it_crlf=p_crlfList->begin();

        int32_t lineNo = 0;
        int32_t keep_pos = 0;


        while(it_k!=(it_KL->end()))
        {
            if ( it_crlf==p_crlfList->end() ||
                 it_k->k_pos < it_crlf->k_pos+it_crlf->k_len )
            {
                it_k->k_pos -= keep_pos;
                GetLine(lineNo)->InsertKeywordUnit(&*it_k);
                it_k++;
            }
            else // it_k->k_pos>=it_crlf->k_pos
            {
                if (it_crlf!=p_crlfList->end())
                {
                    keep_pos = it_crlf->k_pos+it_crlf->k_len;
                    lineNo++;
                    it_crlf++;
                    if (lineNo>=lineCnt)
                    {
                        // Somehow bug just happend.
                        // only casually happen on release build, but not on debug build (Windows). Wonder why.
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #02 p_crlf.size()=%i", p_crlfList->size()));
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #02 svLineText is null index %i of %i", lineNo, lineCnt));
                        break;
                    }
                }
            }
        }
    }

    // hint word
    for(std::vector<vector<keywordUnit> >::iterator it_KL=p_HWLList->begin();
        it_KL!=p_HWLList->end();
        it_KL++)
    {
        // sequential match 
        std::vector<keywordUnit>::iterator it_k=it_KL->begin();
        std::vector<keywordUnit>::iterator it_crlf=p_crlfList->begin();

        int32_t lineNo = 0;
        int32_t keep_pos = 0;


        while(it_k!=(it_KL->end()))
        {
            if ( it_crlf==p_crlfList->end() ||
                 it_k->k_pos < it_crlf->k_pos+it_crlf->k_len )
            {
                it_k->k_pos -= keep_pos;
                GetLine(lineNo)->InsertHintUnit(&*it_k);
                it_k++;
            }
            else // it_k->k_pos>=it_crlf->k_pos
            {
                if (it_crlf!=p_crlfList->end())
                {
                    keep_pos = it_crlf->k_pos+it_crlf->k_len;
                    lineNo++;
                    it_crlf++;
                    if (lineNo>=lineCnt)
                    {
                        // Somehow bug just happend.
                        // only casually happen on release build, but not on debug build (Windows). Wonder why.
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #06 p_crlf.size()=%i", p_crlfList->size()));
                        wxLogMessage(wxString::Format("svBufText::CreateLineKeywordTable #06 svLineText is null index %i of %i", lineNo, lineCnt));
                        break;
                    }
                }
            }
        }
    }

    return true;
}


//
// Regular Expression matching specified pattern.
// for /* and */
//
// !! THE SAME PROCESSING RULES WITH svLineText::REMatchPattern_0 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svBufText::REMatchPattern_0(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, const char *start, const char *end, vector<keywordUnit> *p_keywordList, vector<keywordUnit> *p_bkeywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_0 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
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

    URegularExpression *re = uregex_open(upattern, -1, UREGEX_DOTALL, &pe, &status); 
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
                bool inserted = InsertKeywordUnit(p_keywordList, &ku);
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
                    InsertBlockTagKeywordUnit(p_bkeywordList, &tku);
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
        wxString ErrMsg = wxString::Format(wxT("RE0 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (ustart) free(ustart);
    if (uend) free(uend);
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_0 end");
// #endif
    return U_ZERO_ERROR;
}


/*
 * Regular Expression matching specified pattern.
 */
// !! THE SAME PROCESSING RULES WITH svLineText::REMatchPattern_1 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svBufText::REMatchPattern_1(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_1 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif

    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    UChar *upattern = NULL;

    char const* tab = "\t";
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
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
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

// #ifndef NDEBUGs
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 i=%ld", i));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 u_strlen(dest)=%ld", u_strlen(dest)));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 sPos-s=%ld", sPos-s));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 sPos=%ld", sPos));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 s=%ld", s));
// #endif

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
                            InsertKeywordUnit(p_keywordList, &ku);
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
                        InsertKeywordUnit(p_keywordList, &xku);
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
                            InsertKeywordUnit(p_keywordList, &ku);
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
                        InsertKeywordUnit(p_keywordList, &xku);
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
                    InsertKeywordUnit(p_keywordList, &ku);
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
        wxString ErrMsg = wxString::Format(wxT("RE1 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);        
    }

    uregex_close(re);

    if (upattern) free(upattern);
    if (utab) free(utab);
    if (uspace) free(uspace);
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_1 end");
// #endif
    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// !! THE SAME PROCESSING RULES WITH svLineText::REMatchPattern_2 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svBufText::REMatchKeyword_2(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_keywordList)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_2 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
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
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_2 01");
// #endif
    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE2 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_2 02");
// #endif        
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
                int32_t debug = uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE2 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);
    // #ifndef NDEBUGs
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 u_strlen(dest)=%ld", u_strlen(dest)));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 sPos-s=%ld", sPos-s));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 sPos=%ld", sPos));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 s=%ld", s));
    // #endif
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                ku.k_blocked_pid = SVID_NONE_BLOCKED;
    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_2 02-1 uregex_group=%i"), (int)debug);
    // #endif
                InsertKeywordUnit(p_keywordList, &ku);
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
        wxString ErrMsg = wxString::Format(wxT("RE2 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_2 end");
// #endif
    return U_ZERO_ERROR;
}

/*
 * Regular Expression matching specified keyword.
 */
// !! THE SAME PROCESSING RULES WITH svLineText::REMatchPattern_3 
// NEEDING TO BE UPDATE TOGETHER!! 
UErrorCode svBufText::REMatchKeyword_3(const UChar *ibuf, int32_t ibufSize, const char *pattern, const unsigned int p_pid, const int32_t p_regroup, vector<keywordUnit> *p_hintList)
{
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_3 start");
//     wxLogMessage(wxString::FromAscii(pattern));
// #endif
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
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_3 01");
// #endif
    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
// #ifndef NDEBUG
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 uregex_groupCount return value is %i, p_regroup=%i", gCnt, p_regroup));
// #endif
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("RE3 Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);  
        }
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_3 02");
// #endif
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
                int32_t debug = uregex_group(re, i, dest, sPos-s, &status);
                if (U_FAILURE(status))
                {
                    // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
                    wxString ErrMsg = wxString::Format(wxT("RE3 Seq-06 error(uregex_groupt):%i"), status);
                    wxLogMessage(ErrMsg); 
                }   
                // fprintf(stdout, "Regex Match:");
                // u_printf_u_len(dest, sPos-s);
                // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);

    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 u_strlen(dest)=%ld", u_strlen(dest)));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 sPos-s=%ld", sPos-s));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 sPos=%ld", sPos));
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 s=%ld", s));
    // #endif            
                keywordUnit ku;
                // ku.k_text = dest;
                ku.k_text = (UChar *)malloc(sizeof(UChar) * (sPos-s));
                u_memcpy(ku.k_text, dest, (sPos-s));
                ku.k_pid = p_pid;
                ku.k_pos = s;
                ku.k_len = sPos - s;
                ku.k_blocked_pid = SVID_NONE_BLOCKED;
    // #ifndef NDEBUG
    //     wxLogMessage(wxString::Format("svBufText::REMatchPattern_3 02-1 uregex_group=%i"), (int)debug);
    // #endif 
                InsertHintUnit(p_hintList, &ku);
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
        wxString ErrMsg = wxString::Format(wxT("RE3 Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);  
    }

    uregex_close(re);

    if (upattern) free(upattern);
// #ifndef NDEBUG
//     wxLogMessage("svBufText::REMatchPattern_3 end");
// #endif
    return U_ZERO_ERROR;
}


/*
 * Regular Expression findng specified pattern.
 * save result location on p_locList.
 */
UErrorCode svBufText::REFind(const UChar *ibuf, int32_t ibufSize, const UChar *p_pattern, int32_t p_patternLen, vector<svIntPair> &p_locList, bool p_case)
{
    // Since we stored text file into UChar* buffer.
    // We do regex in the C way. Regex target is UChar*.
    
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;

    // URegularExpression *re = uregex_open(p_pattern, p_patternLen, UREGEX_DOTALL, &pe, &status); 
    URegularExpression *re = NULL;

    if (p_case)
    {
        re = uregex_open(p_pattern, p_patternLen, UREGEX_MULTILINE, &pe, &status); 
    }
    else
    {
        re = uregex_open(p_pattern, p_patternLen, UREGEX_MULTILINE | UREGEX_CASE_INSENSITIVE, &pe, &status);
    }
    
    if (U_FAILURE(status))
    {
        wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-01 error(uregex_open):%i"), status);
        wxLogMessage(ErrMsg);
        return status;
    }
    
    // fprintf(stdout, "ibufSize:%d\n", ibufSize); coold
    uregex_setText(re, ibuf, ibufSize, &status);
    if (U_FAILURE(status))
    {
        wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-02 error(uregex_setText):%i"), status);
        wxLogMessage(ErrMsg);
        return status;
    }

    int32_t sPos = 0;
    while(uregex_find(re, sPos, &status))
    {
        // uregex_groupCount() return 2 means RE found 3 groups which is 0, 1, 2
        int32_t gCnt = uregex_groupCount(re, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-03 error(uregex_groupCount):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-03 error(uregex_groupCount):%i"), status);
            wxLogMessage(ErrMsg);
            return status;
        }
        
        // we only process the regroup specified.
        // if (p_regroup<=gCnt)
        // { 
        //     for(int32_t i=p_regroup; i<=p_regroup; i++)
        //     {

        // int32_t s = uregex_start(re, 0, &status);
        int32_t s = uregex_start(re, 0, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-04 error(uregex_start):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-04 error(uregex_start):%i"), status);
            wxLogMessage(ErrMsg);
            return status;
        }
        int32_t endPos = uregex_end(re, 0, &status);
        if (U_FAILURE(status))
        {
            // fprintf(stdout, "RE Seq-05 error(uregex_end):%d\n", status);
            wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-05 error(uregex_end):%i"), status);
            wxLogMessage(ErrMsg);
            return status;
        }

        sPos = endPos;
        if (s==sPos)  // exception handle. In some situation it happen(like only ^ given).
        {
            ++sPos;
        }
        else
            p_locList.push_back(svIntPair(s, endPos));

        // UChar *dest = (UChar*)malloc(sizeof(UChar)*(sPos-s));
        // uregex_group(re, 0, dest, sPos-s, &status);
        // if (U_FAILURE(status))
        // {
        //     // fprintf(stdout, "RE Seq-06 error(uregex_groupt):%d\n", status);
        //     wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-06 error(uregex_groupt):%i"), status);
        //     wxLogMessage(ErrMsg);
        // }
        // fprintf(stdout, "Regex Match(g=%d):", i);
        // u_printf_u_len(dest, sPos-s);
        // fprintf(stdout, "@%d-%d(%d)\n", s, sPos, sPos-s);

// #ifndef NDEBUGs
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 i=%ld", i));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 u_strlen(dest)=%ld", u_strlen(dest)));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 sPos-s=%ld", sPos-s));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 sPos=%ld", sPos));
//     wxLogMessage(wxString::Format("svBufText::REMatchPattern_1 s=%ld", s));
// #endif

        // free(dest);

        //     }
        // }

    }
    if (U_FAILURE(status))
    {
        // fprintf(stdout, "RE Seq-07 error(regex_find):%d\n", status);
        wxString ErrMsg = wxString::Format(wxT("svBufText::RegexFind Seq-09 error(regex_find):%i"), status);
        wxLogMessage(ErrMsg);
        return status;
    }

    uregex_close(re);

    return U_ZERO_ERROR;
}

/*
 * ICU Convert from wxString to UChar*
 * We only cast it, and not sure if works in every situation.
 */ 
UErrorCode svBufText::ConvertwxStrToUChar(const wxString &p_str, UChar **p_ubuf, int32_t &p_ubufLen)
{
// #ifndef NDEBUG
//     wxLogMessage("svCommonLib::ConvertwxStrToUChar start");
// #endif

    const char *source;
    const char *sourceLimit;
    UChar *uBuf;
    UChar *target;
    UChar *targetLimit;
    int32_t uBufSize = 0;
    UConverter *conv = NULL;
    UErrorCode status = U_ZERO_ERROR;
    uint32_t total=0;

    const char *p_encoding = "UTF-8";

    wxCharBuffer b = p_str.ToUTF8();
    int m_bufferLen = strlen(b.data());

    char* m_buffer;
    m_buffer = (char *) malloc(sizeof(char) * (m_bufferLen+1));
    strncpy(m_buffer, (const char*)p_str.mb_str(wxConvUTF8), m_bufferLen);

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
    p_ubufLen = total;

    ucnv_close(conv);

    *p_ubuf = uBuf;

    free(m_buffer);

// #ifndef NDEBUG
//     wxLogMessage("svCommonLib::ConvertwxStrToUChar end");
// #endif

    return status;
}


/*
 * ICU Convert from UChar* to wxString
 * We only cast it, and not sure if it works in every situation.
 *
 * Please free p_ostr
 */ 
UErrorCode svBufText::ConvertUCharTowxStr(UChar *p_ubuf, int32_t p_ubufLen, wxString &p_str)
{
// #ifndef NDEBUG
//     wxLogMessage("svCommonLib::ConvertwxStrToUChar start");
// #endif

    char *target;
    int32_t targetLimit=0;
    int32_t targetLen=0;
    UErrorCode status = U_ZERO_ERROR;

    target = (char *) malloc(sizeof(char) * (p_ubufLen*2));
    targetLimit = p_ubufLen * 2;

    // ************************* start convert *****************************

    assert(target!=NULL);

    u_strToUTF8( target, targetLimit, &targetLen,
            &*p_ubuf, p_ubufLen, &status);

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

    if (target)
    {
        p_str = wxString::FromUTF8(target);
        free(target);
    }

// #ifndef NDEBUG
//     wxLogMessage("svCommonLib::ConvertwxStrToUChar end");
// #endif

    return status;
}



/*
 * Caret related functions.
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
/*int svBufTextView::GetDiffBufTextIndex(const bufTextIndex& p_oriIdx, int p_diff, bufTextIndex& p_newIdx)
{
    p_newIdx.unwrap_idx = p_oriIdx.unwrap_idx;
    p_newIdx.wrap_idx = p_oriIdx.wrap_idx;
    p_newIdx.wrap_desc.idx = p_oriIdx.wrap_desc.idx;
    p_newIdx.wrap_desc.len = p_oriIdx.wrap_desc.len;

    if (p_diff=0)
        return 0;

    int procLine = 0;

    int cnt=0;  // ÒÑÌŽÀí¹P”µ¡£ How many diff line processed.
    if (p_diff>0)
    {
        int uwi = p_oriIdx.unwrap_idx;
        size_t wlc = GetWrapLineCountAt(p_oriIdx.unwrap_idx);
        bool reachLimit = false;
        if ((wlc-1)>p_oriIdx.wrap_idx)
        {
            cnt = wlc - p_oriIdx.wrap_idx - 1;
        }
        while(cnt<p_diff)
        {
            ++uwi;
            if (uwi<=(int)(LineCntUW()-1))
            {
                // if (!IsStyledWrapProcessedAt(i))
                    ProcWrapAndStyleAt((size_t)uwi, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                wlc = GetWrapLineCountAt((size_t)uwi);
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
            GetWrapLenOnIdxAt(m_bufText->LineCntUW()-1, wlc-1, w);
            p_newIdx.unwrap_idx = LineCntUW()-1;
            p_newIdx.wrap_idx = wlc-1;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = cnt;
        }
        else if (cnt>=p_diff)
        {
            wrapLenDesc w;
            GetWrapLenOnIdxAt(uwi, cnt-p_diff, w);
            p_newIdx.unwrap_idx = uwi;
            p_newIdx.wrap_idx = cnt-p_diff;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = p_diff;
        }
        else
        {
            wxLogMessage(wxT("Call svBufText::GetDiffBufTextIndex() error: nor reachLimit neither cnt>=p_diff."));
        }
    }
    else // p_diff <0
    {
        int uwi = p_oriIdx.unwrap_idx;
        size_t wlc = GetWrapLineCountAt(p_oriIdx.unwrap_idx);
        bool reachLimit = false;

        cnt = p_oriIdx.wrap_idx;

        while(cnt<-p_diff)
        {
            --uwi;
            if (uwi>=0)
            {
                // if (!IsStyledWrapProcessedAt(i))
                    ProcWrapAndStyleAt((size_t)uwi, m_tabSize, m_spaceWidth, *m_font, m_maxPixelWidth);
                wlc = GetWrapLineCountAt((size_t)uwi);
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
            GetWrapLenOnIdxAt(0, 0, w);
            p_newIdx.unwrap_idx = 0;
            p_newIdx.wrap_idx = 0;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = cnt;
        }
        else if (cnt>=-p_diff)
        {
            wrapLenDesc w;
            GetWrapLenOnIdxAt(uwi, wlc-(cnt-p_diff)-1, w);
            p_newIdx.unwrap_idx = uwi;
            p_newIdx.wrap_idx = wlc-(cnt-p_diff)-1;
            p_newIdx.wrap_desc.idx = w.idx;
            p_newIdx.wrap_desc.len = w.len;
            procLine = -p_diff;
        }
        else
        {
            wxLogMessage(wxT("Call svBufText::GetDiffBufTextIndex() error: nor reachLimit neither cnt>=-p_diff."));
        }
    }

    return procLine;
}
*/

// Return svCaret list for carets in specified position range and out of the range.
// vector<svCaret> *p_inCaretList : caret list in the specified range.
// vector<svCaret> *p_outCaretList : caret list out of the specified range.
// vector<svCaret> *svBufText::GetCaretsOfRange(int32_t p_srow, int32_t p_scol, int32_t p_erow, int32_t p_ecol)
bool svBufText::GetCaretsOfRange(int32_t p_srow, int32_t p_scol, int32_t p_erow, int32_t p_ecol, vector<svCaret> **p_inCaretList, vector<svCaret> **p_outCaretList)
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

    // acquire caret on the textView.
    return m_carets.GetCaretsOfRange(p_srow, p_scol, p_erow, p_ecol, &(*p_inCaretList), &(*p_outCaretList));
}

// 單個caret往左移
// p_irow, p_icol 是未折行的caret位置
// p_orow, p_ocol 是回傳的新位置
// p_keepSpace 是回傳的保持位置
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 p_irow, p_icol 的合理性
void svBufText::SingleCaretMoveLeft(const int p_irow, const int p_icol, int &p_orow, int &p_ocol, int &p_keepSpace)
{
    int r, v_c;   // r : row   v_c : visual column
    p_orow = r = p_irow; p_ocol = v_c = p_icol;
    p_keepSpace = 0;

    if (VisibleAt(r)) // caret in a visible line.
    {
        int len = (int)GetLine(r)->TextLen(SVID_NO_CRLF);
        if (!v_c) // col==0
        {
            if (!r) //row == 0
            {
                p_orow = p_irow;
                p_ocol = p_icol;
            }
            else
            {
                int new_r=0;
                // if (GetPrevVisibleLine(r, new_r)) // Move to the previous visible line.
                new_r = r-1; // Move to the previous line even when it's a invisible line.
                {
                    p_orow = new_r;
                    p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
                }
            }
        }
        else
        {
            if (v_c>len)
            {
                p_orow = r;
                // p_ocol = len-1;
                p_ocol = len;    // 若col 大於行字串長度，則只左移到行尾
            }
            else
            {
                p_orow = r;
                p_ocol = v_c-1;
            }
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(p_orow, p_ocol, wrappedLine, k_wrappedCol);                    
        p_keepSpace = GetLine(p_orow)->Col2SpaceW(wrappedLine, k_wrappedCol);

    }
    else // caret in a invisible line.
    {
        int new_r = 0;
        if (GetPrevVisibleLine(r, new_r)) // Move to the previous visible line.
        {
            p_orow = new_r;
            p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
        }
        else // If no previous visible, move to the next visible line.
        {
            if (GetNextVisibleLine(r, new_r))
            {
                p_orow = new_r;
                p_ocol = 0;
            }
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(p_orow, p_ocol, wrappedLine, k_wrappedCol);                    
        p_keepSpace = GetLine(p_orow)->Col2SpaceW(wrappedLine, k_wrappedCol);

    }


    // // processing if caret in an invisible line occasionally.
    // // Exception catching.
    // if (!VisibleAt(r))
    // {
    //     int new_r = 0;
    //     if (GetPrevVisibleLine(r, new_r)) // Move to the previous visible line.
    //     {
    //         p_orow = new_r;
    //         p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
    //     }
    //     else // If no previous visible, move to the next visible line.
    //     {
    //         if (GetNextVisibleLine(r, new_r))
    //         {
    //             p_orow = new_r;
    //             p_ocol = 0;
    //         }
    //     }
    // }

}

// 單個caret往右移
// p_irow, p_icol 是未折行的caret位置
// p_orow, p_ocol 是回傳的新位置
// p_keepSpace 是回傳的保持位置
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 p_irow, p_icol 的合理性
void svBufText::SingleCaretMoveRight(const int p_irow, const int p_icol, int &p_orow, int &p_ocol, int &p_keepSpace)
{
    int r, v_c;    // r : row   v_c : visual column
    p_orow = r = p_irow; p_ocol = v_c = p_icol;
    p_keepSpace = 0;

    if (VisibleAt(r)) // caret in a visible line.
    {        
        int len = (int)GetLine(r)->TextLen(SVID_NO_CRLF);
        if (v_c>=len)  // In the end of the line
        {
            if (r==LineCntUW()-1) // in the end of last line
            {
                p_orow = p_irow;
                p_ocol = p_icol;
            }
            else
            {
                int new_r = 0;
                // if (GetNextVisibleLine(r, new_r)) // Move to next visible line.
                new_r = r + 1; // Move to the previous line even when it's a invisible line.
                {
                    p_orow = new_r;
                    p_ocol = 0;
                }
            }
        }
        else
        {
            p_orow = r;
            p_ocol = v_c+1;
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(p_orow, p_ocol, wrappedLine, k_wrappedCol);                    
        p_keepSpace = GetLine(p_orow)->Col2SpaceW(wrappedLine, k_wrappedCol);

    }
    else // caret in a invisible line.
    {
        int new_r = 0;
        if (GetNextVisibleLine(r ,new_r)) // Move to the next visible line
        {
            p_orow = new_r;
            p_ocol = 0;
        }
        else // If no next visible then Move to the previous visible line
        {
            if (GetPrevVisibleLine(r ,new_r))
            {
                p_orow = new_r;
                p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
            }                
        }

        // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
        int wrappedLine = 0;
        int k_wrappedCol = 0;

        InWhichWrappedLineColAt(p_orow, p_ocol, wrappedLine, k_wrappedCol);                    
        p_keepSpace = GetLine(p_orow)->Col2SpaceW(wrappedLine, k_wrappedCol);

    }


    // // processing if caret in an invisible line occasionally.
    // // Exception catching.
    // if (!VisibleAt(r))
    // {
    //     int new_r = 0;
    //     if (GetNextVisibleLine(r, new_r)) // Move to the next visible line
    //     {
    //         p_orow = new_r;
    //         p_ocol = 0;
    //     }
    //     else // If no next visible then Move to the previous visible line
    //     {
    //         if (GetPrevVisibleLine(r, new_r))
    //         {
    //             p_orow = new_r;
    //             p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
    //         }                
    //     }
    // }
}

// Move Caret Up
// 單個caret往上移
// p_irow, p_icol 是未折行的caret位置
// p_orow, p_ocol 是回傳的新位置
// caret 上下移動時不影響 keepSpace 值
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 p_irow, p_icol 的合理性
void svBufText::SingleCaretMoveUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                                  const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol, bool p_isWrap)
{
    int r, v_c;   // r : row   v_c : visual column
    p_orow = r = p_irow; p_ocol = v_c = p_icol;

#ifndef NDEBUG
    wxLogMessage(wxString::Format(wxT("Call svBufText::Carets log before: r=%i, vc=%i"), r, v_c));
#endif

    int len = (int)(GetLine(r)->TextLen(SVID_NO_CRLF));
    if (!IsStyledWrapProcessedAt(r))
    // if (!GetLine(r)->IsStyledWrapProcessed())
        ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth, p_isWrap);

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

    // 取回紀錄的等價的space值
    // spaceCount = it->GetKeepSpace();
    spaceCount = p_keepSpace;

    int wrapCount = GetLine(r)->m_wrapLenList.size();

    if (wrapCount==0)
    {
        // Error. wrapCount should > 0.
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsUp log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
    }
    else if (wrapCount==1 || wrappedLine==0)
    {
        // We have to move to the last wrapped line of the previous unwrapped line.
        // 移動到前一行的最後一個折行
        if (r>0)
        {
            int new_r=0;
            if (GetPrevVisibleLine(r, new_r))
            {
                r=new_r;
                if (!IsStyledWrapProcessedAt(r))
                // if (!GetLine(r)->IsStyledWrapProcessed())
                    ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth, p_isWrap);
                int wrapCount2 = GetLine(r)->m_wrapLenList.size();

                // 回推space值等值的col位置
                p_orow = r;                
                p_ocol = GetLine(p_orow)->Space2ColW(wrapCount2-1, spaceCount);
            }
        }            
    }
    else  // Move to the previous wrapped line. 移到前一個折行
    {
        // 回推space值等值的col位置
        p_orow = r;
        p_ocol = GetLine(p_orow)->Space2ColW(wrappedLine-1, spaceCount);
    }

    // // processing if caret in an invisible line occasionally.
    // // Exception catching.
    // if (!VisibleAt(r))
    // {
    //     int new_r = 0;
    //     if (GetPrevVisibleLine(r, new_r)) // Move to the previous visible line.
    //     {
    //         p_orow = new_r;
    //         p_ocol = (int)GetLine(p_orow)->TextLen(SVID_NO_CRLF);
    //     }
    //     else // If no previous visible, move to the next visible line.
    //     {
    //         if (GetNextVisibleLine(r ,new_r))
    //         {
    //             p_orow = new_r;
    //             p_ocol = 0;
    //         }
    //     }
    // }
}

// Move Caret Down
// 單個caret往下移
// p_irow, p_icol 是未折行的caret位置
// p_orow, p_ocol 是回傳的新位置
// caret 上下移動時不影響 keepSpace 值
// 呼叫本函式不改變任何值  No side effect 引用透明 (Referential Transparency)
// 不檢查 p_irow, p_icol 的合理性
void svBufText::SingleCaretMoveDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth,
                                    const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol, bool p_isWrap)
{

    int r, v_c;      // r : row   v_c : visual column
    p_orow = r = p_irow; p_ocol = v_c = p_icol;

// #ifndef NDEBUG
//     wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsDown log before: r=%i, vc=%i"), r, v_c));
// #endif

    int len = (int)GetLine(r)->TextLen(SVID_NO_CRLF);
    if (!IsStyledWrapProcessedAt(r))
    // if (!GetLine(r)->IsStyledWrapProcessed())
        ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth, p_isWrap);

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

    // 取回紀錄的等價的space值
    // spaceCount = it->GetKeepSpace();
    spaceCount = p_keepSpace;

    int wrapCount = GetLine(r)->m_wrapLenList.size();

    if (wrapCount==0)
    {
        // Error. wrapCount should > 0.
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsDown log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
    }
    else if (wrapCount==1 || wrappedLine>=wrapCount-1)
    {
        // We have to move to the last wrapped line of the previous unwrapped line.
        // 移動到下一行的第一個折行
        if (r<(int)LineCntUW()-1)
        {
            int new_r=0;
            if (GetNextVisibleLine(r, new_r))
            {
                r=new_r;
                if (!IsStyledWrapProcessedAt(r))
                // if (!GetLine(r)->IsStyledWrapProcessed())
                    ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth, p_isWrap);
                // 回推space值等值的col位置
                p_orow = r;
                p_ocol = GetLine(p_orow)->Space2ColW(0, spaceCount);
            }
        }  
    }
    else   // Move to the  wrapped line. 移到下一個折行
    {
        // 移動到下一個折行
        // 回推space值等值的col位置
        p_orow = r;
        p_ocol = GetLine(p_orow)->Space2ColW(wrappedLine+1, spaceCount);
    }



    // // processing if caret in an invisible line occasionally.
    // // Exception catching.
    // if (!VisibleAt(r))
    // {
    //     int new_r = 0;
    //     if (GetNextVisibleLine(r ,new_r)) // Move to the next visible line
    //     {
    //         p_orow = new_r;
    //         p_ocol = 0;
    //     }
    //     else // If no next visible then Move to the previous visible line
    //     {
    //         if (GetPrevVisibleLine(r, new_r))
    //         {
    //             p_orow = new_r;
    //             p_ocol = (int)(GetLine(p_orow)->TextLen(SVID_NO_CRLF));
    //         }                
    //     }
    // }

}

void svBufText::SingleCaretMoveTo(const int p_irow, const int p_icol, const int p_keepSpace, int &p_orow, int &p_ocol)
{
    // 要重計算 keep_space
    int a = 0;
}

void svBufText::CaretsLeft(void)
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
    m_carets.MergeOverlap();
}

void svBufText::CaretsLeftHead(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    // After CaretsLeft Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int keep_space=0;
        r=v_c=0;
        it->GetPosition(r, v_c);
        int hpos=0;
        if (GetLine(r)->GetKeywordPosition(v_c, SVID_BACKWARD, SVID_HEAD_OF_KEYWORD, hpos))
        {
            it->SetPosition(r, hpos);
            //ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r)->Col2SpaceUW(hpos));
        }
        else
        {
            // search previous line.
            if (r>0)
            {
                if (GetLine(r-1)->GetKeywordPosition(GetLine(r-1)->TextLen(SVID_NO_CRLF)+1, SVID_BACKWARD, SVID_HEAD_OF_KEYWORD, hpos))
                {
                    it->SetPosition(r-1, hpos);
                    ProcWrapAndStyleAt(r-1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                    it->SetKeepSpace(GetLine(r-1)->Col2SpaceUW(hpos));
                }
            }
        }
    }
    m_carets.MergeOverlap();
}

void svBufText::CaretsRight(void)
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
    m_carets.MergeOverlap();
}

void svBufText::CaretsRightEnd(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    // After CaretsRight Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        int keep_space=0;
        r=v_c=0;
        it->GetPosition(r, v_c);
        int hpos=0;
        if (GetLine(r)->GetKeywordPosition(v_c, SVID_FORWARD, SVID_TAIL_OF_KEYWORD, hpos))
        {
            it->SetPosition(r, hpos);
            //ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r)->Col2SpaceUW(hpos));
        }
        else
        {
            // search previous line.
            if (r+1<(int)LineCntUW())
            {
                if (GetLine(r+1)->GetKeywordPosition(-1, SVID_FORWARD, SVID_TAIL_OF_KEYWORD, hpos))
                {
                    it->SetPosition(r+1, hpos);
                    ProcWrapAndStyleAt(r+1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                    it->SetKeepSpace(GetLine(r+1)->Col2SpaceUW(hpos));
                }
            }
        }
    }
    m_carets.MergeOverlap();
}

// Move Caret Up
void svBufText::CaretsUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth, bool p_isWrap)
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
        SingleCaretMoveUp(p_tabSize, p_spaceWidth, p_font, p_maxPixelWidth, r, v_c, it->GetKeepSpace(), nr, nv_c, p_isWrap);
        it->SetPosition(nr, nv_c);
    }
    m_carets.MergeOverlap();
}

// Move Caret Down
void svBufText::CaretsDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth, bool p_isWrap)
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
        SingleCaretMoveDown(p_tabSize, p_spaceWidth, p_font, p_maxPixelWidth, r, v_c, it->GetKeepSpace(), nr, nv_c, p_isWrap);
        it->SetPosition(nr, nv_c);
    }
    m_carets.MergeOverlap();
}

// caret move to the head of line(wrapped line)
void svBufText::CaretsLineHead_W(void)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;    // r : row   v_c : visual column
        it->GetPosition(r, v_c);

        if (VisibleAt(r)) // caret in a visible line.
        {        
            // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
            int wrappedLine = 0;
            int k_wrappedCol = 0;
            int wrappedCnt = 0;;
            wrapLenDesc wd;

            InWhichWrappedLineColAt(r, v_c, wrappedLine, k_wrappedCol);
            // wrappedCnt = GetLine(r).GetWrapLineCount();
            GetLine(r)->GetWrapLenOnIdx(wrappedLine, wd);

            if (wrappedLine==0)
            {
                // In the first wrapped line.
                // Move to the first not space character or move to the first character.
                wxString text = GetLine(r)->GetTextUW();
                text.Trim(false);
                int spaceLen = GetLine(r)->GetTextUW().Length() - text.Length();
                if (k_wrappedCol<=spaceLen)
                {
                    // caret move to the first character.
                    it->SetPosition(r, wd.idx);
                    it->SetKeepSpace(0);
                }
                else
                {
                    // caret move to the first non space character.
                    it->SetPosition(r, spaceLen);
                    it->SetKeepSpace(GetLine(r)->Col2SpaceW(wrappedLine, spaceLen));
                }
            }
            else
            {
                // Move to the first position of the wrapped line.
                it->SetPosition(r, wd.idx);
                it->SetKeepSpace(0);
            }
        }
        else // caret in a invisible line.
        {
            // Doesn't process. The caret should be deleted.
        }
    }
    m_carets.MergeOverlap();
}


// caret move to the end of line(wrapped line)
void svBufText::CaretsLineEnd_W(void)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;    // r : row   v_c : visual column
        it->GetPosition(r, v_c);

        if (VisibleAt(r)) // caret in a visible line.
        {        
            int len = (int)GetLine(r)->TextLen(SVID_NO_CRLF);

            // 計算在第幾個折行及該折行內的第幾個column, 計算其space值
            int wrappedLine = 0;
            int k_wrappedCol = 0;
            int wrappedCnt = 0;;
            wrapLenDesc wd;

            InWhichWrappedLineColAt(r, v_c, wrappedLine, k_wrappedCol);
            wrappedCnt = GetLine(r)->GetWrapLineCount();
            GetLine(r)->GetWrapLenOnIdx(wrappedLine, wd);

            // Move to the last position of the wrapped line.
            if (wrappedLine==wrappedCnt-1)
            {
                // The last wrapped line of the unwrapped line.
                it->SetPosition(r, wd.idx+wd.len);
                it->SetKeepSpace(wd.len);
            }
            else
            {
                // not the last wrapped line of the unwrapped line.
                it->SetPosition(r, wd.idx+wd.len-1);
                it->SetKeepSpace(wd.len-1);
            }
        }
        else // caret in a invisible line.
        {
            // Doesn't processed. The caret should be deleted.
        }
    }
    m_carets.MergeOverlap();
}

// Remove carets in hidden line.
// BAD PRACTICE.
// In the first, I want this function to be a member function of svCaretList class.
// But svBufText.h already include svCaret.h
// It will cause cross including problem if I include svBufText.h in svCaret.h
// So, I move this function here. And also with a bad practice for OO design.
// Forgive me, PLEASE!
void svBufText::RemoveHiddenLineCarets(void)
{
    // BAD PRACTICE!!
    for(int i=(int)m_carets.m_caretsList.size()-1; i>0; i--)
    {
        if (!VisibleAt(m_carets.m_caretsList.at(i).GetRow()))
        {
            m_carets.m_caretsList.erase(m_carets.m_caretsList.begin()+i);
        }
    }
}



/*
void svBufText::CaretsLeft(void)
{
    // After CaretsLeft Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
        if (VisibleAt(r)) // caret in a visible line.
        {
            int len = (int)GetLine(r).TextLen(SVID_NO_CRLF);
            if (!v_c) // c==0
            {
                int new_r = GetPrevVisibleLine(r);
                if (new_r>=0) // r>0
                {
                    r = new_r;
                    v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                    it->SetPosition(r, v_c);
                }
            }
            else
            {
                if (v_c>len)
                {
                    it->SetPosition(r, --len);
                }
                else
                {
                    it->SetPosition(r, --v_c);
                }
            }

            // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column, Ó‹ËãÆäspaceÖµ
            int wrappedLine = 0;
            int k_wrappedCol = 0;

            InWhichWrappedLineColAt(r, it->GetVisualCol(), wrappedLine, k_wrappedCol);                    
            it->SetKeepSpace(GetLine(r).Col2SpaceW(wrappedLine, k_wrappedCol));

        }
        else // caret in a invisible line.
        {
            int new_r = GetPrevVisibleLine(r);
            if (new_r>=0) // Move to the previous visible line.
            {
                r = new_r;
                v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                it->SetPosition(r, v_c);
            }
            else // If no previous visible, move to the next visible line.
            {
                new_r = GetNextVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    it->SetPosition(r, 0);
                }
            }

            // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column, Ó‹ËãÆäspaceÖµ
            int wrappedLine = 0;
            int k_wrappedCol = 0;

            InWhichWrappedLineColAt(r, it->GetVisualCol(), wrappedLine, k_wrappedCol);                    
            it->SetKeepSpace(GetLine(r).Col2SpaceW(wrappedLine, k_wrappedCol));

        }


        // processing if caret in an invisible line occasionally.
        // Exception catching.
        if (!VisibleAt(it->GetRow()))
        {
            int new_r = GetPrevVisibleLine(r);
            if (new_r>=0) // Move to the previous visible line.
            {
                r = new_r;
                v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                it->SetPosition(r, v_c);
            }
            else // If no previous visible, move to the next visible line.
            {
                new_r = GetNextVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    it->SetPosition(r, 0);
                }
            }
        }
    }
}

void svBufText::CaretsRight(void)
{
    // After CaretsRight Call, caret position visualCol always equal to keepCol
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
        if (VisibleAt(r)) // caret in a visible line.
        {        
            int len = (int)GetLine(r).TextLen(SVID_NO_CRLF);
            if (v_c>=len)  // In the end of the line
            {
                int new_r = GetNextVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    it->SetPosition(r, 0);
                }
            }
            else
            {
                it->SetPosition(r, ++v_c);
            }

            // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column, Ó‹ËãÆäspaceÖµ
            int wrappedLine = 0;
            int k_wrappedCol = 0;

            InWhichWrappedLineColAt(r, it->GetVisualCol(), wrappedLine, k_wrappedCol);                    
            it->SetKeepSpace(GetLine(r).Col2SpaceW(wrappedLine, k_wrappedCol));

        }
        else // caret in a invisible line.
        {
            int new_r = GetNextVisibleLine(r);
            if (new_r>=0) // Move to the next visible line
            {
                r = new_r;
                it->SetPosition(r, 0);
            }
            else // If no next visible then Move to the previous visible line
            {
                new_r = GetPrevVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                    it->SetPosition(r, v_c);
                }                
            }

            // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column, Ó‹ËãÆäspaceÖµ
            int wrappedLine = 0;
            int k_wrappedCol = 0;

            InWhichWrappedLineColAt(r, it->GetVisualCol(), wrappedLine, k_wrappedCol);                    
            it->SetKeepSpace(GetLine(r).Col2SpaceW(wrappedLine, k_wrappedCol));

        }


        // processing if caret in an invisible line occasionally.
        // Exception catching.
        if (!VisibleAt(it->GetRow()))
        {
            int new_r = GetNextVisibleLine(r);
            if (new_r>=0) // Move to the next visible line
            {
                r = new_r;
                it->SetPosition(r, 0);
            }
            else // If no next visible then Move to the previous visible line
            {
                new_r = GetPrevVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                    it->SetPosition(r, v_c);
                }                
            }
        }
        
    }
}

// Move Caret Up
void svBufText::CaretsUp(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svBufText::Carets log before: r=%i, vc=%i"), r, v_c));
#endif

        int len = (int)GetLine(r).TextLen(SVID_NO_CRLF);
        if (!IsStyledWrapProcessedAt(r))
            ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);

        // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column
        int wrappedLine = 0;
        int v_wrappedCol = 0;

        InWhichWrappedLineColAt(r, v_c, wrappedLine, v_wrappedCol);
        if (wrappedLine<0) wrappedLine = 0;

        int spaceCount = 0;        // µÈÖµµÄspace”µÁ¿

        // That means we move from a long line into a short line.
        // Then move from the short line into a long line.
        // We have to keep the column value so that it will move to 
        // correct position when we move from short to long line.

        // È¡»Ø¼oä›µÄµÈƒrµÄspaceÖµ
        spaceCount = it->GetKeepSpace();

        int wrapCount = GetLine(r).m_wrapLenList.size();

        if (wrapCount==0)
        {
            // Error. wrapCount should > 0.
#ifndef NDEBUG
            wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsUp log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
        }
        else if (wrapCount==1 || wrappedLine==0)
        {
            // We have to move to the last wrapped line of the previous unwrapped line.
            // ÒÆ„Óµ½Ç°Ò»ÐÐµÄ×îááÒ»‚€ÕÛÐÐ
            if (r>0)
            {
                --r;
                if (!IsStyledWrapProcessedAt(r))
                    ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
                int wrapCount2 = GetLine(r).m_wrapLenList.size();

                // »ØÍÆspaceÖµµÈÖµµÄcolÎ»ÖÃ
                int nv_c = GetLine(r).Space2ColW(wrapCount2-1, spaceCount);
                it->SetPosition(r, nv_c);
            }            
        }
        else  // Move to the previous wrapped line. ÒÆµ½Ç°Ò»‚€ÕÛÐÐ
        {
            // »ØÍÆspaceÖµµÈÖµµÄcolÎ»ÖÃ
            int nv_c = GetLine(r).Space2ColW(wrappedLine-1, spaceCount);
            it->SetPosition(r, nv_c);
        }

        // processing if caret in an invisible line occasionally.
        // Exception catching.
        if (!VisibleAt(it->GetRow()))
        {
            int new_r = GetPrevVisibleLine(r);
            if (new_r>=0) // Move to the previous visible line.
            {
                r = new_r;
                v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                it->SetPosition(r, v_c);
            }
            else // If no previous visible, move to the next visible line.
            {
                new_r = GetNextVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    it->SetPosition(r, 0);
                }
            }
        }
    }
}

// Move Caret Down
void svBufText::CaretsDown(const size_t p_tabSize, const int p_spaceWidth, const wxFont *p_font, const int p_maxPixelWidth)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
#ifndef NDEBUG
        wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsDown log before: r=%i, vc=%i"), r, v_c));
#endif

        int len = (int)GetLine(r).TextLen(SVID_NO_CRLF);
        if (!IsStyledWrapProcessedAt(r))
            ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);

        // Ó‹ËãÔÚµÚŽ×‚€ÕÛÐÐ¼°Ô“ÕÛÐÐƒÈµÄµÚŽ×‚€column
        int wrappedLine = 0;
        int v_wrappedCol = 0;

        InWhichWrappedLineColAt(r, v_c, wrappedLine, v_wrappedCol);
        if (wrappedLine<0) wrappedLine = 0;


        int spaceCount = 0;        // µÈÖµµÄspace”µÁ¿

        // That means we move from a long line into a short line.
        // Then move from the short line into a long line.
        // We have to keep the column value so that it will move to 
        // correct position when we move from short to long line.

        // È¡»Ø¼oä›µÄµÈƒrµÄspaceÖµ
        spaceCount = it->GetKeepSpace();

        int wrapCount = GetLine(r).m_wrapLenList.size();

        if (wrapCount==0)
        {
            // Error. wrapCount should > 0.
#ifndef NDEBUG
            wxLogMessage(wxString::Format(wxT("Call svBufText::CaretsDown log error(m_wrapLenList.size()==0) at UW line=%i"), r));
#endif
        }
        else if (wrapCount==1 || wrappedLine>=wrapCount-1)
        {
            // We have to move to the last wrapped line of the previous unwrapped line.
            // ÒÆ„Óµ½ÏÂÒ»ÐÐµÄµÚÒ»‚€ÕÛÐÐ
            if (r<(int)m_buftext.size()-1)
            {
                ++r;
                if (!IsStyledWrapProcessedAt(r))
                    ProcWrapAndStyleAt(r, p_tabSize, p_spaceWidth, *p_font, p_maxPixelWidth);
                // »ØÍÆspaceÖµµÈÖµµÄcolÎ»ÖÃ
                int nv_c = GetLine(r).Space2ColW(0, spaceCount);
                it->SetPosition(r, nv_c);
            }  
        }
        else   // Move to the  wrapped line. ÒÆµ½ÏÂÒ»‚€ÕÛÐÐ
        {
            // ÒÆ„Óµ½ÏÂÒ»‚€ÕÛÐÐ
            // »ØÍÆspaceÖµµÈÖµµÄcolÎ»ÖÃ
            int nv_c = GetLine(r).Space2ColW(wrappedLine+1, spaceCount);
            it->SetPosition(r, nv_c); 
        }


        // processing if caret in an invisible line occasionally.
        // Exception catching.
        if (!VisibleAt(it->GetRow()))
        {
            int new_r = GetNextVisibleLine(r);
            if (new_r>=0) // Move to the next visible line
            {
                r = new_r;
                it->SetPosition(r, 0);
            }
            else // If no next visible then Move to the previous visible line
            {
                new_r = GetPrevVisibleLine(r);
                if (new_r>=0)
                {
                    r = new_r;
                    v_c = (int)GetLine(r).TextLen(SVID_NO_CRLF);
                    it->SetPosition(r, v_c);
                }                
            }
        }

    }
}
*/

void svBufText::GetCaretData(int idx, int &row, int &vcol, int &kspace)
{
    row = m_carets.At(idx)->GetRow();
    vcol = m_carets.At(idx)->GetVisualCol();
    kspace = m_carets.At(idx)->GetKeepSpace();
}

void svBufText::ClearCaretSelect(void)
{
    for(int i=0; i<(int)m_carets.Size(); ++i)
    {
        m_carets.At(i)->ClearSelect();
    }
}

void svBufText::SetCaretSelect(void)
{
    for(int i=0; i<(int)m_carets.Size(); ++i)
    {
        m_carets.At(i)->SetSelect();
    }
}

/*
 * 給定一個區間的行座標位置(起始row, col 及 結束 row, col)，
 * 及一個指定行(row)，回傳該指定行在給定區間內的範圍
 *
 * 也就是計算交集
 */
bool svBufText::get_only_the_line_position(const int p_baseRow, int &p_scol, int &p_ecol, bool &p_eol, const int p_startRow, const int p_startCol, const int p_endRow, const int p_endCol)
{
    p_eol = false;   // 是否包含行尾的 eol

    if (p_baseRow<p_startRow || p_baseRow>p_endRow)
        return false;

    // p_baseRow 包含在 p_startRow 及 p_endRow之間
    // 指定行在區間內(區間有多行)
    if (p_baseRow>p_startRow && p_baseRow<p_endRow)
    {
        p_scol = 0;
        p_ecol = GetLine(p_baseRow)->TextLen(SVID_NO_CRLF);
        p_eol = true;
        return true;
    }

    if (p_startRow!=p_endRow) //(區間有多行)
    {
        if (p_baseRow==p_startRow) // 指定行在區間第一行
        {
            p_scol = p_startCol;
            p_ecol = GetLine(p_baseRow)->TextLen(SVID_NO_CRLF);
            p_eol = true;
            return true;
        }
        else if (p_baseRow==p_endRow) // 指定行在區間最後一行
        {
            p_scol = 0;
            p_ecol = p_endCol;
            p_eol = false;
            return true;
        }
    }
    else // p_startRow==p_endRow  區間只有一行
    {
        if (p_baseRow==p_startRow)
        {
            p_scol = p_startCol;
            p_ecol = p_endCol;
            p_eol = false;
            return true;
        }
    }

    return false;

}

/*
 * 給定一個區間的行座標位置(起始row, col 及 結束 row, col)，
 * 及一個指定行(row) 及其起始、結束 col，回傳該指定行在給定區間內的範圍
 *
 * 也就是計算交集，一律不計行尾符號
 */
bool svBufText::get_only_the_line_position(const int p_baseRow, const int p_basescol, const int p_baseecol, int &p_scol, int &p_ecol, bool &p_eol, const int p_startRow, const int p_startCol, const int p_endRow, const int p_endCol)
{
    p_eol = false;   // 是否包含行尾的 eol
    int t_scol, t_ecol;
    t_scol=t_ecol=0;

    if (p_baseRow<p_startRow || p_baseRow>p_endRow)
        return false;

    // p_baseRow 包含在 p_startRow 及 p_endRow之間
    // 指定行在區間內(區間有多行)
    if (p_baseRow>p_startRow && p_baseRow<p_endRow)
    {
        t_scol = 0;
        t_ecol = GetLine(p_baseRow)->TextLen(SVID_NO_CRLF);
        int t_xscol = 0;
        int t_xecol = 0;
        bool result = get_position_and(p_basescol, p_baseecol, t_scol, t_ecol, t_xscol, t_xecol);

        if (result)
        {
            p_scol = t_xscol;
            p_ecol = t_xecol;
            p_eol = false;
        }
        return result;
    }

    if (p_startRow!=p_endRow) //(區間有多行)
    {
        if (p_baseRow==p_startRow) // 指定行在區間第一行
        {
            t_scol = p_startCol;
            t_ecol = GetLine(p_baseRow)->TextLen(SVID_NO_CRLF);
            int t_xscol = 0;
            int t_xecol = 0;
            bool result = get_position_and(p_basescol, p_baseecol, t_scol, t_ecol, t_xscol, t_xecol);

            if (result)
            {
                p_scol = t_xscol;
                p_ecol = t_xecol;
                p_eol = false;
            }
            return result;
        }
        else if (p_baseRow==p_endRow) // 指定行在區間最後一行
        {
            t_scol = 0;
            t_ecol = p_endCol;
            int t_xscol = 0;
            int t_xecol = 0;
            bool result = get_position_and(p_basescol, p_baseecol, t_scol, t_ecol, t_xscol, t_xecol);

            if (result)
            {
                p_scol = t_xscol;
                p_ecol = t_xecol;
                p_eol = false;
            }
            return result;
        }
    }
    else // p_startRow==p_endRow  區間只有一行
    {
        if (p_baseRow==p_startRow)
        {
            t_scol = p_startCol;
            t_ecol = p_endCol;
            int t_xscol = 0;
            int t_xecol = 0;
            bool result = get_position_and(p_basescol, p_baseecol, t_scol, t_ecol, t_xscol, t_xecol);

            if (result)
            {
                p_scol = t_xscol;
                p_ecol = t_xecol;
                p_eol = false;
            }
            return result;
        }
    }

    return false;

}


// 給定 兩個起啟及結束 col 值，計算其是否有交集
bool svBufText::get_position_and(const int p_scol1, const int p_ecol1, const int p_scol2, const int p_ecol2, int &p_xscol, int &p_xecol)
{

    if (p_scol1>=p_scol2 && p_ecol1<=p_ecol2)
    {
        p_xscol = p_scol1;
        p_xecol = p_ecol1;
        return true;
    }
    else if (p_scol1<p_scol2 && p_ecol1>p_ecol2)
    {
        p_xscol = p_scol2;
        p_xecol = p_ecol2;
        return true;
    }
    else if (p_scol1>=p_scol2 && p_scol1<p_ecol2)
    {
        p_xscol = p_scol1;
        p_xecol = p_ecol2;
        return true;
    }
    else if (p_ecol1>p_scol2 && p_ecol1<=p_ecol2)
    {
        p_xscol = p_scol2;
        p_xecol = p_ecol1;
        return true;
    }

    return false;

}


/*bool svBufText::get_line_comment(wxString &p_lineComment)
{
    for (std::vector<re_rule>::iterator it=m_synReRules.begin(); it != m_synReRules.end(); ++it)
    {
        if (it->type==1) // line comment. only the first line comment will be return.
        {
            p_lineComment = wxString::FromUTF8(it->cm1);
            return true;
        }
    }

    return false;
}*/

bool svBufText::GetLineCommentSymbol(wxString &p_lineComment)
{
    for (std::vector<re_rule>::iterator it=m_synReRules.begin(); it != m_synReRules.end(); ++it)
    {
        if (it->type==1) // line comment. only the first line comment will be return.
        {
            p_lineComment = wxString::FromUTF8(it->cm1);
            return true;
        }
    }

    return false;
}

/*bool svBufText::get_block_comment(wxString &p_startComment, wxString &p_endComment)
{
    for (std::vector<re_rule>::iterator it=m_synReRules.begin(); it != m_synReRules.end(); ++it)
    {
        if (it->type==0) // block comment. only the first block comment will be return.
        {
            p_startComment = wxString::FromUTF8(it->cm1);
            p_endComment = wxString::FromUTF8(it->cm2);
            return true;
        }
    }
    return false;
}*/

bool svBufText::GetBlockCommentSymbol(wxString &p_startComment, wxString &p_endComment)
{
    for (std::vector<re_rule>::iterator it=m_synReRules.begin(); it != m_synReRules.end(); ++it)
    {
        if (it->type==0) // block comment. only the first block comment will be return.
        {
            p_startComment = wxString::FromUTF8(it->cm1);
            p_endComment = wxString::FromUTF8(it->cm2);
            return true;
        }
    }
    return false;
}

// uw_idx 行數
// w_idx_s ~ w_idx_e 哪個字元到哪個字元
// 所以處理的實際上是一個折行
bool svBufText::GetCaretSelectPixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList)
{
    // 將找到的像素清單存放在 vector 內
    // 為何要用 vector 存放資料呢?
    // 因為有多個 carets，所以同一行可能有多段文字放標示了
    // p_pixelList 的 size() 一律是偶數，奇數值是啟始的pixel值，偶數值是結束的pixel值
    // 如為跨行標示則尾段固定 + p_spaceWidth 以表示eol

    p_pixelList.clear();
    bool founded = false;
    int startPixel = 0;
    int endPixel = 0;

    for(int i=0; i<(int)m_carets.Size(); ++i)
    {
        int sr, sc, er, ec;
        sr = sc = er = ec = 0;
        if (m_carets.At(i)->HasSelect())
        {
            // 判段 uw_idx 這行的 w_idx_s 至 w_idx_e column 區間是否在 caret selected 游標標示區間內
            m_carets.At(i)->GetSelectPos(sr, sc, er, ec);

            int tr = uw_idx;
            int tsc, tec;
            tsc = tec = 0; // 啟始及結束 column
            int idx_s, idx_e;
            idx_s = idx_e = 0;
            bool with_eol = false;
            if (get_only_the_line_position(tr, tsc, tec, with_eol, sr, sc, er, ec))
            {
                idx_s = w_idx_s;
                idx_e = w_idx_e;
                if (idx_s<=tsc && idx_e>=tec)
                {
                    // 計算 tsc 到 tec 的 pixel 值
                    int isc = tsc;   // 最後的起始 column 位置
                    int ise = tec;   // 最後的結束 column 位置
                    GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                    if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                    foldingInfo fi;
                    if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                    p_pixelList.push_back(startPixel);
                    p_pixelList.push_back(endPixel);
                    founded = true;
                }
                else if (idx_s>=tsc && idx_e<=tec)
                {
                    // 計算 tsc 到 tec 的 pixel 值
                    int isc = idx_s;   // 最後的起始 column 位置
                    int ise = idx_e;   // 最後的結束 column 位置
                    GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                    if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                    foldingInfo fi;
                    if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                    p_pixelList.push_back(startPixel);
                    p_pixelList.push_back(endPixel);
                    founded = true;
                }
                // else if (idx_s<tsc && idx_e>=tsc && idx_e<=tec)
                else if (idx_s<tsc && idx_e>tsc && idx_e<=tec)
                {
                    // 計算 tsc 到 tec 的 pixel 值
                    int isc = tsc;       // 最後的起始 column 位置
                    int ise = idx_e;     // 最後的結束 column 位置
                    GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                    if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                    foldingInfo fi;
                    if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                    p_pixelList.push_back(startPixel);
                    p_pixelList.push_back(endPixel);
                    founded = true;
                }
                else if (idx_s>=tsc && idx_s<=tec && idx_e>tec)
                {
                    // 計算 tsc 到 tec 的 pixel 值
                    // int wsc = GetLine(uw_idx)->GetWrappedLineStartColOnPosition(idx_s);  // 該折行第一個 col 值
                    int isc = idx_s;     // 最後的起始 column 位置
                    int ise = tec;       // 最後的結束 column 位置
                    GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                    if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                    foldingInfo fi;
                    if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                    p_pixelList.push_back(startPixel);
                    p_pixelList.push_back(endPixel);
                    founded = true;
                }
            }
        }
    }  

    return founded;  

}

// uw_idx 行數
// w_idx_s ~ w_idx_e 哪個字元到哪個字元
// 所以處理的實際上是一個折行
// 與 GetCaretSelectPixelAtW 相同 邏輯, 唯 caret 資料由參數傳入
bool svBufText::GetTextRangePixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList, const vector<textRange> p_rangeText)
{
    // 將找到的像素清單存放在 vector 內
    // 為何要用 vector 存放資料呢?
    // 因為有多個 carets，所以同一行可能有多段文字放標示了
    // p_pixelList 的 size() 一律是偶數，奇數值是啟始的pixel值，偶數值是結束的pixel值

    p_pixelList.clear();
    bool founded = false;
    int startPixel = 0;
    int endPixel = 0;

    for(int i=0; i<(int)p_rangeText.size(); ++i)
    {
        int sr, sc, er, ec;
        sr = sc = er = ec = 0;

        // 判段 uw_idx 這行的 w_idx_s 至 w_idx_e column 區間是否在 caret selected 游標標示區間內
        sr = p_rangeText.at(i).start_row;
        sc = p_rangeText.at(i).start_col;
        er = p_rangeText.at(i).end_row;
        ec = p_rangeText.at(i).end_col;

        int tr = uw_idx;
        int tsc, tec;
        tsc = tec = 0; // 啟始及結束 column
        int idx_s, idx_e;
        idx_s = idx_e = 0;
        bool with_eol = false;
        if (get_only_the_line_position(tr, tsc, tec, with_eol, sr, sc, er, ec))
        {
            idx_s = w_idx_s;
            idx_e = w_idx_e;
            if (idx_s<=tsc && idx_e>=tec)
            {
                // 計算 tsc 到 tec 的 pixel 值
                int isc = tsc;   // 最後的起始 column 位置
                int ise = tec;   // 最後的結束 column 位置
                GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                // if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                // foldingInfo fi;
                // if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                p_pixelList.push_back(startPixel);
                p_pixelList.push_back(endPixel);
                founded = true;
            }
            else if (idx_s>=tsc && idx_e<=tec)
            {
                // 計算 tsc 到 tec 的 pixel 值
                int isc = idx_s;   // 最後的起始 column 位置
                int ise = idx_e;   // 最後的結束 column 位置
                GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                // if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                // foldingInfo fi;
                // if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                p_pixelList.push_back(startPixel);
                p_pixelList.push_back(endPixel);
                founded = true;
            }
            // else if (idx_s<tsc && idx_e>=tsc && idx_e<=tec)
            else if (idx_s<tsc && idx_e>tsc && idx_e<=tec)
            {
                // 計算 tsc 到 tec 的 pixel 值
                int isc = tsc;       // 最後的起始 column 位置
                int ise = idx_e;     // 最後的結束 column 位置
                GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                // if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                // foldingInfo fi;
                // if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                p_pixelList.push_back(startPixel);
                p_pixelList.push_back(endPixel);
                founded = true;
            }
            else if (idx_s>=tsc && idx_s<=tec && idx_e>tec)
            {
                // 計算 tsc 到 tec 的 pixel 值
                int isc = idx_s;     // 最後的起始 column 位置
                int ise = tec;       // 最後的結束 column 位置
                GetLine(uw_idx)->TextCol2PixelW(isc, ise, startPixel, endPixel);
                // if (with_eol) endPixel += p_spaceWidth;  // 如包含行尾eol則加上20表示為eol字元
                // foldingInfo fi;
                // if (GetLine(uw_idx)->HadFolding(fi)) endPixel += p_spaceWidth;  // 如包含行尾folding tail則再加上20表示為eol字元
                p_pixelList.push_back(startPixel);
                p_pixelList.push_back(endPixel);
                founded = true;
            }
        }
        
    }  

    return founded;  

}

// return the pixel location (x, y) for the last caret
// Move to svTextView
/*bool svBufText::GetLastCaretPixelXY(int &p_x, int &p_y)
{
    int o_row, o_col;
    o_row = o_col = 0;

    m_carets.GetLastCaret()->GetPosition(o_row, o_col);

    TextCol2PixelXAt( o_row, o_col, p_y);

    return true;

}*/

bool svBufText::CaretsHasSelect(void)
{
    return m_carets.HasSelect();
}

// 將 carets 內的游標清空，並依據傳入的位置新增一個新游標
void svBufText::ResetCaretPosition(const size_t p_row, const size_t p_col)
{
    m_carets.Clear();
    m_carets.Append(svCaret(p_row, p_col));
}

// 依據傳入的位置新增一個新游標
void svBufText::AppendCaretPosition(const size_t p_row, const size_t p_col)
{
    m_carets.Append(svCaret(p_row, p_col));
}

// 依據傳入的位置新增一個新游標
void svBufText::LastCaretMoveTo(const size_t p_row, const size_t p_col)
{
    int o_row, o_col;
    o_row = o_col = 0;
    int n_row, n_col;
    n_row=n_col=0;
    int n_keep_space;
    n_keep_space = 0;

    m_carets.GetLastCaret()->GetPosition(o_row, o_col);
    n_row = p_row;
    n_col = p_col;
    SingleCaretMoveTo(o_row, o_col, n_keep_space, n_row, n_col);
    m_carets.GetLastCaret()->SetPosition(n_row, n_col);
    m_carets.GetLastCaret()->SetKeepSpace(n_keep_space);
}

// 將 carets 選擇 p_length 個字元，往前數
// do no exception check.
void svBufText::CaretsSelectCharacters(int p_length)
{
    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;    // r : row   v_c : visual column
        it->GetPosition(r, v_c);

        // it->SetSelect();
        int len = p_length;
        if (v_c-len>=0)
        {
            it->SetSelect(r, v_c-len, v_c-len);
        }
        else
        {
            it->SetSelect(r, 0, 0);
        }
        // it->SetPosition(r, v_c+len);
        // svUndoAction act(SVID_UNDO_NOTHING, r, v_c, "");
        // m_undoActions.Add2TheLast(act);
    }

}

bool svBufText::CaretsMergeOverlap(void)
{
    if (m_carets.MergeOverlap())
    {
        // 重算 keepSpace
        int r, v_c;    // r : row   v_c : visual column

        svCaret* last = m_carets.GetLastCaret();
        last->GetPosition(r, v_c);
        last->SetKeepSpace(GetLine(r)->Col2SpaceUW(v_c));
        return true;
    }
    return false;
}

// 如果所有的 carets 所在的位置有相同的 hint keyword 則 return true
// 並將資料存放在 p_curText 及 p_len
// 否則回傳 false
// PLEASE free(p_curText) AFTER CALL THIS FUNCTION.
bool svBufText::GetCaretsCurrentUChar(UChar **p_curText, int &p_len, int &p_offset)
{
    UChar *ou_text = NULL;
    int ou_len = 0;
    int ou_offset = 0;
    int index = 0;

    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        int r, v_c;    // r : row   v_c : visual column
        it->GetPosition(r, v_c);

        if (VisibleAt(r)) // caret in a visible line.
        {
            UChar *u_text=NULL;
            int u_len=0;
            int u_offset=0;
            if (GetLine(r)->GetPositionHint(v_c, &u_text, u_len, u_offset))
            {
                if (index) // index != 0;
                {
                    if (u_len!=ou_len || 
                        u_memcmp(u_text, ou_text, ou_len)!=0)
                    {
                        if (u_text) free(u_text);
                        if (ou_text) free(ou_text);
                        return false;
                    }
                }
                if (ou_text) free(ou_text);
                ou_text = (UChar *)malloc(sizeof(UChar) * u_len);
                u_memcpy(ou_text, u_text, u_len);
                if (u_text) free(u_text);
                ou_len = u_len;
                ou_offset = u_offset;
            }
            ++index;
        }
    }

    if (ou_text)
    {
        *p_curText = ou_text;
        p_len = ou_len;
        p_offset = ou_offset;
        return true;
    }
    else
        return false;
}

// 計算 caret location uchar length.
bool svBufText::GetCaretsCurrentUCharLen(int &p_len, int &p_offset)
{
    UChar *u_text=NULL;
    int u_len=0;
    int u_offset=0;

    if (GetCaretsCurrentUChar(&u_text, u_len, u_offset))
    {
        if (u_text) free(u_text);
        p_len = u_len;
        p_offset = u_offset;

        return true;
    }
    else
    {
        p_len = 0;
        return false;
    }

}

void svBufText::UpdateAvailableHint(void)
{
    UChar *u_text=NULL;
    int u_len=0;
    int u_offset=0;

    m_availableHint.clear();

    if (GetCaretsCurrentUChar(&u_text, u_len, u_offset))
    {
        m_availableHint = m_hintDict.GetHitTextList(u_text, u_len);
        if (u_text) free(u_text);
    }
}

// // There should be only 1 caret available when calling this fucntion.
// // p_newRow will be set to the location keyword found.
// bool svBufText::CaretMoveToKeywordUChar(const UCharText &p_keyword, const char p_direction, int &p_newRow)
// {
//     for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
//         it!=m_carets.m_caretsList.end();
//         ++it)
//     {
//         int r, v_c;    // r : row   v_c : visual column
//         it->GetPosition(r, v_c);

//         int frow, fcol;
//         frow=fcol=0;

//         bool found = false;

//         if (p_direction==SVID_FORWARD)
//             found = FindNextKeywordUCharFrom(p_keyword, r, v_c+1, frow, fcol);
//         else
//             found = FindPrevKeywordUCharFrom(p_keyword, r, v_c-1, frow, fcol);

//         if (found)
//         {
//             // move caret to frow, fcol+p_keyword.len
//             it->SetPosition(frow, fcol);
//             it->SetSelect();
//             it->SetPosition(frow, fcol+p_keyword.len);
//             it->SetKeepSpace(GetLine(frow)->Col2SpaceUW(fcol+p_keyword.len));
//             p_newRow = frow;
//             return true;
//         }
//         break;
//     }

//     return false;
// }

// // There should be only 1 caret available when calling this fucntion.
// // p_newRow will be set to the location keyword found.
// bool svBufText::CaretMoveToKeywordAllUChar(const UCharText &p_keyword, const char p_direction, int &p_newRow, int p_srow, int p_scol)
// {
//     bool found = false;
//     vector<int> locations;
//     int frow, fcol;
//     frow=fcol=0;

//     // 改由畫面第一個位置開始搜尋
//     // std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
//     // int r, v_c;    // r : row   v_c : visual column
//     // it->GetPosition(r, v_c);

//     found = FindAllKeywordUCharFrom(p_keyword, p_srow, p_scol, frow, fcol, locations);
//     // found = FindAllKeywordUCharFrom(p_keyword, r, v_c, frow, fcol, locations);
//     // found = FindAllKeywordUCharFrom(p_keyword, r, 0, frow, fcol, locations);

//     if (found)
//     {
//         // move caret to frow, fcol+p_keyword.len
//         p_newRow = frow;
//         m_carets.Clear();

//         for(std::vector<int>::iterator it=locations.begin();
//             it!=locations.end();
//             ++it)
//         {
//             // adding found locations into carets
//             int nr = *it;
//             ++it;
//             int nc = *it;
//             svCaret c(nr, nc);
//             c.SetSelect();
//             c.SetPosition(nr, nc+p_keyword.len);
//             c.SetKeepSpace(GetLine(nr)->Col2SpaceUW(nc+p_keyword.len));
//             m_carets.Append(c);
//             // 因為全域搜尋，某些行的 m_text 仍可能是 NULL，要判斷並計算 m_text <- 已改寫 svTextLine GetTextUW GetTextBeforeUW GetTextAfterUW
//             // GetLine(nr)->CheckTextUW(m_charSet);
//         }
//         return true;
//     }

//     return false;
// }

// // There should be only 1 caret available when calling this fucntion.
// // p_newRow will be set to the location keyword found.
// bool svBufText::CaretMoveToKeywordwxStr(const wxString &p_keyword, const char p_direction, int &p_newRow)
// {
//     for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
//         it!=m_carets.m_caretsList.end();
//         ++it)
//     {
//         int r, v_c;    // r : row   v_c : visual column
//         it->GetPosition(r, v_c);

//         int frow, fcol;
//         frow=fcol=0;

//         bool found = false;

//         if (p_direction==SVID_FORWARD)
//             found = FindNextKeywordwxStrFrom(p_keyword, m_frOption.m_case, r, v_c+1, frow, fcol);
//         else
//             found = FindPrevKeywordwxStrFrom(p_keyword, m_frOption.m_case, r, v_c-1, frow, fcol);

//         if (found)
//         {
//             // move caret to frow, fcol+p_keyword.len
//             it->SetPosition(frow, fcol);
//             it->SetSelect();
//             it->SetPosition(frow, fcol+p_keyword.Length());
//             it->SetKeepSpace(GetLine(frow)->Col2SpaceUW(fcol+p_keyword.Length()));
//             p_newRow = frow;
//             return true;
//         }
//         break;
//     }

//     return false;
// }

// // There should be only 1 caret available when calling this fucntion.
// // p_newRow will be set to the location keyword found.
// bool svBufText::CaretMoveToKeywordAllwxStr(const wxString &p_keyword, const char p_direction, int &p_newRow, int p_srow, int p_scol)
// {


//     bool found = false;
//     vector<int> locations;
//     int frow, fcol;
//     frow=fcol=0;

//     // 改由畫面第一個位置開始搜尋
//     // std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
//     // int r, v_c;    // r : row   v_c : visual column
//     // it->GetPosition(r, v_c);

//     found = FindKeywordAllwxStrFrom(p_keyword, m_frOption.m_case, p_srow, p_scol, frow, fcol, locations);
//     // found = FindKeywordAllwxStrFrom(p_keyword, r, v_c+1, frow, fcol, locations);
//     // found = FindKeywordAllwxStrFrom(p_keyword, r, 0, frow, fcol, locations);

//     if (found)
//     {
//         // move caret to frow, fcol+p_keyword.len
//         p_newRow = frow;
//         m_carets.Clear();

//         for(std::vector<int>::iterator it=locations.begin();
//             it!=locations.end();
//             ++it)
//         {
//             // adding found locations into carets
//             int nr = *it;
//             ++it;
//             int nc = *it;
//             svCaret c(nr, nc);
//             c.SetSelect();
//             c.SetPosition(nr, nc+p_keyword.Length());
//             c.SetKeepSpace(GetLine(nr)->Col2SpaceUW(nc+p_keyword.Length()));
//             m_carets.Append(c);
//             // 因為全域搜尋，某些行的 m_text 仍可能是 NULL，要判斷並計算 m_text <- 已改寫 svTextLine GetTextUW GetTextBeforeUW GetTextAfterUW
//             // GetLine(nr)->CheckTextUW(m_charSet);
//         }
//         return true;
//     }

//     return false;
// }


/* =======================================================================
 *
 *
 *
 * ======================================================================= */
bool svBufText::VisibleAt(size_t uw_idx)
{
    return GetLine(uw_idx)->Visible();
}

void svBufText::VisibleAt(size_t uw_idx, bool p_visible)
{
    if (uw_idx!=0) // Nevew let line 0 invisible.
    {
        GetLine(uw_idx)->Visible(p_visible);
    }
}

bool svBufText::GetPrevVisibleLine(size_t uw_idx, int &o_row)
{
    int idx = (int)uw_idx - 1;
    while(idx>=0 && !VisibleAt((size_t)idx))
    {
        idx--;
    }

    if (idx>=0)
    {
        o_row = idx;
        return true;
    }
    else
    {
        return false;
    }
}

bool svBufText::GetNextVisibleLine(size_t uw_idx, int &o_row)
{
    int idx = (int)uw_idx + 1;
    while(idx<(int)LineCntUW() && !VisibleAt((size_t)idx))
    {
        idx++;
    }

    if (idx>=(int)LineCntUW())
    {
        return false;
    }
    else
    {
        o_row = idx;
        return true;
    }
}

int svBufText::GetLastVisibleLine(void)
{
    int idx = (int)LineCntUW() - 1;
    while(idx >0 && !VisibleAt((size_t)idx))
    {
        idx--;
    }
    // if every line in buffer is not visibled. 0 will be returned.
    return idx;
}

/***********************************************************************/
/*                                                                     */
/*              Edit behavior related functions.                       */
/*                                                                     */
/***********************************************************************/

// EditingInsertChar is for insert wxString without newline symbol.
// p_cont : 是否為連續的 insert 動作
// return the length of the string inserted.
void svBufText::EditingInsertChar(const wxString &p_str, bool p_cont, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    int caretMoveRightCount = 0;  // How many column caret shoud move to the right.
    
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    extern svPreference g_preference;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)        
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
        
        vector<pairSymbol> v_ps = GetFoldingSymbol();

        wxString l_str;
        if (p_str==_("\t") && g_preference.GetAlignPrevLine())
        {
            // tab to space translate.
            // When tab is hit and caret is on col 0, insert spaces or tabs to move to the previous line non spaces col.
            // 當 tab 輸入，且 caret 位在 col 0, 則插入足夠多的 space 或 tab 以讓 caret 移至與上一行第一個非空白字元出現處。(如上一行行尾是 { 則再多一個tab(或等值的spaces))

            if (g_preference.GetTabToSpace())  // tab to space 
            {
                if (r>0 && v_c==0 && GetLine(r)->TextLen(SVID_NO_CRLF)==0) // Not the first line and caret in the fist column. And that line is a blank line.
                {
                    // Insert spaces and move to the previous line first non space column.
                    int pos;
                    pairSymbol ps;
                    int end_cnt=0;
                    
                    int spaceCnt = 0;
                    spaceCnt = GetLine(r-1)->FindFirstNonSpaceColUW();
                    spaceCnt = GetLine(r-1)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
                    
                    if (GetLine(r-1)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps))
                    {
                        // 前一行有 { 符號，再內縮一層
                        pos = GetLine(r-1)->Col2SpaceUW(pos);  // translate tab into spaces
                        spaceCnt = pos + g_preference.GetTabSize();
                    }
                    
                    if (spaceCnt==0)
                    {
                        // tab is pressed when caret on column 0.
                        spaceCnt = g_preference.GetTabSize();
                    }
                    l_str = wxString((char)32, spaceCnt);
                }
                else
                {
                    l_str = g_preference.TabToSpaceTranslate(v_c);
                }
            }
            else   // tab not 2 space
            {
                if (r>0 && v_c==0 && GetLine(r)->TextLen(SVID_NO_CRLF)==0) // Not the first line and caret in the fist column. And that line is a blank line.
                {
                    // Insert tab and cloase to the previous line first non space column.
                    int pos;
                    pairSymbol ps;
                    int end_cnt=0;
                    
                    int spaceCnt = 0;
                    spaceCnt = GetLine(r-1)->FindFirstNonSpaceColUW();
                    spaceCnt = GetLine(r-1)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
                    
                    if (GetLine(r-1)->KeywordContainSymbolStart(v_ps, -1, pos, end_cnt, ps))
                    {
                        // 前一行有 { 符號，再內縮一層
                        pos = GetLine(r-1)->Col2SpaceUW(pos);  // translate tab into spaces
                        spaceCnt = pos + g_preference.GetTabSize();
                    }
                    int tabCnt = spaceCnt/g_preference.GetTabSize();
                    if (tabCnt==0) tabCnt=1; // In case previous line start from column 0.
                    if (spaceCnt%g_preference.GetTabSize()!=0)
                        ++tabCnt;
                    l_str.Empty();
                    for (int i=0; i<tabCnt; i++)
                    {
                        l_str += _("\t");
                    }
                }
                else
                {
                    l_str = p_str;
                }
            }
            
            caretMoveRightCount = l_str.Length();
        }
        else
        {
            
            // Check if the insert character is the start folding symbol.
            // If it is, automatic insert end folding symbol.
            UChar *uBufStr = NULL;
            int32_t uBufStrLen = 0;
            bool insertStartEmbraceSymbol=false;
            bool insertEndEmbraceSymbol=false;
            wxString endEmbraceSymbol;
            v_ps = GetEmbraceSymbol();
            
            for (vector<pairSymbol>::iterator it_p=v_ps.begin();
                 it_p!=v_ps.end();
                 ++it_p)
            {
                if (ConvertwxStrToUChar(p_str, &uBufStr, uBufStrLen)==U_ZERO_ERROR)
                {
                    if ((*it_p).MatchStart(uBufStr, uBufStrLen))
                    {
                        // When insert the start symbol of the embrance pair symbol like { of {}
                        if (ConvertUCharTowxStr((*it_p).e_text, (*it_p).e_len, endEmbraceSymbol)==U_ZERO_ERROR)
                        {
                            insertStartEmbraceSymbol = true;
                            break;
                        }
                    }
                    else if ((*it_p).MatchEnd(uBufStr, uBufStrLen))
                    {
                        // When insert the end symbol of the embrance pair symbol like } of {}
                        // and the caret position is a end symbol.
                        if (ConvertUCharTowxStr((*it_p).e_text, (*it_p).e_len, endEmbraceSymbol)==U_ZERO_ERROR)
                        {
                            if (GetLine(r)->GetTextUW().Mid(v_c, endEmbraceSymbol.Length())==endEmbraceSymbol)
                            {
                                insertEndEmbraceSymbol = true;
                            }
                            break;
                        }
                    }
                }
                if (uBufStr) free(uBufStr);
                uBufStr = NULL;
                uBufStrLen = 0;
            }
            if (uBufStr) free(uBufStr);    // for break condition.        
            uBufStr = NULL;
            uBufStrLen = 0;
            
            
            /* ------------------------------------- */
            
            if (insertStartEmbraceSymbol)
            {
                // when insert { automatic insert } if needed.
                l_str = p_str + endEmbraceSymbol;
                caretMoveRightCount = p_str.Length();
            }
            else if (insertEndEmbraceSymbol)
            {
                // when insert } on caret on a } , move right
                caretMoveRightCount = p_str.Length();

                // move caret
                for(int i=0; i<caretMoveRightCount; i++)
                {
                    int o_r, ov_c;
                    int nr, nv_c;
                    int keep_space=0;
                    o_r=ov_c=0;
                    nr=nv_c=0;
                    it->GetPosition(o_r, ov_c);
                    SingleCaretMoveRight(o_r, ov_c, nr, nv_c, keep_space);
                    it->SetPosition(nr, nv_c);
                    //it->SetKeepSpace(keep_space);
                    ProcWrapAndStyleAt(nr, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                    it->SetKeepSpace(GetLine(nr)->Col2SpaceUW(nv_c));
                }

                // break this function. Don't insert anything.
                return;
            }
            else  // regular insert string.
            {
                l_str = p_str;
                caretMoveRightCount = l_str.Length();
            }
        }

        // 如果打入的字串無法轉碼為原始編碼字串，則將之改為?
        // 打入的字串一律都是 wxString (UTF-16?)
        GetLine(r)->InsertTextUW(v_c, GetLine(r)->ConvertString2Buffer(l_str, m_charSet));
        GetLine(r)->ConvertString2Buffer(m_charSet);
        GetLine(r)->ConvertBufferICU(m_CScharSet);
        CreateKeywordTableAt(r);
        // CheckBlockTag();  // 重算 block comment 類的語法高亮度
        // it->ResetPosition(r, v_c+1);

        // Record actions for undo.
        svUndoAction act(SVID_UNDO_DELETE, r, v_c, r, v_c + l_str.Length(), l_str);
        if (p_cont)
            m_undoActions.Merge2TheLast(act, std::distance(m_carets.m_caretsList.rbegin(), it), SVID_UNDO_INSERT);
        else
            m_undoActions.Add2TheLast(act);


        // Because of tab to space setting.
        // Every  caret may move right with different distance.
        // We recalculate it's new position.
        // 只有 EditingInsertChar 在此處理 caret
        // 其餘 Editing function 都在 svTextView 處理
        //for(int i=0; i<(int)l_str.Length(); i++)
        for(int i=0; i<caretMoveRightCount; i++)
        {
            int o_r, ov_c;
            int nr, nv_c;
            int keep_space=0;
            o_r=ov_c=0;
            nr=nv_c=0;
            it->GetPosition(o_r, ov_c);
            SingleCaretMoveRight(o_r, ov_c, nr, nv_c, keep_space);
            it->SetPosition(nr, nv_c);
            //it->SetKeepSpace(keep_space);
            ProcWrapAndStyleAt(nr, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(nr)->Col2SpaceUW(nv_c));
        }

        // caret below on the same line shift it's colum position.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r3, v_c3;
            r3=v_c3=0;
            it2->GetPosition(r3, v_c3);

            if (r3==r)
            {
                it2->SetPosition(r3, v_c3+l_str.Length());
                ProcWrapAndStyleAt(r3, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3+l_str.Length()));
            }
        }

    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度

}


void svBufText::EditingSplitLine(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    extern svPreference g_preference;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);
        
        bool insertNewLine = false;   // if new line insert is needed when split line.
        int blankLineSpace=0;         // when new line insert is needed, the heading space count.
        wxString nl_str;

        SplitLineAt(r, v_c);
        // GetLine(r).SplitLineAt(v_c);

        int new_col = 0;
        wxString l_str;
        l_str.Empty();

        if (g_preference.GetAlignPrevLine())
        {
            // When new line, insert spaces or tabs to move to the previous line non spaces col.
            // 當產生一個新行, 則插入足夠多的 space 或 tab 以讓 caret 移至與上一行第一個非空白字元出現處。
            if (g_preference.GetTabToSpace())
            {
                if (r>=0) // not the first line.
                {
                    // Insert spaces and move to the previous line first non space column.
                   
                    int pos;
                    pairSymbol ps;
                    int end_cnt=0;
                    int start_count = 0;
                    vector<pairSymbol> v_ps = GetFoldingSymbol();
                    
                    int spaceCnt = 0;
                    spaceCnt = GetLine(r)->FindFirstNonSpaceColUW();
                    spaceCnt = GetLine(r)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
                    
                    // looking for the { before the v_c position
                    if (v_c>0 && GetLine(r)->KeywordContainSymbolStart(v_ps, v_c-1, pos, end_cnt, ps))
                    {
                        // 前一行有 { 符號，再內縮一層
                        pos = GetLine(r)->Col2SpaceUW(pos);  // translate tab into spaces
                        int end_pos=0;
                        bool endExist = GetLine(r)->KeywordContainSymbolEnd(v_ps, v_c, end_pos, start_count, ps);
                        if (endExist && end_pos==v_c)
                        {
                            // caret between {} , auto split {} and insert a new line.
                            insertNewLine = true;
                            blankLineSpace = pos + g_preference.GetTabSize();
                            spaceCnt = pos;
                        }
                        else
                            spaceCnt = pos + g_preference.GetTabSize();
                    }
                    
                    l_str = wxString((char)32, spaceCnt);
                    if (insertNewLine)
                        nl_str = wxString((char)32, blankLineSpace);
                    
                }
            }
            else
            {
                if (r>=0) // not the first line.                
                {
                    // Insert tab and close to the previous line first non space column.
                    
                    int pos;
                    pairSymbol ps;
                    int end_cnt=0;
                    int start_count = 0;
                    vector<pairSymbol> v_ps = GetFoldingSymbol();
                    
                    int spaceCnt = 0;
                    spaceCnt = GetLine(r)->FindFirstNonSpaceColUW();
                    spaceCnt = GetLine(r)->Col2SpaceUW(spaceCnt);  // translate tab into spaces
                    
                    if (v_c>0 && GetLine(r)->KeywordContainSymbolStart(v_ps, v_c-1, pos, end_cnt, ps))
                    {
                        // 前一行有 { 符號，再內縮一層
                        pos = GetLine(r)->Col2SpaceUW(pos);  // translate tab into spaces
                        int end_pos=0;
                        bool endExist = GetLine(r)->KeywordContainSymbolEnd(v_ps, v_c, end_pos, start_count, ps);
                        if (endExist && end_pos==v_c)
                        {
                            // caret between {} , auto split {} and insert a new line.
                            insertNewLine = true;
                            blankLineSpace = pos + g_preference.GetTabSize();
                            spaceCnt = pos;
                        }
                        else
                            spaceCnt = pos + g_preference.GetTabSize();
                    }
                    int tabCnt = spaceCnt/g_preference.GetTabSize();
                    if (tabCnt==0) tabCnt=1; // In case previous line start from column 0.
                    if (spaceCnt%g_preference.GetTabSize()!=0)
                        ++tabCnt;
                    l_str.Empty();
                    for (int i=0; i<tabCnt; i++)
                    {
                        l_str += _("\t");
                    }
                    
                    if (insertNewLine)   
                    {
                        int tabCnt2 = blankLineSpace/g_preference.GetTabSize();
                        if (blankLineSpace%g_preference.GetTabSize()!=0)
                            ++tabCnt2;
                        nl_str.Empty();
                        for (int i=0; i<tabCnt2; i++)
                        {
                            nl_str += _("\t");
                        }
                    }
                }
            }
            GetLine(r+1)->SetTextUW(l_str+GetLine(r+1)->GetTextUW());
            new_col = l_str.Length();

        }
        else
        {
        }

        if (insertNewLine)
        {
            InsertAfterLineAt(r, nl_str);
        }

        GetLine(r)->ConvertString2Buffer(m_charSet);
        GetLine(r)->ConvertBufferICU(m_CScharSet);

        GetLine(r+1)->ConvertString2Buffer(m_charSet);
        GetLine(r+1)->ConvertBufferICU(m_CScharSet);
        
        if (insertNewLine)
        {
            GetLine(r+2)->ConvertString2Buffer(m_charSet);
            GetLine(r+2)->ConvertBufferICU(m_CScharSet);
        }

        CreateKeywordTableAt(r);
        CreateKeywordTableAt(r+1);
        if (insertNewLine)
        {
            CreateKeywordTableAt(r+2);
        }

        // CheckBlockTag();   // 重算 block comment 類的語法高亮度

        if (insertNewLine)
            it->SetPosition(r+1, nl_str.Length());
        else
            it->SetPosition(r+1, new_col);
        ProcWrapAndStyleAt(r+1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
        it->SetKeepSpace(GetLine(r+1)->Col2SpaceUW(new_col));


        // Record actions for undo.
        svUndoAction act1(SVID_UNDO_JOIN, r, v_c, "");
        m_undoActions.Add2TheLast(act1);

        // Record actions for undo.
        if (g_preference.GetAlignPrevLine())
        {
            svUndoAction act2(SVID_UNDO_DELETE, r+1, 0, r+1, 0 + l_str.Length(), l_str);
            m_undoActions.Add2TheLast(act2);
        }

        if (insertNewLine)
        {
            // Record actions for inserted blank line.
            svUndoAction act3(SVID_UNDO_CUT, r+1, 0, r+2, 0, l_str);
            m_undoActions.Add2TheLast(act3);
        }

        // caret below add 1 row also.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r2, v_c2;
            r2=v_c2=0;
            it2->GetPosition(r2, v_c2);
            it2->SetPosition(r2+1, v_c2);
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

// p_cont : 是否為連續的 delete 動作
void svBufText::EditingTextDelete(bool p_cont)
{
#ifndef NDEBUG
    wxLogMessage("svBufText::EditingTextDelete start");
    m_carets.DumpCarets();
#endif
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();
#ifndef NDEBUG
    wxLogMessage("svBufText::EditingTextDelete after remove duplication & hidden carets");
    m_carets.DumpCarets();
#endif

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);

        if (GetLine(r)->GetTextUW().Length()==0 || 
                GetLine(r)->GetTextUW().Length()==v_c) 
        {
            // In the end of line, next line will be move up and append to the current line.
            // 游標在行末，下一行往上提

            // if caret in the last line, skip the caret
            if (r==LineCntUW()-1)
                continue;

            int prvLineLen = GetLine(r)->GetTextUW().Length();

            if (!GetLine(r+1)->Visible())  // Next line will be joined. If it's not visible, Unfolding it.
                UnfoldingAt(r);

            JoinNextLineAt(r);

            GetLine(r)->ConvertString2Buffer(m_charSet);
            GetLine(r)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(r);
            // CheckBlockTag();  // 重算 block comment 類的語法高亮度

            // !!! <<<< 這段要再觀察執行結果 >>>> !!!
            // Record actions for undo.
            // if (!LastUndoActionsIsEmpty())
            // {
            //     InitialNewUndoActions(); // 因為 SPLIT 無法連續貯存 undo 動作，所以要判斷如果不是空的 undo list, 要 append 一個新的
            // }
            svUndoAction act(SVID_UNDO_SPLIT, r, v_c, "");
            if (p_cont)
                m_undoActions.Merge2TheLast(act, std::distance(m_carets.m_caretsList.rbegin(), it), SVID_UNDO_DELETE);
            else
                m_undoActions.Add2TheLast(act);            

            // caret belows should sub 1 row also.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r2, v_c2;
                r2=v_c2=0;
                it2->GetPosition(r2, v_c2);
                if (r2-1==r) 
                {
                    // caret in the next line of current line. and the line is been merged into the current line.
                    // 這一行被移到上一行了，caret位置上移一行後，要重算
                    int moveCount = prvLineLen;
                    moveCount = moveCount - v_c + v_c2;
                    int nr, nv_c, ks;
                    if (moveCount>0)
                    {
                        int kr, kv_c;
                        kr = r; kv_c = v_c;
                        for (int i=0; i<moveCount; ++i)
                        {
                            SingleCaretMoveRight(kr, kv_c, nr, nv_c, ks);
                            kr = nr; kv_c = nv_c;
                        }
                        it2->SetPosition(nr, nv_c);
                        it2->SetKeepSpace(ks);
                    }
                    else
                    {
                        it2->SetPosition(r, v_c);
                        it2->SetKeepSpace(it->GetKeepSpace());
                    }
                }
                else
                {
                    it2->SetPosition(r2-1, v_c2);
                }
            }
        }
        else
        {
            // caret not in the end of line.

            wxString txt = DeleteTextAt(r, v_c, v_c+1);

            GetLine(r)->ConvertString2Buffer(m_charSet);
            GetLine(r)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(r);
            // CheckBlockTag();  // 重算 block comment 類的語法高亮度

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_INSERT, r, v_c, txt);
            if (p_cont)
                m_undoActions.Merge2TheLast(act, std::distance(m_carets.m_caretsList.rbegin(), it), SVID_UNDO_DELETE);
            else
                m_undoActions.Add2TheLast(act);

            // caret after.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r2, v_c2;
                r2=v_c2=0;
                it2->GetPosition(r2, v_c2);

                if (r2==r)  // caret on the same line of the current line. caret should move left.
                {
                    if (v_c2>v_c)
                    {
                        int nr, nv_c, ks;
                        SingleCaretMoveLeft(r2, v_c2, nr, nv_c, ks);
                        wxLogMessage(wxString::Format("svBufText::EditingTextDelete after SingleCaretMoveLeft (%i,%i)=>(%i,%i) ", r2, v_c2, nr, nv_c));
                        it2->SetPosition(nr, nv_c);
                        it2->SetKeepSpace(ks);
                    }
                }
            }
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度

#ifndef NDEBUG
    wxLogMessage("svBufText::EditingTextDelete end");
    // m_carets.DumpCarets();
#endif

}

// p_cont : 是否為連續的 backdelete 動作
void svBufText::EditingTextBackDelete(bool p_cont)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);

        // if (r==0 && v_c==0)
        if (!r && !v_c) continue;  // boundry check: In the head of file.

        if (v_c==0 && r>0)
        {
            // In the first position of line.
            int prvLineLen = GetLine(r-1)->GetTextUW().Length();

            int nr, nv_c, ks;
            SingleCaretMoveLeft(r, v_c, nr, nv_c, ks);

            if (!GetLine(r-1)->Visible())  // Next line will be joined. If it's not visible, Unfolding it.
                UnfoldingAt(r-1);

            JoinNextLineAt(r-1);

            GetLine(r-1)->ConvertString2Buffer(m_charSet);
            GetLine(r-1)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(r-1);
            // CheckBlockTag();  // 重算 block comment 類的語法高亮度

            it->SetPosition(nr, nv_c);
            it->SetKeepSpace(ks);

            // !!! <<<< 這段要再觀察執行結果 >>>> !!!
            // Record actions for undo.
            // if (!LastUndoActionsIsEmpty())
            // {
            //     InitialNewUndoActions(); // 因為 SPLIT 無法連續貯存 undo 動作，所以要判斷如果不是空的 undo list, 要 append 一個新的
            // }
            svUndoAction act(SVID_UNDO_SPLIT, nr, nv_c, "");
            if (p_cont)
                m_undoActions.Merge2TheLast(act, std::distance(m_carets.m_caretsList.rbegin(), it), SVID_UNDO_BACKDEL);
            else
                m_undoActions.Add2TheLast(act);

            int new_r, new_v_c;
            new_r = nr; new_v_c = nv_c;

            // caret below sub 1 row also.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {

                int r2, v_c2;
                r2=v_c2=0;
                it2->GetPosition(r2, v_c2);
                if (r2==r)
                {
                    // caret in the next line of current line. and the line is been merged into the current line.
                    // 這一行被移到上一行了，caret位置上移一行後，要重算
                    int moveCount = prvLineLen;
                    moveCount = v_c2 - 1;
                    int nr, nv_c, ks;
                    if (moveCount>0)
                    {
                        int kr, kv_c;
                        kr = new_r; kv_c = new_v_c;
                        for (int i=0; i<moveCount; ++i)
                        {
                            SingleCaretMoveRight(kr, kv_c, nr, nv_c, ks);
                            kr = nr; kv_c = nv_c;
                        }
                        it2->SetPosition(nr, nv_c);
                        it2->SetKeepSpace(ks);
                    }
                    else
                    {
                        it2->SetPosition(new_r, new_v_c);
                        it2->SetKeepSpace(it->GetKeepSpace());
                    }
                }
                else
                {
                    it2->SetPosition(r2-1, v_c2);
                }
            }

        }
        else
        {
            assert(v_c>0);

            int nr, nv_c, ks;
            SingleCaretMoveLeft(r, v_c, nr, nv_c, ks);

            wxString txt = DeleteTextAt(r, v_c-1, v_c);

            GetLine(r)->ConvertString2Buffer(m_charSet);
            GetLine(r)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(r);
            // CheckBlockTag();  // 重算 block comment 類的語法高亮度

            it->SetPosition(nr, nv_c);
            it->SetKeepSpace(ks);

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_INSERT, nr, nv_c, txt);
            if (p_cont)
                m_undoActions.Merge2TheLast(act, std::distance(m_carets.m_caretsList.rbegin(), it), SVID_UNDO_BACKDEL);
            else
                m_undoActions.Add2TheLast(act);

            // caret after.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r2, v_c2;
                r2=v_c2=0;
                it2->GetPosition(r2, v_c2);

                if (r2==r)  // caret on the same line of the current line. caret should move left.
                {
                    if (v_c2>=v_c)
                    {
                        int nr, nv_c, ks;
                        SingleCaretMoveLeft(r2, v_c2, nr, nv_c, ks);
                        it2->SetPosition(nr, nv_c);
                        it2->SetKeepSpace(ks);
                    }
                }
            }            
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

void svBufText::EditingSelectedTextDelete(void)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    // if (!m_carets.HasSelect())
    //     return;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        if (!it->HasSelect()) 
            continue;

        int sr, sc, er, ec;
        sr=sc=er=ec=0;
        it->GetSelectPos(sr, sc, er, ec);
        if (it->SelectType()==SVID_CARET_ON_TAIL)
            it->SetKeepSpace(it->GetSelectKeepSpace());
        int deleteLineCnt = er-sr;  // 往上提的行數
        wxString txt = DeleteTextRange(sr, sc, er, ec);
        it->SetPosition(sr, sc);
        // 已處理，仍待觀察==>目前尚未考慮若被刪的文字行內含 block comment則其後的 syntax hilight 處理

        // for (int i=0; i<deleteLineCnt+1; i++)
        // for (int i=0; i<2; i++)  // ConvertString2Buffer 只處理兩行即可，若處理太多可能會遇上 m_text = null 的文字行
        // {
        //     if (sr+i<(int)LineCntUW())
        //     {
        //         GetLine(sr+i)->ConvertString2Buffer(m_charSet);
        //         GetLine(sr+i)->ConvertBufferICU(m_CScharSet);
        //         CreateKeywordTableAt(sr+i);
        //     }
        // }
        GetLine(sr)->ConvertString2Buffer(m_charSet);
        GetLine(sr)->ConvertBufferICU(m_CScharSet);
        CreateKeywordTableAt(sr);
        // CheckBlockTag();  // 重算 block comment 類的語法高亮度

        // Record actions for undo.
        svUndoAction act(SVID_UNDO_PASTE, sr, sc, txt);
        m_undoActions.Add2TheLast(act);

        // caret belows should sub n row also.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r2, v_c2;
            r2=v_c2=0;
            it2->GetPosition(r2, v_c2);
            if (r2-deleteLineCnt==sr) 
            {
                // caret in the next line of current line. and the line is been merged into the current line.
                // 這一行被移到上n行了，caret位置上移n行後，要重算
                int moveCount = 0;
                moveCount = v_c2 - ec;
                int nr, nv_c, ks;
                if (moveCount>0)
                {
                    int kr, kv_c;
                    kr = sr; kv_c = sc;
                    for (int i=0; i<moveCount; ++i)
                    {
                        SingleCaretMoveRight(kr, kv_c, nr, nv_c, ks);
                        kr = nr; kv_c = nv_c;
                    }
                    it2->SetPosition(nr, nv_c);
                    it2->SetKeepSpace(ks);
                }
                else
                {
                    it2->SetPosition(sr, v_c2);
                    it2->SetKeepSpace(it->GetKeepSpace());
                }
            }
            else
            {
                it2->SetPosition(r2-deleteLineCnt, v_c2);
            }
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

void svBufText::EditingTextCopySelected(void)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    wxString selectedText;

    for(std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
        it!=m_carets.m_caretsList.end();
        ++it)
    {
        if (!it->HasSelect()) 
            continue;

        int sr, sc, er, ec;
        sr=sc=er=ec=0;
        it->GetSelectPos(sr, sc, er, ec);
        wxString txt = GetTextRange(sr, sc, er, ec);

        if (selectedText.IsEmpty())
            selectedText = txt;
        else
            // selectedText = selectedText + m_EOL_wxStr + txt;
            selectedText = selectedText + "\n" + txt;
    }

    // Write text to the clipboard
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(selectedText));
        wxTheClipboard->Close();
    }
    else
    {
        wxLogMessage("svBufText::EditingTextCopySelected write text to clipboard fail.");
    }
}

void svBufText::EditingPaste(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    wxString clipboardText;

    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            clipboardText = data.GetText();
        }
        wxTheClipboard->Close();
    }
    else
    {
        wxLogMessage("svBufText::EditingPaste read from clipboard fail.");
    }

    if (clipboardText.IsEmpty())
    {
        return;
    }


    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);

        int insertLineCnt = 0;
        int lastLineLen = 0;
        // insertLineCnt = InsertTextAt(r, v_c, clipboardText);
        //auto t0 = Clock::now();
        //auto t1 = Clock::now();
        InsertTextAt(r, v_c, clipboardText, insertLineCnt, lastLineLen);
        
        //auto t2 = Clock::now();
        for(int i=0; i<insertLineCnt; i++)
        {
            GetLine(r+i)->ConvertString2Buffer(m_charSet);
            GetLine(r+i)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(r+i);
        }
        // CheckBlockTag();  // 重算 block comment 類的語法高亮度
        //auto t3 = Clock::now();

        //wxLogMessage(wxString::Format("EditingPaste performance evaluation"));
        //wxLogMessage(wxString::Format("InsertTextAt time consumming: %ul nanoseconds", std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()));
        //wxLogMessage(wxString::Format("Convert buffers time consumming: %ul nanoseconds", std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count()));
        
        if (insertLineCnt>1)
        {
            it->SetPosition(r+insertLineCnt-1, lastLineLen);
            ProcWrapAndStyleAt(r+insertLineCnt-1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r+insertLineCnt-1)->Col2SpaceUW(lastLineLen));

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_CUT, r, v_c, r+insertLineCnt-1, lastLineLen, clipboardText);
            m_undoActions.Add2TheLast(act);
        }
        else
        {
            it->SetPosition(r+insertLineCnt-1, v_c+lastLineLen);
            ProcWrapAndStyleAt(r+insertLineCnt-1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r+insertLineCnt-1)->Col2SpaceUW(v_c+lastLineLen));

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_CUT, r, v_c, r+insertLineCnt-1,v_c+lastLineLen, clipboardText);
            m_undoActions.Add2TheLast(act);
        }



        // caret below add n row also.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r2, v_c2;
            r2=v_c2=0;
            it2->GetPosition(r2, v_c2);
            if (r2==r)
            {
                // caret in the right half of the line.
                // 游標在被分裂行的後半段，要重算位置
                int moveCount = lastLineLen;
                moveCount = moveCount - v_c + v_c2;
                int nr, nv_c, ks;
                if (moveCount>0)
                {
                    int kr, kv_c;
                    kr = r+insertLineCnt-1;
                    if (insertLineCnt==1)  // 只有插入一行文字
                    {
                        kv_c = v_c;
                    }
                    else                   // 插入多行文字 
                    {
                        kv_c = 0;
                    }
                    for (int i=0; i<moveCount; ++i)
                    {
                        SingleCaretMoveRight(kr, kv_c, nr, nv_c, ks);
                        kr = nr; kv_c = nv_c;
                    }
                    it2->SetPosition(nr, nv_c);
                    it2->SetKeepSpace(ks);
                }
                else
                {
                    it2->SetPosition(r, v_c);
                    it2->SetKeepSpace(it->GetKeepSpace());
                }                
            }
            else
            {
                it2->SetPosition(r2+insertLineCnt-1, v_c2);
            }
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度

}

// For duplicating a line.
void svBufText::EditingDuplicateLine(void)
{
    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    extern svPreference g_preference;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        r=v_c=0;
        it->GetPosition(r, v_c);

        wxString txt = DuplicateLineAt(r);

        GetLine(r+1)->ConvertString2Buffer(m_charSet);
        GetLine(r+1)->ConvertBufferICU(m_CScharSet);

        CreateKeywordTableAt(r+1);
        // CheckBlockTag();   // 重算 block comment 類的語法高亮度

        it->SetPosition(r+1, v_c);
        // ProcWrapAndStyleAt(r+1, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
        // it->SetKeepSpace(GetLine(r+1)->Col2SpaceUW(new_col));

        // Record actions for undo.
        svUndoAction act(SVID_UNDO_DELETE_LINE, r+1, v_c, r+1, v_c, txt);
        m_undoActions.Add2TheLast(act);

        // caret below add 1 row also.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r2, v_c2;
            r2=v_c2=0;
            it2->GetPosition(r2, v_c2);
            it2->SetPosition(r2+1, v_c2);
        }
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

// For line comment or line uncomment.
void svBufText::EditingLineComment(const wxString &p_lineComment, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    // wxString lineComment;
    // if (!get_line_comment(lineComment))
    //     return;

    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    int prev_r = -1;   // record last caret row. prevent line comment at the same line twice or more.
    char prev_result = -1; // record last result. prevent line comment at the same line twice or more.

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        int r1, v_c1;
        int r2, v_c2;
        r=v_c=0;
        r1=v_c1=0;
        r2=v_c2=0;
        it->GetPosition(r, v_c);

        if (r==prev_r)
        {
            // 考慮多個 caret 在同一行的狀況，先作一些前置處理。
            if (prev_result==SVID_LINE_COMMENTED)
            {
                it->SetPosition(r, v_c+p_lineComment.Length());
                ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r)->Col2SpaceUW(v_c+p_lineComment.Length()));
            }
            else if (prev_result==SVID_LINE_UNCOMMENTED)
            {
                it->SetPosition(r, v_c-p_lineComment.Length());
                ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r)->Col2SpaceUW(v_c-p_lineComment.Length()));
            }
            continue;  
        } 

        char result=0;

        if (it->HasSelect())
        {
            // caret has selected.
            // line comment every line between selected line.
            it->GetSelectPos(r1, v_c1, r2, v_c2);
            if (v_c2==0) --r2;   // If block chosed, and the caret location is in the beginning of the line. Don't comment it
            for (int i=r1; i<=r2; i++)
            {
                char tmp_result = LineCommentLineAt(i, p_lineComment);
                if (i==r)
                    result = tmp_result;

                if (tmp_result!=SVID_LINE_NOT_CHANGED)
                {
                    GetLine(i)->ConvertString2Buffer(m_charSet);
                    GetLine(i)->ConvertBufferICU(m_CScharSet);
    
                    CreateKeywordTableAt(i);
    
                     //Record actions for undo.
                    svUndoAction act(SVID_UNDO_LINE_COMMENT, i, v_c, i, v_c, "");
                    m_undoActions.Add2TheLast(act);
                }
            }
        }
        else
        {
            result = LineCommentLineAt(r, p_lineComment);

            if (result!=SVID_LINE_NOT_CHANGED)
            {
                GetLine(r)->ConvertString2Buffer(m_charSet);
                GetLine(r)->ConvertBufferICU(m_CScharSet);
    
                CreateKeywordTableAt(r);
    
                // Record actions for undo.
                svUndoAction act(SVID_UNDO_LINE_COMMENT, r, v_c, r, v_c, "");
                m_undoActions.Add2TheLast(act);
            }
        }

        // CheckBlockTag();   // 重算 block comment 類的語法高亮度

        if (result==SVID_LINE_COMMENTED)
        {
            it->SetPosition(r, v_c+p_lineComment.Length());
            ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r)->Col2SpaceUW(v_c+p_lineComment.Length()));
        }
        else if (result==SVID_LINE_UNCOMMENTED)
        {
            it->SetPosition(r, v_c-p_lineComment.Length());
            ProcWrapAndStyleAt(r, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            it->SetKeepSpace(GetLine(r)->Col2SpaceUW(v_c-p_lineComment.Length()));
        }

        // caret below add 1 row also.
        // for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
        //     it2!=it;
        //     ++it2)
        // {
        //     int r2, v_c2;
        //     r2=v_c2=0;
        //     it2->GetPosition(r2, v_c2);
        //     it2->SetPosition(r2+1, v_c2);
        // }

        prev_r = r;
        prev_result = result;

    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}


// For block comment or block uncomment.
// In simply and stupid way, add or remove /* */ from caret select position.
void svBufText::EditingBlockComment(const wxString &p_startComment, const wxString &p_endComment, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{

    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        int r1, v_c1;
        int r2, v_c2;
        r=v_c=0;
        r1=v_c1=0;
        r2=v_c2=0;
        it->GetPosition(r, v_c);

        char result=0;

        if (!it->HasSelect())
            continue;


        // caret must has selected to be process.
        // block comment between start and end position.
        it->GetSelectPos(r1, v_c1, r2, v_c2);
        result = BlockCommentAt(r1, v_c1, r2, v_c2, p_startComment, p_endComment);

        GetLine(r1)->ConvertString2Buffer(m_charSet);
        GetLine(r1)->ConvertBufferICU(m_CScharSet);

        GetLine(r2)->ConvertString2Buffer(m_charSet);
        GetLine(r2)->ConvertBufferICU(m_CScharSet);

        CreateKeywordTableAt(r1);
        CreateKeywordTableAt(r2);

        if (result==SVID_BLOCK_COMMENTED_MULTI_LINE)
        {
            // Record actions for undo.
            svUndoAction act1(SVID_UNDO_DELETE, r2, v_c2, r2, v_c2 + p_endComment.Length(), p_endComment);
            m_undoActions.Add2TheLast(act1);

            svUndoAction act2(SVID_UNDO_DELETE, r1, v_c1, r1, v_c1 + p_startComment.Length(), p_startComment);
            m_undoActions.Add2TheLast(act2);

            // shift caret's column position.
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                // caret position No Change.
            }
            else
            {
                it->SetPosition(r2, v_c2+p_endComment.Length());
                ProcWrapAndStyleAt(r2, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r2)->Col2SpaceUW(v_c2+p_endComment.Length()));
            }

            // caret below on the same line shift it's colum position.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r3, v_c3;
                r3=v_c3=0;
                it2->GetPosition(r3, v_c3);

                if (r3==r2)
                {
                    it2->SetPosition(r3, v_c3+p_endComment.Length());
                    it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3+p_endComment.Length()));
                }
            }

        }
        else if (result==SVID_BLOCK_UNCOMMENTED_MULTI_LINE)// SVID_BLOCK_UNCOMMENTED
        {
            // Record actions for undo.
            svUndoAction act1(SVID_UNDO_INSERT, r2, v_c2 - p_endComment.Length(), r2, v_c2, p_endComment);
            m_undoActions.Add2TheLast(act1);

            svUndoAction act2(SVID_UNDO_INSERT, r1, v_c1, r1, v_c1 + p_startComment.Length(), p_startComment);
            m_undoActions.Add2TheLast(act2);

            // shift caret's column position.
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                // caret postion No change.
            }
            else
            {
                it->SetPosition(r2, v_c2-p_endComment.Length());
                ProcWrapAndStyleAt(r2, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r2)->Col2SpaceUW(v_c2-p_endComment.Length()));
            }

            // caret below on the same line shift it's colum position.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r3, v_c3;
                r3=v_c3=0;
                it2->GetPosition(r3, v_c3);

                if (r3==r2)
                {
                    it2->SetPosition(r3, v_c3-p_endComment.Length());
                    it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3-p_endComment.Length()));
                }
            }

        }
        else if (result==SVID_BLOCK_COMMENTED_SINGLE_LINE)
        {
            // Record actions for undo.
            svUndoAction act1(SVID_UNDO_DELETE, r2, v_c2, r2, v_c2 + p_endComment.Length(), p_endComment);
            m_undoActions.Add2TheLast(act1);

            svUndoAction act2(SVID_UNDO_DELETE, r1, v_c1, r1, v_c1 + p_startComment.Length(), p_startComment);
            m_undoActions.Add2TheLast(act2);

            // shift caret's column position.
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                // caret position No Change.
            }
            else
            {
                it->SetPosition(r2, v_c2+p_startComment.Length()+p_endComment.Length());
                ProcWrapAndStyleAt(r2, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r2)->Col2SpaceUW(v_c2+p_startComment.Length()+p_endComment.Length()));
            }

            // caret below on the same line shift it's colum position.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r3, v_c3;
                r3=v_c3=0;
                it2->GetPosition(r3, v_c3);

                if (r3==r2)
                {
                    it2->SetPosition(r3, v_c3+p_endComment.Length());
                    it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3+p_endComment.Length()));
                }
            }

        }
        else if (result==SVID_BLOCK_UNCOMMENTED_SINGLE_LINE)// SVID_BLOCK_UNCOMMENTED_SINGLE_LINE
        {
            // Record actions for undo.
            svUndoAction act1(SVID_UNDO_INSERT, r2, v_c2 - p_endComment.Length(), r2, v_c2, p_endComment);
            m_undoActions.Add2TheLast(act1);

            svUndoAction act2(SVID_UNDO_INSERT, r1, v_c1, r1, v_c1 + p_startComment.Length(), p_startComment);
            m_undoActions.Add2TheLast(act2);

            // shift caret's column position.
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                // caret postion No change.
            }
            else
            {
                it->SetPosition(r2, v_c2-p_startComment.Length()-p_endComment.Length());
                ProcWrapAndStyleAt(r2, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
                it->SetKeepSpace(GetLine(r2)->Col2SpaceUW(v_c2-p_endComment.Length()-p_startComment.Length()));
            }

            // caret below on the same line shift it's colum position.
            for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
                it2!=it;
                ++it2)
            {
                int r3, v_c3;
                r3=v_c3=0;
                it2->GetPosition(r3, v_c3);

                if ( r3==r2 && it->SelectType()==SVID_CARET_ON_TAIL )
                {
                    it2->SetPosition(r3, v_c3-p_endComment.Length());
                    it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3-p_endComment.Length()));
                }
            }
        }

        // CheckBlockTag();   // 重算 block comment 類的語法高亮度
    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

// Indent.
void svBufText::EditingIndent(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{

    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    extern svPreference g_preference;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        int r1, v_c1;
        int r2, v_c2;
        r=v_c=0;
        r1=v_c1=0;
        r2=v_c2=0;
        it->GetPosition(r, v_c);

        int sRow, eRow;
        sRow=eRow=0;

        char result=0;

        if (it->HasSelect())
        {
            it->GetSelectPos(r1, v_c1, r2, v_c2);
            if (v_c2==0) --r2;  // the caret is in the begin of the line.
            sRow = r1;
            eRow = r2;
        }
        else
        {
            sRow = r;
            eRow = r;
        }

        wxString l_str;

        if (g_preference.GetTabToSpace())
        {
            l_str = wxString((char)32, g_preference.GetTabSize());
        }
        else
        {
            l_str = "\t";
        }

        // Inserting tab or spaces in front of the line(s).
        for (int i=sRow; i<=eRow; i++)
        {
            // 如果打入的字串無法轉碼為原始編碼字串，則將之改為?
            // 打入的字串一律都是 wxString (UTF-16?)
            GetLine(i)->InsertTextUW(0, GetLine(i)->ConvertString2Buffer(l_str, m_charSet));
            GetLine(i)->ConvertString2Buffer(m_charSet);
            GetLine(i)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(i);

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_DELETE, i, 0, i, 0 + l_str.Length(), l_str);
            m_undoActions.Add2TheLast(act);

        }


        // recalculate caret position.
        if (!(it->SelectType()==SVID_CARET_ON_TAIL && v_c2==0)) // caret in the last line and in the beginning of the line.
        {
            // Because of tab to space setting.
            // Every  caret may move right with different distance.
            // We recalculate it's new position.
            // 只有 EditingInsertChar 在此處理 caret
            // 其餘 Editing function 都在 svTextView 處理
            int selectRow, selectCol;
            selectRow = selectCol = 0;
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                selectRow = r2;
                selectCol = v_c2;
            }
            else if (it->SelectType()==SVID_CARET_ON_TAIL)
            {
                selectRow = r1;
                selectCol = v_c1;
            }
            else   // No selection.
            {
                selectRow = r;
                selectCol = v_c;
            }
    
            ProcWrapAndStyleAt(selectRow, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            if (it->HasSelect())
                it->SetSelect(selectRow, selectCol+l_str.Length(), GetLine(selectRow)->Col2SpaceUW(selectCol+l_str.Length()));
            else
                it->ClearSelect();
    
            for(int i=0; i<(int)l_str.Length(); i++)
            {
                int o_r, ov_c;
                int nr, nv_c;
                int keep_space=0;
                o_r=ov_c=0;
                nr=nv_c=0;
                it->GetPosition(o_r, ov_c);
                SingleCaretMoveRight(o_r, ov_c, nr, nv_c, keep_space);
                it->SetPosition(nr, nv_c);
                it->SetKeepSpace(keep_space);
            }
        }

        // caret below on the same line shift it's colum position.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r3, v_c3;
            r3=v_c3=0;
            it2->GetPosition(r3, v_c3);

            if ( r3==eRow && it->SelectType()==SVID_CARET_ON_TAIL )
            {
                it2->SetPosition(r3, v_c3+l_str.Length());
                it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3+l_str.Length()));
            }
        }

    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

// Outdent.
void svBufText::EditingOutdent(size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{

    m_carets.RemoveDuplication();
    // RemoveHiddenLineCarets();
    UnfoldingAtCarets();

    extern svPreference g_preference;

    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)
    {
        int r, v_c;
        int r1, v_c1;
        int r2, v_c2;
        r=v_c=0;
        r1=v_c1=0;
        r2=v_c2=0;
        it->GetPosition(r, v_c);

        int sRow, eRow;
        sRow=eRow=0;

        char result=0;

        if (it->HasSelect())
        {
            it->GetSelectPos(r1, v_c1, r2, v_c2);
            if (v_c2==0) --r2; // caret in the beginning of the line. don't process it.
            sRow = r1;
            eRow = r2;
        }
        else
        {
            sRow = r;
            eRow = r;
        }

        wxString l_str;

        // Outdenting tab or spaces in front of the line(s).
        for (int i=sRow; i<=eRow; i++)
        {
            int delCnt = GetLine(i)->FindFirstNonSpaceColUW(g_preference.GetTabSize(), g_preference.GetTabSize());
            wxString delStr = GetLine(i)->DeleteTextUW(0, delCnt);

            GetLine(i)->ConvertString2Buffer(m_charSet);
            GetLine(i)->ConvertBufferICU(m_CScharSet);
            CreateKeywordTableAt(i);

            // Record actions for undo.
            svUndoAction act(SVID_UNDO_INSERT, i, 0, i, 0 + delCnt, delStr);
            m_undoActions.Add2TheLast(act);

            if (it->SelectType()==SVID_CARET_ON_HEAD && i==sRow)
            {
                l_str = delStr;
            }
            else if (it->SelectType()==SVID_CARET_ON_TAIL && i==eRow)
            {
                l_str = delStr;
            }
        }

        // recalculate caret position.
        if (!(it->SelectType()==SVID_CARET_ON_TAIL && v_c2==0)) // caret in the last line and in the beginning of the line.
        {
            // Because of tab to space setting.
            // Every  caret may move right with different distance.
            // We recalculate it's new position.
            // 只有 EditingInsertChar 在此處理 caret
            // 其餘 Editing function 都在 svTextView 處理
            int selectRow, selectCol;
            selectRow = selectCol = 0;
            if (it->SelectType()==SVID_CARET_ON_HEAD)
            {
                selectRow = r2;
                selectCol = v_c2;
            }
            else if (it->SelectType()==SVID_CARET_ON_TAIL)
            {
                selectRow = r1;
                selectCol = v_c1;
            }
            else  // No selection.
            {
                selectRow = r;
                selectCol = v_c;
            }
    
            ProcWrapAndStyleAt(selectRow, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
            if (it->HasSelect())
                it->SetSelect(selectRow, selectCol-l_str.Length(), GetLine(selectRow)->Col2SpaceUW(selectCol-l_str.Length()));
            else
                it->ClearSelect();
    
            for(int i=0; i<(int)l_str.Length(); i++)
            {
                int o_r, ov_c;
                int nr, nv_c;
                int keep_space=0;
                o_r=ov_c=0;
                nr=nv_c=0;
                it->GetPosition(o_r, ov_c);
                SingleCaretMoveLeft(o_r, ov_c, nr, nv_c, keep_space);
                it->SetPosition(nr, nv_c);
                it->SetKeepSpace(keep_space);
            }
        }

        // caret below on the same line shift it's colum position.
        for(std::vector<svCaret>::reverse_iterator it2=m_carets.m_caretsList.rbegin();
            it2!=it;
            ++it2)
        {
            int r3, v_c3;
            r3=v_c3=0;
            it2->GetPosition(r3, v_c3);

            if ( r3==eRow && it->SelectType()==SVID_CARET_ON_TAIL )
            {
                it2->SetPosition(r3, v_c3-l_str.Length());
                it2->SetKeepSpace(GetLine(r3)->Col2SpaceUW(v_c3-l_str.Length()));
            }
        }

    }

    CheckBlockTag();  // 重算 block comment 類的語法高亮度
}

void svBufText::UndoEditing(void)
{
    if (m_undoActions.Size())
    {
        vector<svUndoAction> actList = m_undoActions.GetLastUndoActions();
        carets_info caretInfo = m_undoActions.GetLastCarets();

        for(std::vector<svUndoAction>::reverse_iterator it=actList.rbegin();
            it!=actList.rend();
            ++it)
        {
            int r = it->GetStartRow();
            int c = it->GetStartCol();

            switch(it->GetType())
            {
                case SVID_UNDO_DELETE:
                    UndoEditingDelete(*it);
                    break;
                case SVID_UNDO_INSERT:
                    UndoEditingInsert(*it);
                    break;
                case SVID_UNDO_CUT:
                    UndoEditingCut(*it);
                    break;
                case SVID_UNDO_PASTE:
                    UndoEditingPaste(*it);
                    break;            
                case SVID_UNDO_JOIN:
                    UndoEditingJoin(*it);
                    break;
                case SVID_UNDO_SPLIT:
                    UndoEditingSplit(*it);
                    break;
                case SVID_UNDO_DELETE_LINE:
                    UndoEditingDeleteLine(*it);
                    break;
                case SVID_UNDO_LINE_COMMENT:
                    UndoEditingLineComment(*it);
                    break;
            }
        }

        CheckBlockTag();  // 重算 block comment 類的語法高亮度

        m_carets.m_caretsList = caretInfo.caretsList;
        m_carets.SetFirstIndex(caretInfo.first_index);
        m_carets.SetLastIndex(caretInfo.last_index);

        if (m_savedSeqCode == m_undoActions.DeleteLastUndoActions())
        {
            // Changing the buffer display name from foo* to foo.
            Modified(false);
        }
        else
        {
            Modified(true);
        }

    }

}

// 原則上是單行的資料
void svBufText::UndoEditingDelete(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();
    int c = p_act.GetStartCol();

    wxString txt = DeleteTextAt(r, c, c+p_act.GetText().Length());
    // wxString txt = DeleteTextAt(r, c, c+p_act.GetText().Length());

    GetLine(r)->ConvertString2Buffer(m_charSet);
    GetLine(r)->ConvertBufferICU(m_CScharSet);
    CreateKeywordTableAt(r);
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行

}

// 原則上是單行的資料
void svBufText::UndoEditingInsert(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();
    int c = p_act.GetStartCol();

    GetLine(r)->InsertTextUW(c, p_act.GetText());

    GetLine(r)->ConvertString2Buffer(m_charSet);
    GetLine(r)->ConvertBufferICU(m_CScharSet);
    CreateKeywordTableAt(r);
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行

}

// 能處理多行的資料
void svBufText::UndoEditingCut(const svUndoAction &p_act)
{
    int sr = p_act.GetStartRow();
    int sc = p_act.GetStartCol();
    int er = p_act.GetEndRow();
    int ec = p_act.GetEndCol();

    wxString txt = DeleteTextRange(sr, sc, er, ec);

    GetLine(sr)->ConvertString2Buffer(m_charSet);
    GetLine(sr)->ConvertBufferICU(m_CScharSet);
    CreateKeywordTableAt(sr);
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行

}

// 能處理多行的資料
void svBufText::UndoEditingPaste(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();
    int c = p_act.GetStartCol();

    int insertedLineCnt = 0;
    int lastLineLen = 0;
    InsertTextAt(r, c, p_act.GetText(), insertedLineCnt, lastLineLen);

    for(int i=0; i<insertedLineCnt; i++)
    {
        GetLine(r+i)->ConvertString2Buffer(m_charSet);
        GetLine(r+i)->ConvertBufferICU(m_CScharSet);
        CreateKeywordTableAt(r+i);
    }
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行

}

void svBufText::UndoEditingJoin(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();
    int c = p_act.GetStartCol();

    JoinNextLineAt(r);

    GetLine(r)->ConvertString2Buffer(m_charSet);
    GetLine(r)->ConvertBufferICU(m_CScharSet);
    CreateKeywordTableAt(r);
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行
}

void svBufText::UndoEditingSplit(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();
    int c = p_act.GetStartCol();

    SplitLineAt(r, c);
    // GetLine(r).SplitLineAt(v_c);
    GetLine(r)->ConvertString2Buffer(m_charSet);
    GetLine(r)->ConvertBufferICU(m_CScharSet);

    GetLine(r+1)->ConvertString2Buffer(m_charSet);
    GetLine(r+1)->ConvertBufferICU(m_CScharSet);

    CreateKeywordTableAt(r);
    CreateKeywordTableAt(r+1);
    // CheckBlockTag();  // 重算 block comment 類的語法高亮度 << 避免重覆執行，移到 UndoEditing 執行
}

void svBufText::UndoEditingDeleteLine(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();

    DeleteLineAt(r);
}

void svBufText::UndoEditingLineComment(const svUndoAction &p_act)
{
    int r = p_act.GetStartRow();

    wxString lineComment;
    if (GetLineCommentSymbol(lineComment))
    {
        LineCommentLineAt(r, lineComment);    

        GetLine(r)->ConvertString2Buffer(m_charSet);
        GetLine(r)->ConvertBufferICU(m_CScharSet);

        CreateKeywordTableAt(r);
    }
}

// Initialize a new UndoActions
// UndoActions is a vector of svUndoAction.
// Everytime we initialize a undoActions means buffer updated.
void svBufText::InitialNewUndoActions(void)
{
    // Make Sure save the correct sequentialCode
    sv_seq_code seqCode;
    if (IsModified())
    {
        seqCode = m_seqCodeGen.GetCurCode();
    }
    else
    {
        seqCode = m_savedSeqCode;
        m_seqCodeGen.GetCurCode();
    }

    m_undoActions.Append(seqCode);   // initial a empty actions list
    carets_info c;
    c.caretsList = m_carets.m_caretsList;
    c.first_index = m_carets.GetFirstIndex();
    c.last_index = m_carets.GetLastIndex();
    m_undoActions.RecordCarets(c);  // keep the current carets information.

    Modified(true);
}

// 檢查是否是與上個動作相同的操作
// 如:重覆按下delete、backdelete或重覆輸入新增文字
// 用以在作 undo 處理時, 將相同的動作合併在一起
// 只判斷單行的狀況
// 要考慮多重 carets 的狀況，比對座標時要找到對的 carets 來比較
bool svBufText::CheckContinousUndoOperation(const int p_originalType)
{
    if (p_originalType!=SVID_UNDO_DELETE &&
        p_originalType!=SVID_UNDO_BACKDEL &&
        p_originalType!=SVID_UNDO_INSERT)
        return false;

    int undoAction=0;

    // 取得對應的 undo 動作
    switch (p_originalType)
    {
        case SVID_UNDO_DELETE:
            undoAction = SVID_UNDO_INSERT;
            break;
        case SVID_UNDO_BACKDEL:
            undoAction = SVID_UNDO_INSERT;
            break;
        case SVID_UNDO_INSERT:
            undoAction = SVID_UNDO_DELETE;
            break;
    }


    if (!m_undoActions.Size())         // 沒有前一個紀錄的 undoActions
        return false;

    if (!IsModified())                 // 之前存檔過了, 視為不連續
        return false;

    vector<svUndoAction> actList = m_undoActions.GetLastUndoActions();

    
    int as = actList.size();            // 沒有前一個紀錄的 undoActions
    if (!as)
        return false;

    if (!(as>=(int)m_carets.Size()))          // 前一個紀錄的 undoActions 的 carets 與 現有的 carets 數不相同
        return false;

    for (int i=(int)m_carets.Size(); i>0; i--)   // 比較是否所有的 carets 的 undoActionType 皆與傳入值的 undoActions 相同
    {
        if (actList.at(as-i).GetType()!=undoAction)
            return false;
    }

    carets_info ci = m_undoActions.GetLastCarets();
    vector<svCaret> cList = ci.caretsList;

    if (!cList.size())
        return false;

    if (cList.size()!=m_carets.Size())       // 前一個紀錄的 carets 數量與現有 carets 數量不符
        return false;

    // 比對位置是否是連續的位置(重覆按下 delete, backdel, 或重覆輸入字元)
    for (int i=0; i<(int)cList.size(); i++)
    {
        int actIdx = actList.size()-1-i;   // 對應的 undoAction 位在 vector 的哪一個位置
        switch(p_originalType)
        {
            case SVID_UNDO_DELETE:
                if (!(cList.at(i).GetRow()==m_carets.At(i)->GetRow() &&
                     // cList.at(i).GetVisualCol()==m_carets.At(i)->GetVisualCol()))
                     actList.at(actIdx).GetStartCol()==m_carets.At(i)->GetVisualCol()))
                {
                    return false;
                }
                break;
            case SVID_UNDO_INSERT:
                if (!(cList.at(i).GetRow()==m_carets.At(i)->GetRow() &&
                     // cList.at(i).GetVisualCol()+actList.at(actIdx).GetText().Length()==m_carets.At(i)->GetVisualCol()))
                     actList.at(actIdx).GetStartCol()+actList.at(actIdx).GetText().Length()==m_carets.At(i)->GetVisualCol()))
                {
                    return false;
                }
                break;
            case SVID_UNDO_BACKDEL:
                if (!(cList.at(i).GetRow()==m_carets.At(i)->GetRow() &&
                     // cList.at(i).GetVisualCol()-actList.at(actIdx).GetText().Length()==m_carets.At(i)->GetVisualCol()))
                     // actList.at(actIdx).GetStartCol()-actList.at(actIdx).GetText().Length()==m_carets.At(i)->GetVisualCol()))
                     actList.at(actIdx).GetStartCol()==m_carets.At(i)->GetVisualCol()))
                {
                    return false;
                }
                break;
        }
    }

    return true;

}


/* -------------------------------------------------------------------------------------
 * find & replace related functions.
 * ------------------------------------------------------------------------------------- */


bool svBufText::FindMatchLocations(const svFindReplaceOption &p_fro)
{
    m_matchLocations.clear();

    // record the find and replace option.
    // Only used on find situation.
    // When F3/Alt+F3(find next & find prev), awvic have to check if the file was modified and research(refind) again.
    m_frOption = p_fro;
    if (p_fro.m_from == svFindReplaceOption::SVID_FIND_FROM_KEYSTROKE)
    {
        m_frOption.m_from = svFindReplaceOption::SVID_FIND_FROM_LAST_KEYSTROKE;
    }
    bool result = false;
    int ofs=0;  // useless

    switch (p_fro.GetType())
    {
        case svFindReplaceOption::SVID_FIND_WXSTR:
            result = FindwxStrMatchLocations(m_frOption, m_matchLocations);
            break;
        case svFindReplaceOption::SVID_FIND_UCHAR:
            if (!GetCaretsCurrentUChar(&m_frOption.m_currentWord2Find.uctext, m_frOption.m_currentWord2Find.len, ofs))
                return false;
            result = FindUCharMatchLocations(m_frOption.m_currentWord2Find, m_matchLocations);
            break;
        case svFindReplaceOption::SVID_FIND_LAST_UCHAR:
            if (m_frOption.m_currentWord2Find.IsEmpty())
                return false;
            result = FindUCharMatchLocations(m_frOption.m_currentWord2Find, m_matchLocations);
            break;
        case svFindReplaceOption::SVID_FIND_REGEX:
            result = FindRegexMatchLocations(m_frOption, m_matchLocations);
            break;
    }

    // In Selection option check.
    if (result)
    {
        m_searchUndoStatus = m_undoActions.GetUndoStatus();
        if (p_fro.m_inSelect)
        {
            MatchOnSelection();
            if (!m_matchLocations.size())
                result = false;
        }
    }

    return result;

}

// 尋找所有能找到的 keyword
// All founded locations(row, col) will be stored in p_locations.
bool svBufText::FindUCharMatchLocations(const UCharText &p_keyword, vector<svInt2Pair> &p_locations)
{
    int totalLine = LineCntUW();
    int remainedCount = totalLine;  // 最多要搜尋的次數
    int row_id = 0;
    int fcol = 0;
    int scol = 0;

    bool result = false;
    bool found = false;
    while(remainedCount>0)
    {
        found = false;
        found = GetLine(row_id)->FindNextKeywordUCharFrom(p_keyword, scol, fcol);
        if (found)
        {
            result = true;
            scol = fcol + 1;
            p_locations.push_back(svInt2Pair(row_id, fcol, row_id, fcol + p_keyword.len));
        }
        else
        {
            scol = 0;
            --remainedCount;
            ++row_id;
        }
    }

    return result;
}

// 將找到所有 match p_keyword 的字串儲存在 p_locations 內
// 目前不處理 p_keyword 跨行的狀況
bool svBufText::FindwxStrMatchLocations(const svFindReplaceOption &p_fro, vector<svInt2Pair> &p_locations)
{
    int totalLine = LineCntUW();
    int remainedCount = totalLine;  // 最多要搜尋的次數
    int row_id = 0;
    int fcol = 0;
    int scol = 0;

    bool result = false;
    bool found = false;

    if (p_fro.m_wxStr2Find.Length()==0)
        return result;

    while(remainedCount>0)
    {
        found = false;
        found = GetLine(row_id)->FindNextKeywordwxStrFrom(p_fro.m_wxStr2Find, m_charSet, scol, fcol, p_fro.m_case);
        if (found)
        {
            result = true;
            scol = fcol + 1;
            p_locations.push_back(svInt2Pair(row_id, fcol, row_id, fcol+p_fro.m_wxStr2Find.Length()));
        }
        else
        {
            scol = 0;
            --remainedCount;
            ++row_id;
        }
    }

    return result;

}


// Find by regular expression and return match location(s). (row, col)
bool svBufText::FindRegexMatchLocations(const svFindReplaceOption &p_fro, vector<svInt2Pair> &p_locList)
{
    bool result = false;

    if (p_fro.m_wxStr2Find.Length()==0)
        return result;

    vector<svIntPair> matchLocations;
    // vector<svIntPair> newlineLocations;

    UChar *uBufNewLine = NULL;
    int32_t uBufNewLineLen = 0;

    UChar *uBufPattern = NULL;
    int32_t uBufPatternLen = 0;

    char *cBufFile = NULL;
    int32_t cBufFileLen = 0;

    // UChar *uBufFile = NULL;
    // int32_t uBufFileLen = 0;

    p_locList.clear();

    wxString newlinePattern = _("(\r\n|\n|\r)");

    if (ConvertwxStrToUChar(newlinePattern, &uBufNewLine, uBufNewLineLen)==U_ZERO_ERROR)
    {
        if (ConvertwxStrToUChar(p_fro.m_wxStr2Find, &uBufPattern, uBufPatternLen)==U_ZERO_ERROR)
        {
            // if (m_bufUCharAllDirty)
            // if (m_searchUndoStatus!=m_undoActions.GetUndoStatus())  // The file was modified since last search.
            if (FileChangedSinceLastSearch()) // The file was modified since last search.
            {
                if (m_bufUCharAll) free(m_bufUCharAll);
                m_bufUCharAll = NULL;
                m_newlineLocations.clear();

                if (SaveToBigBuffer(&cBufFile, cBufFileLen))
                {
                    if (ConvBufferICU(cBufFile, cBufFileLen, &m_bufUCharAll, &m_bufUCharAllLen)==U_ZERO_ERROR)
                    {
                        // m_bufUCharAllDirty = false;
                        // match pattern locations.
                        // if (REFind(uBufFile, uBufFileLen, uBufPattern, uBufPatternLen, matchLocations, p_fro.m_case)==U_ZERO_ERROR)
                        if (REFind(m_bufUCharAll, m_bufUCharAllLen, uBufPattern, uBufPatternLen, matchLocations, p_fro.m_case)==U_ZERO_ERROR)
                        {
                            // WHY NESTING SO DEEP! 

                            // newline locations.
                            REFind(m_bufUCharAll, m_bufUCharAllLen, uBufNewLine, uBufNewLineLen, m_newlineLocations, p_fro.m_case);

                            // cal (row, col) locations.
                            CreateLineRegexFindResult(matchLocations, m_newlineLocations, p_locList);

                            result = true;
                        }
                    }
                }

            }
            else // improving the performance. Not reconverting UChar buffer every times.
            {
                // match pattern locations.
                if (REFind(m_bufUCharAll, m_bufUCharAllLen, uBufPattern, uBufPatternLen, matchLocations, p_fro.m_case)==U_ZERO_ERROR)
                {
                    // WHY NESTING SO DEEP! 

                    // cal (row, col) locations.
                    CreateLineRegexFindResult(matchLocations, m_newlineLocations, p_locList);

                    result = true;
                }
            }
        }
    }


    if (uBufNewLine)
    {
        free(uBufNewLine);
    }

    if (uBufPattern)
    {
        free(uBufPattern);
    }

    if (cBufFile)
    {
        free(cBufFile);
    }

    // if (uBufFile)
    // {
    //     free(uBufFile);
    // }

    return result;

}

// Break regular match result into lines by line
// 逐一比對 crlf(newline)的位置，以算出是哪一行，再將 regular match 結果寫入該行
bool svBufText::CreateLineRegexFindResult(vector<svIntPair> &p_foundList, vector<svIntPair> &p_crlfList, vector<svInt2Pair> &p_resultList)
{
    int32_t lineNo = 0;
    int32_t keep_pos = 0;

    int lineCnt = LineCntUW();  // max line num.



    vector<svIntPair> sposList, eposList;  // Start position and end position.


    std::vector<svIntPair>::iterator it_FL=p_foundList.begin();
    std::vector<svIntPair>::iterator it_crlf=p_crlfList.begin();

    // sequential match and write result to sposList. For start position.
    while(it_FL!=p_foundList.end())
    {
        if (it_FL->num1<it_crlf->num2)
        {
            sposList.push_back(svIntPair(lineNo, it_FL->num1-keep_pos));
        }
        // else if (it_FL->num1==it_crlf->num2)
        // {
        //     if (it_crlf!=p_crlfList.end())
        //     {
        //         keep_pos=it_crlf->num2;
        //         ++it_crlf;
        //         ++lineNo;
        //     }
        //     sposList.push_back(svIntPair(lineNo, it_FL->num1-keep_pos));
        // }
        // else if (it_FL->num1>it_crlf->num2)
        else if (it_FL->num1>=it_crlf->num2)
        {
            while(it_crlf!=p_crlfList.end()&&it_FL->num1>=it_crlf->num2)
            {
                keep_pos=it_crlf->num2;
                ++it_crlf;
                ++lineNo;
            }
            sposList.push_back(svIntPair(lineNo, it_FL->num1-keep_pos));
        }
        ++it_FL;
    }

    /* --------------------------------------------------------------- */

    it_FL=p_foundList.begin();
    it_crlf=p_crlfList.begin();

    lineNo = 0;
    keep_pos = 0;

    // sequential match and write result to eposList. For end position.
    // num1 = match string start position
    // num2 = match string end position
    while(it_FL!=p_foundList.end())
    {
        if (it_FL->num2<it_crlf->num2)
        {
            eposList.push_back(svIntPair(lineNo, it_FL->num2-keep_pos));
        }
        // else if (it_FL->num2==it_crlf->num2)
        // {
        //     if (it_crlf!=p_crlfList.end())
        //     {
        //         keep_pos=it_crlf->num2;
        //         ++it_crlf;
        //         ++lineNo;
        //     }
        //     eposList.push_back(svIntPair(lineNo, it_FL->num2-keep_pos));
        // }
        // else if (it_FL->num2>it_crlf->num2)
        else if (it_FL->num2>=it_crlf->num2)
        {
            while(it_crlf!=p_crlfList.end()&&it_FL->num2>=it_crlf->num2)
            {
                keep_pos=it_crlf->num2;
                ++it_crlf;
                ++lineNo;
            }
            eposList.push_back(svIntPair(lineNo, it_FL->num2-keep_pos));
        }
        ++it_FL;
    }

    /* ----------------------------------------------------------------- */
    if (sposList.size()==eposList.size())
    {
        p_resultList.clear();
        for (int i=0; i<(int)sposList.size(); i++)
        {
            p_resultList.push_back(svInt2Pair(sposList.at(i).num1, sposList.at(i).num2,
                                              eposList.at(i).num1, eposList.at(i).num2));
        }
    }
    else
    {
        wxLogMessage(wxString::Format("svBufText::CreateLineRegexFindResult the number of start position and end position is different. s=%i e=%i", sposList.size(), eposList.size()));
    }

    return true;
}

void svBufText::MatchOnSelection(void)
{
    if (!m_matchLocations.size())
        return;

    if (!CaretsHasSelect())
        return;

    vector<svInt2Pair> m_onSelectLocations;

    for (std::vector<svInt2Pair>::iterator it=m_matchLocations.begin();
         it!=m_matchLocations.end();
         ++it)
    {
        bool match = false;
        // svCaret should be ordered by it's position.
        for (std::vector<svCaret>::iterator it2=m_carets.m_caretsList.begin();
             it2!=m_carets.m_caretsList.end();
             ++it2)
        {
            int ssr, ssc, ser, sec;
            ssr=ssc=ser=sec=0;
            it2->GetSelectPos(ssr, ssc, ser, sec);
            if (it->num1>ser)
                continue;
            if (it->num3<ssr)
                break;

            double bs = it->num1 * 10000000000 + it->num2;
            double be = it->num3 * 10000000000 + it->num4;
            double ts = ssr * 10000000000 + ssc;
            double te = ser * 10000000000 + sec;

            if ((bs>=ts&&bs<=te) &&
                (be>=ts&&be<=te))
            {
                match = true;
                break;
            }
        }

        if (match)
        {
            m_onSelectLocations.push_back(*it);
        }
    }

    m_matchLocations.clear();
    m_matchLocations = m_onSelectLocations;
}

// There should be only 1 caret left before call this function.
bool svBufText::CaretMoveToMatch(int p_direction, int &p_destRow, int &p_destCol, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    if (!m_matchLocations.size())
        return false;

    if (!m_carets.Size())
        return false;

    std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
    int r, v_c;    // r : row   v_c : visual column
    it->GetPosition(r, v_c);

    bool found = false;

    int startRow, startCol;
    startRow = startCol = 0;

    if (p_direction==SVID_NEXT_MATCH)
    {
        for (std::vector<svInt2Pair>::iterator it2=m_matchLocations.begin();
             it2!=m_matchLocations.end();
             ++it2)
        {
            if ( it2->num1>r || 
                (it2->num1==r&&it2->num2>v_c))
            {
                found = true;
                startRow = it2->num1;
                startCol = it2->num2;
                p_destRow = it2->num3;
                p_destCol = it2->num4;
                break;
            }
        }

        if (!found)
        {
            std::vector<svInt2Pair>::iterator it2=m_matchLocations.begin();
            found = true;
            startRow = it2->num1;
            startCol = it2->num2;
            p_destRow = it2->num3;
            p_destCol = it2->num4;
        }
    }
    else if (p_direction==SVID_PREV_MATCH)
    {
        for (std::vector<svInt2Pair>::reverse_iterator it2=m_matchLocations.rbegin();
             it2!=m_matchLocations.rend();
             ++it2)
        {
            if ( it2->num3<r || 
                (it2->num3==r&&it2->num4<v_c))
            {
                found = true;
                startRow = it2->num1;
                startCol = it2->num2;
                p_destRow = it2->num3;
                p_destCol = it2->num4;
                break;
            }
        }

        if (!found)
        {
            std::vector<svInt2Pair>::reverse_iterator it2=m_matchLocations.rbegin();
            found = true;
            startRow = it2->num1;
            startCol = it2->num2;
            p_destRow = it2->num3;
            p_destCol = it2->num4;
        }
    }

    if (found)
    {
        it->SetPosition(startRow, startCol);
        it->SetSelect();
        it->SetPosition(p_destRow, p_destCol);
        ProcWrapAndStyleAt(p_destRow, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
        it->SetKeepSpace(GetLine(p_destRow)->Col2SpaceUW(p_destCol));
    }

    return  found;

}

// There should be only 1 caret available when calling this fucntion.
// p_destRow will be set to the location keyword found.
bool svBufText::CaretMoveToAllMatch(int &p_destRow, int &p_destCol, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{

    if (!m_matchLocations.size())
        return false;

    if (!m_carets.Size())
        return false;

    // 先找到最接近目前游標的一個字串，保留其值
    std::vector<svCaret>::iterator it=m_carets.m_caretsList.begin();
    int r, v_c;    // r : row   v_c : visual column
    it->GetPosition(r, v_c);

    bool found = false;

    for (std::vector<svInt2Pair>::iterator it2=m_matchLocations.begin();
         it2!=m_matchLocations.end();
         ++it2)
    {
        if ( it2->num1>r || 
            (it2->num1==r&&it2->num2>v_c))
        {
            found = true;
            p_destRow = it2->num3;
            p_destCol = it2->num4;
            break;
        }
    }

    if (!found)
    {
        std::vector<svInt2Pair>::iterator it2=m_matchLocations.begin();
        found = true;
        p_destRow = it2->num3;
        p_destCol = it2->num4;
    }

    // clear all carets
    m_carets.Clear();

    for(std::vector<svInt2Pair>::iterator it=m_matchLocations.begin();
        it!=m_matchLocations.end();
        ++it)
    {
        // adding found locations into carets
        svCaret c(it->num1, it->num2);
        c.SetSelect();
        c.SetPosition(it->num3, it->num4);
        ProcWrapAndStyleAt(it->num3, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap); // 為了呼叫下一行所作的準備，下一行的處理需要折行的相關資料，所以要先行計算
        c.SetKeepSpace(GetLine(it->num3)->Col2SpaceUW(it->num4));
        m_carets.Append(c);
    }

    return true;

}

// uw_idx 行數
// w_idx_s ~ w_idx_e 哪個字元到哪個字元
// 所以處理的實際上是一個折行
// 取得 與 m_currentWrod2find 相同字串的位置資訊
// 作法參考 svBufText::GetCaretSelectPixelAt
bool svBufText::GetFindMatchPixelAtW(const size_t uw_idx, const size_t w_idx_s, const size_t w_idx_e, const int p_spaceWidth, vector<int> &p_pixelList)
{
    // 將找到的像素清單存放在 vector 內
    // 為何要用 vector 存放資料呢?
    // 因為有同一行可能有多段文字與m_currentWord2Find相同
    // p_pixelList 的 size() 一律是偶數，奇數值是啟始的pixel值，偶數值是結束的pixel值
    // 如為跨行標示則尾段固定 + p_spaceWidth 以表示eol

    p_pixelList.clear();
    bool founded = false;
    int startPixel = 0;
    int endPixel = 0;

    for(std::vector<svInt2Pair>::iterator it=m_matchLocations.begin();
        it!=m_matchLocations.end();
        ++it)
    {
        int srow, scol, erow, ecol;
        srow = it->num1;
        scol = it->num2;
        erow = it->num3;
        ecol = it->num4;

        if ((int)uw_idx>=srow && (int)uw_idx<=erow)
        {
            int fscol, fecol;
            fscol=fecol=0;
            bool feol=false;

            bool result = get_only_the_line_position(uw_idx, w_idx_s, w_idx_e, fscol, fecol, feol, srow, scol, erow, ecol);

            if (result)
            {
                // 計算 pixel 值
                GetLine(uw_idx)->TextCol2PixelW(fscol, fecol, startPixel, endPixel);
                p_pixelList.push_back(startPixel);
                p_pixelList.push_back(endPixel);
                founded = true;
            }

        }
    }

    return founded;  

}

bool svBufText::Replace(int p_direction, const svFindReplaceOption &p_fro, int &p_newRow, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    if (!m_matchLocations.size())
    {
        if (!FindMatchLocations(p_fro))   // refind again.
            return false;
        if (!m_matchLocations.size())
            return false;
    }

    int destRow, destCol;
    destRow = destCol = 0;

    bool found = false; 
    if (p_direction==SVID_NEXT_MATCH)
        found = CaretMoveToMatch(SVID_NEXT_MATCH, destRow, destCol, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);
    else
        found = CaretMoveToMatch(SVID_PREV_MATCH, destRow, destCol, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);


    if (found)
    {
        p_newRow = destRow;
        EditingSelectedTextDelete();
        if (p_fro.m_wxStr2Replace.Length())
        {
            EditingInsertChar(p_fro.m_wxStr2Replace, false, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);
            CaretsSelectCharacters(p_fro.m_wxStr2Replace.Length());
        }
        // m_bufUCharAllDirty = true;
        FindMatchLocations(p_fro);
    }

    return found;
}

bool svBufText::ReplaceAll(const svFindReplaceOption &p_fro, int &p_newRow, size_t p_tabWidth, size_t p_spacePixWidth, const wxFont& p_font, size_t p_screenWidth, bool p_isWrap)
{
    if (!m_matchLocations.size())
    {
        if (!FindMatchLocations(p_fro))   // refind again.
            return false;
        if (!m_matchLocations.size())
            return false;
    }

    int dstLine = 0;
    int dstCol = 0;
    bool found = CaretMoveToAllMatch(dstLine, dstCol, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);

    if (found)
    {
        p_newRow = dstLine;
        EditingSelectedTextDelete();
        ClearCaretSelect();
        if (p_fro.m_wxStr2Replace.Length())
        {
            EditingInsertChar(p_fro.m_wxStr2Replace, false, p_tabWidth, p_spacePixWidth, p_font, p_screenWidth, p_isWrap);
            CaretsSelectCharacters(p_fro.m_wxStr2Replace.Length());
        }
        // m_bufUCharAllDirty = true;
        FindMatchLocations(p_fro);
    }

    return found;
}

/* ----------------------------------------------------------------------- */

// Save data to file.
bool svBufText::SaveToFile(const wxString &p_filename)
{
    const char *cfilename = svCommonLib::wxStrFilename2Char(p_filename);

    FILE *f;

    f = fopen(cfilename, "wb");
    if (!f)
    {
        wxLogMessage("svBufText::SaveToFile Error: Unable to open file.");
        return false;
    }
    for (int i=0; i<(int)LineCntUW(); ++i)
    {
        fwrite(GetLine(i)->GetBuffer(), GetLine(i)->GetBufferLen(), 1, f);
    }
    fclose(f);

    m_filename = p_filename;

    m_savedSeqCode = m_seqCodeGen.CurCode();
    Modified(false);

    return true;

}

// Save data to a buffer.
// REMEMBER TO FREE p_buffer. <= I REALLY LIKE POINTER!!! lol
bool svBufText::SaveToBigBuffer(char **p_buffer, int &p_bufLen)
{
    int totalLen = 0;
    int subTotal = 0;

    for (int i=0; i<(int)LineCntUW(); ++i)
    {
        totalLen += GetLine(i)->GetBufferLen();
    }

    char *buffer = (char *) malloc (sizeof(char) * totalLen);
    p_bufLen = totalLen;

    for (int i=0; i<(int)LineCntUW(); ++i)
    {
        memcpy(buffer+subTotal, GetLine(i)->GetBuffer(), GetLine(i)->GetBufferLen());
        subTotal += GetLine(i)->GetBufferLen();
    }

    *p_buffer = buffer;

    return true;

}


/*----------------------------------------------------------------------------------------
 *
 * Folding related functions
 *
 *----------------------------------------------------------------------------------------*/

// for example
// 10 if (a>0)
// 11 {
// 12     abc
// 13     def
// 14 }
//
// call AddFoldingAt (11, 14) and stored a fi on line 11 as fi.range=2.
bool svBufText::FoldingAt(int uw_idx_s, int uw_idx_e)
{
    if (uw_idx_e+1<=uw_idx_s) return false;

    foldingInfo fi(uw_idx_e - uw_idx_s - 1);
    GetLine(uw_idx_s)->AddFolding(fi);
    for (int i=uw_idx_s+1; i<uw_idx_e; i++)
    {
        GetLine(i)->Visible(false);
    }
    return true;
}

// 將指定行及相關行 unfolding
// If the specified line has folding information. unfolding it.
// If the specified line is invisible and has no folding information.
//   find out which line's floding information contain the specified line and unfolding them.
bool svBufText::UnfoldingAt(int uw_idx)
{
    foldingInfo fi;
    foldingInfo fi2;
    if (GetLine(uw_idx)->RemoveFolding(fi))
    {
        for (int i=uw_idx+1; i<=uw_idx+fi.range; i++)
        {
            GetLine(i)->Visible(true);
            if (GetLine(i)->HadFolding(fi2))
            {
                // for nested folding.
                i += fi2.range;
                continue;
            }
        }
        return true;
    }

    else if (!GetLine(uw_idx)->Visible() && uw_idx>0)
    {
        int r = uw_idx;
        // nested folding
        while (!GetLine(r)->Visible())
        {
            int new_r=0;
            if (GetPrevVisibleLine(r, new_r))
            {
                if (GetLine(new_r)->RemoveFolding(fi))
                {
                    for (int i=new_r+1; i<=new_r+fi.range; i++)
                    {
                        GetLine(i)->Visible(true);
                        if (GetLine(i)->HadFolding(fi2))
                        {
                            // for nested folding.
                            i += fi2.range;
                            continue;
                        }
                    }
                    return true;
                }

                // r = new_r;
                if (r<=0)    // Exception handle.
                    break;
            }
            else
            {
                break;
            }
        }        

    }
    return false;
}

// unfolding on carets position line.
void svBufText::UnfoldingAtCarets(void)
{
    for(std::vector<svCaret>::reverse_iterator it=m_carets.m_caretsList.rbegin();
        it!=m_carets.m_caretsList.rend();
        ++it)        
    {
        if (it->HasSelect())
        {
            // If carets has selection. unfolding it.

            int sr, sc, er, ec;
            sr=sc=er=ec=0;
            it->GetSelectPos(sr, sc, er, ec);

            // nested folding
            while (!GetLine(sr)->Visible())
            {
                int new_r=0;
                if (GetPrevVisibleLine(sr, new_r))
                {
                    UnfoldingAt(new_r);
                    // r = new_r;
                    if (sr<=0)    // Exception handle.
                        break;
                }
                else
                {
                    break;
                }
            }

            // nested folding
            while (!GetLine(er)->Visible())
            {
                int new_r=0;
                if (GetPrevVisibleLine(er, new_r))
                {
                    UnfoldingAt(new_r);
                    // r = new_r;
                    if (er<=0)    // Exception handle.
                        break;
                }
                else
                {
                    break;
                }
            }          

        }
        else
        {
            int r, v_c;
            r=v_c=0;
            it->GetPosition(r, v_c);

            // nested folding
            while (!GetLine(r)->Visible())
            {
                int new_r=0;
                if (GetPrevVisibleLine(r, new_r))
                {
                    UnfoldingAt(new_r);
                    // r = new_r;
                    if (r<=0)    // Exception handle.
                        break;
                }
                else
                {
                    break;
                }
            }
        }
    }

}


// (p_cr, p_cc) base position (row, col)
// p_ps pairSymbol
// p_range_sr the start row number the range we searched.
// p_range_er the end col number the range we searched.
// (p_sr, p_sc) the location of start embrace symbol in (row, col)
// (p_er, p_ec) the location of end   embrace symbol in (row, col)
void svBufText::FindEmbracePosInRange(const int p_cr, const int p_cc, vector<pairSymbol> &p_ps, const int p_range_sr, const int p_range_er, int &p_sr, int &p_sc, int &p_er, int &p_ec, bool &p_sfound, bool &p_efound, pairSymbol &p_sFoundSymbol, pairSymbol &p_eFoundSymbol)
{
    vector<pairSymbol> pairSymbols;

    for(std::vector<pairSymbol>::iterator it=m_embraceSymbol.begin();
        it!=m_embraceSymbol.end();
        ++it)
    {

        pairSymbols.clear();
        pairSymbols.push_back(*it);


        pairSymbol ps;
        int start_col = p_cc;
        int found_Col = 0;

        p_sr = p_sc = p_er = p_ec = -1;
        p_sfound = p_efound = false;

        int end_cnt, start_cnt;
        end_cnt = start_cnt = 0;


        // Finding start symbol.

        for (int i=p_cr; i>=p_range_sr&&i>=0; i--)
        {
            if (GetLine(i)->KeywordContainSymbolStart(pairSymbols, start_col, found_Col, end_cnt, ps))
            {
                p_sfound = true;
                p_sr = i;
                p_sc = found_Col;
                p_sFoundSymbol = ps;
                break;
            }
            start_col = -1;  // only the first line need to be start from specified col.
        }

        // Finding End Symbol.

        start_col = p_cc;
        for (int i=p_cr; i<=p_range_er; i++)
        {
            // if (GetLine(i)->KeywordContainSymbolEnd(p_ps, start_col, found_Col, start_cnt, ps))
            if (GetLine(i)->KeywordContainSymbolEnd(pairSymbols, start_col, found_Col, start_cnt, ps))
            {
                p_efound = true;
                p_er = i;
                p_ec = found_Col;
                p_eFoundSymbol = ps;
                break;
            }
            start_col = -1;  // only the first line need to be start from specified col.
        }

        if (p_sfound&&p_efound)
            return;

    }

}

bool svBufText::GetDefinitionLineNo(vector<svIntText> &p_defLineNo)
{
    // if (m_synDefPid<0)
    if (!m_synDefPid.size())
        return false;

    p_defLineNo.clear();

    for (int i=0; i<(int)LineCntUW(); i++)
    {
        keywordUnit k;
        wxString str;
        if (GetLine(i)->KeywordContainPID(m_synDefPid, k))
        {
            if (ConvertUCharTowxStr(k.k_text, k.k_len, str)==U_ZERO_ERROR)
            {
                svIntText lt;
                lt.m_lineNo = i;
                lt.m_text = str;
                lt.m_keyword = k;
                p_defLineNo.push_back(lt);
            }
        }
    }

    if (p_defLineNo.size()>0)
    {
        std::sort(p_defLineNo.begin(), p_defLineNo.end());
        return true;
    }
    else
        return false;

}
