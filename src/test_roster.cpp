#include "core.hpp"
#include <cassert>
#include <iostream>
static void identical(const std::vector<Team>& a,const std::vector<Team>& b) {
    assert(a.size()==b.size());
    for(size_t i=0;i<a.size();++i) {
        assert(a[i].name==b[i].name);
        assert(a[i].code==b[i].code);
        assert(a[i].group==b[i].group);
    }
}
static void rejected(const std::vector<Team>& data,const std::string& expected) {
    bool caught=false;
    try {serializeTeams(data);} catch(const std::exception& e) {assert(e.what()==expected);caught=true;}
    assert(caught);
}
int main() {
    auto data=defaultTeams();
    identical(data,parseTeams(serializeTeams(data)));
    data[0]={L"Nowa nazwa — Łódź, Żory i Kraków",L"X01",1};
    data.push_back({L"Nowy klub sezonu 2027/2028",L"N01",3});
    auto restored=parseTeams(serializeTeams(data));
    identical(data,restored);
    assert(restored.size()==55);
    assert(belongs(restored[0].group,1));
    assert(!belongs(restored[0].group,0));
    assert(belongs(restored[0].group,5));
    assert(!belongs(restored[0].group,4));
    assert(filenames(L"2027-10-09",1,L"1",restored[0].code,restored.back().code)[0]==L"2027-10-09_TLK-001_PE_X01-N01.pdf");
    data.erase(data.begin()+2);
    identical(data,parseTeams(serializeTeams(data)));
    assert(parseTeams(serializeTeams({})).empty());
    auto bad=data;bad.back().code=bad.front().code;rejected(bad,"roster_duplicate");
    bad=data;bad.back().name=L" \t";rejected(bad,"roster_name");
    bad=data;bad.back().name=L"Nie\nprawidłowa";rejected(bad,"roster_name");
    bad=data;bad.back().group=4;rejected(bad,"roster_group");
    bad=data;bad.back().code=L"ab1";rejected(bad,"roster_code");
    bad=data;bad.back().code=L"/../";rejected(bad,"roster_code");
    for(const auto& malformed:{L"bad",L"eScorerDOCs-teams-v1\n4\tABC\tName\n",L"eScorerDOCs-teams-v1\n0\tABC",L"eScorerDOCs-teams-v1\n0\tABC\tName\n1\tABC\tOther\n"}) {
        bool caught=false;try{parseTeams(malformed);}catch(...){caught=true;}assert(caught);
    }
    teams()=restored;
    assert(teams().size()==55);
    teams()=defaultTeams();
    assert(teams().size()==54);
    std::cout<<"PASS: roster save/load, Polish characters, renamed teams, updated codes, promotions, additions, deletions, empty roster, invalid and duplicate codes\n";
}
