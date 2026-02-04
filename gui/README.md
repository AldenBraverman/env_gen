# Env Gen Web GUI

React + TypeScript + Tailwind + shadcn-style UI that runs inside the plugin via JUCE `WebBrowserComponent` (WebView2 on Windows, WebKit on macOS).

## Development

```bash
npm install
npm run dev
```

Then build the plugin with web GUI and open it in a host (e.g. Reaper):

```bash
cmake -B build -DENVGEN_USE_WEB_GUI=ON
cmake --build build --config Release
```

With `ENVGEN_WEB_DEV=1` (or when no embedded bundle is found), the plugin loads the UI from `http://localhost:5173`.

## Build for embedding

```bash
npm run build
```

Then copy `gui/dist` to `EnvGenGui/` next to the plugin executable (or run the CMake target `EnvGenGui` to build and copy).
