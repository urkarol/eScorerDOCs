#pragma once
#include <algorithm>
#include <cwctype>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------- błędy ----

// Komunikaty (what()) zostały takie same jak wcześniej: "league", "number",
// "team", "same", "roster_group", "roster_name", "roster_code",
// "roster_duplicate", "roster_format".
class DocError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// ------------------------------------------------------- typy domenowe -----

enum class Group : int { MenTop = 0, WomenTop = 1, Men1 = 2, Women1 = 3 };

enum class LeagueId : int { TLM, TLK, T1M, T1K, PPM, PPK, SPM, SPK, Count };

struct Team {
    std::wstring name;
    std::wstring code;
    Group group;
};

struct League {
    const wchar_t* name;
    const wchar_t* code;
};

inline bool isMenGroup(Group g) { return static_cast<int>(g) % 2 == 0; }

// ------------------------------------------------------ narzędzia tekstu ---

inline bool isSpace(wchar_t c) { return std::iswspace(static_cast<wint_t>(c)) != 0; }

inline std::wstring trimmed(const std::wstring& s) {
    size_t b = 0, e = s.size();
    while (b < e && isSpace(s[b])) ++b;
    while (e > b && isSpace(s[e - 1])) --e;
    return s.substr(b, e - b);
}

inline std::wstring upperAscii(std::wstring s) {
    for (auto& c : s)
        if (c >= L'a' && c <= L'z') c = static_cast<wchar_t>(c - L'a' + L'A');
    return s;
}

inline bool isValidCode(const std::wstring& s) {
    if (s.size() != 3) return false;
    return std::all_of(s.begin(), s.end(), [](wchar_t c) {
        return (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9');
    });
}

// Przyjmuje "BEL", "bel", " bel " oraz "PGE GiEK SKRA Bełchatów [BEL]".
inline std::wstring codeFromText(const std::wstring& text) {
    std::wstring s = trimmed(text);
    if (!s.empty() && s.back() == L']') {
        const auto p = s.rfind(L" [");
        if (p != std::wstring::npos)
            s = trimmed(s.substr(p + 2, s.size() - p - 3));
    }
    return upperAscii(std::move(s));
}

// -------------------------------------------------------------- składy -----

inline const std::vector<Team>& defaultTeams() {
    constexpr Group MT = Group::MenTop, WT = Group::WomenTop,
                    M1 = Group::Men1, W1 = Group::Women1;
    static const std::vector<Team> v = {
        // TAURON LIGA (mężczyźni)
        {L"PGE GiEK SKRA Bełchatów", L"BEL", MT},
        {L"ChKS Chełm", L"CHE", MT},
        {L"Steam Hemarpol Politechnika Częstochowa", L"CZN", MT},
        {L"Energa Trefl Gdańsk", L"GDA", MT},
        {L"Stilon Gorzów", L"GOR", MT},
        {L"GKS Katowice", L"KAT", MT},
        {L"ZAKSA Kędzierzyn-Koźle", L"KED", MT},
        {L"BOGDANKA LUK Lublin", L"LUK", MT},
        {L"Jastrzębie Barkom", L"LWW", MT},
        {L"Energa Indykpol AZS Olsztyn", L"OLS", MT},
        {L"Asseco Resovia Rzeszów", L"RZM", MT},
        {L"Ślepsk Malow Suwałki", L"SUW", MT},
        {L"PGE Projekt Warszawa", L"WAM", MT},
        {L"Aluron CMC Warta Zawiercie", L"ZAW", MT},
        // TAURON Liga Kobiet
        {L"BKS ZGO Bielsko-Biała", L"BBK", WT},
        {L"Metalkas Pałac Bydgoszcz", L"BDK", WT},
        {L"Netland MKS Kalisz", L"KAL", WT},
        {L"PGE Budowlani Łódź", L"LDZ", WT},
        {L"ŁKS Commercecon Łódź", L"LKS", WT},
        {L"ITA TOOLS STAL Mielec", L"MIE", WT},
        {L"Sokół & Hagric Mogilno", L"MOG", WT},
        {L"UNI Opole", L"OPO", WT},
        {L"LOTTO Chemik Police", L"POL", WT},
        {L"MOYA Radomka Radom", L"RAK", WT},
        {L"KS DevelopRes Rzeszów", L"RZK", WT},
        {L"#VolleyWrocław", L"WRK", WT},
        // TAURON 1. Liga (mężczyźni)
        {L"BBTS Bielsko-Biała", L"BBM", M1},
        {L"BKS Bydgoszcz", L"BDM", M1},
        {L"NOWAK-MOSTY MKS Będzin", L"BED", M1},
        {L"KS Sparta Grodzisk Mazowiecki", L"GRO", M1},
        {L"MCKiS Jaworzno", L"JAW", M1},
        {L"KKS MICKIEWICZ Kluczbork", L"KLU", M1},
        {L"FORT Poznań", L"POZ", M1},
        {L"Stal Nysa", L"NYS", M1},
        {L"KPS Płock", L"PLO", M1},
        {L"ICT PIERROT Czarni Radom", L"RAM", M1},
        {L"KPS Siedlce", L"SDL", M1},
        {L"AVIA Sędziszów Małopolski", L"SMM", M1},
        {L"SMS PZPS Spała", L"SPA", M1},
        {L"PZL LEONARDO Avia Świdnik", L"SWI", M1},
        {L"Lechia Tomaszów Mazowiecki", L"TOM", M1},
        {L"CUK Anioły Toruń", L"TOR", M1},
        // TAURON 1. Liga Kobiet
        {L"Credo Płomień Sosnowiec", L"SOS", W1},
        {L"Enea Klub Siatkarski Piła", L"PIL", W1},
        {L"Gedania Politechnika Gdańsk", L"GDK", W1},
        {L"KS BAS Kombinat Budowlany Białystok", L"BIA", W1},
        {L"KSG Warszawa", L"WAK", W1},
        {L"NOWEL LOS Nowy Dwór Mazowiecki", L"NDM", W1},
        {L"MARBA Sędziszów Małopolski", L"SMK", W1},
        {L"Beskid MEDIA MKS Imielin", L"IME", W1},
        {L"NTSK PANS Komunalnik Nysa", L"NYK", W1},
        {L"PGE LTS Legionovia Legionowo", L"LEG", W1},
        {L"SMS PZPS Szczyrk", L"SZK", W1},
        {L"WTS Solna Wieliczka", L"WIE", W1},
    };
    return v;
}

inline std::vector<Team>& teams() {
    static std::vector<Team> data = defaultTeams();
    return data;
}

inline const std::vector<League>& leagues() {
    // Kolejność musi odpowiadać enumowi LeagueId.
    static const std::vector<League> v = {
        {L"TAURON LIGA", L"TLM"},
        {L"TAURON Liga Kobiet", L"TLK"},
        {L"TAURON 1. Liga", L"T1M"},
        {L"TAURON 1. Liga Kobiet", L"T1K"},
        {L"Puchar Polski Mężczyzn", L"PPM"},
        {L"Puchar Polski Kobiet", L"PPK"},
        {L"Superpuchar Mężczyzn", L"SPM"},
        {L"Superpuchar Kobiet", L"SPK"},
    };
    return v;
}

// Czy drużyna z danej grupy może grać w danych rozgrywkach.
inline bool belongs(Group group, LeagueId league) {
    switch (league) {
        case LeagueId::TLM: return group == Group::MenTop;
        case LeagueId::TLK: return group == Group::WomenTop;
        case LeagueId::T1M: return group == Group::Men1;
        case LeagueId::T1K: return group == Group::Women1;
        case LeagueId::PPM:
        case LeagueId::SPM: return isMenGroup(group);
        case LeagueId::PPK:
        case LeagueId::SPK: return !isMenGroup(group);
        default: return false;
    }
}

inline bool belongs(int group, int league) {
    if (group < 0 || group > 3 || league < 0 || league >= static_cast<int>(LeagueId::Count))
        return false;
    return belongs(static_cast<Group>(group), static_cast<LeagueId>(league));
}

// --------------------------------------------------------- nazwy plików ----

inline const std::vector<std::wstring>& documentKinds() {
    static const std::vector<std::wstring> kinds = {
        L"PE", L"PL", L"RM1", L"RM2", L"VC1", L"VC2", L"SD", L"SR"};
    return kinds;
}

inline std::vector<std::wstring> filenames(const std::wstring& date, int league,
                                           const std::wstring& num,
                                           const std::wstring& homeText,
                                           const std::wstring& awayText) {
    if (league < 0 || league >= static_cast<int>(LeagueId::Count)) throw DocError("league");

    const bool digitsOnly = num.find_first_not_of(L"0123456789") == std::wstring::npos;
    if (num.empty() || num.size() > 5 || !digitsOnly ||
        num.find_first_not_of(L'0') == std::wstring::npos)  // same zera
        throw DocError("number");

    const std::wstring home = codeFromText(homeText);
    const std::wstring away = codeFromText(awayText);
    if (!isValidCode(home) || !isValidCode(away)) throw DocError("team");
    if (home == away) throw DocError("same");

    std::wstring n = num;
    if (n.size() < 3) n.insert(0, 3 - n.size(), L'0');

    std::vector<std::wstring> out;
    out.reserve(documentKinds().size());
    for (const auto& kind : documentKinds())
        out.push_back(date + L"_" + leagues()[league].code + L"-" + n + L"_" + kind +
                      L"_" + home + L"-" + away + L".pdf");
    return out;
}

// -------------------------------------------------------------- pusty PDF --

inline std::string blankPdf() {
    const std::vector<std::string> objects = {
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595.275591 841.889764] >>",
    };

    std::string p = "%PDF-1.4\n%\xe2\xe3\xcf\xd3\n";
    std::vector<size_t> offsets;
    for (size_t i = 0; i < objects.size(); ++i) {
        offsets.push_back(p.size());
        p += std::to_string(i + 1) + " 0 obj\n" + objects[i] + "\nendobj\n";
    }

    const size_t xrefPos = p.size();
    p += "xref\n0 " + std::to_string(objects.size() + 1) + "\n0000000000 65535 f \n";
    for (size_t off : offsets) {
        std::string num = std::to_string(off);
        p += std::string(10 - num.size(), '0') + num + " 00000 n \n";
    }
    p += "trailer\n<< /Size " + std::to_string(objects.size() + 1) +
         " /Root 1 0 R >>\nstartxref\n" + std::to_string(xrefPos) + "\n%%EOF\n";
    return p;
}

// ------------------------------------------------- walidacja i zapis/odczyt -

inline void validateTeams(const std::vector<Team>& data) {
    std::set<std::wstring> seen;
    for (const auto& team : data) {
        const int g = static_cast<int>(team.group);
        if (g < 0 || g > 3) throw DocError("roster_group");
        if (trimmed(team.name).empty() || team.name.size() > 180 ||
            team.name.find_first_of(L"\t\r\n") != std::wstring::npos)
            throw DocError("roster_name");
        if (!isValidCode(team.code)) throw DocError("roster_code");
        if (!seen.insert(team.code).second) throw DocError("roster_duplicate");
    }
}

inline const wchar_t* rosterHeader() { return L"eScorerDOCs-teams-v1"; }

inline std::wstring serializeTeams(const std::vector<Team>& data) {
    validateTeams(data);
    std::wstring out = std::wstring(rosterHeader()) + L"\n";
    for (const auto& team : data)
        out += std::to_wstring(static_cast<int>(team.group)) + L"\t" + team.code + L"\t" +
               team.name + L"\n";
    return out;
}

inline std::vector<Team> parseTeams(const std::wstring& text) {
    auto chomp = [](std::wstring& s) {
        if (!s.empty() && s.back() == L'\r') s.pop_back();  // pliki z Windows
    };

    std::wistringstream input(text);
    std::wstring line;
    if (!std::getline(input, line)) throw DocError("roster_format");
    chomp(line);
    if (!line.empty() && line.front() == 0xFEFF) line.erase(0, 1);  // BOM
    if (line != rosterHeader()) throw DocError("roster_format");

    std::vector<Team> result;
    while (std::getline(input, line)) {
        chomp(line);
        if (trimmed(line).empty()) continue;

        const auto first = line.find(L'\t');
        if (first != 1) throw DocError("roster_format");
        const auto second = line.find(L'\t', first + 1);
        if (second == std::wstring::npos) throw DocError("roster_format");
        if (line[0] < L'0' || line[0] > L'3') throw DocError("roster_format");

        result.push_back({trimmed(line.substr(second + 1)),
                          codeFromText(line.substr(first + 1, second - first - 1)),
                          static_cast<Group>(line[0] - L'0')});
    }
    validateTeams(result);
    return result;
}
