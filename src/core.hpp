#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <cwctype>
struct Team { std::wstring name; std::wstring code; int group; };
inline const std::vector<Team>& defaultTeams(){ static const std::vector<Team> v={
{L"PGE GiEK SKRA Bełchatów",L"BEL",0},{L"ChKS Chełm",L"CHE",0},{L"Steam Hemarpol Politechnika Częstochowa",L"CZN",0},{L"Energa Trefl Gdańsk",L"GDA",0},{L"Stilon Gorzów",L"GOR",0},{L"GKS Katowice",L"KAT",0},{L"ZAKSA Kędzierzyn-Koźle",L"KED",0},{L"BOGDANKA LUK Lublin",L"LUK",0},{L"Jastrzębie Barkom",L"LWW",0},{L"Energa Indykpol AZS Olsztyn",L"OLS",0},{L"Asseco Resovia Rzeszów",L"RZM",0},{L"Ślepsk Malow Suwałki",L"SUW",0},{L"PGE Projekt Warszawa",L"WAM",0},{L"Aluron CMC Warta Zawiercie",L"ZAW",0},
{L"BKS ZGO Bielsko-Biała",L"BBK",1},{L"Metalkas Pałac Bydgoszcz",L"BDK",1},{L"Netland MKS Kalisz",L"KAL",1},{L"PGE Budowlani Łódź",L"LDZ",1},{L"ŁKS Commercecon Łódź",L"LKS",1},{L"ITA TOOLS STAL Mielec",L"MIE",1},{L"Sokół & Hagric Mogilno",L"MOG",1},{L"UNI Opole",L"OPO",1},{L"LOTTO Chemik Police",L"POL",1},{L"MOYA Radomka Radom",L"RAK",1},{L"KS DevelopRes Rzeszów",L"RZK",1},{L"#VolleyWrocław",L"WRK",1},
{L"BBTS Bielsko-Biała",L"BBM",2},{L"BKS Bydgoszcz",L"BDM",2},{L"NOWAK-MOSTY MKS Będzin",L"BED",2},{L"KS Sparta Grodzisk Mazowiecki",L"GRO",2},{L"MCKiS Jaworzno",L"JAW",2},{L"KKS MICKIEWICZ Kluczbork",L"KLU",2},{L"FORT Poznań",L"POZ",2},{L"Stal Nysa",L"NYS",2},{L"KPS Płock",L"PLO",2},{L"ICT PIERROT Czarni Radom",L"RAM",2},{L"KPS Siedlce",L"SDL",2},{L"AVIA Sędziszów Małopolski",L"SMM",2},{L"SMS PZPS Spała",L"SPA",2},{L"PZL LEONARDO Avia Świdnik",L"SWI",2},{L"Lechia Tomaszów Mazowiecki",L"TOM",2},{L"CUK Anioły Toruń",L"TOR",2},
{L"Credo Płomień Sosnowiec",L"SOS",3},{L"Enea Klub Siatkarski Piła",L"PIL",3},{L"Gedania Politechnika Gdańsk",L"GDK",3},{L"KS BAS Kombinat Budowlany Białystok",L"BIA",3},{L"KSG Warszawa",L"WAK",3},{L"NOWEL LOS Nowy Dwór Mazowiecki",L"NDM",3},{L"MARBA Sędziszów Małopolski",L"SMK",3},{L"Beskid MEDIA MKS Imielin",L"IME",3},{L"NTSK PANS Komunalnik Nysa",L"NYK",3},{L"PGE LTS Legionovia Legionowo",L"LEG",3},{L"SMS PZPS Szczyrk",L"SZK",3},{L"WTS Solna Wieliczka",L"WIE",3}
};return v;}
inline std::vector<Team>& teams(){ static std::vector<Team> data=defaultTeams(); return data; }
struct League {const wchar_t* name; const wchar_t* code;};
inline const std::vector<League>& leagues(){static const std::vector<League> v={
{L"TAURON LIGA",L"TLM"},{L"TAURON Liga Kobiet",L"TLK"},{L"TAURON 1. Liga",L"T1M"},{L"TAURON 1. Liga Kobiet",L"T1K"},{L"Puchar Polski Mężczyzn",L"PPM"},{L"Puchar Polski Kobiet",L"PPK"},{L"Superpuchar Mężczyzn",L"SPM"},{L"Superpuchar Kobiet",L"SPK"}};return v;}
inline bool belongs(int group,int league){return league<4?group==league:((league==4||league==6)?group%2==0:group%2==1);}
inline std::wstring codeFromText(std::wstring s){auto p=s.rfind(L" [");if(p!=std::wstring::npos&&s.back()==L']')s=s.substr(p+2,s.size()-p-3);while(!s.empty()&&iswspace(s.front()))s.erase(s.begin());while(!s.empty()&&iswspace(s.back()))s.pop_back();for(auto& c:s)if(c>=L'a'&&c<=L'z')c-=32;return s;}
inline const std::vector<std::wstring>& documentKinds(){
    static const std::vector<std::wstring> kinds={L"PE",L"PL",L"RM1",L"RM2",L"VC1",L"VC2",L"SD",L"SR"};
    return kinds;
}
inline std::vector<std::wstring> filenames(const std::wstring& date,int league,const std::wstring& num,std::wstring home,std::wstring away){
if(league<0||league>=8)throw std::runtime_error("league");
if(num.empty()||num.size()>5||num.find_first_not_of(L"0123456789")!=std::wstring::npos||std::stoul(num)==0)throw std::runtime_error("number");
home=codeFromText(home);away=codeFromText(away);
for(auto s:{home,away})if(s.size()!=3||s.find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")!=std::wstring::npos)throw std::runtime_error("team");
if(home==away)throw std::runtime_error("same");
std::wstring n=num;while(n.size()<3)n=L"0"+n;
std::vector<std::wstring> out;for(const auto& kind:documentKinds())out.push_back(date+L"_"+leagues()[league].code+L"-"+n+L"_"+kind+L"_"+home+L"-"+away+L".pdf");return out;
}
inline std::string blankPdf(){
std::string p="%PDF-1.4\n%\xe2\xe3\xcf\xd3\n";std::vector<size_t> offsets={0};
std::vector<std::string> objects={"<< /Type /Catalog /Pages 2 0 R >>","<< /Type /Pages /Kids [3 0 R] /Count 1 >>","<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595.275591 841.889764] /Resources << >> /Contents 4 0 R >>","<< /Length 0 >>\nstream\nendstream"};
for(size_t i=0;i<objects.size();++i){offsets.push_back(p.size());p+=std::to_string(i+1)+" 0 obj\n"+objects[i]+"\nendobj\n";}
auto x=p.size();p+="xref\n0 5\n0000000000 65535 f \n";for(size_t i=1;i<offsets.size();++i){std::ostringstream o;o<<std::setw(10)<<std::setfill('0')<<offsets[i]<<" 00000 n \n";p+=o.str();}p+="trailer\n<< /Size 5 /Root 1 0 R >>\nstartxref\n"+std::to_string(x)+"\n%%EOF\n";return p;
}

inline std::wstring trimmed(std::wstring s) {
    while (!s.empty() && iswspace(s.front())) s.erase(s.begin());
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    return s;
}
inline void validateTeams(const std::vector<Team>& data) {
    std::vector<std::wstring> seen;
    for (const auto& team : data) {
        if (team.group < 0 || team.group > 3) throw std::runtime_error("roster_group");
        if (team.name.empty() || trimmed(team.name).empty() || team.name.size() > 180 ||
            team.name.find_first_of(L"\t\r\n") != std::wstring::npos)
            throw std::runtime_error("roster_name");
        if (team.code.size() != 3 ||
            team.code.find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::wstring::npos)
            throw std::runtime_error("roster_code");
        if (std::find(seen.begin(), seen.end(), team.code) != seen.end())
            throw std::runtime_error("roster_duplicate");
        seen.push_back(team.code);
    }
}
inline std::wstring serializeTeams(const std::vector<Team>& data) {
    validateTeams(data);
    std::wstring out=L"eScorerDOCs-teams-v1\n";
    for (const auto& team : data)
        out += std::to_wstring(team.group) + L"\t" + team.code + L"\t" + team.name + L"\n";
    return out;
}
inline std::vector<Team> parseTeams(const std::wstring& text) {
    std::wistringstream input(text);
    std::wstring line;
    if (!std::getline(input, line) || line != L"eScorerDOCs-teams-v1")
        throw std::runtime_error("roster_format");
    std::vector<Team> result;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        auto first=line.find(L'\t'), second=line.find(L'\t', first==std::wstring::npos?0:first+1);
        if (first != 1 || second == std::wstring::npos || line[0]<L'0' || line[0]>L'3')
            throw std::runtime_error("roster_format");
        result.push_back({line.substr(second+1), line.substr(first+1,second-first-1), line[0]-L'0'});
    }
    validateTeams(result);
    return result;
}
