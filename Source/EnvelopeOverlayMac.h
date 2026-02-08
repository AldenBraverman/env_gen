#pragma once

#if JUCE_MAC

#ifdef __cplusplus
extern "C" {
#endif

/** Makes the given NSView (e.g. WKWebView) draw a transparent background so content behind shows through. */
void envgen_make_webview_background_transparent(void* nsView);

#ifdef __cplusplus
}
#endif

#endif
