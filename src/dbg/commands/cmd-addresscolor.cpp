#include "cmd-addresscolor.h"
#include "addresscolor.h"
#include "value.h"
#include "console.h"

static duint parsePreset(const char* presetStr)
{
    duint count = 0;
    if(!BridgeSettingGetUint("Colors", "AddressColorCount", &count) || count == 0)
        count = 6;

    duint preset = DbgValFromString(presetStr);
    if(preset >= 1 && preset <= count)
        return preset;

    dprintf(QT_TRANSLATE_NOOP("DBG", "Invalid color preset '%s' (expected: 1-%u)\n"), presetStr, (unsigned int)count);
    return 0;
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
    if(preset == 0)
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
    if(preset == 0)
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
