#pragma once
#include "naming.hpp"
#include <filesystem>
#include <cerrno>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <codecvt>
#include <locale>
#endif
inline std::wstring join(const std::wstring& dir,const std::wstring& name){if(dir.empty())return name;return dir+((dir.back()==L'\\'||dir.back()==L'/')?L"":L"/")+name;}
inline std::filesystem::path diskPath(const std::wstring& path){
#ifdef _WIN32
return std::filesystem::path(path);
#else
return std::filesystem::u8path(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(path));
#endif
}
inline bool writeBatchContents(const std::wstring& dir,const std::vector<std::wstring>& names,const std::vector<std::string>& payloads,std::wstring& error){
if(names.size()!=payloads.size()){error=L"Nieprawidłowy komplet dokumentów.";return false;}
std::error_code ec;
if(dir.empty()||!std::filesystem::is_directory(diskPath(dir),ec)){error=L"Wybierz istniejący folder zapisu.";return false;}
for(const auto& name:names)if(std::filesystem::exists(diskPath(join(dir,name)),ec)){error=L"Plik już istnieje:\r\n"+name+L"\r\n\r\nWybierz inny folder lub zmień dane meczu. Nie zapisano żadnego pliku.";return false;}
std::vector<std::wstring> created;size_t index=0;
auto rollback=[&](){for(const auto& path:created){std::error_code ignored;std::filesystem::remove(diskPath(path),ignored);}};
for(const auto& name:names){const auto& pdf=payloads[index++];auto path=join(dir,name);bool ok=false;unsigned long code=0;
#ifdef _WIN32
HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
if(file==INVALID_HANDLE_VALUE){code=GetLastError();}else{created.push_back(path);DWORD count=0;ok=WriteFile(file,pdf.data(),(DWORD)pdf.size(),&count,nullptr)&&count==pdf.size();if(ok)ok=FlushFileBuffers(file)!=0;code=GetLastError();if(!CloseHandle(file)){ok=false;code=GetLastError();}}
#else
int file=open(diskPath(path).c_str(),O_WRONLY|O_CREAT|O_EXCL,0666);
if(file<0){code=errno;}else{created.push_back(path);ssize_t count=write(file,pdf.data(),pdf.size());ok=count==(ssize_t)pdf.size();if(ok)ok=fsync(file)==0;code=errno;if(close(file)<0){ok=false;code=errno;}}
#endif
if(!ok){rollback();error=L"Nie udało się zapisać pliku:\r\n"+name+L"\r\nKod błędu: "+std::to_wstring(code)+L". Sprawdź folder i uprawnienia do zapisu.";return false;}
}
return true;
}

inline bool writeBatch(const std::wstring& dir,const std::vector<std::wstring>& names,std::wstring& error){
    return writeBatchContents(dir,names,std::vector<std::string>(names.size(),blankPdf()),error);
}
inline bool writeMatchBatchContents(const std::wstring& base,const std::wstring& folder,
                           const std::vector<std::wstring>& files,const std::vector<std::string>& payloads,std::wstring& error) {
    std::error_code ec;
    if(base.empty()||!std::filesystem::is_directory(diskPath(base),ec)) {
        error=L"Wybierz istniejący folder zapisu.";return false;
    }
    if(!validComponent(folder)||std::any_of(files.begin(),files.end(),[](const auto& file){return !validComponent(file);})) {
        error=L"Nazwa folderu lub dokumentu zawiera niedozwolone znaki.";return false;
    }
    auto target=join(base,folder);
    bool created=std::filesystem::create_directory(diskPath(target),ec);
    if(ec||!std::filesystem::is_directory(diskPath(target),ec)) {
        error=L"Nie udało się utworzyć folderu meczu. Sprawdź uprawnienia do zapisu.";return false;
    }
    if(!writeBatchContents(target,files,payloads,error)) {
        if(created) {std::error_code ignored;std::filesystem::remove(diskPath(target),ignored);}
        return false;
    }
    return true;
}
inline bool writeMatchBatch(const std::wstring& base,const std::wstring& folder,
                           const std::vector<std::wstring>& files,std::wstring& error) {
    return writeMatchBatchContents(base,folder,files,std::vector<std::string>(files.size(),blankPdf()),error);
}
