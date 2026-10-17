#pragma once
static void loadTeamData() {
    auto stored=readString(L"Zespoly");
    if (stored.empty()) return;
    try { teams()=parseTeams(stored); }
    catch (...) {
        MessageBoxW(root,L"Nie udało się odczytać zapisanej listy zespołów. Wczytano listę początkową. Twoje zapisane dane nie zostały zmienione.",L"eScorerDOCs",MB_OK|MB_ICONWARNING);
    }
}
struct TeamEditorState {
    std::vector<size_t> visible;
    int selected=-1;
};
static std::wstring rosterError(const std::exception& error) {
    std::string reason=error.what();
    if(reason=="roster_name") return L"Wpisz nazwę drużyny (maksymalnie 180 znaków).";
    if(reason=="roster_code") return L"Skrót PLS musi mieć dokładnie 3 znaki: litery A-Z lub cyfry.";
    if(reason=="roster_duplicate") return L"Ten skrót PLS jest już używany przez inną drużynę. Każda drużyna musi mieć własny skrót.";
    return L"Wybierz ligę drużyny.";
}
static bool storeRoster(HWND owner,const std::vector<Team>& candidate) {
    try {
        auto serialized=serializeTeams(candidate);
        if(!saveString(L"Zespoly",serialized)) {
            MessageBoxW(owner,L"Nie udało się zapisać listy zespołów. Zmiany nie zostały zastosowane.",L"eScorerDOCs",MB_OK|MB_ICONERROR);
            return false;
        }
        teams()=candidate;
        fillTeams();
        return true;
    } catch (const std::exception& error) {
        MessageBoxW(owner,rosterError(error).c_str(),L"Sprawdź dane zespołu",MB_OK|MB_ICONWARNING);
        return false;
    }
}
static void editorNew(HWND dialog,TeamEditorState& state) {
    state.selected=-1;
    SendDlgItemMessageW(dialog,202,LB_SETCURSEL,(WPARAM)-1,0);
    SetDlgItemTextW(dialog,203,L"");
    SetDlgItemTextW(dialog,204,L"");
    SendDlgItemMessageW(dialog,205,CB_SETCURSEL,SendDlgItemMessageW(dialog,201,CB_GETCURSEL,0,0),0);
    EnableWindow(GetDlgItem(dialog,208),FALSE);
    SetFocus(GetDlgItem(dialog,203));
}
static void editorSelected(HWND dialog,TeamEditorState& state) {
    int row=(int)SendDlgItemMessageW(dialog,202,LB_GETCURSEL,0,0);
    if(row<0||row>=(int)state.visible.size()) { editorNew(dialog,state); return; }
    state.selected=(int)state.visible[row];
    const auto& team=teams()[state.selected];
    SetDlgItemTextW(dialog,203,team.name.c_str());
    SetDlgItemTextW(dialog,204,team.code.c_str());
    SendDlgItemMessageW(dialog,205,CB_SETCURSEL,team.group,0);
    EnableWindow(GetDlgItem(dialog,208),TRUE);
}
static void editorReload(HWND dialog,TeamEditorState& state,int select=-1) {
    int group=(int)SendDlgItemMessageW(dialog,201,CB_GETCURSEL,0,0);
    SendDlgItemMessageW(dialog,202,LB_RESETCONTENT,0,0);
    state.visible.clear();
    int selectedRow=-1;
    for(size_t i=0;i<teams().size();++i) if(teams()[i].group==group) {
        if((int)i==select) selectedRow=(int)state.visible.size();
        state.visible.push_back(i);
        auto text=teams()[i].code+L"  —  "+teams()[i].name;
        SendDlgItemMessageW(dialog,202,LB_ADDSTRING,0,(LPARAM)text.c_str());
    }
    if(selectedRow<0&&!state.visible.empty()) selectedRow=0;
    SendDlgItemMessageW(dialog,202,LB_SETCURSEL,selectedRow,0);
    editorSelected(dialog,state);
}
static void editorSave(HWND dialog,TeamEditorState& state) {
    Team entry{trimmed(getText(GetDlgItem(dialog,203))),codeFromText(getText(GetDlgItem(dialog,204))),
               (int)SendDlgItemMessageW(dialog,205,CB_GETCURSEL,0,0)};
    auto candidate=teams();
    int position=state.selected;
    if(position<0) { position=(int)candidate.size(); candidate.push_back(entry); }
    else candidate[position]=entry;
    if(!storeRoster(dialog,candidate)) return;
    SendDlgItemMessageW(dialog,201,CB_SETCURSEL,entry.group,0);
    editorReload(dialog,state,position);
    SetDlgItemTextW(dialog,209,L"Zespół zapisany. Zmiana jest już dostępna w głównym oknie.");
}
static void editorDelete(HWND dialog,TeamEditorState& state) {
    if(state.selected<0) return;
    auto question=L"Usunąć z listy zespół „"+teams()[state.selected].name+L"”?";
    if(MessageBoxW(dialog,question.c_str(),L"Usuń zespół",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES) return;
    auto candidate=teams();
    candidate.erase(candidate.begin()+state.selected);
    if(!storeRoster(dialog,candidate)) return;
    editorReload(dialog,state);
    SetDlgItemTextW(dialog,209,L"Zespół usunięty z listy.");
}
static INT_PTR CALLBACK teamEditorProc(HWND dialog,UINT message,WPARAM w,LPARAM l) {
    auto state=(TeamEditorState*)GetWindowLongPtrW(dialog,DWLP_USER);
    if(message==WM_INITDIALOG) {
        state=(TeamEditorState*)l;
        SetWindowLongPtrW(dialog,DWLP_USER,l);
        SendMessageW(dialog,WM_SETICON,ICON_SMALL,(LPARAM)LoadIconW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(101)));
        EnumChildWindows(dialog,[](HWND child,LPARAM value)->BOOL {
            SendMessageW(child,WM_SETFONT,(WPARAM)value,TRUE); return TRUE;
        },(LPARAM)font);
        for(int group=0;group<4;++group) {
            std::wstring text=std::wstring(leagues()[group].name)+L" ["+leagues()[group].code+L"]";
            for(int id:{201,205}) SendDlgItemMessageW(dialog,id,CB_ADDSTRING,0,(LPARAM)text.c_str());
        }
        int league=currentLeague();
        int group=league<4?league:((league==4||league==6)?0:1);
        SendDlgItemMessageW(dialog,201,CB_SETCURSEL,group,0);
        SendDlgItemMessageW(dialog,203,EM_SETLIMITTEXT,180,0);
        SendDlgItemMessageW(dialog,204,EM_SETLIMITTEXT,3,0);
        enableWholeFieldClick(GetDlgItem(dialog,201));
        enableWholeFieldClick(GetDlgItem(dialog,205));
        editorReload(dialog,*state);
        RECT owner{},bounds{};
        GetWindowRect(root,&owner); GetWindowRect(dialog,&bounds);
        SetWindowPos(dialog,nullptr,owner.left+(owner.right-owner.left-(bounds.right-bounds.left))/2,
                     owner.top+(owner.bottom-owner.top-(bounds.bottom-bounds.top))/2,0,0,SWP_NOSIZE|SWP_NOZORDER);
        return TRUE;
    }
    if(message==WM_COMMAND&&state) {
        int id=LOWORD(w),event=HIWORD(w);
        if(id==201&&event==CBN_SELCHANGE) {editorReload(dialog,*state);SetDlgItemTextW(dialog,209,L"Zmiany zapisuj przyciskiem „Zapisz zespół”.");return TRUE;}
        if(id==202&&event==LBN_SELCHANGE) {editorSelected(dialog,*state);return TRUE;}
        if(event==BN_CLICKED) {
            if(id==206) {editorNew(dialog,*state);SetDlgItemTextW(dialog,209,L"Wpisz nazwę i kod nowego zespołu, następnie kliknij „Zapisz zespół”.");return TRUE;}
            if(id==207) {editorSave(dialog,*state);return TRUE;}
            if(id==208) {editorDelete(dialog,*state);return TRUE;}
            if(id==IDCANCEL||id==IDOK) {EndDialog(dialog,IDCANCEL);return TRUE;}
        }
    }
    if(message==WM_CLOSE) {EndDialog(dialog,IDCANCEL);return TRUE;}
    return FALSE;
}
static void openTeamEditor() {
    TeamEditorState state;
    if(DialogBoxParamW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(201),root,teamEditorProc,(LPARAM)&state)==-1)
        MessageBoxW(root,L"Nie udało się otworzyć edytora zespołów.",L"eScorerDOCs",MB_OK|MB_ICONERROR);
}
