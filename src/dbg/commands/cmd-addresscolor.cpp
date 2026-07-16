#include "cmd-addresscolor.h"
#include "addresscolor.h"
#include "value.h"
#include "console.h"
#include <unordered_map>

static const std::unordered_map<std::string, duint> presetMap =
{
    { "red",    addresscolor_red },
    { "green",  addresscolor_green },
    { "blue",   addresscolor_blue },
    { "yellow", addresscolor_yellow },
    { "orange", addresscolor_orange },
    { "purple", addresscolor_purple },
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
    return addresscolor_none;
}

bool cbDebugAddressColorSet(int argc, char* argv[])
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
    if(preset == addresscolor_none)
        return false;
    bool ok = false;
    for(duint addr = start; addr <= end; addr++)
        ok = AddressColorSet(addr, preset, true) || ok;
    if(!ok)
        return false;
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugAddressColorSetRange(int argc, char* argv[])
{
    if(argc < 4)
        return false;
    duint start = DbgValFromString(argv[1]);
    duint end = DbgValFromString(argv[2]);
    duint preset = parsePreset(argv[3]);
    if(preset == addresscolor_none)
        return false;

    for(duint addr = start; addr <= end; addr++)
        AddressColorSet(addr, preset, true);
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugAddressColorDelete(int argc, char* argv[])
{
    if(argc >= 3)
    {
        duint start = DbgValFromString(argv[1]);
        duint end = DbgValFromString(argv[2]);
        AddressColorDelRange(start, end, true);
    }
    else if(argc >= 2)
    {
        duint addr = DbgValFromString(argv[1]);
        if(!AddressColorDelete(addr))
            return false;
    }
    else
    {
        SELECTIONDATA sel;
        if(!GuiSelectionGet(GUI_DISASSEMBLY, &sel))
            return false;
        AddressColorDelRange(sel.start, sel.end, true);
    }
    GuiUpdateDisassemblyView();
    return true;
}

bool cbDebugAddressColorClear(int argc, char* argv[])
{
    AddressColorClear();
    GuiUpdateDisassemblyView();
    return true;
}
