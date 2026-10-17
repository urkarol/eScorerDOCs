#import <Cocoa/Cocoa.h>
#import <PDFKit/PDFKit.h>
#include "../batch.hpp"
#include "../lineup.hpp"
#include <codecvt>
#include <locale>

static NSString *str(const std::wstring& s) {
    auto utf8=std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(s);
    return [[NSString alloc] initWithBytes:utf8.data() length:utf8.size() encoding:NSUTF8StringEncoding];
}
static std::wstring wide(NSString *s) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(s.UTF8String ?: "");
}
static NSString *message(const std::exception& e) {
    std::string k=e.what();
    if(k=="number") return @"Wpisz numer meczu od 1 do 99999.";
    if(k=="same") return @"Gospodarz i gość muszą mieć różne kody.";
    if(k=="team") return @"Wybierz obie drużyny.";
    if(k=="date") return @"Sprawdź datę meczu.";
    if(k=="roster_duplicate") return @"Każda drużyna musi mieć inny skrót.";
    if(k=="roster_code") return @"Skrót drużyny musi mieć 3 znaki: A–Z lub cyfry.";
    if(k.rfind("roster_",0)==0) return @"Sprawdź nazwę, skrót i ligę drużyny.";
    return @"Sprawdź format daty, kolejność elementów i separatory.";
}
static void alert(NSString *text) {
    NSAlert *a=[NSAlert new]; a.messageText=@"eScorerDOCs"; a.informativeText=text; [a runModal];
}
static bool store(NSString *key,NSString *value) {
    // Atomic files make save failures visible. No settings are written into the .app.
    NSURL *base=[NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL *dir=[base URLByAppendingPathComponent:@"eScorerDOCs" isDirectory:YES];
    NSError *error=nil;
    if(![NSFileManager.defaultManager createDirectoryAtURL:dir withIntermediateDirectories:YES attributes:nil error:&error]) {
        alert(error.localizedDescription); return false;
    }
    if(![value writeToURL:[dir URLByAppendingPathComponent:key] atomically:YES encoding:NSUTF8StringEncoding error:&error]) {
        alert(error.localizedDescription); return false;
    }
    return true;
}
static NSString *load(NSString *key) {
    NSURL *base=[NSFileManager.defaultManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL *url=[[base URLByAppendingPathComponent:@"eScorerDOCs"] URLByAppendingPathComponent:key];
    return [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:NULL];
}

@interface TopView : NSView @end
@implementation TopView
- (BOOL)isFlipped { return YES; }
@end
static NSTextField *label(NSView *v,NSString *title,CGFloat x,CGFloat y,CGFloat w) {
    NSTextField *f=[NSTextField labelWithString:title]; f.frame=NSMakeRect(x,y,w,20);
    f.font=[NSFont systemFontOfSize:13]; [v addSubview:f]; return f;
}
static NSTextField *field(NSView *v,NSString *text,CGFloat x,CGFloat y,CGFloat w,CGFloat h) {
    NSTextField *f=[[NSTextField alloc] initWithFrame:NSMakeRect(x,y,w,h)];
    f.stringValue=text ?: @""; f.font=[NSFont systemFontOfSize:13]; [v addSubview:f]; return f;
}
static NSButton *button(NSView *v,NSString *title,CGFloat x,CGFloat y,CGFloat w,CGFloat h,id target,SEL action) {
    NSButton *b=[NSButton buttonWithTitle:title target:target action:action];
    b.frame=NSMakeRect(x,y,w,h); b.bezelStyle=NSBezelStyleRounded; [v addSubview:b]; return b;
}
static NSButton *iconButton(NSView *v,NSString *symbol,NSString *name,CGFloat x,CGFloat y,id target,SEL action) {
    NSButton *b=button(v,@"",x,y,30,30,target,action);
    b.image=[NSImage imageWithSystemSymbolName:symbol accessibilityDescription:name];
    b.imagePosition=NSImageOnly; b.toolTip=name; [b setAccessibilityLabel:name]; return b;
}
static NSWindow *window(NSString *title,CGFloat w,CGFloat h,BOOL resizable) {
    NSWindowStyleMask style=NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable;
    if(resizable)style|=NSWindowStyleMaskResizable;
    NSWindow *n=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,w,h) styleMask:style backing:NSBackingStoreBuffered defer:NO];
    n.title=title; n.releasedWhenClosed=NO; n.contentView=[[TopView alloc] initWithFrame:NSMakeRect(0,0,w,h)]; [n center]; return n;
}
static NSPopUpButton *popup(NSView *v,CGFloat x,CGFloat y,CGFloat w,id target,SEL action) {
    NSPopUpButton *p=[[NSPopUpButton alloc] initWithFrame:NSMakeRect(x,y,w,28) pullsDown:NO];
    p.target=target;p.action=action;[v addSubview:p];return p;
}
static NSTableView *table(NSView *v,CGFloat x,CGFloat y,CGFloat w,CGFloat h,NSArray<NSString*> *titles,NSArray<NSNumber*> *widths,id owner) {
    NSTableView *t=[NSTableView new]; t.delegate=owner; t.dataSource=owner; t.rowHeight=25; t.allowsMultipleSelection=NO;
    for(NSUInteger i=0;i<titles.count;i++) {
        NSTableColumn *c=[[NSTableColumn alloc] initWithIdentifier:[NSString stringWithFormat:@"%lu",(unsigned long)i]];
        c.title=titles[i]; c.width=widths[i].doubleValue; c.editable=NO; [t addTableColumn:c];
    }
    NSScrollView *s=[[NSScrollView alloc] initWithFrame:NSMakeRect(x,y,w,h)];
    s.documentView=t; s.hasVerticalScroller=YES; s.borderType=NSBezelBorder; [v addSubview:s]; return t;
}

@interface Preview : NSWindowController <NSWindowDelegate>
@property (nonatomic, strong) PDFView *pdf;
@property (nonatomic, strong) NSString *temporaryPath;
@property (nonatomic, copy) void (^onClose)(Preview *);
- (instancetype)initWithPath:(NSString *)path temporary:(BOOL)temporary;
- (void)printDocument:(id)sender;
- (void)cleanup;
@end
@implementation Preview
- (instancetype)initWithPath:(NSString *)path temporary:(BOOL)temporary {
    // Read the complete PDF into memory: it stays printable while the main window is closed.
    NSData *bytes=[NSData dataWithContentsOfFile:path];
    PDFDocument *doc=bytes?[[PDFDocument alloc] initWithData:bytes]:nil;
    if(!doc || doc.pageCount!=1)return nil;
    NSWindow *w=window(path.lastPathComponent,930,680,YES);
    self=[super initWithWindow:w];
    if(self) {
        self.temporaryPath=temporary?path:nil; w.delegate=self;
        PDFView *pdf=[[PDFView alloc] initWithFrame:NSMakeRect(0,0,930,634)];
        pdf.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;
        pdf.document=doc; pdf.autoScales=YES; pdf.displayMode=kPDFDisplaySinglePage; [w.contentView addSubview:pdf]; self.pdf=pdf;
        NSButton *b=button(w.contentView,@"Drukuj…",804,641,110,30,self,@selector(printDocument:));
        b.autoresizingMask=NSViewMinXMargin|NSViewMinYMargin; b.keyEquivalent=@"p"; b.keyEquivalentModifierMask=NSEventModifierFlagCommand;
    }
    return self;
}
- (void)printDocument:(id)sender {
    NSPrintInfo *info=[NSPrintInfo.sharedPrintInfo copy];
    info.paperSize=NSMakeSize(595.275591,841.889764); info.orientation=NSLandscapeOrientation;
    info.topMargin=0;info.bottomMargin=0;info.leftMargin=0;info.rightMargin=0;
    NSPrintOperation *operation=[self.pdf.document printOperationForPrintInfo:info scalingMode:kPDFPrintPageScaleDownToFit autoRotate:YES];
    if(operation)[operation runOperation];
}
- (void)cleanup {
    if(!self.temporaryPath)return;
    NSFileManager *fm=NSFileManager.defaultManager;
    [fm removeItemAtPath:self.temporaryPath error:NULL];
    NSString *dir=self.temporaryPath.stringByDeletingLastPathComponent;
    NSArray *remaining=[fm contentsOfDirectoryAtPath:dir error:NULL];
    if(remaining && remaining.count==0)[fm removeItemAtPath:dir error:NULL];
    self.temporaryPath=nil;
}
- (void)windowWillClose:(NSNotification *)notification {
    [self cleanup]; if(self.onClose)self.onClose(self);
}
@end

@interface TeamEditor : NSWindowController <NSTableViewDelegate,NSTableViewDataSource>
@property (nonatomic, strong) NSTableView *list;
@property (nonatomic, strong) NSTextField *nameField;
@property (nonatomic, strong) NSTextField *codeField;
@property (nonatomic, strong) NSPopUpButton *group;
@property (nonatomic, copy) void (^onSave)(void);
- (void)saveAll:(id)sender;
@end
@implementation TeamEditor {
    std::vector<Team> draft;
}
- (instancetype)init {
    self=[super initWithWindow:window(@"Zespoły",620,440,NO)];
    if(self) {
        draft=teams();NSView *v=self.window.contentView;
        self.list=table(v,16,16,588,246,@[@"Zespół",@"Skrót",@"Liga"],@[@345,@65,@150],self);
        label(v,@"Nazwa zespołu",16,276,300);self.nameField=field(v,@"",16,298,370,26);
        label(v,@"Skrót",398,276,75);self.codeField=field(v,@"",398,298,75,26);
        self.group=popup(v,16,334,240,nil,NULL);
        for(int i=0;i<4;i++)[self.group addItemWithTitle:str(leagues()[i].name)];
        button(v,@"Nowy",270,334,75,28,self,@selector(newTeam:));
        button(v,@"Zastosuj",350,334,110,28,self,@selector(applyTeam:));
        button(v,@"Usuń",472,334,132,28,self,@selector(deleteTeam:));
        button(v,@"Importuj…",16,396,110,28,self,@selector(importTeams:));
        button(v,@"Eksportuj…",132,396,110,28,self,@selector(exportTeams:));
        button(v,@"Anuluj",388,396,100,28,self,@selector(cancel:));
        button(v,@"Zapisz",494,396,110,28,self,@selector(saveAll:));
    }return self;
}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {return draft.size();}
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r {
    const auto& team=draft.at(r);NSInteger col=c.identifier.integerValue;
    return col==0?str(team.name):(col==1?str(team.code):str(leagues()[team.group].name));
}
- (void)tableViewSelectionDidChange:(NSNotification *)note {
    NSInteger r=self.list.selectedRow;if(r<0)return;
    self.nameField.stringValue=str(draft[r].name);self.codeField.stringValue=str(draft[r].code);[self.group selectItemAtIndex:draft[r].group];
}
- (void)newTeam:(id)sender {
    [self.list deselectAll:nil];self.nameField.stringValue=@"";self.codeField.stringValue=@"";[self.window makeFirstResponder:self.nameField];
}
- (BOOL)applyCurrent {
    auto candidate=draft;NSInteger r=self.list.selectedRow;
    Team entry{trimmed(wide(self.nameField.stringValue)),codeFromText(wide(self.codeField.stringValue)),(int)self.group.indexOfSelectedItem};
    if(r<0)candidate.push_back(entry);else candidate[r]=entry;
    try{validateTeams(candidate);}catch(const std::exception& e){alert(message(e));return NO;}
    draft=std::move(candidate);[self.list reloadData];
    [self.list selectRowIndexes:[NSIndexSet indexSetWithIndex:r<0?draft.size()-1:r] byExtendingSelection:NO];return YES;
}
- (void)applyTeam:(id)sender {[self.window makeFirstResponder:nil];[self applyCurrent];}
- (void)deleteTeam:(id)sender {
    NSInteger r=self.list.selectedRow;if(r<0)return;draft.erase(draft.begin()+r);[self.list reloadData];[self newTeam:nil];
}
- (void)importTeams:(id)sender {
    NSOpenPanel *p=[NSOpenPanel openPanel];p.canChooseDirectories=NO;p.allowsMultipleSelection=NO;
    if([p runModal]!=NSModalResponseOK)return;
    NSError *error=nil;NSString *text=[NSString stringWithContentsOfURL:p.URL encoding:NSUTF8StringEncoding error:&error];
    if(!text){alert(error.localizedDescription);return;}
    try{draft=parseTeams(wide(text));[self.list reloadData];[self newTeam:nil];}catch(const std::exception& e){alert(message(e));}
}
- (void)exportTeams:(id)sender {
    NSSavePanel *p=[NSSavePanel savePanel];p.nameFieldStringValue=@"eScorerDOCs-zespoly.txt";
    if([p runModal]!=NSModalResponseOK)return;NSError *error=nil;
    if(![str(serializeTeams(draft)) writeToURL:p.URL atomically:YES encoding:NSUTF8StringEncoding error:&error])alert(error.localizedDescription);
}
- (void)saveAll:(id)sender {
    [self.window makeFirstResponder:nil];
    // Save the pending form as well, so an edited name is never silently discarded.
    NSInteger r=self.list.selectedRow;
    bool pending=r>=0 ? (wide(self.nameField.stringValue)!=draft[r].name || codeFromText(wide(self.codeField.stringValue))!=draft[r].code || self.group.indexOfSelectedItem!=draft[r].group)
                      : (!self.nameField.stringValue.length?false:true);
    if(pending && ![self applyCurrent])return;
    if(!store(@"teams.txt",str(serializeTeams(draft))))return;
    teams()=draft;if(self.onSave)self.onSave();[self close];
}
- (void)cancel:(id)sender {[self close];}
@end

@interface FormatEditor : NSWindowController <NSTableViewDelegate,NSTableViewDataSource,NSTextFieldDelegate>
@property (nonatomic, strong) NSTableView *list;
@property (nonatomic, strong) NSSegmentedControl *mode;
@property (nonatomic, strong) NSTextField *dateFormat;
@property (nonatomic, strong) NSTextField *sample;
@property (nonatomic, copy) void (^onSave)(void);
@end
@implementation FormatEditor {
    NamingConfig draft;
    BOOL folderMode;
}
- (NamePattern&)pattern {return folderMode?draft.folder:draft.files;}
- (instancetype)init {
    self=[super initWithWindow:window(@"Format nazw",520,405,NO)];
    if(self) {
        draft=naming();NSView *v=self.window.contentView;
        self.mode=[NSSegmentedControl segmentedControlWithLabels:@[@"Dokumenty",@"Folder meczu"] trackingMode:NSSegmentSwitchTrackingSelectOne target:self action:@selector(changeMode:)];
        self.mode.frame=NSMakeRect(16,14,488,28);self.mode.selectedSegment=0;[v addSubview:self.mode];
        label(v,@"Przeciągnij elementy. Kliknij dwukrotnie, aby zmienić separator.",16,51,488);
        self.list=table(v,16,78,488,187,@[@"Element",@"Separator po elemencie"],@[@245,@215],self);
        [self.list registerForDraggedTypes:@[@"pl.novamanus.escorerdocs.part"]];
        [self.list setDraggingSourceOperationMask:NSDragOperationMove forLocal:YES];
        self.list.tableColumns[1].editable=YES;
        label(v,@"Format daty",16,280,100);self.dateFormat=field(v,str(draft.files.dateFormat),120,276,170,26);self.dateFormat.delegate=self;
        label(v,@"np. dd.MM.yyyy lub yyyyMMdd",300,280,204);
        self.sample=label(v,@"",16,315,488);self.sample.lineBreakMode=NSLineBreakByTruncatingMiddle;
        button(v,@"Domyślne",16,361,100,28,self,@selector(reset:));
        button(v,@"Anuluj",292,361,100,28,self,@selector(cancel:));
        button(v,@"Zapisz",398,361,106,28,self,@selector(save:));[self updateSample];
    }return self;
}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)t {return [self pattern].order.size();}
- (id)tableView:(NSTableView *)t objectValueForTableColumn:(NSTableColumn *)c row:(NSInteger)r {
    auto& p=[self pattern];
    if(c.identifier.integerValue==0)return str(partLabel(p.order.at(r)));
    return (size_t)r<p.separators.size()?str(p.separators[r]):@"";
}
- (BOOL)tableView:(NSTableView *)t shouldEditTableColumn:(NSTableColumn *)c row:(NSInteger)r {
    return c.identifier.integerValue==1 && (size_t)r<[self pattern].separators.size();
}
- (void)tableView:(NSTableView *)t setObjectValue:(id)value forTableColumn:(NSTableColumn *)c row:(NSInteger)r {
    if(c.identifier.integerValue==1 && (size_t)r<[self pattern].separators.size())[self pattern].separators[r]=wide(value);
    [self updateSample];
}
- (id<NSPasteboardWriting>)tableView:(NSTableView *)t pasteboardWriterForRow:(NSInteger)r {
    NSPasteboardItem *item=[NSPasteboardItem new];[item setString:[NSString stringWithFormat:@"%ld",(long)r] forType:@"pl.novamanus.escorerdocs.part"];return item;
}
- (NSDragOperation)tableView:(NSTableView *)t validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)r proposedDropOperation:(NSTableViewDropOperation)op {
    if(info.draggingSource!=t)return NSDragOperationNone;
    [t setDropRow:r dropOperation:NSTableViewDropAbove];return NSDragOperationMove;
}
- (BOOL)tableView:(NSTableView *)t acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)r dropOperation:(NSTableViewDropOperation)op {
    if(info.draggingSource!=t)return NO;
    NSString *s=[info.draggingPasteboard stringForType:@"pl.novamanus.escorerdocs.part"];if(!s)return NO;
    NSInteger from=s.integerValue;NSInteger count=[self pattern].order.size();
    if(from<0||from>=count||r<0||r>count)return NO;
    if(r>from)--r;movePart([self pattern],from,r);[t reloadData];[self updateSample];return YES;
}
- (void)updateSample {
    [self pattern].dateFormat=wide(self.dateFormat.stringValue);
    try{self.sample.stringValue=str(renderPattern([self pattern],matchValues(L"2026-10-10",0,L"30",L"BEL",L"GDA"),folderMode));}
    catch(const std::exception& e){self.sample.stringValue=message(e);}
}
- (void)controlTextDidChange:(NSNotification *)note {[self updateSample];}
- (void)changeMode:(id)sender {
    [self.window makeFirstResponder:nil];[self pattern].dateFormat=wide(self.dateFormat.stringValue);
    folderMode=self.mode.selectedSegment==1;self.dateFormat.stringValue=str([self pattern].dateFormat);[self.list reloadData];[self updateSample];
}
- (void)reset:(id)sender {
    [self.window makeFirstResponder:nil];draft=defaultNaming();self.dateFormat.stringValue=str([self pattern].dateFormat);[self.list reloadData];[self updateSample];
}
- (void)save:(id)sender {
    [self.window makeFirstResponder:nil];[self updateSample];
    try {
        validateNaming(draft);
        // Also reject patterns that produce invalid completed file/folder names.
        formattedFilenames(L"2026-10-10",0,L"30",L"BEL",L"GDA",draft);matchFolder(L"2026-10-10",0,L"30",L"BEL",L"GDA",draft);
        if(!store(@"formats.txt",str(serializeNaming(draft))))return;
        naming()=draft;if(self.onSave)self.onSave();[self close];
    }catch(const std::exception& e){alert(message(e));}
}
- (void)cancel:(id)sender {[self close];}
@end

@interface App : NSObject <NSApplicationDelegate,NSWindowDelegate,NSTextFieldDelegate>
@property (nonatomic, strong) NSWindow *mainWindow;
@property (nonatomic, strong) NSDatePicker *date;
@property (nonatomic, strong) NSPopUpButton *league;
@property (nonatomic, strong) NSPopUpButton *home;
@property (nonatomic, strong) NSPopUpButton *away;
@property (nonatomic, strong) NSTextField *number;
@property (nonatomic, strong) NSTextField *folder;
@property (nonatomic, strong) NSTextView *preview;
@property (nonatomic, strong) NSTextField *status;
@property (nonatomic, strong) NSMutableArray<Preview*> *previews;
@property (nonatomic, strong) TeamEditor *teamEditor;
@property (nonatomic, strong) FormatEditor *formatEditor;
- (void)refresh:(id)sender;
@end
@implementation App
- (void)applicationDidFinishLaunching:(NSNotification *)note {
    try{NSString *s=load(@"teams.txt");if(s)teams()=parseTeams(wide(s));}catch(const std::exception& e){alert(@"Nie można odczytać zapisanej listy zespołów. Użyto listy domyślnej.");}
    try{NSString *s=load(@"formats.txt");if(s)naming()=parseNaming(wide(s));}catch(const std::exception& e){alert(@"Nie można odczytać formatu nazw. Użyto formatu domyślnego.");}
    self.previews=[NSMutableArray new];self.mainWindow=window(@"eScorerDOCs",480,536,NO);self.mainWindow.delegate=self;
    NSView *v=self.mainWindow.contentView;
    label(v,@"Data meczu",16,10,160);
    self.date=[[NSDatePicker alloc] initWithFrame:NSMakeRect(16,31,155,26)];
    self.date.datePickerStyle=NSTextFieldAndStepperDatePickerStyle;self.date.datePickerElements=NSYearMonthDayDatePickerElementFlag;
    self.date.dateValue=[NSDate date];self.date.target=self;self.date.action=@selector(refresh:);[v addSubview:self.date];
    button(v,@"Dzisiaj",179,31,70,26,self,@selector(today:));label(v,@"Numer meczu",290,10,174);
    self.number=field(v,@"",290,31,174,26);self.number.delegate=self;
    label(v,@"Rozgrywki",16,73,280);button(v,@"Zespoły…",364,68,100,24,self,@selector(editTeams:));
    self.league=popup(v,16,94,448,self,@selector(changeLeague:));
    for(const auto& l:leagues())[self.league addItemWithTitle:[NSString stringWithFormat:@"%@ [%@]",str(l.name),str(l.code)]];
    NSInteger league=[NSUserDefaults.standardUserDefaults integerForKey:@"league"];[self.league selectItemAtIndex:league>=0&&league<8?league:0];
    label(v,@"Gospodarz",16,132,400);self.home=popup(v,16,153,448,self,@selector(refresh:));
    label(v,@"Gość",16,189,150);iconButton(v,@"arrow.triangle.2.circlepath",@"Zamień gospodarza i gościa",225,181,self,@selector(swap:));
    self.away=popup(v,16,212,448,self,@selector(refresh:));
    label(v,@"Folder zapisu",16,250,400);self.folder=field(v,[NSUserDefaults.standardUserDefaults stringForKey:@"folder"],16,271,360,26);self.folder.delegate=self;
    button(v,@"Wybierz…",385,271,79,26,self,@selector(browse:));
    label(v,@"Folder meczu i dokumenty",16,314,288);button(v,@"Format nazw…",344,309,120,24,self,@selector(editFormat:));
    NSScrollView *scroll=[[NSScrollView alloc] initWithFrame:NSMakeRect(16,335,448,107)];scroll.hasVerticalScroller=YES;scroll.hasHorizontalScroller=YES;scroll.borderType=NSBezelBorder;
    self.preview=[[NSTextView alloc] initWithFrame:NSMakeRect(0,0,800,107)];self.preview.editable=NO;self.preview.font=[NSFont monospacedSystemFontOfSize:11 weight:NSFontWeightRegular];
    self.preview.horizontallyResizable=YES;self.preview.verticallyResizable=YES;self.preview.textContainer.widthTracksTextView=NO;self.preview.textContainer.containerSize=NSMakeSize(2000,CGFLOAT_MAX);
    scroll.documentView=self.preview;[v addSubview:scroll];
    self.status=label(v,@"",16,451,448);self.status.frame=NSMakeRect(16,451,448,32);self.status.maximumNumberOfLines=2;
    iconButton(v,@"folder",@"Otwórz folder",16,491,self,@selector(openFolder:));
    iconButton(v,@"rectangle.grid.2x2",@"Generuj kartki na ustawienie",282,491,self,@selector(generateLineups:));
    NSButton *generate=button(v,@"Generuj PDF",323,491,141,30,self,@selector(generate:));generate.keyEquivalent=@"\r";
    [self fillTeams];
    for(NSPopUpButton *p in @[self.home,self.away]) {
        NSString *code=[NSUserDefaults.standardUserDefaults stringForKey:p==self.home?@"home":@"away"];
        for(NSMenuItem *item in p.itemArray)if([item.representedObject isEqual:code])[p selectItem:item];
    }
    [self refresh:nil];[self.mainWindow makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
}
- (std::wstring)dateText {
    NSDateFormatter *f=[NSDateFormatter new];f.locale=[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
    f.calendar=[[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];f.dateFormat=@"yyyy-MM-dd";return wide([f stringFromDate:self.date.dateValue]);
}
- (std::wstring)homeCode {return wide(self.home.selectedItem.representedObject);}
- (std::wstring)awayCode {return wide(self.away.selectedItem.representedObject);}
- (void)fillTeams {
    NSString *h=self.home.selectedItem.representedObject,*a=self.away.selectedItem.representedObject;
    for(NSPopUpButton *p in @[self.home,self.away]) {
        [p removeAllItems];[p addItemWithTitle:@"Wybierz drużynę…"];p.lastItem.representedObject=@"";
        for(const auto& t:teams())if(belongs(t.group,(int)self.league.indexOfSelectedItem)) {
            [p addItemWithTitle:[NSString stringWithFormat:@"%@ [%@]",str(t.name),str(t.code)]];p.lastItem.representedObject=str(t.code);
        }
        NSString *old=p==self.home?h:a;for(NSMenuItem *item in p.itemArray)if([item.representedObject isEqual:old])[p selectItem:item];
    }
}
- (void)persist {
    NSUserDefaults *d=NSUserDefaults.standardUserDefaults;
    [d setInteger:self.league.indexOfSelectedItem forKey:@"league"];[d setObject:self.folder.stringValue forKey:@"folder"];
    [d setObject:str([self homeCode]) forKey:@"home"];[d setObject:str([self awayCode]) forKey:@"away"];
}
- (void)refresh:(id)sender {
    try {
        auto number=wide(self.number.stringValue);std::wstring text;
        if(trimmed(number).empty()) {
            auto files=lineupFilenames([self homeCode],[self awayCode]);
            text=L"Kartki na ustawienie:\n"+files[0]+L"\n"+files[1]+L"\n\nBez numeru meczu: podglądy tymczasowe.";
            self.status.stringValue=@"Kartki gotowe. Dokumenty meczowe wymagają numeru.";
        }else {
            text=matchFolder([self dateText],(int)self.league.indexOfSelectedItem,number,[self homeCode],[self awayCode])+L"/\n";
            for(const auto& name:formattedFilenames([self dateText],(int)self.league.indexOfSelectedItem,number,[self homeCode],[self awayCode]))text+=name+L"\n";
            self.status.stringValue=@"Gotowe do zapisu.";
        }
        self.preview.string=str(text);
    }catch(const std::exception& e){self.preview.string=@"Uzupełnij dane meczu, aby zobaczyć nazwy plików.";self.status.stringValue=message(e);}
}
- (void)controlTextDidChange:(NSNotification *)note {[self refresh:nil];}
- (void)today:(id)sender {self.date.dateValue=[NSDate date];[self refresh:nil];}
- (void)changeLeague:(id)sender {[self fillTeams];[self persist];[self refresh:nil];}
- (void)swap:(id)sender {NSInteger h=self.home.indexOfSelectedItem;[self.home selectItemAtIndex:self.away.indexOfSelectedItem];[self.away selectItemAtIndex:h];[self refresh:nil];}
- (void)browse:(id)sender {
    NSOpenPanel *p=[NSOpenPanel openPanel];p.canChooseDirectories=YES;p.canChooseFiles=NO;p.canCreateDirectories=YES;
    if(self.folder.stringValue.length)p.directoryURL=[NSURL fileURLWithPath:self.folder.stringValue];
    if([p runModal]==NSModalResponseOK){self.folder.stringValue=p.URL.path;[self persist];}
}
- (void)openFolder:(id)sender {
    NSString *target=self.folder.stringValue;
    if(!target.length){alert(@"Wybierz folder zapisu.");return;}
    try {
        NSString *sub=str(matchFolder([self dateText],(int)self.league.indexOfSelectedItem,wide(self.number.stringValue),[self homeCode],[self awayCode]));
        NSString *candidate=[target stringByAppendingPathComponent:sub];BOOL directory=NO;
        if([NSFileManager.defaultManager fileExistsAtPath:candidate isDirectory:&directory]&&directory)target=candidate;
    }catch(...){}
    if(![NSWorkspace.sharedWorkspace openURL:[NSURL fileURLWithPath:target]])alert(@"Nie udało się otworzyć folderu.");
}
- (void)generate:(id)sender {
    [self.mainWindow makeFirstResponder:nil];
    try {
        auto files=formattedFilenames([self dateText],(int)self.league.indexOfSelectedItem,wide(self.number.stringValue),[self homeCode],[self awayCode]);
        auto sub=matchFolder([self dateText],(int)self.league.indexOfSelectedItem,wide(self.number.stringValue),[self homeCode],[self awayCode]);std::wstring error;
        if(!writeMatchBatch(wide(self.folder.stringValue),sub,files,error)){alert(str(error));return;}
        [self persist];self.status.stringValue=@"Pliki zapisane w folderze meczu.";
    }catch(const std::exception& e){alert(message(e));}
}
- (void)generateLineups:(id)sender {
    [self.mainWindow makeFirstResponder:nil];NSString *temp=nil;
    try {
        auto plan=planLineups([self dateText],(int)self.league.indexOfSelectedItem,wide(self.number.stringValue),[self homeCode],[self awayCode]);
        auto pdfs=std::vector<std::string>{lineupPdf([self homeCode]),lineupPdf([self awayCode])};std::wstring error,target;
        if(plan.temporary) {
            temp=[NSTemporaryDirectory() stringByAppendingPathComponent:[@"eScorerDOCs-" stringByAppendingString:NSUUID.UUID.UUIDString]];
            NSError *err=nil;
            if(![NSFileManager.defaultManager createDirectoryAtPath:temp withIntermediateDirectories:NO attributes:@{NSFilePosixPermissions:@0700} error:&err]){alert(err.localizedDescription);return;}
            target=wide(temp);
            if(!writeBatchContents(target,plan.files,pdfs,error)){[NSFileManager.defaultManager removeItemAtPath:temp error:NULL];alert(str(error));return;}
        }else {
            auto base=wide(self.folder.stringValue);
            if(!writeMatchBatchContents(base,plan.folder,plan.files,pdfs,error)){alert(str(error));return;}target=join(base,plan.folder);
        }
        __weak App *weak=self;
        for(const auto& file:plan.files) {
            NSString *path=str(join(target,file));
            if(plan.temporary) {
                Preview *p=[[Preview alloc] initWithPath:path temporary:YES];
                if(!p){[NSFileManager.defaultManager removeItemAtPath:path error:NULL];alert(@"Nie udało się odczytać PDF-u.");continue;}
                p.onClose=^(Preview *closed){[weak.previews removeObject:closed];};[self.previews addObject:p];[p showWindow:nil];
            }else if(![NSWorkspace.sharedWorkspace openURL:[NSURL fileURLWithPath:path]])alert(@"Kartkę zapisano, ale nie udało się otworzyć czytnika PDF.");
        }
        if(temp && [NSFileManager.defaultManager contentsOfDirectoryAtPath:temp error:NULL].count==0)[NSFileManager.defaultManager removeItemAtPath:temp error:NULL];
        [self persist];self.status.stringValue=plan.temporary?@"Podglądy tymczasowe — pliki znikną po ich zamknięciu.":@"Kartki zapisane w folderze meczu i otwarte.";
    }catch(const std::exception& e){if(temp)[NSFileManager.defaultManager removeItemAtPath:temp error:NULL];alert(message(e));}
}
- (void)editTeams:(id)sender {
    if(self.teamEditor.window.visible){[self.teamEditor.window makeKeyAndOrderFront:nil];return;}
    self.teamEditor=[TeamEditor new];__weak App *weak=self;
    self.teamEditor.onSave=^{[weak fillTeams];[weak refresh:nil];};[self.teamEditor showWindow:nil];
}
- (void)editFormat:(id)sender {
    if(self.formatEditor.window.visible){[self.formatEditor.window makeKeyAndOrderFront:nil];return;}
    self.formatEditor=[FormatEditor new];__weak App *weak=self;self.formatEditor.onSave=^{[weak refresh:nil];};[self.formatEditor showWindow:nil];
}
- (void)windowWillClose:(NSNotification *)notification {
    if(notification.object==self.mainWindow){[self persist];[self.teamEditor close];[self.formatEditor close];}
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {return YES;}
- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)visible {
    if(!self.mainWindow.visible)[self.mainWindow makeKeyAndOrderFront:nil];return YES;
}
- (void)applicationWillTerminate:(NSNotification *)note {
    [self persist];for(Preview *p in [self.previews copy])[p cleanup];
}
@end

static void menus() {
    NSMenu *bar=[NSMenu new];NSMenuItem *app=[NSMenuItem new];[bar addItem:app];
    NSMenu *a=[NSMenu new];[a addItemWithTitle:@"Zakończ eScorerDOCs" action:@selector(terminate:) keyEquivalent:@"q"];app.submenu=a;
    NSMenuItem *edit=[NSMenuItem new];edit.title=@"Edycja";[bar addItem:edit];NSMenu *e=[[NSMenu alloc] initWithTitle:@"Edycja"];edit.submenu=e;
    [e addItemWithTitle:@"Cofnij" action:@selector(undo:) keyEquivalent:@"z"];
    [e addItemWithTitle:@"Wytnij" action:@selector(cut:) keyEquivalent:@"x"];
    [e addItemWithTitle:@"Kopiuj" action:@selector(copy:) keyEquivalent:@"c"];
    [e addItemWithTitle:@"Wklej" action:@selector(paste:) keyEquivalent:@"v"];
    [e addItemWithTitle:@"Zaznacz wszystko" action:@selector(selectAll:) keyEquivalent:@"a"];
    NSMenuItem *file=[NSMenuItem new];file.title=@"Plik";[bar addItem:file];NSMenu *f=[[NSMenu alloc] initWithTitle:@"Plik"];file.submenu=f;
    [f addItemWithTitle:@"Drukuj…" action:@selector(printDocument:) keyEquivalent:@"p"];
    [f addItemWithTitle:@"Zamknij okno" action:@selector(performClose:) keyEquivalent:@"w"];
    NSApp.mainMenu=bar;
}
int main(int argc,char **argv) {
    @autoreleasepool {
        [NSApplication sharedApplication];[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];menus();
        __attribute__((objc_precise_lifetime)) App *delegate=[App new];NSApp.delegate=delegate;[NSApp run];
    }return 0;
}
