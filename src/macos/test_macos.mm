// Compile the actual UI implementation, not a mock of the native frameworks.
#define main escorer_application_main
#include "main.mm"
#undef main
#include <cassert>
#include <iostream>
static void selectCode(NSPopUpButton *p,NSString *code) {
    for(NSMenuItem *item in p.itemArray)if([item.representedObject isEqual:code]){[p selectItem:item];return;}
    assert(false);
}
int main() {
    @autoreleasepool {
        [NSApplication sharedApplication];[NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        assert(escorerToWide(str(L"Zażółć gęślą jaźń 🏐"))==L"Zażółć gęślą jaźń 🏐");
        naming()=defaultNaming();teams()=defaultTeams();
        App *app=[App new];[app applicationDidFinishLaunching:nil];
        assert(app.mainWindow.contentView.frame.size.width==480);
        assert(app.mainWindow.contentView.frame.size.height==536);
        [app.league selectItemAtIndex:0];[app changeLeague:nil];
        assert(app.home.numberOfItems==15);selectCode(app.home,@"BEL");selectCode(app.away,@"GDA");
        [app.league selectItemAtIndex:1];[app changeLeague:nil];
        assert(app.home.numberOfItems==13);assert([app homeCode].empty());assert([app awayCode].empty());
        [app.league selectItemAtIndex:0];[app changeLeague:nil];selectCode(app.home,@"BEL");selectCode(app.away,@"GDA");
        [app swap:nil];assert([app homeCode]==L"GDA");[app swap:nil];
        NSString *base=[NSTemporaryDirectory() stringByAppendingPathComponent:[@"eScorerDOCs-test-żółć-" stringByAppendingString:NSUUID.UUID.UUIDString]];
        assert([NSFileManager.defaultManager createDirectoryAtPath:base withIntermediateDirectories:NO attributes:nil error:NULL]);
        app.folder.stringValue=base;app.number.stringValue=@"30";
        [app generate:nil];
        auto folder=matchFolder([app dateText],0,L"30",L"BEL",L"GDA");
        for(const auto& file:formattedFilenames([app dateText],0,L"30",L"BEL",L"GDA")) {
            PDFDocument *p=[[PDFDocument alloc] initWithURL:[NSURL fileURLWithPath:str(join(join(escorerToWide(base),folder),file))]];
            assert(p.pageCount==1);assert(p.string.length==0);
        }
        // Empty match number must work even without an output directory.
        app.number.stringValue=@"";app.folder.stringValue=@"";[app generateLineups:nil];assert(app.previews.count==2);
        NSArray<Preview*> *previews=[app.previews copy];NSMutableArray<NSString*> *paths=[NSMutableArray new];
        for(Preview *p in previews) {
            assert(p.pdf.document.pageCount==1);
            [paths addObject:p.temporaryPath];
            assert([NSFileManager.defaultManager fileExistsAtPath:p.temporaryPath]);
            NSRect bounds=[[p.pdf.document pageAtIndex:0] boundsForBox:kPDFDisplayBoxMediaBox];
            NSInteger rotation=[p.pdf.document pageAtIndex:0].rotation;
            if(rotation%180!=0)std::swap(bounds.size.width,bounds.size.height);
            assert(bounds.size.width>bounds.size.height);
            // Build a print operation without opening the print dialog or using a printer.
            assert([p.pdf.document printOperationForPrintInfo:[NSPrintInfo.sharedPrintInfo copy] scalingMode:kPDFPrintPageScaleDownToFit autoRotate:YES]!=nil);
        }
        [app.mainWindow close];assert(app.previews.count==2);
        [previews[0] close];assert(![NSFileManager.defaultManager fileExistsAtPath:paths[0]]);
        assert([NSFileManager.defaultManager fileExistsAtPath:paths[1]]);
        [previews[1] close];assert(app.previews.count==0);
        assert(![NSFileManager.defaultManager fileExistsAtPath:paths[1].stringByDeletingLastPathComponent]);
        FormatEditor *formats=[FormatEditor new];assert([formats pattern].order.size()==6);
        [formats close];
        [NSFileManager.defaultManager removeItemAtPath:base error:NULL];
        std::cout<<"PASS: native UI size, Unicode, league filters, PDFKit, temporary lifecycle, printing setup\n";
    }return 0;
}
