#ifndef UNICODE
#define UNICODE
#endif
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include "batch.hpp"
#include "lineup.hpp"
#include "pdf_preview.hpp"

static HWND root, dateBox, leagueBox, numberBox, homeBox, awayBox, folderBox, previewBox, statusBox;
static HFONT font, titleFont, monoFont; static HBRUSH background; static int scaleDpi=96;
static std::vector<HICON> buttonIcons;
static const wchar_t* regPath=L"Software\\KarolTobiasz\\GeneratorPDFMecz";
static int px(int n){return MulDiv(n,scaleDpi,96);} static int currentLeague(){return (int)SendMessageW(leagueBox,CB_GETCURSEL,0,0);}
static std::wstring getText(HWND h){int n=GetWindowTextLengthW(h);std::wstring s(n+1,L'\0');GetWindowTextW(h,s.data(),n+1);s.resize(n);return s;}
static void setText(HWND h,const std::wstring& s){SetWindowTextW(h,s.c_str());}
static HWND add(const wchar_t* cls,const wchar_t* text,DWORD style,int x,int y,int w,int h,int id=0,DWORD ex=0){HWND v=CreateWindowExW(ex,cls,text,WS_CHILD|WS_VISIBLE|style,px(x),px(y),px(w),px(h),root,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);SendMessageW(v,WM_SETFONT,(WPARAM)font,TRUE);return v;}
static void iconButton(const wchar_t* accessibleName,int resource,const wchar_t* tooltip,int x,int y,int id) {
    HWND button=add(L"BUTTON",accessibleName,BS_ICON|WS_TABSTOP,x,y,30,30,id);
    HICON icon=(HICON)LoadImageW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(resource),IMAGE_ICON,px(22),px(22),0);
    buttonIcons.push_back(icon);SendMessageW(button,BM_SETIMAGE,IMAGE_ICON,(LPARAM)icon);
    HWND tip=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,WS_POPUP|TTS_ALWAYSTIP,
                            CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,root,nullptr,GetModuleHandleW(nullptr),nullptr);
    TOOLINFOW tool{};tool.cbSize=sizeof(tool);tool.uFlags=TTF_IDISHWND|TTF_SUBCLASS;tool.hwnd=root;
    tool.uId=(UINT_PTR)button;tool.lpszText=const_cast<wchar_t*>(tooltip);SendMessageW(tip,TTM_ADDTOOLW,0,(LPARAM)&tool);
}
static void label(const wchar_t* text,int x,int y,int w){add(L"STATIC",text,SS_LEFT,x,y,w,23);}
static bool saveString(const wchar_t* key,const std::wstring& value) {
    HKEY k;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,regPath,0,nullptr,0,KEY_SET_VALUE,nullptr,&k,nullptr)!=ERROR_SUCCESS) return false;
    LONG result=RegSetValueExW(k,key,0,REG_SZ,(const BYTE*)value.c_str(),(DWORD)((value.size()+1)*sizeof(wchar_t)));
    RegCloseKey(k);
    return result==ERROR_SUCCESS;
}
static std::wstring readString(const wchar_t* key){DWORD bytes=0;if(RegGetValueW(HKEY_CURRENT_USER,regPath,key,RRF_RT_REG_SZ,nullptr,nullptr,&bytes)!=ERROR_SUCCESS||bytes>1048576)return L"";std::wstring v(bytes/2,L'\0');if(RegGetValueW(HKEY_CURRENT_USER,regPath,key,RRF_RT_REG_SZ,nullptr,v.data(),&bytes)!=ERROR_SUCCESS)return L"";v.resize(wcslen(v.c_str()));return v;}
static std::wstring selectedDate(){SYSTEMTIME t{};if(DateTime_GetSystemtime(dateBox,&t)!=GDT_VALID)throw std::runtime_error("date");wchar_t b[32];swprintf(b,32,L"%04u-%02u-%02u",t.wYear,t.wMonth,t.wDay);return b;}
static std::vector<std::wstring> names(){return formattedFilenames(selectedDate(),currentLeague(),getText(numberBox),getText(homeBox),getText(awayBox));}
static std::wstring subfolder(){return matchFolder(selectedDate(),currentLeague(),getText(numberBox),getText(homeBox),getText(awayBox));}
static std::wstring readableError(const std::exception& e){std::string k=e.what();if(k.rfind("format_",0)==0)return L"Sprawdź ustawiony format nazw i dane meczu.";if(k=="number")return L"Wpisz numer meczu od 1 do 99999.";if(k=="team")return L"Wybierz obie drużyny lub wpisz ich trzyznakowe kody (litery A-Z i cyfry).";if(k=="same")return L"Gospodarz i gość muszą mieć różne kody.";return L"Uzupełnij datę oraz wybierz rozgrywki.";}
static void refresh(){
if(trimmed(getText(numberBox)).empty()) {
    try{auto files=lineupFilenames(getText(homeBox),getText(awayBox));
        setText(previewBox,L"Kartki na ustawienie:\r\n"+files[0]+L"\r\n"+files[1]+L"\r\n\r\nBez numeru meczu: podglądy tymczasowe.");
        setText(statusBox,L"Kartki gotowe. Dokumenty meczowe wymagają numeru.");return;
    }catch(const std::exception& e){
        setText(previewBox,L"Wybierz dwie różne drużyny, aby otworzyć kartki na ustawienie.\r\n\r\nNumer meczu jest potrzebny tylko do zapisu w folderze meczu i do pozostałych dokumentów.");
        setText(statusBox,readableError(e));return;
    }
}
try{auto v=names();std::wstring p=subfolder()+L"\\\r\n";for(auto& s:v)p+=s+L"\r\n";setText(previewBox,p);setText(statusBox,L"Gotowe do zapisu.");}catch(const std::exception& e){setText(previewBox,L"Uzupełnij dane meczu, aby zobaczyć nazwy plików.");setText(statusBox,readableError(e));}}
static void fillTeams(){auto hc=codeFromText(getText(homeBox)),ac=codeFromText(getText(awayBox));SendMessageW(homeBox,CB_RESETCONTENT,0,0);SendMessageW(awayBox,CB_RESETCONTENT,0,0);int i=0,hi=-1,ai=-1;for(const auto& t:teams())if(belongs(t.group,currentLeague())){std::wstring s=std::wstring(t.name)+L" ["+t.code+L"]";SendMessageW(homeBox,CB_ADDSTRING,0,(LPARAM)s.c_str());SendMessageW(awayBox,CB_ADDSTRING,0,(LPARAM)s.c_str());if(hc==t.code)hi=i;if(ac==t.code)ai=i;++i;}if(hi>=0)SendMessageW(homeBox,CB_SETCURSEL,hi,0);if(ai>=0)SendMessageW(awayBox,CB_SETCURSEL,ai,0);refresh();}
static void persist(){saveString(L"Folder",getText(folderBox));saveString(L"Rozgrywki",std::to_wstring(currentLeague()));saveString(L"Gospodarz",codeFromText(getText(homeBox)));saveString(L"Gosc",codeFromText(getText(awayBox)));}
static void generate(){try{auto v=names();std::wstring error,dir=getText(folderBox);if(dir.empty()){MessageBoxW(root,L"Najpierw wybierz folder zapisu.",L"Folder zapisu",MB_OK|MB_ICONINFORMATION);return;}if(!writeMatchBatch(dir,subfolder(),v,error)){MessageBoxW(root,error.c_str(),L"Nie zapisano dokumentów",MB_OK|MB_ICONWARNING);return;}persist();setText(statusBox,L"Pliki zapisane w folderze meczu.");MessageBoxW(root,L"Pliki PDF zostały zapisane w podfolderze meczu.",L"Gotowe",MB_OK|MB_ICONINFORMATION);}catch(const std::exception& e){MessageBoxW(root,readableError(e).c_str(),L"Sprawdź dane meczu",MB_OK|MB_ICONWARNING);}}
static void generateLineups() {
    std::wstring temporary;
    try {
        auto plan=planLineups(selectedDate(),currentLeague(),getText(numberBox),getText(homeBox),getText(awayBox));
        std::wstring error,target;
        std::vector<std::string> pdfs={lineupPdf(getText(homeBox)),lineupPdf(getText(awayBox))};
        if(plan.temporary) {
            temporary=pdfPreview::newTempFolder();target=temporary;
            if(!writeBatchContents(target,plan.files,pdfs,error)) {
                RemoveDirectoryW(target.c_str());MessageBoxW(root,error.c_str(),L"Nie zapisano kartek",MB_OK|MB_ICONWARNING);return;
            }
        } else {
            auto base=getText(folderBox);
            if(base.empty()){MessageBoxW(root,L"Najpierw wybierz folder zapisu.",L"Folder zapisu",MB_OK|MB_ICONINFORMATION);return;}
            if(!writeMatchBatchContents(base,plan.folder,plan.files,pdfs,error)) {
                MessageBoxW(root,error.c_str(),L"Nie zapisano kartek",MB_OK|MB_ICONWARNING);return;
            }
            target=join(base,plan.folder);
        }
        int failed=0;
        for(const auto& name:plan.files) {
            auto path=join(target,name);
            bool opened=plan.temporary?pdfPreview::openTemporary(path):
                (INT_PTR)ShellExecuteW(root,L"open",path.c_str(),nullptr,nullptr,SW_SHOWNORMAL)>32;
            if(!opened){++failed;if(plan.temporary)DeleteFileW(path.c_str());}
        }
        if(plan.temporary)RemoveDirectoryW(target.c_str());
        persist();
        setText(statusBox,plan.temporary?L"Podglądy tymczasowe — pliki znikną po ich zamknięciu.":L"Kartki zapisane w folderze meczu i otwarte.");
        if(failed)MessageBoxW(root,plan.temporary?L"Nie udało się uruchomić jednego z podglądów. Nieużywane pliki tymczasowe usunięto.":L"Kartki zapisano, ale nie udało się otworzyć jednego z PDF-ów. Sprawdź domyślny czytnik PDF w Windows.",L"Otwieranie PDF",MB_OK|MB_ICONWARNING);
    } catch(const std::exception& e) {
        if(!temporary.empty())RemoveDirectoryW(temporary.c_str());
        MessageBoxW(root,std::string(e.what())=="temp"?L"Nie udało się utworzyć folderu tymczasowego.":readableError(e).c_str(),L"Sprawdź dane meczu",MB_OK|MB_ICONWARNING);
    }
}
static int CALLBACK browseInit(HWND hwnd,UINT msg,LPARAM,LPARAM data){if(msg==BFFM_INITIALIZED&&data)SendMessageW(hwnd,BFFM_SETSELECTIONW,TRUE,data);return 0;}
static void browse(){auto old=getText(folderBox);BROWSEINFOW bi{};bi.hwndOwner=root;bi.lpszTitle=L"Wybierz folder zapisu";bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;bi.lpfn=browseInit;bi.lParam=(LPARAM)old.c_str();auto pid=SHBrowseForFolderW(&bi);if(pid){wchar_t path[MAX_PATH];if(SHGetPathFromIDListW(pid,path)){setText(folderBox,path);saveString(L"Folder",path);}CoTaskMemFree(pid);}}
static void openOutputFolder() {
    auto base=getText(folderBox),target=base;
    if(base.empty()) {MessageBoxW(root,L"Wybierz folder zapisu.",L"Folder",MB_OK|MB_ICONINFORMATION);return;}
    try { auto path=join(base,subfolder()); if(std::filesystem::is_directory(diskPath(path))) target=path; } catch(...) {}
    DWORD attributes=GetFileAttributesW(target.c_str());
    if(attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY)) ShellExecuteW(root,L"open",target.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
    else MessageBoxW(root,L"Wybierz istniejący folder.",L"Folder",MB_OK|MB_ICONINFORMATION);
}
static void swapTeams(){auto h=getText(homeBox),a=getText(awayBox);setText(homeBox,a);setText(awayBox,h);refresh();}
// Editable team fields open their lists on a click anywhere in the field.
// Keyboard editing remains available for custom PLS codes.
static LRESULT CALLBACK comboClick(HWND hwnd, UINT message, WPARAM w, LPARAM l,
                                   UINT_PTR, DWORD_PTR) {
    if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
        COMBOBOXINFO info{};
        info.cbSize = sizeof(info);
        if (GetComboBoxInfo(hwnd, &info)) {
            POINT point{GET_X_LPARAM(l), GET_Y_LPARAM(l)};
            if (!PtInRect(&info.rcButton, point)) {
                SetFocus(hwnd);
                SendMessageW(hwnd, CB_SHOWDROPDOWN, TRUE, 0);
                return 0;
            }
        }
    }
    if (message == WM_NCDESTROY) RemoveWindowSubclass(hwnd, comboClick, 1);
    return DefSubclassProc(hwnd, message, w, l);
}
static LRESULT CALLBACK editClick(HWND hwnd, UINT message, WPARAM w, LPARAM l,
                                  UINT_PTR, DWORD_PTR combo) {
    if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
        SetFocus(hwnd);
        SendMessageW((HWND)combo, CB_SHOWDROPDOWN, TRUE, 0);
        return 0;
    }
    if (message == WM_NCDESTROY) RemoveWindowSubclass(hwnd, editClick, 1);
    return DefSubclassProc(hwnd, message, w, l);
}
static void enableWholeFieldClick(HWND combo) {
    SetWindowSubclass(combo, comboClick, 1, 0);
    COMBOBOXINFO info{};
        info.cbSize = sizeof(info);
    if (GetComboBoxInfo(combo, &info) && info.hwndItem)
        SetWindowSubclass(info.hwndItem, editClick, 1, (DWORD_PTR)combo);
}
#include "team_editor.hpp"
#include "format_editor.hpp"

static void makeUi() {
    font = CreateFontW(-px(13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    monoFont = CreateFontW(-px(12), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");

    label(L"Data meczu", 16, 10, 160);
    dateBox = add(DATETIMEPICK_CLASSW, L"", DTS_SHORTDATEFORMAT | WS_TABSTOP,
                  16, 31, 155, 26, 100);
    SendMessageW(dateBox, DTM_SETFORMATW, 0, (LPARAM)L"yyyy-MM-dd");
    SYSTEMTIME st;
    GetLocalTime(&st);
    DateTime_SetSystemtime(dateBox, GDT_VALID, &st);
    add(L"BUTTON", L"Dzisiaj", WS_TABSTOP, 179, 31, 70, 26, 101);

    label(L"Numer meczu", 290, 10, 150);
    numberBox = add(L"EDIT", L"", WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL,
                    290, 31, 174, 26, 102, WS_EX_CLIENTEDGE);
    SendMessageW(numberBox, EM_SETLIMITTEXT, 5, 0);
    

    label(L"Rozgrywki", 16, 73, 280);
    add(L"BUTTON", L"Zespoły…", WS_TABSTOP, 364, 68, 100, 24, 113);
    leagueBox = add(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                    16, 94, 448, 260, 103);
    for (const auto& league : leagues()) {
        std::wstring text = std::wstring(league.name) + L" [" + league.code + L"]";
        SendMessageW(leagueBox, CB_ADDSTRING, 0, (LPARAM)text.c_str());
    }
    int league = 0;
    try { league = std::stoi(readString(L"Rozgrywki")); } catch (...) {}
    if (league < 0 || league > 7) league = 0;
    SendMessageW(leagueBox, CB_SETCURSEL, league, 0);
    enableWholeFieldClick(leagueBox);

    label(L"Gospodarz", 16, 132, 400);
    homeBox = add(WC_COMBOBOXW, L"", CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP,
                  16, 153, 448, 300, 104);
    label(L"Gość", 16, 189, 150);
    iconButton(L"Zamień drużyny",104,L"Zamień gospodarza i gościa",225,181,106);
    awayBox = add(WC_COMBOBOXW, L"", CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP,
                  16, 212, 448, 300, 105);
    for (HWND combo : {homeBox, awayBox}) {
        SendMessageW(combo, CB_SETDROPPEDWIDTH, px(448), 0);
        SendMessageW(combo, CB_SETITEMHEIGHT, (WPARAM)-1, px(20));
        enableWholeFieldClick(combo);
    }

    label(L"Folder zapisu", 16, 250, 400);
    folderBox = add(L"EDIT", readString(L"Folder").c_str(), ES_AUTOHSCROLL | WS_TABSTOP,
                    16, 271, 360, 26, 107, WS_EX_CLIENTEDGE);
    add(L"BUTTON", L"Wybierz…", WS_TABSTOP, 385, 271, 79, 26, 108);

    label(L"Folder meczu i dokumenty", 16, 314, 288);
    add(L"BUTTON", L"Format nazw…", WS_TABSTOP, 344, 309, 120, 24, 114);
    previewBox = add(L"EDIT", L"", ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL | WS_VSCROLL,
                     16, 335, 448, 107, 109, WS_EX_CLIENTEDGE);
    SendMessageW(previewBox, WM_SETFONT, (WPARAM)monoFont, TRUE);
    statusBox = add(L"STATIC", L"", SS_LEFT, 16, 451, 448, 32, 110);
    iconButton(L"Otwórz folder",105,L"Otwórz folder meczu lub folder zapisu",16,491,111);
    add(L"BUTTON", L"Generuj PDF", BS_DEFPUSHBUTTON | WS_TABSTOP, 323, 491, 141, 30, 112);
    iconButton(L"Kartki na ustawienie",102,L"Generuj i otwórz kartki na ustawienie dla obu drużyn",282,491,115);

    fillTeams();
    auto home = readString(L"Gospodarz"), away = readString(L"Gosc");
    setText(homeBox, home);
    setText(awayBox, away);
    for (const auto& team : teams()) if (belongs(team.group, league)) {
        std::wstring text = std::wstring(team.name) + L" [" + team.code + L"]";
        if (home == team.code) setText(homeBox, text);
        if (away == team.code) setText(awayBox, text);
    }
    refresh();
    SetFocus(numberBox);
}
static LRESULT CALLBACK windowProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){case WM_CREATE:root=h;loadTeamData();loadNameFormats();makeUi();return 0;
case WM_COMMAND:{int id=LOWORD(w),event=HIWORD(w);if(id==103&&event==CBN_SELCHANGE){fillTeams();saveString(L"Rozgrywki",std::to_wstring(currentLeague()));return 0;}if((id==104||id==105)&&(event==CBN_SELCHANGE||event==CBN_EDITCHANGE)){refresh();return 0;}if(id==102&&event==EN_CHANGE){refresh();return 0;}if(event==BN_CLICKED){if(id==101){SYSTEMTIME st;GetLocalTime(&st);DateTime_SetSystemtime(dateBox,GDT_VALID,&st);refresh();}if(id==106)swapTeams();if(id==108)browse();if(id==111)openOutputFolder();if(id==112)generate();if(id==113)openTeamEditor();if(id==114)openFormatEditor();if(id==115)generateLineups();}return 0;}
case WM_NOTIFY:if(((NMHDR*)l)->idFrom==100&&((NMHDR*)l)->code==DTN_DATETIMECHANGE)refresh();return 0;
case WM_CTLCOLORSTATIC:SetBkMode((HDC)w,TRANSPARENT);SetTextColor((HDC)w,RGB(30,41,59));return (LRESULT)background;
case WM_CLOSE:persist();DestroyWindow(h);return 0;
case WM_DESTROY:for(auto icon:buttonIcons)if(icon)DestroyIcon(icon);DeleteObject(font);DeleteObject(titleFont);DeleteObject(monoFont);PostQuitMessage(0);return 0;
}return DefWindowProcW(h,m,w,l);}
static int selfTest(const std::wstring& base){CreateDirectoryW(base.c_str(),nullptr);for(int league=0;league<8;++league){auto dir=join(base,leagues()[league].code);CreateDirectoryW(dir.c_str(),nullptr);std::wstring error;auto v=filenames(L"2026-10-09",league,L"30",L"bdm",L"jas");if(!writeBatch(dir,v,error))return 10+league;if(writeBatch(dir,v,error))return 30+league;for(auto& f:v){HANDLE h=CreateFileW(join(dir,f).c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);if(h==INVALID_HANDLE_VALUE)return 50;std::string actual(blankPdf().size(),'\0');DWORD count=0;BOOL ok=ReadFile(h,actual.data(),(DWORD)actual.size(),&count,nullptr);CloseHandle(h);if(!ok||actual!=blankPdf()||count!=actual.size())return 51;}}
return 0;}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int show){int argc;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(argc==3&&std::wstring(argv[1])==L"--preview-temp"){std::wstring path=argv[2];LocalFree(argv);return pdfPreview::run(path,instance,show);}if(argc==3&&std::wstring(argv[1])==L"--self-test"){int result=selfTest(argv[2]);LocalFree(argv);return result;}LocalFree(argv);
SetProcessDPIAware();HDC screen=GetDC(nullptr);scaleDpi=GetDeviceCaps(screen,LOGPIXELSX);ReleaseDC(nullptr,screen);INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_DATE_CLASSES|ICC_STANDARD_CLASSES};InitCommonControlsEx(&ic);RECT available;SystemParametersInfoW(SPI_GETWORKAREA,0,&available,0);scaleDpi=std::min(scaleDpi,std::max(72,(int)((available.bottom-available.top-64)*96/536)));CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);background=CreateSolidBrush(RGB(247,249,252));WNDCLASSEXW wc{};wc.cbSize=sizeof(wc);wc.hInstance=instance;wc.lpfnWndProc=windowProc;wc.lpszClassName=L"GeneratorPDFMeczWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(101));if(!wc.hIcon)wc.hIcon=LoadIconW(nullptr,IDI_APPLICATION);wc.hbrBackground=background;RegisterClassExW(&wc);DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;RECT r{0,0,px(480),px(536)};AdjustWindowRectEx(&r,style,FALSE,0);RECT work;SystemParametersInfoW(SPI_GETWORKAREA,0,&work,0);int width=r.right-r.left,height=r.bottom-r.top;int y=std::max(work.top,(work.bottom-work.top-height)/2+work.top);auto window=CreateWindowExW(0,wc.lpszClassName,L"eScorerDOCs",style,(work.right-work.left-width)/2+work.left,y,width,height,nullptr,nullptr,instance,nullptr);if(!window)return 1;ShowWindow(window,show);UpdateWindow(window);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){if(!IsDialogMessageW(window,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}DeleteObject(background);CoUninitialize();return (int)msg.wParam;}
