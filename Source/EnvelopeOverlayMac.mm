#if JUCE_MAC

#include "EnvelopeOverlayMac.h"
#import <AppKit/AppKit.h>

extern "C" void envgen_make_webview_background_transparent(void* nsView)
{
    NSView* view = (NSView*) nsView;
    if (view == nil)
        return;
    [view setWantsLayer: YES];
    view.layer.backgroundColor = [[NSColor clearColor] CGColor];
}

#endif
