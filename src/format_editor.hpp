#pragma once
static std::wstring nameFormatError(const std::exception& error) {
    std::string reason=error.what();
    if(reason=="format_separator") return L"Separator może mieć do 12 znaków. Nie używaj znaków: < > : \" / \\ | ? * ani znaków nowej linii.";
    if(reason=="format_name") return L"Format tworzy nazwę niedozwoloną w Windows. Sprawdź separatory i dane meczu.";
    if(reason=="format_date") return L"Użyj raz yyyy/yy, MM/M i dd/d. Separatory: - _ . lub spacja.";
    return L"Nieprawidłowy układ elementów nazwy.";
}
static void loadNameFormats() {
    auto stored=readString(L"FormatyNazw");
    if(stored.empty()) return;
    try {naming()=parseNaming(stored);}
    catch(...) {MessageBoxW(root,L"Nie udało się odczytać zapisanych formatów nazw. Wczytano układ domyślny.",L"eScorerDOCs",MB_OK|MB_ICONWARNING);}
}
struct FormatEditorState {
    NamingConfig draft;
    std::array<std::wstring,6> values;
    bool folder=false,updating=false;
    int dragged=-1;
};
static NamePattern& editorPattern(FormatEditorState& state) {return state.folder?state.draft.folder:state.draft.files;}
static void formatPreview(HWND dialog,FormatEditorState& state) {
    try {
        SetDlgItemTextW(dialog,307,renderPattern(editorPattern(state),state.values,state.folder).c_str());
        SetDlgItemTextW(dialog,310,L"");
        EnableWindow(GetDlgItem(dialog,IDOK),TRUE);
    } catch(const std::exception& error) {
        SetDlgItemTextW(dialog,307,L"");
        SetDlgItemTextW(dialog,310,nameFormatError(error).c_str());
        EnableWindow(GetDlgItem(dialog,IDOK),FALSE);
    }
}
static std::wstring patternRow(const NamePattern& pattern,size_t index) {
    std::wstring text=partLabel(pattern.order[index]);
    if(index<pattern.separators.size()) {
        auto separator=pattern.separators[index];
        text+=L"     →     "+(separator.empty()?std::wstring(L"(bez separatora)"):L"„"+separator+L"”");
    }
    return text;
}
static void formatSelected(HWND dialog,FormatEditorState& state) {
    int selected=(int)SendDlgItemMessageW(dialog,302,LB_GETCURSEL,0,0);
    auto& pattern=editorPattern(state);
    state.updating=true;
    bool editable=selected>=0&&selected<(int)pattern.separators.size();
    SetDlgItemTextW(dialog,305,editable?pattern.separators[selected].c_str():L"");
    EnableWindow(GetDlgItem(dialog,305),editable);
    EnableWindow(GetDlgItem(dialog,303),selected>0);
    EnableWindow(GetDlgItem(dialog,304),selected>=0&&selected+1<(int)pattern.order.size());
    state.updating=false;
}
static void formatReload(HWND dialog,FormatEditorState& state,int selected=0) {
    auto& pattern=editorPattern(state);
    state.updating=true;
    SetDlgItemTextW(dialog,311,pattern.dateFormat.c_str());
    state.updating=false;
    SendDlgItemMessageW(dialog,302,LB_RESETCONTENT,0,0);
    for(size_t i=0;i<pattern.order.size();++i) {
        auto text=patternRow(pattern,i);
        SendDlgItemMessageW(dialog,302,LB_ADDSTRING,0,(LPARAM)text.c_str());
    }
    SendDlgItemMessageW(dialog,302,LB_SETCURSEL,selected,0);
    formatSelected(dialog,state);
    formatPreview(dialog,state);
}
static int formatDropIndex(HWND list,POINT screenPoint) {
    POINT local=screenPoint;ScreenToClient(list,&local);RECT bounds{};GetClientRect(list,&bounds);
    if(local.x<bounds.left||local.x>=bounds.right||local.y<bounds.top||local.y>=bounds.bottom) return -1;
    int row=LBItemFromPt(list,screenPoint,FALSE);
    if(row>=0) {
        RECT item{};SendMessageW(list,LB_GETITEMRECT,row,(LPARAM)&item);
        return row+(local.y>=(item.top+item.bottom)/2?1:0);
    }
    return (int)SendMessageW(list,LB_GETCOUNT,0,0);
}
static void formatMove(HWND dialog,FormatEditorState& state,int from,int to) {
    auto& pattern=editorPattern(state);
    if(from<0||to<0||from>=(int)pattern.order.size()||to>=(int)pattern.order.size()||from==to) return;
    movePart(pattern,(size_t)from,(size_t)to);
    formatReload(dialog,state,to);
}
static INT_PTR CALLBACK formatEditorProc(HWND dialog,UINT message,WPARAM w,LPARAM l) {
    static const UINT dragMessage=RegisterWindowMessageW(DRAGLISTMSGSTRING);
    auto state=(FormatEditorState*)GetWindowLongPtrW(dialog,DWLP_USER);
    if(message==WM_INITDIALOG) {
        state=(FormatEditorState*)l;SetWindowLongPtrW(dialog,DWLP_USER,l);
        SendMessageW(dialog,WM_SETICON,ICON_SMALL,(LPARAM)LoadIconW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101)));
        EnumChildWindows(dialog,[](HWND child,LPARAM value)->BOOL{SendMessageW(child,WM_SETFONT,(WPARAM)value,TRUE);return TRUE;},(LPARAM)font);
        // Size the separator input from its actual font metrics at the active DPI.
        HWND separator=GetDlgItem(dialog,305);
        HDC dc=GetDC(separator);
        HGDIOBJ previous=SelectObject(dc,font);
        TEXTMETRICW metrics{};
        GetTextMetricsW(dc,&metrics);
        SelectObject(dc,previous);
        ReleaseDC(separator,dc);
        RECT input{};
        GetWindowRect(separator,&input);
        int height=metrics.tmHeight+px(8);
        POINT position{input.left,input.top+(input.bottom-input.top-height)/2};
        ScreenToClient(dialog,&position);
        SetWindowPos(separator,nullptr,position.x,position.y,input.right-input.left,height,SWP_NOZORDER|SWP_NOACTIVATE);
        SendMessageW(separator,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,MAKELPARAM(px(4),px(4)));
        SendDlgItemMessageW(dialog,301,CB_ADDSTRING,0,(LPARAM)L"Nazwy dokumentów");
        SendDlgItemMessageW(dialog,301,CB_ADDSTRING,0,(LPARAM)L"Nazwa folderu meczu");
        SendDlgItemMessageW(dialog,301,CB_SETCURSEL,0,0);
        SendDlgItemMessageW(dialog,305,EM_SETLIMITTEXT,12,0);
        for(auto mask:{L"yyyy-MM-dd",L"yyyyMMdd",L"dd.MM.yyyy",L"dd-MM-yyyy",L"dd_MM_yyyy",L"ddMMyyyy",L"yyMMdd",L"dd.MM.yy",L"M-d-yyyy"})
            SendDlgItemMessageW(dialog,311,CB_ADDSTRING,0,(LPARAM)mask);
        SendDlgItemMessageW(dialog,311,CB_LIMITTEXT,32,0);
        enableWholeFieldClick(GetDlgItem(dialog,301));
        enableWholeFieldClick(GetDlgItem(dialog,311));
        MakeDragList(GetDlgItem(dialog,302));
        formatReload(dialog,*state);
        RECT owner{},bounds{};GetWindowRect(root,&owner);GetWindowRect(dialog,&bounds);
        SetWindowPos(dialog,nullptr,owner.left+(owner.right-owner.left-(bounds.right-bounds.left))/2,
                     owner.top+(owner.bottom-owner.top-(bounds.bottom-bounds.top))/2,0,0,SWP_NOSIZE|SWP_NOZORDER);
        return TRUE;
    }
    if(message==dragMessage&&state) {
        auto info=(DRAGLISTINFO*)l;
        auto result=[&](INT_PTR value)->INT_PTR {SetWindowLongPtrW(dialog,DWLP_MSGRESULT,value);return TRUE;};
        switch(info->uNotification) {
        case DL_BEGINDRAG:
            state->dragged=LBItemFromPt(info->hWnd,info->ptCursor,FALSE);
            return result(state->dragged>=0?TRUE:FALSE);
        case DL_DRAGGING: {
            int target=formatDropIndex(info->hWnd,info->ptCursor);
            DrawInsert(dialog,info->hWnd,target);
            return result(target>=0?DL_MOVECURSOR:DL_STOPCURSOR);
        }
        case DL_DROPPED: {
            int target=formatDropIndex(info->hWnd,info->ptCursor);
            DrawInsert(dialog,info->hWnd,-1);
            int from=state->dragged;state->dragged=-1;
            if(from>=0&&target>=0) {if(target>from)--target;formatMove(dialog,*state,from,target);}
            return result(TRUE);
        }
        case DL_CANCELDRAG:DrawInsert(dialog,info->hWnd,-1);state->dragged=-1;return result(TRUE);
        }
    }
    if(message==WM_COMMAND&&state) {
        int id=LOWORD(w),event=HIWORD(w);
        if(id==301&&event==CBN_SELCHANGE) {state->folder=SendDlgItemMessageW(dialog,301,CB_GETCURSEL,0,0)==1;formatReload(dialog,*state);return TRUE;}
        if(id==302&&event==LBN_SELCHANGE) {formatSelected(dialog,*state);return TRUE;}
        if(id==311&&!state->updating&&(event==CBN_SELCHANGE||event==CBN_EDITCHANGE)) {
            auto& pattern=editorPattern(*state);
            if(event==CBN_SELCHANGE) {
                int selected=(int)SendDlgItemMessageW(dialog,311,CB_GETCURSEL,0,0);
                wchar_t mask[33]{};
                if(selected!=CB_ERR) SendDlgItemMessageW(dialog,311,CB_GETLBTEXT,selected,(LPARAM)mask);
                pattern.dateFormat=mask;
            } else pattern.dateFormat=getText(GetDlgItem(dialog,311));
            formatPreview(dialog,*state);return TRUE;
        }
        if(id==305&&event==EN_CHANGE&&!state->updating) {
            int row=(int)SendDlgItemMessageW(dialog,302,LB_GETCURSEL,0,0);
            auto& pattern=editorPattern(*state);
            if(row>=0&&row<(int)pattern.separators.size()) {
                pattern.separators[row]=getText(GetDlgItem(dialog,305));
                auto text=patternRow(pattern,(size_t)row);
                SendDlgItemMessageW(dialog,302,LB_DELETESTRING,row,0);
                SendDlgItemMessageW(dialog,302,LB_INSERTSTRING,row,(LPARAM)text.c_str());
                SendDlgItemMessageW(dialog,302,LB_SETCURSEL,row,0);
                formatPreview(dialog,*state);
            }
            return TRUE;
        }
        if(event==BN_CLICKED) {
            if(id==303||id==304) {int row=(int)SendDlgItemMessageW(dialog,302,LB_GETCURSEL,0,0);formatMove(dialog,*state,row,row+(id==303?-1:1));return TRUE;}
            if(id==306) {auto defaults=defaultNaming();editorPattern(*state)=state->folder?defaults.folder:defaults.files;formatReload(dialog,*state);return TRUE;}
            if(id==IDOK) {
                try {
                    auto stored=serializeNaming(state->draft);
                    if(!saveString(L"FormatyNazw",stored)) {MessageBoxW(dialog,L"Nie udało się zapisać formatu nazw. Zmiany nie zostały zastosowane.",L"eScorerDOCs",MB_OK|MB_ICONERROR);return TRUE;}
                    naming()=state->draft;refresh();EndDialog(dialog,IDOK);
                } catch(const std::exception& error) {MessageBoxW(dialog,nameFormatError(error).c_str(),L"Sprawdź format",MB_OK|MB_ICONWARNING);}
                return TRUE;
            }
            if(id==IDCANCEL) {EndDialog(dialog,IDCANCEL);return TRUE;}
        }
    }
    if(message==WM_CLOSE) {EndDialog(dialog,IDCANCEL);return TRUE;}
    return FALSE;
}
static void openFormatEditor() {
    FormatEditorState state;state.draft=naming();
    try {state.values=matchValues(selectedDate(),currentLeague(),getText(numberBox),getText(homeBox),getText(awayBox));}
    catch(...) {state.values=matchValues(selectedDate(),std::max(0,currentLeague()),L"30",L"BEL",L"GDA");}
    if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(301),root,formatEditorProc,(LPARAM)&state)==-1)
        MessageBoxW(root,L"Nie udało się otworzyć edytora formatów nazw.",L"eScorerDOCs",MB_OK|MB_ICONERROR);
}
