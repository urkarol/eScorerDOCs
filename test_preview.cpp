// Windows-only integration test: system PDF rendering and temp file lifetime.
#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include "batch.hpp"
#include "lineup.hpp"
#include "pdf_preview.hpp"
#include <cassert>
int wmain() {
    assert(SUCCEEDED(RoInitialize(RO_INIT_MULTITHREADED)));
    auto dir=pdfPreview::newTempFolder();auto names=lineupFilenames(L"BEL",L"GDA");std::wstring error;
    auto first=join(dir,names[0]),second=join(dir,names[1]);
    assert(pdfPreview::ownedTempPath(first));assert(!pdfPreview::ownedTempPath(L"C:\\BEL-R5.pdf"));
    assert(!pdfPreview::ownedTempPath(join(dir,L"other.pdf")));
    assert(writeBatchContents(dir,names,{lineupPdf(L"BEL"),lineupPdf(L"GDA")},error));
    HANDLE file=CreateFileW(first.c_str(),GENERIC_READ|DELETE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           nullptr,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,nullptr);
    assert(file!=INVALID_HANDLE_VALUE);
    pdfPreview::State state;state.pdf=lineupPdf(L"BEL");state.cancel=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    pdfPreview::render(state);
    assert(state.width==3508&&state.height==2480&&!state.pixels.empty());
    assert(std::count(state.pixels.begin(),state.pixels.end(),BYTE(0))>10000);
    CloseHandle(state.cancel);CloseHandle(file);
    assert(GetFileAttributesW(first.c_str())==INVALID_FILE_ATTRIBUTES);
    assert(GetFileAttributesW(second.c_str())!=INVALID_FILE_ATTRIBUTES);
    assert(!RemoveDirectoryW(dir.c_str())); // The other open preview's file must survive.
    assert(DeleteFileW(second.c_str()));assert(RemoveDirectoryW(dir.c_str()));
    RoUninitialize();return 0;
}
