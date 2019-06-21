#include "wordfilter.h"
#include <unordered_map>
#include <vector>
#include <assert.h>

typedef std::string String;

#define LOG_DEBUG printf

struct Utf8Char
{
    union Data
    {
        uint64_t num;
        char ch[8];
    };

    Utf8Char()
    {
        m_data.num = 0;
    }

    operator const char*() const
    {
        return m_data.ch;
    }

    const char *c_str() const
    {
        return m_data.ch;
    }

    operator uint64_t() const
    {
        return m_data.num;
    }

    void operator = (const String &r)
    {
        assert(r.size() < sizeof(m_data.ch));
        memcpy(m_data.ch, r.c_str(), r.size());
    }

    void operator = (const char *r)
    {
        assert(strlen(r) < sizeof(m_data.ch));
        strncpy(m_data.ch, r, sizeof(m_data.ch));
    }

    Data m_data;
};


typedef std::vector<Utf8Char> Utf8ChSeq;


struct NextWord
{
    std::unordered_map<uint64_t, NextWord*> nextmap;
};

static NextWord gs_wordsLib;
static std::vector<NextWord*> gs_NWCache;

size_t utf8_to_charset(const String &input, Utf8ChSeq *output) 
{
    Utf8Char ch; 
    for (size_t i = 0, len = 0; i != input.length(); i += len) 
    {
        unsigned char byte = (unsigned)input[i];
        if (byte >= 0xFC) // lenght 6
            len = 6;  
        else if (byte >= 0xF8)
            len = 5;
        else if (byte >= 0xF0)
            len = 4;
        else if (byte >= 0xE0)
            len = 3;
        else if (byte >= 0xC0)
            len = 2;
        else
            len = 1;
        ch = input.substr(i, len);
        output->emplace_back(ch);
        //LOG_DEBUG(ch.c_str());
    }   
    return output->size();
}

bool FilterDirtyWord(const std::string &input, char *output)
{
	std::cout << "enter filter"<<std::endl;
    bool filtered = false;
    Utf8ChSeq out;
    utf8_to_charset(input, &out);
    auto *nextmap = &gs_wordsLib.nextmap;
    int startIdx = 0;
    for (int i = 0; startIdx + i < (int)out.size(); i++)
    {
        const Utf8Char &ch = out[i + startIdx];
        auto it = nextmap->find(ch);
        if (it != nextmap->end())
        {
            //LOG_DEBUG("hit %s %d\n", ch.c_str(), startIdx);
            nextmap = &it->second->nextmap;
            if (nextmap->find(0) != nextmap->end())
            {
                filtered = true;
                for (int j = 0; j < i + 1; j++)
                {
                    //LOG_DEBUG("%s -> %s\n", out[startIdx + j].c_str(), "*");
                    out[startIdx + j] = "*";
                }
                nextmap = &gs_wordsLib.nextmap;
                startIdx = i + 1 + startIdx;
                i = -1;
            }
        }
        else
        {
            //LOG_DEBUG("jump %s %d\n", ch.c_str(), i + startIdx);
            nextmap = &gs_wordsLib.nextmap;
            startIdx++;
            i = -1;
        }
    }

    LOG_DEBUG("finished\n");
    
    if (filtered)
    {
        //output->clear();
		memset(output,'\0',sizeof(char)*10000);
		
		int index = 0;
        for (auto &&ch : out)
        {
            // LOG_DEBUG("%s\n", ch.c_str());
            char *temp = (char*)ch.c_str();
            int len = strlen(temp);
            //cout<<len<<endl;
            unsigned char byte = (unsigned)temp[0];
            if (byte >= 0xFC) // lenght 6
                len = 6;
            else if (byte >= 0xF8)
                len = 5;
            else if (byte >= 0xF0)
                len = 4;
            else if (byte >= 0xE0) //中文字符
                len = 3;
            else if (byte >= 0xC0)
                len = 2;
            else
                len = 1;//英文，换行符0x0A
            //cout<<len<<endl;
            for(int t=0;t<len;t++) output[index++] = temp[t];

        }
       
        std::cout << "filtered = " << filtered << std::endl;
        std::cout <<output<< std::endl;
    }

    return filtered;
}

int AddWord(const String &w)
{
    Utf8ChSeq out;
    utf8_to_charset(w, &out);
    auto *nextmap = &gs_wordsLib.nextmap;
    for (int i = 0; i < (int)out.size(); i++)
    {
        const Utf8Char &ch = out[i];
        auto ib = nextmap->emplace(ch, nullptr);
        NextWord *&next = ib.first->second;
        if (!ib.second)
        {
            nextmap = &(next->nextmap);
        }
        else
        {
            next = new NextWord;
            gs_NWCache.emplace_back(next);   
        }
        
        nextmap = &(next->nextmap);
        if (i == (int)out.size() - 1)
        {
            nextmap->emplace(0, nullptr);
        }
    }
    
    return 0;
}

void initKeyWords(){
    if(freopen("./keywords.txt","r",stdin)!=NULL){
        cout << "open keywords success" << endl;
        string s;
        while(cin>>s){
            AddWord(s);
        }
	}else{
		cout << "open file  ./keywords.txt err" << endl ; 
	}
}

void ClearWordLib()
{
    decltype(gs_wordsLib.nextmap) emptymap;
    gs_wordsLib.nextmap.swap(emptymap);
    
    for (auto &&ptr : gs_NWCache)
    {
        delete ptr;
    }
    gs_NWCache.clear();
}

