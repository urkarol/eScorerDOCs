#include "lineup.hpp"
#include "batch.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
int main(int argc,char** argv) {
    assert(lineupPdf(L"BEL")==lineupPdf(L"PGE GiEK SKRA Bełchatów [BEL]"));
    for(const auto* code:{L"",L"AB",L"ABCD",L"A/B",L"ĄBC"}) {
        bool rejected=false;try{lineupPdf(code);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    }
    auto files=lineupFilenames(L"BEL",L"GDA");
    assert(files[0]==L"BEL-R5.pdf" && files[1]==L"GDA-R5.pdf");
    assert(files==lineupFilenames(L"PGE GiEK SKRA Bełchatów [BEL]",L"gda"));
    auto temporary=planLineups(L"",-1,L"",L"BEL",L"GDA");
    assert(temporary.temporary && temporary.folder.empty() && temporary.files==files);
    assert(planLineups(L"",-1,L"   ",L"BEL",L"GDA").temporary);
    auto persistent=planLineups(L"2026-10-10",0,L"30",L"BEL",L"GDA");
    assert(!persistent.temporary && persistent.folder==L"2026-10-10_TLM_030_BEL-GDA" && persistent.files==files);
    for(const auto* number:{L"0",L"abc",L"123456"}) {
        bool rejected=false;try{planLineups(L"2026-10-10",0,number,L"BEL",L"GDA");}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    }
    for(const auto* away:{L"BEL",L"",L"AA",L"A/B"}) {
        bool rejected=false;try{planLineups(L"",-1,L"",L"BEL",away);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
    }
    auto cfg=defaultNaming();cfg.files.dateFormat=L"dd.MM.yyyy";std::reverse(cfg.files.order.begin(),cfg.files.order.end());
    cfg.folder.dateFormat=L"dd.MM.yyyy";
    auto custom=planLineups(L"2026-10-10",0,L"30",L"BEL",L"GDA",cfg);
    assert(custom.files==files && custom.folder==L"10.10.2026_TLM_030_BEL-GDA");
    auto dir=std::filesystem::absolute(argc>1?argv[1]:"lineup-tests");std::filesystem::create_directories(dir);
    std::wstring base=dir.wstring(),folder=L"2026-10-10_TLM_030_BEL-GDA",error;
    std::vector<std::string> payloads={lineupPdf(L"BEL"),lineupPdf(L"GDA")};
    assert(writeMatchBatchContents(base,folder,files,payloads,error));
    assert(!writeMatchBatchContents(base,folder,files,payloads,error));
    for(size_t i=0;i<files.size();++i) {std::ifstream f(diskPath(join(join(base,folder),files[i])),std::ios::binary);std::string actual((std::istreambuf_iterator<char>(f)),{});assert(actual==payloads[i]);}
    // A failed second write rolls the first PDF back, including new payloads.
    assert(!writeBatchContents(base,{L"rollback.pdf",L"missing/failure.pdf"},payloads,error));
    assert(!std::filesystem::exists(diskPath(join(base,L"rollback.pdf"))));
    // Collision on the second file prevents the first file from being written.
    auto collision=L"collision";std::filesystem::create_directory(diskPath(join(base,collision)));
    {std::ofstream f(diskPath(join(join(base,collision),files[1])));f<<"existing";}
    assert(!writeMatchBatchContents(base,collision,files,payloads,error));
    assert(!std::filesystem::exists(diskPath(join(join(base,collision),files[0]))));
    // Exercise all supported code characters, including unequal visible glyph heights.
    for(const auto& code:{L"WWW",L"III",L"A1Z",L"000",L"JQG",L"IME"}) {
        auto pdf=lineupPdf(code);std::ofstream f(dir/(std::string(code,code+3)+".pdf"),std::ios::binary);f.write(pdf.data(),pdf.size());
    }
    std::cout<<"Lineup PDF, code validation, naming and collision protection passed.\n";
}
