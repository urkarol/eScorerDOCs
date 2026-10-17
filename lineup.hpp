#pragma once
#include "naming.hpp"
#include "lineup_data.hpp"
#include <locale>
#include <limits>

// The stored source template is immutable. Each result appends a new page
// content stream and an incremental cross-reference section (PDF 1.4).
inline std::string lineupPdf(std::wstring code) {
    code=codeFromText(code);
    if(code.size()!=3||code.find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")!=std::wstring::npos)
        throw std::runtime_error("team");
    std::string text;
    for(size_t i=0;i<code.size();++i) {if(i)text+=' ';text+=(char)code[i];}
    double advance=0,xmin=std::numeric_limits<double>::max(),xmax=-xmin,ymin=xmin,ymax=-xmin;
    std::ostringstream hex;
    for(char c:text) {
        size_t index=std::string(lineupData::chars).find(c);
        const auto& g=lineupData::glyphs[index];
        if(c!=' ') {xmin=std::min(xmin,advance+g[2]);xmax=std::max(xmax,advance+g[4]);
                    ymin=std::min(ymin,(double)g[3]);ymax=std::max(ymax,(double)g[5]);}
        advance+=g[1];
        hex<<std::hex<<std::setw(4)<<std::setfill('0')<<g[0];
    }
    double unit=14.0/lineupData::unitsPerEm;
    std::ostringstream content;content.imbue(std::locale::classic());content<<std::fixed<<std::setprecision(6);
    for(const auto& center:lineupData::centers) {
        double x=center[0]-(ymin+ymax)*unit/2;
        double y=center[1]-(xmin+xmax)*unit/2;
        content<<"q\nBT\n0 g\n/TeamCode 14 Tf\n0 -1 1 0 "<<x<<' '<<842-y<<" Tm\n<"<<hex.str()<<"> Tj\nET\nQ\n";
    }
    auto stream=content.str();
    std::string pdf(reinterpret_cast<const char*>(lineupData::pdf),sizeof(lineupData::pdf));
    pdf+='\n';size_t pageOffset=pdf.size();
    pdf+=std::to_string(lineupData::pageObject)+" 0 obj\n"+lineupData::pageDictionary+"\nendobj\n";
    size_t streamOffset=pdf.size();
    pdf+=std::to_string(lineupData::streamObject)+" 0 obj\n<< /Length "+std::to_string(stream.size())+" >>\nstream\n"+stream+"endstream\nendobj\n";
    size_t xref=pdf.size();
    auto entry=[](size_t offset){std::ostringstream s;s<<std::setw(10)<<std::setfill('0')<<offset<<" 00000 n \n";return s.str();};
    pdf+="xref\n"+std::to_string(lineupData::pageObject)+" 1\n"+entry(pageOffset);
    pdf+=std::to_string(lineupData::streamObject)+" 1\n"+entry(streamOffset);
    pdf+="trailer\n<< /Size "+std::to_string(lineupData::streamObject+1)+" /Root "+lineupData::rootReference+
         " /Prev "+std::to_string(lineupData::previousXref)+" >>\nstartxref\n"+std::to_string(xref)+"\n%%EOF\n";
    return pdf;
}

inline std::vector<std::wstring> lineupFilenames(const std::wstring& home,const std::wstring& away) {
    auto h=codeFromText(home),a=codeFromText(away);
    for(const auto& code:{h,a})
        if(code.size()!=3||code.find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")!=std::wstring::npos)
            throw std::runtime_error("team");
    if(h==a)throw std::runtime_error("same");
    return {h+L"-R5.pdf",a+L"-R5.pdf"};
}
struct LineupPlan {bool temporary;std::wstring folder;std::vector<std::wstring> files;};
inline LineupPlan planLineups(const std::wstring& date,int league,const std::wstring& number,
                             const std::wstring& home,const std::wstring& away,const NamingConfig& config=naming()) {
    LineupPlan plan{trimmed(number).empty(),L"",lineupFilenames(home,away)};
    if(!plan.temporary)plan.folder=matchFolder(date,league,number,home,away,config);
    return plan;
}
