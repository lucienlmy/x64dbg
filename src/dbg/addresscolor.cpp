#include "addresscolor.h"

struct AddressColorSerializer : AddrInfoSerializer<ADDRESSCOLORINFO>
{
    bool Save(const ADDRESSCOLORINFO & value) override
    {
        AddrInfoSerializer::Save(value);
        setHex("color", value.color);
        return true;
    }

    bool Load(ADDRESSCOLORINFO & value) override
    {
        return AddrInfoSerializer::Load(value) &&
               getHex("color", value.color);
    }
};

struct AddressColors : AddrInfoHashMap<LockAddressColors, ADDRESSCOLORINFO, AddressColorSerializer>
{
    const char* jsonKey() const override
    {
        return "addresscolors";
    }
};

static AddressColors addressColors;

bool AddressColorSet(duint Address, duint color, bool Manual)
{
    ADDRESSCOLORINFO info;
    if(!addressColors.PrepareValue(info, Address, Manual))
        return false;
    info.color = color;
    return addressColors.Add(info);
}

bool AddressColorGet(duint Address, duint* color)
{
    ADDRESSCOLORINFO info;
    if(!addressColors.Get(AddressColors::VaKey(Address), info))
        return false;
    if(color)
        *color = info.color;
    return true;
}

bool AddressColorDelete(duint Address)
{
    return addressColors.Delete(AddressColors::VaKey(Address));
}

void AddressColorDelRange(duint Start, duint End, bool Manual)
{
    addressColors.DeleteRangeWhere(Start, End, [Manual](duint start, duint end, const ADDRESSCOLORINFO & value)
    {
        if(Manual ? !value.manual : value.manual)
            return false;
        return value.addr >= start && value.addr <= end;
    });
}

void AddressColorCacheSave(JSON Root)
{
    addressColors.CacheSave(Root);
}

void AddressColorCacheLoad(JSON Root)
{
    addressColors.CacheLoad(Root);
}

bool AddressColorEnum(ADDRESSCOLORINFO* List, size_t* Size)
{
    return addressColors.Enum(List, Size);
}

void AddressColorClear()
{
    addressColors.Clear();
}

void AddressColorGetList(std::vector<ADDRESSCOLORINFO> & list)
{
    addressColors.GetList(list);
}

bool AddressColorGetInfo(duint Address, ADDRESSCOLORINFO* info)
{
    return addressColors.GetInfo(AddressColors::VaKey(Address), info);
}