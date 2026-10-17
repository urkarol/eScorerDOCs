#include "batch.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <locale>
static void identical(const NamingConfig& a,const NamingConfig& b) {
    assert(a.files.order==b.files.order&&a.files.separators==b.files.separators);
    assert(a.folder.order==b.folder.order&&a.folder.separators==b.folder.separators);
    assert(a.files.dateFormat==b.files.dateFormat&&a.folder.dateFormat==b.folder.dateFormat);
}
int main() {
    auto config=defaultNaming();
    auto values=matchValues(L"2026-10-09",0,L"30",L"BEL",L"GDA");
    assert(matchFolder(L"2026-10-09",0,L"30",L"BEL",L"GDA")==L"2026-10-09_TLM_030_BEL-GDA");
    assert(formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA")==filenames(L"2026-10-09",0,L"30",L"BEL",L"GDA"));
    identical(config,parseNaming(serializeNaming(config)));
    auto legacy=L"eScorerDOCs-formats-v1\n0\t_\n1\t-\n2\t_\n3\t_\n4\t-\n5\t\n0\t_\n1\t_\n2\t_\n4\t-\n5\t\n";
    identical(config,parseNaming(legacy));
    auto legacyCustom=std::wstring(legacy);legacyCustom.replace(legacyCustom.find(L"0\t_"),3,L"0\t.");
    auto migrated=parseNaming(legacyCustom);assert(migrated.files.separators[0]==L".");
    assert(migrated.files.dateFormat==L"yyyy-MM-dd");identical(migrated,parseNaming(serializeNaming(migrated)));
    for(auto item:{std::pair<const wchar_t*,const wchar_t*>{L"yyyyMMdd",L"20260102"},
             {L"dd.MM.yyyy",L"02.01.2026"},{L"dd-MM-yy",L"02-01-26"},{L"yyMMdd",L"260102"},
             {L"d_M_yyyy",L"2_1_2026"},{L"MM dd yyyy",L"01 02 2026"},{L"ddMMyyyy",L"02012026"}}) {
        assert(formatDate(L"2026-01-02",item.first)==item.second);
        auto dated=defaultNaming();dated.files.dateFormat=item.first;dated.folder.dateFormat=L"yyyyMMdd";
        identical(dated,parseNaming(serializeNaming(dated)));
        auto files=formattedFilenames(L"2026-01-02",0,L"30",L"BEL",L"GDA",dated);
        assert(files[0]==std::wstring(item.second)+L"_TLM-030_PE_BEL-GDA.pdf");
        assert(matchFolder(L"2026-01-02",0,L"30",L"BEL",L"GDA",dated)==L"20260102_TLM_030_BEL-GDA");
        movePart(dated.files,0,5);
        assert(formattedFilenames(L"2026-01-02",0,L"30",L"BEL",L"GDA",dated)[0]==L"TLM_030-PE_BEL_GDA-"+std::wstring(item.second)+L".pdf");
    }
    assert(formatDate(L"2024-02-29",L"dd.MM.yyyy")==L"29.02.2024");
    assert(formatDate(L"2000-02-29",L"yyyyMMdd")==L"20000229");
    for(auto illegal:{L"",L"dd.MM",L"YYYY-MM-DD",L"yyyy/MM/dd",L"yyyy-MM-dd-dd",L"yyy-MM-dd",L"yyyy-MMM-dd",L"yyyy-MM-dd\n",L"yyyy-MM-dd*"}) {
        auto invalid=defaultNaming();invalid.folder.dateFormat=illegal;
        bool rejected=false;try{serializeNaming(invalid);}catch(...){rejected=true;}assert(rejected);
    }
    for(auto illegalDate:{L"2026-02-29",L"1900-02-29",L"2026-13-01",L"2026-04-31",L"2026-01-00",L"0000-01-01",L"2026-1a-09"}) {
        bool rejected=false;try{formatDate(illegalDate,L"yyyyMMdd");}catch(...){rejected=true;}assert(rejected);
    }
    auto order=config.files.order;std::sort(order.begin(),order.end());int permutations=0;
    do {config.files.order=order;identical(config,parseNaming(serializeNaming(config)));auto result=formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA",config);assert(result.size()==9);assert(result[0].substr(result[0].size()-4)==L".pdf");++permutations;}while(std::next_permutation(order.begin(),order.end()));
    assert(permutations==720);
    config=defaultNaming();order=config.folder.order;std::sort(order.begin(),order.end());permutations=0;
    do {config.folder.order=order;assert(validComponent(matchFolder(L"2026-10-09",0,L"30",L"BEL",L"GDA",config)));++permutations;}while(std::next_permutation(order.begin(),order.end()));
    assert(permutations==120);
    config=defaultNaming();movePart(config.files,3,0);
    assert(renderPattern(config.files,values,false)==L"PE_2026-10-09-TLM_030_BEL-GDA.pdf");
    assert(formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA",config).back()==L"SK-10_2026-10-09-TLM_030_BEL-GDA.pdf");
    config.files.separators={L" ",L"_",L"",L" - ",L"–"};
    identical(config,parseNaming(serializeNaming(config)));
    assert(renderPattern(config.files,values,false)==L"PE 2026-10-09_TLM030 - BEL–GDA.pdf");
    for(auto illegal:{L"/",L"\\",L"*",L"?",L"\n",L"\t",L"\"",L":",L"|",L"<",L">",L"1234567890123"}) {
        auto bad=defaultNaming();bad.files.separators[0]=illegal;bool caught=false;try{serializeNaming(bad);}catch(...){caught=true;}assert(caught);
    }
    auto bad=defaultNaming();bad.files.order[0]=1;bool caught=false;try{serializeNaming(bad);}catch(...){caught=true;}assert(caught);
    caught=false;try{parseNaming(L"bad");}catch(...){caught=true;}assert(caught);
    caught=false;try{parseNaming(serializeNaming(defaultNaming())+L"extra\n");}catch(...){caught=true;}assert(caught);
    auto base=std::filesystem::absolute("match-output-test");std::filesystem::create_directory(base);
    std::wstring root=std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(base.u8string());
    auto folder=matchFolder(L"2026-10-09",0,L"30",L"BEL",L"GDA");
    auto files=formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA");std::wstring error;
    std::ofstream sentinel(base/"existing.txt");sentinel<<"keep";sentinel.close();
    assert(writeMatchBatch(root,folder,files,error));
    auto target=base/diskPath(folder);for(auto& name:files)assert(std::filesystem::file_size(target/diskPath(name))==blankPdf().size());
    assert(!writeMatchBatch(root,folder,files,error));
    std::ofstream original(target/diskPath(files[0]),std::ios::binary);original<<"original PDF";original.close();
    assert(!writeMatchBatch(root,folder,files,error));assert(std::filesystem::file_size(target/diskPath(files[0]))==12);
    assert(!writeMatchBatch(root,L"../escape",files,error));
    assert(!writeMatchBatch(root,L"invalid",{L"bad/path.pdf"},error));assert(!std::filesystem::exists(base/"invalid"));
    std::ofstream conflict(base/"folder-is-file");conflict<<"keep";conflict.close();
    assert(!writeMatchBatch(root,L"folder-is-file",files,error));
    config=defaultNaming();config.files.separators[0]=L" - ";auto changed=formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA",config);
    assert(writeMatchBatch(root,folder,changed,error));assert(std::filesystem::file_size(target/diskPath(files[0]))==12);
    config.files.dateFormat=L"dd.MM.yyyy";config.folder.dateFormat=L"yyyyMMdd";
    auto datedFolder=matchFolder(L"2026-10-09",0,L"30",L"BEL",L"GDA",config);
    auto datedFiles=formattedFilenames(L"2026-10-09",0,L"30",L"BEL",L"GDA",config);
    assert(writeMatchBatch(root,datedFolder,datedFiles,error));
    for(auto& name:datedFiles)assert(std::filesystem::file_size(base/diskPath(datedFolder)/diskPath(name))==blankPdf().size());
    std::ifstream kept(base/"existing.txt");std::string content;kept>>content;assert(content=="keep");kept.close();
    std::filesystem::remove_all(base);
    std::cout<<"PASS: date masks, independent file/folder dates, legacy migration, leap years, invalid masks, 720 document layouts, 120 folder layouts, separators, save/load, match subfolder, preservation of existing files\n";
}
