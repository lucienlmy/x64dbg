#include "cmd-linecolor.h"
#include "linecolor.h"
#include "value.h"
#include "console.h"
#include <unordered_map>

static const std::unordered_map<std::string, duint> presetMap =
{
    { "red",    linecolor_red },
    { "green",  linecolor_green },
    { "blue",   linecolor_blue },
    { "yellow", linecolor_yellow },
    { "orange", linecolor_orange },
    { "purple", linecolor_purple },
};

static duint parsePreset(const char* colorStr)
{
    std::string lower(colorStr);
    for(auto & c : lower)
        c = tolower(c);

    auto it = presetMap.find(lower);
    if(it != presetMap.end())
        return it->second;

    dprintf(QT_TRANSLATE_NOOP("DBG", "Invalid color '%s' (expected: red, green, blue, yellow, orange, purple)\n"), colorStr);
    return linecolor_none;
}

bool cbDebugLineColorSet(int argc, char* argv[])
{
    if(argc < 2)
        return false;
    duint start;
    duint end;
    duint preset;
    if(argc >= 3)
    {
        start = DbgValFromString(argv[1]);
        end = start;
        preset = parsePreset(argv[2]);
    }
    else
    {
        SELECTIONDATA sel;
        if(!GuiSelectionGet(GUI_DISASSEMBLY, &sel))
            return false;
        start = sel.start;
        end = sel.end;
        preset = parsePreset(argv[1]);
    }
    if(preset == linecolor_none)
        return false;
    bool ok = false;
    for(duint addr = start; addr <= end; addr++)
        ok = LineColorSet(addr, preset, true) || ok;
    if(!ok)
        return false;
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugLineColorSetRange(int argc, char* argv[])
{
    if(argc < 4)
        return false;
    duint start = DbgValFromString(argv[1]);
    duint end = DbgValFromString(argv[2]);
    duint preset = parsePreset(argv[3]);
    if(preset == linecolor_none)
        return false;

    for(duint addr = start; addr <= end; addr++)
        LineColorSet(addr, preset, true);
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugLineColorDelete(int argc, char* argv[])
{
    if(argc >= 2)
    {
        duint addr = DbgValFromString(argv[1]);
        if(!LineColorDelete(addr))
            return false;
    }
    else
    {
        SELECTIONDATA sel;
        if(!GuiSelectionGet(GUI_DISASSEMBLY, &sel))
            return false;
        LineColorDelRange(sel.start, sel.end, true);
    }
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugLineColorDeleteRange(int argc, char* argv[])
{
    if(argc < 3)
        return false;
    duint start = DbgValFromString(argv[1]);
    duint end = DbgValFromString(argv[2]);
    LineColorDelRange(start, end, true);
    GuiUpdateDisassemblyView();
    return true;
}
