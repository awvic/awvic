/*
   Copyright Notice in awvic.cpp
*/

#include "svDictionary.h"

#include "stdwx.h"

svDictionary::svDictionary()
{
}

svDictionary::~svDictionary()
{
    for (map<UCharText, innerMap>::iterator it=m_dict.begin();
         it!=m_dict.end();
         ++it)
    {
        it->second.clear();
    }
    m_dict.clear();
}

void svDictionary::Add(const UChar *p_ktext, const size_t p_klen)
{
    UCharText uc;

    uc.uctext = (UChar *)malloc(sizeof(UChar) * p_klen);
    u_memcpy(uc.uctext, p_ktext, p_klen);
    uc.len = p_klen;

    Add(uc);

}

void svDictionary::Add(const UCharText &p_key)
{
    UCharText lower_key;

    // 將 p_key 內的 UChar * 轉為小寫作為搜尋的 key 
    lower_key = svDictionary::UChar2Upper(p_key);

    std::map<UCharText, innerMap>::iterator it = m_dict.find(lower_key);

    if (it != m_dict.end())
    {
        // 再找下一層(innerMap)
        innerMap::iterator it2 = it->second.find(p_key);

        if (it2 != it->second.end())
        {
            it2->second.count += 1;
        }
        else
        {
            svVocabulary voc;

            // Convert UChar to char* encoding in UTF-8
            voc.text = svDictionary::UChar2WxStr(p_key);
            voc.count = 1;
#if __cplusplus > 199711L
            it->second.emplace(p_key, voc);
#else
            it->second.insert(pair<UCharText,svVocabulary>(p_key, voc));
#endif
        }
    }
    else
    {
        svVocabulary voc;

        // Convert UChar to char* encoding in UTF-8
        voc.text = svDictionary::UChar2WxStr(p_key);
        voc.count = 1;

        innerMap k;

#if __cplusplus > 199711L
        k.emplace(p_key, voc);
        m_dict.emplace(lower_key, k);
#else
        k.insert(pair<UCharText, svVocabulary>(p_key, voc));
        m_dict.insert(pair<UCharText,innerMap>(lower_key, k));
#endif
    }

}

void svDictionary::Sub(const UChar *p_ktext, const size_t p_klen)
{
    UCharText uc;

    uc.uctext = (UChar *)malloc(sizeof(UChar) * p_klen);
    u_memcpy(uc.uctext, p_ktext, p_klen);
    uc.len = p_klen;

    Sub(uc);

}

void svDictionary::Sub(const UCharText &p_key)
{
    UCharText lower_key;

    // 將 p_key 內的 UChar * 轉為小寫作為搜尋的 key 
    lower_key = svDictionary::UChar2Upper(p_key);

    std::map<UCharText, innerMap>::iterator it = m_dict.find(lower_key);

    if (it != m_dict.end())
    {
        // 再找下一層(innerMap)
        innerMap::iterator it2 = it->second.find(p_key);

        if (it2 != it->second.end())
        {
            it2->second.count -= 1;
            if (it2->second.count<=0)
            {
                it->second.erase(it2++);
            }
        }
    }

}

void svDictionary::Remove(const UCharText &p_key)
{
    UCharText lower_key;

    // 將 p_key 內的 UChar * 轉為小寫作為搜尋的 key 
    lower_key = svDictionary::UChar2Upper(p_key);

    std::map<UCharText, innerMap>::iterator it = m_dict.find(lower_key);

    if (it != m_dict.end())
    {
        it->second.clear();
        m_dict.erase(it++);
    }
}

vector<wxString> svDictionary::GetHitTextList(const UChar *p_ktext, const size_t p_klen)
{
    UCharText uc;

    uc.uctext = (UChar *)malloc(sizeof(UChar) * p_klen);
    u_memcpy(uc.uctext, p_ktext, p_klen);
    uc.len = p_klen;

    return GetHitTextList(uc);
}

vector<wxString> svDictionary::GetHitTextList(const UCharText &p_key)
{
    /*
        取得提示字串的規則：
        1.第一個字母要相同
        2.前 n 個字元要與現有字串相同
        3.大小寫完全相符者(提示字串與現有字串)不顯示

    */

    UCharText lower_key;

    // 將 p_key 內的 UChar * 轉為小寫作為搜尋的 key 
    lower_key = svDictionary::UChar2Upper(p_key);

    pair< map<UCharText, innerMap>::iterator, map<UCharText, innerMap>::iterator > ret;
    ret = m_dict.equal_range(lower_key);

    std::vector<wxString> r_hint;
    int count = 0;

    wxString p_wxstr = svDictionary::UChar2WxStr(p_key);     // Convert to wxString for first character compariment.

    while (ret.first != m_dict.end() && 
           count<SVID_MAX_HINT)
    {
        bool b_break = false;

        for(innerMap::iterator it2=ret.first->second.begin();
            it2!=ret.first->second.end();
            ++it2)
        {
            if (it2->second.text.Left(1).Lower()!=p_wxstr.Left(1).Lower())
            {
                b_break = true;
            }
            else
            {
                // if (it2->first.len!=p_key.len ||          
                if (it2->first.len>p_key.len ||          
                    //u_memcmp(ret.first->first.uctext, p_key.uctext, p_key.len)!=0) // 與所在字元相同者不顯示
                    // u_strncasecmp(it2->first.uctext, p_key.uctext, p_key.len, 0)!=0)  // 與所在字元相同者不顯示
                    u_strncmp(it2->first.uctext, p_key.uctext, p_key.len)!=0)  // 與所在字元相同者不顯示
                {
                    r_hint.push_back(it2->second.text);
                    ++count;
                }
            }
        }

        if (b_break) break;
        ++ret.first;
    }

    return r_hint;

}
