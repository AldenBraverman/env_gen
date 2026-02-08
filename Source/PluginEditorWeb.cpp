#if ENVGEN_USE_WEB_GUI && JUCE_WEB_BROWSER

#include "PluginEditorWeb.h"

namespace
{
    constexpr int kDesignWidth  = 900;
    constexpr int kDesignHeight = 560;
    constexpr int kScopeHeight  = 200;
    constexpr int kScopeMargin = 10;

    static const char* const kParamIds[] = {
        "inputGain", "outputGain", "dryPass",
        "lane1_step0", "lane1_step1", "lane1_step2", "lane1_step3",
        "lane1_step4", "lane1_step5", "lane1_step6", "lane1_step7",
        "lane1_step8", "lane1_step9", "lane1_step10", "lane1_step11",
        "lane1_step12", "lane1_step13", "lane1_step14", "lane1_step15",
        "lane1_attack", "lane1_hold", "lane1_decay", "lane1_rate", "lane1_destination", "lane1_amount"
    };

    juce::File getGuiRootDirectory()
    {
        auto exe = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        auto dir = exe.getParentDirectory();
        // Standalone: exe is in .../MacOS/ or .../Debug/; try EnvGenGui next to it
        auto envGenGui = dir.getChildFile("EnvGenGui");
        if (envGenGui.exists() && envGenGui.getChildFile("index.html").exists())
            return envGenGui;
        // macOS app bundle: Contents/Resources/EnvGenGui
        auto resources = dir.getParentDirectory().getChildFile("Resources");
        envGenGui = resources.getChildFile("EnvGenGui");
        if (envGenGui.exists() && envGenGui.getChildFile("index.html").exists())
            return envGenGui;
        // Source tree for development: executable might be in build/.../Debug, gui/dist at repo root
        auto maybeDist = exe.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile("gui").getChildFile("dist");
        if (maybeDist.exists() && maybeDist.getChildFile("index.html").exists())
            return maybeDist;
        return {};
    }
}

//==============================================================================
EnvGenEditorWeb::EnvGenEditorWeb(EnvGenAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      guiRootDir(getGuiRootDirectory())
{
    setSize(kDesignWidth, kDesignHeight);
    setResizable(true, true);

    oscilloscope = std::make_unique<OsciloscopeComponent>();
    oscilloscope->setShowGrid(true);
    oscilloscope->setShowEnvelope(true);
    addAndMakeVisible(oscilloscope.get());
    processorRef.setScopeSink(oscilloscope.get());

    auto& apvts = processorRef.apvts;
    for (const auto* id : kParamIds)
        apvts.addParameterListener(id, this);

    juce::WebBrowserComponent::Options options;
    options = options.withNativeIntegrationEnabled(true);

#if JUCE_WINDOWS
    options = options.withWinWebView2Options(
        options.getWinWebView2BackendOptions().withUserDataFolder(
            juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("EnvGenWebView2")));
#endif

#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
    if (guiRootDir.exists())
    {
        auto provider = [root = guiRootDir](const juce::String& path) -> std::optional<juce::WebBrowserComponent::Resource>
        {
            juce::String pathTrimmed = path.trimCharactersAtStart("/");
            if (pathTrimmed.isEmpty())
                pathTrimmed = "index.html";
            juce::File f = root.getChildFile(pathTrimmed);
            if (!f.existsAsFile())
                return std::nullopt;
            juce::MemoryBlock mb;
            if (!f.loadFileAsData(mb))
                return std::nullopt;
            juce::WebBrowserComponent::Resource r;
            r.data.resize(mb.getSize());
            std::memcpy(r.data.data(), mb.getData(), mb.getSize());
            juce::String ext = f.getFileExtension().toLowerCase();
            if (ext == ".html" || ext == ".htm")
                r.mimeType = "text/html";
            else if (ext == ".js")
                r.mimeType = "application/javascript";
            else if (ext == ".css")
                r.mimeType = "text/css";
            else if (ext == ".json")
                r.mimeType = "application/json";
            else
                r.mimeType = "application/octet-stream";
            return r;
        };
        options = options.withResourceProvider(std::move(provider), "http://localhost:5173");
    }
#endif

    options = options.withNativeFunction("setParameter", [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion)
    {
        if (args.size() < 2)
        {
            if (completion)
                completion(juce::var(false));
            return;
        }
        juce::String id = args[0].toString();
        float value = static_cast<float>(args[1]);
        auto* param = processorRef.apvts.getParameter(id);
        if (param != nullptr)
        {
            param->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, value));
            if (completion)
                completion(juce::var(true));
        }
        else if (completion)
            completion(juce::var(false));
    });

    options = options.withNativeFunction("getState", [this](const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion completion)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        auto& apvtsRef = processorRef.apvts;
        for (const auto* id : kParamIds)
        {
            if (auto* param = apvtsRef.getParameter(id))
                obj->setProperty(juce::Identifier(id), param->getValue());
        }
        if (completion)
            completion(juce::var(obj.get()));
    });

    options = options.withNativeFunction("resetAllParameters", [this](const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion completion)
    {
        processorRef.resetAllParametersToDefault();
        if (completion)
            completion(juce::var(true));
    });

    webBrowser = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(webBrowser.get());

    const bool useDevEnv = juce::SystemStats::getEnvironmentVariable("ENVGEN_WEB_DEV", {}).equalsIgnoreCase("1");
#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
    const bool haveEmbedded = guiRootDir.exists();
#else
    const bool haveEmbedded = false;
#endif
    if (useDevEnv || !haveEmbedded)
        webBrowser->goToURL("http://localhost:5173");
#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
    else if (haveEmbedded)
        webBrowser->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
    else
        webBrowser->goToURL("data:text/html,<body style='margin:0;background:#1c1c1e;color:#e5e5e7;font-family:sans-serif;display:flex;align-items:center;justify-content:center;height:100vh'><p>Env Gen GUI: run npm run dev in gui/ then open the plugin (localhost:5173), or set ENVGEN_WEB_DEV=1 when launching the host.</p></body>");
}

EnvGenEditorWeb::~EnvGenEditorWeb()
{
    processorRef.setScopeSink(nullptr);
    auto& apvts = processorRef.apvts;
    for (const auto* id : kParamIds)
        apvts.removeParameterListener(id, this);
}

void EnvGenEditorWeb::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1c1c1e));
}

void EnvGenEditorWeb::resized()
{
    auto bounds = getLocalBounds();
    if (oscilloscope != nullptr)
    {
        auto scopeArea = bounds.removeFromTop(kScopeHeight + kScopeMargin).reduced(kScopeMargin, 0);
        scopeArea.removeFromBottom(kScopeMargin);
        oscilloscope->setBounds(scopeArea);
    }
    if (webBrowser != nullptr)
        webBrowser->setBounds(bounds);
}

void EnvGenEditorWeb::parameterChanged(const juce::String& parameterID, float newValue)
{
    (void) newValue;
    if (auto* param = processorRef.apvts.getParameter(parameterID))
        pushParameterToWeb(parameterID, param->getValue());
}

juce::String EnvGenEditorWeb::escapeJsString(const juce::String& s)
{
    juce::String out;
    for (juce::juce_wchar c : s)
    {
        if (c == '\\')
            out << "\\\\";
        else if (c == '\'')
            out << "\\'";
        else if (c == '\n')
            out << "\\n";
        else if (c == '\r')
            out << "\\r";
        else
            out << c;
    }
    return out;
}

void EnvGenEditorWeb::pushParameterToWeb(const juce::String& id, float normalisedValue)
{
    if (webBrowser == nullptr)
        return;
    juce::String script = "if (window.__ENVGEN__ && typeof window.__ENVGEN__.updateParams === 'function') { window.__ENVGEN__.updateParams('"
        + escapeJsString(id) + "', " + juce::String(normalisedValue) + "); }";
    webBrowser->evaluateJavascript(script, nullptr);
}

#endif
