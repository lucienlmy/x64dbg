#include "QDarkApplication.h"

#include <QStyleFactory>
#include <QPalette>

#ifdef Q_OS_WIN
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
#    include <QTimer>
#    include <QPointer>
#endif

QDarkApplication::QDarkApplication(int & argc, char** argv)
    : QApplication(argc, argv)
{
    // https://www.qt.io/blog/dark-mode-on-windows-11-with-qt-6.5
    setStyle(QStyleFactory::create("Fusion"));

#if 0
    QColor lightGray(190, 190, 190);
    QColor gray(128, 128, 128);
    QColor midDarkGray(100, 100, 100);
    QColor darkGray(53, 53, 53);
    QColor black(25, 25, 25);
    QColor blue(42, 130, 218);
    QColor white(255, 255, 255);

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, darkGray);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, black);
    darkPalette.setColor(QPalette::AlternateBase, darkGray);
    darkPalette.setColor(QPalette::ToolTipBase, darkGray);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, lightGray);
    darkPalette.setColor(QPalette::Button, darkGray);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Link, blue);
    darkPalette.setColor(QPalette::Highlight, blue);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Light, blue);
    darkPalette.setColor(QPalette::Dark, midDarkGray);

    darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);

    setPalette(darkPalette);
#endif

#if 0
    QColor black(40, 42, 54);
    QColor white(248, 248, 242);
    QColor lightBlue(139, 233, 253);
    QColor green(80, 250, 123);
    QColor yellow(229, 238, 138);
    QColor red(255, 85, 85);
    QColor purple(189, 147, 249);
    QColor darkBlue(98, 114, 164);
    QColor grey(68, 71, 90);
    QColor orange(255, 184, 108);
    QColor pink(255, 121, 198);

    QPalette dark;
    dark.setColor(QPalette::Window, black);
    dark.setColor(QPalette::WindowText, white);
    dark.setColor(QPalette::Base, black);
    dark.setColor(QPalette::AlternateBase, grey);
    dark.setColor(QPalette::ToolTipBase, black);
    dark.setColor(QPalette::ToolTipText, lightBlue);
    dark.setColor(QPalette::Text, white);
    dark.setColor(QPalette::Button, black);
    dark.setColor(QPalette::ButtonText, white);
    dark.setColor(QPalette::BrightText, grey);
    dark.setColor(QPalette::Link, green);
    dark.setColor(QPalette::LinkVisited, purple);
    dark.setColor(QPalette::Highlight, grey);
    dark.setColor(QPalette::HighlightedText, white);
    dark.setColor(QPalette::Light, grey);

    dark.setColor(QPalette::Disabled, QPalette::Button, darkBlue);
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, darkBlue);
    dark.setColor(QPalette::Disabled, QPalette::Text, darkBlue);
    dark.setColor(QPalette::Disabled, QPalette::WindowText, darkBlue);

    setPalette(dark);
#endif

    QPalette palette;

    QColor oneDarkBackground("#282c34");
    QColor oneDarkForeground("#abb2bf");
    QColor oneDarkComment("#5c6370");
    QColor oneDarkKeyword("#c678dd");
    QColor oneDarkFunction("#61afef");
    QColor oneDarkString("#98c379");
    QColor oneDarkNumber("#d19a66");
    QColor oneDarkVariable("#e06c75");
    QColor oneDarkConstant("#56b6c2");

    palette.setColor(QPalette::Window, oneDarkBackground);
    palette.setColor(QPalette::WindowText, oneDarkForeground);
    palette.setColor(QPalette::Base, QColor("#21252b"));
    palette.setColor(QPalette::AlternateBase, QColor("#2c313c")); // used for alternate row color in tree
    palette.setColor(QPalette::ToolTipBase, QColor("#3a3f4b"));
    palette.setColor(QPalette::ToolTipText, oneDarkForeground);
    palette.setColor(QPalette::Text, oneDarkForeground);
    palette.setColor(QPalette::Button, QColor("#3a3f4b"));
    palette.setColor(QPalette::ButtonText, oneDarkForeground);
    palette.setColor(QPalette::BrightText, oneDarkVariable);
    palette.setColor(QPalette::Link, oneDarkFunction);
    palette.setColor(QPalette::LinkVisited, oneDarkKeyword);
    palette.setColor(QPalette::Highlight, oneDarkFunction);
    palette.setColor(QPalette::HighlightedText, oneDarkBackground);

    // Fusion draws bevels/frames/grooves from these five shading roles. They are
    // hand-tuned (same values as REVIDE's DarkTheme.h) so that raised elements read
    // as raised and sunken elements read as sunken on a dark background; deriving
    // them with lighter()/darker() makes the bevels too faint to see.
    palette.setColor(QPalette::Light, QColor("#4b5263"));
    palette.setColor(QPalette::Midlight, QColor("#3f4654"));
    palette.setColor(QPalette::Mid, QColor("#2c313c"));
    palette.setColor(QPalette::Dark, QColor("#1d2027"));
    palette.setColor(QPalette::Shadow, QColor("#151820"));

    palette.setColor(QPalette::PlaceholderText, oneDarkComment);

    palette.setColor(QPalette::Disabled, QPalette::WindowText, oneDarkComment);
    palette.setColor(QPalette::Disabled, QPalette::Text, oneDarkComment);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, oneDarkComment);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor("#3a3f4b")); // button(): keep disabled selections visible
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, oneDarkComment);
    palette.setColor(QPalette::Disabled, QPalette::Base, oneDarkBackground); // flatten disabled inputs into the window

    setPalette(palette);

    QColor separator(99, 99, 99);
    QColor header(75, 75, 75);

    // TODO: how is this derived by qt?
    separator = QColor("#616671");
    header = QColor("#484d59");

    ConfigurationPalette p;
    p.background = oneDarkBackground;
    p.darkGrey = separator;
    p.lightGrey = header;
    p.black = oneDarkForeground;

    // Create the global configuration instance
    mConfiguration = std::make_unique<Configuration>(p);

    auto hexText = Config()->Colors["HexDumpTextColor"];
    Config()->Colors["HexDumpByte00Color"] = oneDarkConstant;
    Config()->Colors["HexDumpByte7FColor"] = oneDarkVariable;
    Config()->Colors["HexDumpByteFFColor"] = oneDarkVariable;
    Config()->Colors["HexDumpByteIsPrintColor"] = oneDarkString;

    // One Dark colors for the disassembly instruction tokens. The Configuration
    // defaults for these are hardcoded for a light theme (black text, yellow/cyan
    // backgrounds), which is unreadable on a dark background.
    const QColor& text = oneDarkForeground;
    const QColor& mnemonic = oneDarkKeyword;
    const QColor& call = oneDarkFunction;
    const QColor& jump = oneDarkString;
    const QColor& ret = oneDarkVariable;
    const QColor& number = oneDarkNumber;
    const QColor& reg = oneDarkVariable;
    const QColor& comment = oneDarkComment;
    const QColor& constant = oneDarkConstant;
    const QColor& accent = oneDarkFunction;

    auto setColorPair = [this](const QString& name, const QColor& fg)
    {
        Config()->Colors[name + "Color"] = fg;
        Config()->Colors[name + "BackgroundColor"] = Qt::transparent;
    };

    setColorPair("InstructionComma", text);
    setColorPair("InstructionPrefix", mnemonic);
    setColorPair("InstructionUncategorized", text);
    setColorPair("InstructionAddress", accent);
    setColorPair("InstructionValue", number);
    setColorPair("TraceNewValue", QColor(Qt::red));
    setColorPair("InstructionMnemonic", mnemonic);
    setColorPair("InstructionPushPop", mnemonic);
    setColorPair("InstructionCall", call);
    setColorPair("InstructionRet", ret);
    setColorPair("InstructionConditionalJump", jump);
    setColorPair("InstructionUnconditionalJump", jump);
    setColorPair("InstructionNop", comment);
    setColorPair("InstructionFar", ret);
    setColorPair("InstructionInt3", ret);
    setColorPair("InstructionUnusual", ret);
    setColorPair("InstructionMemorySize", comment);
    setColorPair("InstructionMemorySegment", constant);
    setColorPair("InstructionMemoryBrackets", text);
    setColorPair("InstructionMemoryStackBrackets", constant);
    setColorPair("InstructionMemoryBaseRegister", reg);
    setColorPair("InstructionMemoryIndexRegister", reg);
    setColorPair("InstructionMemoryScale", number);
    setColorPair("InstructionMemoryOperator", text);
    setColorPair("InstructionGeneralRegister", reg);
    setColorPair("InstructionFpuRegister", constant);
    setColorPair("InstructionMmxRegister", constant);
    setColorPair("InstructionXmmRegister", constant);
    setColorPair("InstructionYmmRegister", constant);
    setColorPair("InstructionZmmRegister", constant);

    Config()->Colors["DisassemblyCommentColor"] = comment;
    Config()->Colors["DisassemblyAutoCommentColor"] = number;
}

// Based on DarkTheme::applyToWindow() from REVIDE (https://github.com/x64dbg/REVIDE)
void QDarkApplication::applyDarkTitleBar(QWidget* window)
{
#ifdef Q_OS_WIN
    if(!window)
        return;

    auto apply = [](QWidget* w)
    {
        HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
        if(!dwmapi)
            return;

        using DwmSetWindowAttributeFn = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
        auto setWindowAttribute = reinterpret_cast<DwmSetWindowAttributeFn>(GetProcAddress(dwmapi, "DwmSetWindowAttribute"));
        if(setWindowAttribute)
        {
            const BOOL enabled = TRUE;
            HWND hwnd = reinterpret_cast<HWND>(w->winId());
            constexpr DWORD ImmersiveDarkMode = 20;
            constexpr DWORD ImmersiveDarkModeBefore20H1 = 19;
            setWindowAttribute(hwnd, ImmersiveDarkModeBefore20H1, &enabled, sizeof(enabled));
            setWindowAttribute(hwnd, ImmersiveDarkMode, &enabled, sizeof(enabled));
            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        FreeLibrary(dwmapi);
    };

    // The window frame is not always ready when show() returns, so retry on the
    // next event loop iterations (QPointer guards against early destruction)
    apply(window);
    QPointer<QWidget> guard(window);
    QTimer::singleShot(0, window, [guard, apply]()
    {
        if(guard)
            apply(guard);
    });
    QTimer::singleShot(100, window, [guard, apply]()
    {
        if(guard)
            apply(guard);
    });
#else
    Q_UNUSED(window);
#endif
}
