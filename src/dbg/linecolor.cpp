#include "linecolor.h"

struct LineColorSerializer : AddrInfoSerializer<LINECOLORINFO>
{
    bool Save(const LINECOLORINFO & value) override
    {
        AddrInfoSerializer::Save(value);
        setHex("color", value.color);
        return true;
    }

    bool Load(LINECOLORINFO & value) override
    {
        return AddrInfoSerializer::Load(value) &&
               getHex("color", value.color);
    }
};

struct LineColors : AddrInfoHashMap<LockLineColors, LINECOLORINFO, LineColorSerializer>
{
    const char* jsonKey() const override
    {
        return "linecolors";
    }
};

static LineColors lineColors;

bool LineColorSet(duint Address, duint color, bool Manual)
{
    LINECOLORINFO info;
    if(!lineColors.PrepareValue(info, Address, Manual))
        return false;
    info.color = color;
    return lineColors.Add(info);
}

bool LineColorGet(duint Address, duint* color)
{
    LINECOLORINFO info;
    if(!lineColors.Get(LineColors::VaKey(Address), info))
        return false;
    if(color)
        *color = info.color;
    return true;
}

bool LineColorDelete(duint Address)
{
    return lineColors.Delete(LineColors::VaKey(Address));
}

void LineColorDelRange(duint Start, duint End, bool Manual)
{
    lineColors.DeleteRangeWhere(Start, End, [Manual](duint start, duint end, const LINECOLORINFO & value)
    {
        if(Manual ? !value.manual : value.manual)
            return false;
        return value.addr >= start && value.addr <= end;
    });
}

void LineColorCacheSave(JSON Root)
{
    lineColors.CacheSave(Root);
}

void LineColorCacheLoad(JSON Root)
{
    lineColors.CacheLoad(Root);
}

bool LineColorEnum(LINECOLORINFO* List, size_t* Size)
{
    return lineColors.Enum(List, Size);
}

void LineColorClear()
{
    lineColors.Clear();
}

void LineColorGetList(std::vector<LINECOLORINFO> & list)
{
    lineColors.GetList(list);
}

bool LineColorGetInfo(duint Address, LINECOLORINFO* info)
{
    return lineColors.GetInfo(LineColors::VaKey(Address), info);
}