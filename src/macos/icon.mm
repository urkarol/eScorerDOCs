#import <Cocoa/Cocoa.h>
#include <initializer_list>
// Generate a document icon with the same compiler used for the app.
int main(int argc,char **argv) {
    @autoreleasepool {
        if(argc!=2)return 1;
        NSString *dir=[NSString stringWithUTF8String:argv[1]];
        for(int size : {16,32,64,128,256,512,1024}) {
            NSBitmapImageRep *rep=[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:size pixelsHigh:size bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 bitsPerPixel:0];
            [NSGraphicsContext saveGraphicsState];[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:rep]];
            NSAffineTransform *scale=[NSAffineTransform transform];[scale scaleBy:size/1024.0];[scale concat];
            [[NSColor colorWithSRGBRed:0.10 green:0.22 blue:0.38 alpha:1] setFill];
            [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(32,32,960,960) xRadius:205 yRadius:205] fill];
            [[NSColor colorWithSRGBRed:0.35 green:0.63 blue:0.90 alpha:1] setFill];
            [[NSBezierPath bezierPathWithRoundedRect:NSMakeRect(215,148,490,654) xRadius:35 yRadius:35] fill];
            [NSColor.whiteColor setFill];NSBezierPath *page=[NSBezierPath bezierPath];
            [page moveToPoint:NSMakePoint(319,245)];[page lineToPoint:NSMakePoint(319,880)];[page lineToPoint:NSMakePoint(663,880)];
            [page lineToPoint:NSMakePoint(809,734)];[page lineToPoint:NSMakePoint(809,245)];[page closePath];[page fill];
            [[NSColor colorWithSRGBRed:0.80 green:0.89 blue:0.98 alpha:1] setFill];NSBezierPath *fold=[NSBezierPath bezierPath];
            [fold moveToPoint:NSMakePoint(663,880)];[fold lineToPoint:NSMakePoint(663,734)];[fold lineToPoint:NSMakePoint(809,734)];[fold closePath];[fold fill];
            [[NSColor colorWithSRGBRed:0.10 green:0.22 blue:0.38 alpha:1] setStroke];
            for(int y : {625,505,385}){NSBezierPath *line=[NSBezierPath bezierPath];line.lineWidth=35;line.lineCapStyle=NSRoundLineCapStyle;[line moveToPoint:NSMakePoint(399,y)];[line lineToPoint:NSMakePoint(729,y)];[line stroke];}
            [NSGraphicsContext restoreGraphicsState];NSData *png=[rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
            NSString *name=[NSString stringWithFormat:@"render-%d.png",size];if(![png writeToFile:[dir stringByAppendingPathComponent:name] atomically:YES])return 2;
        }
    }return 0;
}
