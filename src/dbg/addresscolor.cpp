#include "addresscolor.h"
#include "database_cb_batcher.h"

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

protected:
    bool populateDbOperation(DbOperation & op, const ADDRESSCOLORINFO & value) const override
    {
        op.itemType = DbItemTypeAddressColor;
        op.manual = value.manual;
        op.modhash = value.modhash;
        op.address = value.addr;
        op.color = value.color;
        return true;
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

bool AddressColorSetRange(duint Start, duint End, duint color, bool Manual)
{
    if(Start > End)
        return false;

    DbCallbackBatcher batcher;
    bool result = false;
    for(duint address = Start; ; address++)
    {
        result = AddressColorSet(address, color, Manual) || result;
        if(address == End)
            break;
    }
    return result;
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
    DbCallbackBatcher batcher;
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
    DbCallbackBatcher batcher(true);
    addressColors.CacheLoad(Root);
}

bool AddressColorEnum(ADDRESSCOLORINFO* List, size_t* Size)
{
    return addressColors.Enum(List, Size);
}

void AddressColorClear(bool Terminating)
{
    DbCallbackBatcher batcher;
    addressColors.Clear(Terminating);
}

void AddressColorGetList(std::vector<ADDRESSCOLORINFO> & list)
{
    addressColors.GetList(list);
}

bool AddressColorGetInfo(duint Address, ADDRESSCOLORINFO* info)
{
    return addressColors.GetInfo(AddressColors::VaKey(Address), info);
}