#pragma once
#include <roapi.h>
#include <winstring.h>
#include <inspectable.h>
#include <asyncinfo.h>
#include <shcore.h>
#include <shlwapi.h>
#include <wincodec.h>
#include <commdlg.h>
#include <memory>

// Windows.Data.Pdf ABI: Microsoft windows-rs, Windows/Data/Pdf/mod.rs.
// Uses the system PDF renderer on Windows 10/11, without external runtimes.
namespace pdfPreview {
struct Document;
struct Page : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE Render(IUnknown*,IUnknown**)=0;
    virtual HRESULT STDMETHODCALLTYPE RenderWithOptions(IUnknown*,IUnknown*,IUnknown**)=0;
};
struct Document : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE GetPage(UINT32,Page**)=0;
    virtual HRESULT STDMETHODCALLTYPE PageCount(UINT32*)=0;
};
struct DocumentStatics : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE LoadFile(IUnknown*,IUnknown**)=0;
    virtual HRESULT STDMETHODCALLTYPE LoadFilePassword(IUnknown*,HSTRING,IUnknown**)=0;
    virtual HRESULT STDMETHODCALLTYPE LoadStream(IUnknown*,IUnknown**)=0;
};
struct Operation : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE SetCompleted(IUnknown*)=0;
    virtual HRESULT STDMETHODCALLTYPE GetCompleted(IUnknown**)=0;
    virtual HRESULT STDMETHODCALLTYPE GetResults(Document**)=0;
};
struct RectF {float x,y,width,height;};
struct Options : IInspectable {
    virtual HRESULT STDMETHODCALLTYPE SourceRect(RectF*)=0;
    virtual HRESULT STDMETHODCALLTYPE SetSourceRect(RectF)=0;
    virtual HRESULT STDMETHODCALLTYPE Width(UINT32*)=0;
    virtual HRESULT STDMETHODCALLTYPE SetWidth(UINT32)=0;
    virtual HRESULT STDMETHODCALLTYPE Height(UINT32*)=0;
    virtual HRESULT STDMETHODCALLTYPE SetHeight(UINT32)=0;
};
inline const GUID staticsId={0x433a0b5f,0xc007,0x4788,{0x90,0xf2,0x08,0x14,0x3d,0x92,0x25,0x99}};
inline const GUID optionsId={0x3c98056f,0xb7cf,0x4c29,{0x9a,0x04,0x52,0xd9,0x02,0x67,0xf4,0x25}};
inline const GUID randomId={0x905a0fe1,0xbc53,0x11df,{0x8c,0x49,0x00,0x1e,0x4f,0xc6,0x86,0xda}};
inline const GUID asyncId={0x00000036,0x0000,0x0000,{0xc0,0,0,0,0,0,0,0x46}};
template<class T> struct Ref {
    T* p=nullptr;~Ref(){if(p)p->Release();}T** out(){return &p;}T* operator->()const{return p;}
    Ref()=default;Ref(const Ref&)=delete;Ref& operator=(const Ref&)=delete;
};
struct String {
    HSTRING value=nullptr;
    explicit String(const wchar_t* text){WindowsCreateString(text,(UINT32)wcslen(text),&value);}
    ~String(){WindowsDeleteString(value);}
};
inline void check(HRESULT hr){if(FAILED(hr))throw hr;}
inline void await(IUnknown* operation,HANDLE cancel) {
    Ref<IAsyncInfo> info;check(operation->QueryInterface(asyncId,(void**)info.out()));
    for(;;) {
        AsyncStatus status;check(info->get_Status(&status));
        if(status==Completed)return;
        if(status==Error){HRESULT hr=E_FAIL;info->get_ErrorCode(&hr);throw hr;}
        if(status==Canceled)throw HRESULT(E_ABORT);
        if(WaitForSingleObject(cancel,10)==WAIT_OBJECT_0){info->Cancel();throw HRESULT(E_ABORT);}
    }
}
struct State {
    HWND hwnd=nullptr;HANDLE cancel=nullptr,thread=nullptr,file=INVALID_HANDLE_VALUE;
    std::wstring path;std::string pdf;std::vector<BYTE> pixels;
    UINT width=0,height=0,stride=0;bool ready=false,complete=false;HRESULT error=S_OK;
};
inline void render(State& state) {
    Ref<IStream> input;input.p=SHCreateMemStream((const BYTE*)state.pdf.data(),(UINT)state.pdf.size());
    if(!input.p)throw HRESULT(E_OUTOFMEMORY);
    Ref<IUnknown> randomIn;check(CreateRandomAccessStreamOverStream(input.p,BSOS_DEFAULT,randomId,(void**)randomIn.out()));
    String name(L"Windows.Data.Pdf.PdfDocument");Ref<DocumentStatics> factory;
    check(RoGetActivationFactory(name.value,staticsId,(void**)factory.out()));
    Ref<Operation> load;check(factory->LoadStream(randomIn.p,(IUnknown**)load.out()));await(load.p,state.cancel);
    Ref<Document> document;check(load->GetResults(document.out()));UINT32 count=0;check(document->PageCount(&count));
    if(count!=1)throw HRESULT(E_INVALIDARG);
    Ref<Page> page;check(document->GetPage(0,page.out()));
    String optionsName(L"Windows.Data.Pdf.PdfPageRenderOptions");Ref<IInspectable> instance;
    check(RoActivateInstance(optionsName.value,instance.out()));Ref<Options> options;
    check(instance->QueryInterface(optionsId,(void**)options.out()));
    check(options->SetWidth(3508));check(options->SetHeight(2480));
    Ref<IStream> image;check(CreateStreamOnHGlobal(nullptr,TRUE,image.out()));
    Ref<IUnknown> randomOut;check(CreateRandomAccessStreamOverStream(image.p,BSOS_DEFAULT,randomId,(void**)randomOut.out()));
    Ref<IUnknown> action;check(page->RenderWithOptions(randomOut.p,options.p,action.out()));await(action.p,state.cancel);
    LARGE_INTEGER zero{};check(image->Seek(zero,STREAM_SEEK_SET,nullptr));
    Ref<IWICImagingFactory> wic;check(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(void**)wic.out()));
    Ref<IWICBitmapDecoder> decoder;check(wic->CreateDecoderFromStream(image.p,nullptr,WICDecodeMetadataCacheOnLoad,decoder.out()));
    Ref<IWICBitmapFrameDecode> frame;check(decoder->GetFrame(0,frame.out()));
    Ref<IWICFormatConverter> converter;check(wic->CreateFormatConverter(converter.out()));
    check(converter->Initialize(frame.p,GUID_WICPixelFormat24bppBGR,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom));
    check(converter->GetSize(&state.width,&state.height));
    if(!state.width||!state.height||state.width>10000||state.height>10000)throw HRESULT(E_FAIL);
    state.stride=(state.width*3+3)&~3u;state.pixels.resize((size_t)state.stride*state.height);
    check(converter->CopyPixels(nullptr,state.stride,(UINT)state.pixels.size(),state.pixels.data()));
}
inline DWORD WINAPI worker(void* parameter) {
    State& state=*(State*)parameter;HRESULT init=RoInitialize(RO_INIT_MULTITHREADED);
    if(FAILED(init))state.error=init;
    else {try{render(state);}catch(HRESULT hr){state.error=hr;}catch(...){state.error=E_FAIL;}RoUninitialize();}
    PostMessageW(state.hwnd,WM_APP+1,0,0);return 0;
}
inline void draw(HDC dc,const State& state,int x,int y,int w,int h) {
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=state.width;info.bmiHeader.biHeight=-(LONG)state.height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=24;info.bmiHeader.biCompression=BI_RGB;
    SetStretchBltMode(dc,HALFTONE);SetBrushOrgEx(dc,0,0,nullptr);
    StretchDIBits(dc,x,y,w,h,0,0,state.width,state.height,state.pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
}
inline void print(HWND window,const State& state) {
    PRINTDLGW dialog{};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=window;dialog.Flags=PD_RETURNDEFAULT;
    PrintDlgW(&dialog);
    if(dialog.hDevMode) {auto mode=(DEVMODEW*)GlobalLock(dialog.hDevMode);if(mode){mode->dmFields|=DM_ORIENTATION|DM_PAPERSIZE;mode->dmOrientation=DMORIENT_LANDSCAPE;mode->dmPaperSize=DMPAPER_A4;GlobalUnlock(dialog.hDevMode);}}
    dialog.Flags=PD_RETURNDC|PD_NOPAGENUMS|PD_NOSELECTION|PD_USEDEVMODECOPIESANDCOLLATE;dialog.nCopies=1;
    if(PrintDlgW(&dialog)) {
        DOCINFOW doc{};doc.cbSize=sizeof(doc);doc.lpszDocName=state.path.c_str();
        int width=GetDeviceCaps(dialog.hDC,HORZRES),height=GetDeviceCaps(dialog.hDC,VERTRES);
        double scale=std::min((double)width/state.width,(double)height/state.height);
        int w=(int)(state.width*scale),h=(int)(state.height*scale);
        bool ok=StartDocW(dialog.hDC,&doc)>0;
        if(ok){ok=StartPage(dialog.hDC)>0;if(ok){draw(dialog.hDC,state,(width-w)/2,(height-h)/2,w,h);ok=EndPage(dialog.hDC)>0;}if(ok)ok=EndDoc(dialog.hDC)>0;else AbortDoc(dialog.hDC);}
        if(!ok)MessageBoxW(window,L"Nie udało się wydrukować dokumentu.",L"Drukowanie",MB_OK|MB_ICONWARNING);
    }
    if(dialog.hDC)DeleteDC(dialog.hDC);
    if(dialog.hDevMode)GlobalFree(dialog.hDevMode);if(dialog.hDevNames)GlobalFree(dialog.hDevNames);
}
inline LRESULT CALLBACK proc(HWND window,UINT message,WPARAM w,LPARAM l) {
    auto state=(State*)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(message==WM_NCCREATE){state=(State*)((CREATESTRUCTW*)l)->lpCreateParams;state->hwnd=window;SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)state);}
    if(!state)return DefWindowProcW(window,message,w,l);
    switch(message) {
    case WM_CREATE: {
        HWND b=CreateWindowW(L"BUTTON",L"Drukuj…",WS_CHILD|WS_VISIBLE|WS_TABSTOP,12,9,90,28,window,(HMENU)1,nullptr,nullptr);
        EnableWindow(b,FALSE);SendMessageW(b,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
        b=CreateWindowW(L"BUTTON",L"Zamknij",WS_CHILD|WS_VISIBLE|WS_TABSTOP,112,9,90,28,window,(HMENU)2,nullptr,nullptr);
        SendMessageW(b,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
        state->thread=CreateThread(nullptr,0,worker,state,0,nullptr);
        if(!state->thread){state->error=HRESULT_FROM_WIN32(GetLastError());PostMessageW(window,WM_APP+1,0,0);}return 0;
    }
    case WM_APP+1:state->complete=true;state->ready=SUCCEEDED(state->error);EnableWindow(GetDlgItem(window,1),state->ready);InvalidateRect(window,nullptr,TRUE);return 0;
    case WM_SIZE:InvalidateRect(window,nullptr,TRUE);return 0;
    case WM_COMMAND:if(LOWORD(w)==1&&state->ready)print(window,*state);if(LOWORD(w)==2)DestroyWindow(window);return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint;HDC dc=BeginPaint(window,&paint);RECT client;GetClientRect(window,&client);
        FillRect(dc,&client,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
        if(state->ready) {
            double scale=std::max(0.01,std::min((double)std::max(1L,client.right-24)/state->width,(double)std::max(1L,client.bottom-60)/state->height));
            int width=(int)(state->width*scale),height=(int)(state->height*scale);
            draw(dc,*state,(client.right-width)/2,48+(client.bottom-48-height)/2,width,height);
        }else {RECT area{16,60,client.right-16,client.bottom-16};SetBkMode(dc,TRANSPARENT);
            DrawTextW(dc,state->complete&&FAILED(state->error)?L"Nie udało się otworzyć podglądu PDF. Zamknij okno, aby usunąć plik tymczasowy.":L"Otwieranie PDF…",-1,&area,DT_CENTER|DT_WORDBREAK);}
        EndPaint(window,&paint);return 0;
    }
    case WM_CLOSE:DestroyWindow(window);return 0;
    case WM_DESTROY:PostQuitMessage(0);return 0;
    }return DefWindowProcW(window,message,w,l);
}
inline std::wstring tempRoot(){std::vector<wchar_t> buffer(32768);DWORD size=GetTempPathW((DWORD)buffer.size(),buffer.data());if(!size||size>=buffer.size())throw std::runtime_error("temp");return buffer.data();}
inline bool ownedTempPath(const std::wstring& path) {
    auto file=std::filesystem::path(path);auto parent=file.parent_path();auto name=file.filename().wstring();
    if(name.size()!=10||name.substr(3)!=L"-R5.pdf"||name.substr(0,3).find_first_not_of(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")!=std::wstring::npos)return false;
    auto prefix=join(tempRoot(),L"eScorerDOCs-");auto directory=parent.wstring();
    if(directory.rfind(prefix,0)!=0||directory.size()!=prefix.size()+38)return false;
    GUID id;auto suffix=directory.substr(prefix.size());if(FAILED(CLSIDFromString(suffix.c_str(),&id)))return false;
    DWORD attributes=GetFileAttributesW(directory.c_str());
    return attributes!=INVALID_FILE_ATTRIBUTES&&!(attributes&FILE_ATTRIBUTE_REPARSE_POINT);
}
inline int run(const std::wstring& path,HINSTANCE instance,int show) {
    try {if(!ownedTempPath(path))return 2;}catch(...){return 2;}
    State state;state.path=path;
    state.file=CreateFileW(path.c_str(),GENERIC_READ|DELETE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                           nullptr,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,nullptr);
    if(state.file==INVALID_HANDLE_VALUE)return 3;
    LARGE_INTEGER length{};DWORD read=0;
    bool valid=GetFileSizeEx(state.file,&length)&&length.QuadPart>0&&length.QuadPart<32*1024*1024;
    if(valid){state.pdf.resize((size_t)length.QuadPart);valid=ReadFile(state.file,state.pdf.data(),(DWORD)state.pdf.size(),&read,nullptr)&&read==state.pdf.size();}
    if(!valid){CloseHandle(state.file);RemoveDirectoryW(std::filesystem::path(path).parent_path().c_str());return 4;}
    state.cancel=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    if(!state.cancel){CloseHandle(state.file);RemoveDirectoryW(std::filesystem::path(path).parent_path().c_str());return 5;}
    SetProcessDPIAware();CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    WNDCLASSW cls{};cls.lpfnWndProc=proc;cls.hInstance=instance;cls.lpszClassName=L"eScorerDOCsPdfPreview";
    cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(102));cls.hbrBackground=(HBRUSH)GetStockObject(LTGRAY_BRUSH);RegisterClassW(&cls);
    RECT area;SystemParametersInfoW(SPI_GETWORKAREA,0,&area,0);int width=std::min(1120L,area.right-area.left),height=std::min(850L,area.bottom-area.top);
    auto title=std::filesystem::path(path).filename().wstring()+L" — podgląd tymczasowy";
    HWND window=CreateWindowW(cls.lpszClassName,title.c_str(),WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                             area.left+(area.right-area.left-width)/2,area.top+(area.bottom-area.top-height)/2,width,height,nullptr,nullptr,instance,&state);
    if(window){ShowWindow(window,show);MSG message;while(GetMessageW(&message,nullptr,0,0)>0){
        if(message.message==WM_KEYDOWN&&message.wParam==VK_ESCAPE){DestroyWindow(window);continue;}
        if(message.message==WM_KEYDOWN&&message.wParam=='P'&&(GetKeyState(VK_CONTROL)&0x8000)&&state.ready){print(window,state);continue;}
        if(!IsDialogMessageW(window,&message)){TranslateMessage(&message);DispatchMessageW(&message);}}
    }
    SetEvent(state.cancel);if(state.thread){WaitForSingleObject(state.thread,INFINITE);CloseHandle(state.thread);}
    CloseHandle(state.cancel);CloseHandle(state.file); // Deletes this PDF, including on process crash.
    RemoveDirectoryW(std::filesystem::path(path).parent_path().c_str());CoUninitialize();return window?0:6;
}
inline std::wstring newTempFolder() {
    GUID id;if(FAILED(CoCreateGuid(&id)))throw std::runtime_error("temp");wchar_t suffix[40];StringFromGUID2(id,suffix,40);
    auto path=join(tempRoot(),std::wstring(L"eScorerDOCs-")+suffix);
    if(!CreateDirectoryW(path.c_str(),nullptr))throw std::runtime_error("temp");return path;
}
inline bool openTemporary(const std::wstring& path) {
    std::vector<wchar_t> exe(32768);DWORD size=GetModuleFileNameW(nullptr,exe.data(),(DWORD)exe.size());if(!size||size>=exe.size())return false;
    std::wstring command=L"\""+std::wstring(exe.data())+L"\" --preview-temp \""+path+L"\"";
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    if(!CreateProcessW(exe.data(),command.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process))return false;
    CloseHandle(process.hThread);CloseHandle(process.hProcess);return true;
}
}
