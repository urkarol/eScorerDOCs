#pragma once
#include "core.hpp"
#include <array>
struct NamePattern {
    std::vector<int> order;
    std::vector<std::wstring> separators;
    std::wstring dateFormat=L"yyyy-MM-dd";
};
struct NamingConfig { NamePattern files,folder; };
inline NamingConfig defaultNaming() {
    return {{{0,1,2,3,4,5},{L"_",L"-",L"_",L"_",L"-"}},
            {{0,1,2,4,5},{L"_",L"_",L"_",L"-"}}};
}
inline NamingConfig& naming() {static NamingConfig config=defaultNaming();return config;}
inline const wchar_t* partLabel(int id) {
    static const wchar_t* labels[]={L"Data",L"Skrót ligi",L"Numer meczu",L"Rodzaj dokumentu",L"Gospodarz",L"Gość"};
    return id>=0&&id<6?labels[id]:L"";
}
inline bool validComponent(const std::wstring& text) {
    return !text.empty() && text.size()<=255 && text!=L"." && text!=L".." &&
        text.back()!=L'.' && text.back()!=L' ' &&
        std::none_of(text.begin(),text.end(),[](wchar_t c){return c<32 || c==127;}) &&
        text.find_first_of(L"<>:\"/\\|?*")==std::wstring::npos;
}
inline std::wstring formatDate(const std::wstring& date,const std::wstring& mask) {
    if(date.size()!=10||date[4]!=L'-'||date[7]!=L'-') throw std::runtime_error("date");
    for(size_t i=0;i<date.size();++i)
        if(i!=4&&i!=7&&(date[i]<L'0'||date[i]>L'9')) throw std::runtime_error("date");
    int year=std::stoi(date.substr(0,4)),month=std::stoi(date.substr(5,2)),day=std::stoi(date.substr(8,2));
    const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
    bool leap=year%4==0&&(year%100!=0||year%400==0);
    if(year<1||month<1||month>12||day<1||day>days[month-1]+(month==2&&leap?1:0)) throw std::runtime_error("date");
    if(mask.empty()||mask.size()>32) throw std::runtime_error("format_date");
    bool seenYear=false,seenMonth=false,seenDay=false;
    std::wstring result;
    for(size_t i=0;i<mask.size();) {
        wchar_t token=mask[i];
        if(token==L'y'||token==L'M'||token==L'd') {
            size_t end=i+1;while(end<mask.size()&&mask[end]==token)++end;
            size_t length=end-i;
            bool& seen=token==L'y'?seenYear:token==L'M'?seenMonth:seenDay;
            if(seen||(token==L'y'?(length!=2&&length!=4):(length!=1&&length!=2))) throw std::runtime_error("format_date");
            seen=true;
            if(token==L'y') result+=length==4?date.substr(0,4):date.substr(2,2);
            else if(token==L'M') result+=length==2?date.substr(5,2):std::to_wstring(month);
            else result+=length==2?date.substr(8,2):std::to_wstring(day);
            i=end;
        } else {
            if(std::wstring(L"-_. ").find(token)==std::wstring::npos) throw std::runtime_error("format_date");
            result+=token;++i;
        }
    }
    if(!seenYear||!seenMonth||!seenDay) throw std::runtime_error("format_date");
    return result;
}
inline void validatePattern(const NamePattern& pattern,bool folder) {
    formatDate(L"2026-10-09",pattern.dateFormat);
    const std::vector<int> expected=folder?std::vector<int>{0,1,2,4,5}:std::vector<int>{0,1,2,3,4,5};
    auto actual=pattern.order;std::sort(actual.begin(),actual.end());
    if(actual!=expected||pattern.separators.size()+1!=pattern.order.size()) throw std::runtime_error("format_fields");
    for(const auto& separator:pattern.separators) {
        if(separator.size()>12||separator.find_first_of(L"<>:\"/\\|?*")!=std::wstring::npos||
           std::any_of(separator.begin(),separator.end(),[](wchar_t c){return c<32||c==127;}))
            throw std::runtime_error("format_separator");
    }
}
inline void validateNaming(const NamingConfig& config) {validatePattern(config.files,false);validatePattern(config.folder,true);}
inline std::array<std::wstring,6> matchValues(const std::wstring& date,int league,const std::wstring& number,
                                           const std::wstring& home,const std::wstring& away,const std::wstring& kind=L"PE") {
    filenames(date,league,number,home,away); // Reuse the input validation of the established filename rules.
    auto padded=number;while(padded.size()<3)padded=L"0"+padded;
    return {date,leagues()[league].code,padded,kind,codeFromText(home),codeFromText(away)};
}
inline std::wstring renderPattern(const NamePattern& pattern,const std::array<std::wstring,6>& values,bool folder) {
    validatePattern(pattern,folder);
    std::wstring output;
    for(size_t i=0;i<pattern.order.size();++i) {
        output+=pattern.order[i]==0?formatDate(values[0],pattern.dateFormat):values[pattern.order[i]];
        if(i<pattern.separators.size()) output+=pattern.separators[i];
    }
    if(!folder) output+=L".pdf";
    if(!validComponent(output)) throw std::runtime_error("format_name");
    return output;
}
inline std::vector<std::wstring> formattedFilenames(const std::wstring& date,int league,const std::wstring& number,
                   const std::wstring& home,const std::wstring& away,const NamingConfig& config=naming()) {
    std::vector<std::wstring> result;
    for(const auto& kind:documentKinds())
        result.push_back(renderPattern(config.files,matchValues(date,league,number,home,away,kind),false));
    return result;
}
inline std::wstring matchFolder(const std::wstring& date,int league,const std::wstring& number,
                   const std::wstring& home,const std::wstring& away,const NamingConfig& config=naming()) {
    return renderPattern(config.folder,matchValues(date,league,number,home,away),true);
}
inline void movePart(NamePattern& pattern,size_t from,size_t to) {
    if(from>=pattern.order.size()||to>=pattern.order.size()) throw std::runtime_error("format_fields");
    int value=pattern.order[from];pattern.order.erase(pattern.order.begin()+from);pattern.order.insert(pattern.order.begin()+to,value);
}
inline std::wstring serializeNaming(const NamingConfig& config) {
    validateNaming(config);std::wstring result=L"eScorerDOCs-formats-v2\n"+config.files.dateFormat+L"\n"+config.folder.dateFormat+L"\n";
    for(const auto* pattern:{&config.files,&config.folder}) {
        for(size_t i=0;i<pattern->order.size();++i)
            result+=std::to_wstring(pattern->order[i])+L"\t"+(i<pattern->separators.size()?pattern->separators[i]:L"")+L"\n";
    }
    return result;
}
inline NamingConfig parseNaming(const std::wstring& text) {
    std::wistringstream input(text);std::wstring line;
    if(!std::getline(input,line)||(line!=L"eScorerDOCs-formats-v1"&&line!=L"eScorerDOCs-formats-v2")) throw std::runtime_error("format_fields");
    bool version2=line==L"eScorerDOCs-formats-v2";
    NamingConfig config;
    if(version2&&(!std::getline(input,config.files.dateFormat)||!std::getline(input,config.folder.dateFormat))) throw std::runtime_error("format_fields");
    for(auto entry:{std::pair<NamePattern*,size_t>{&config.files,6},std::pair<NamePattern*,size_t>{&config.folder,5}}) {
        for(size_t i=0;i<entry.second;++i) {
            if(!std::getline(input,line)||line.size()<2||line[1]!=L'\t'||line[0]<L'0'||line[0]>L'5') throw std::runtime_error("format_fields");
            entry.first->order.push_back(line[0]-L'0');
            if(i+1<entry.second) entry.first->separators.push_back(line.substr(2));
            else if(line.size()!=2) throw std::runtime_error("format_fields");
        }
    }
    if(std::getline(input,line)) throw std::runtime_error("format_fields");
    validateNaming(config);return config;
}
