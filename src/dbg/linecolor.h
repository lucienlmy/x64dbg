#ifndef _LINECOLOR_H
#define _LINECOLOR_H

#include "_global.h"
#include "addrinfo.h"

struct LINECOLORINFO : AddrInfo
{
    duint color;
};

bool LineColorSet(duint Address, duint color, bool Manual);
bool LineColorGet(duint Address, duint* color);
bool LineColorDelete(duint Address);
void LineColorDelRange(duint Start, duint End, bool Manual);
void LineColorCacheSave(JSON Root);
void LineColorCacheLoad(JSON Root);
bool LineColorEnum(LINECOLORINFO* List, size_t* Size);
void LineColorClear();
void LineColorGetList(std::vector<LINECOLORINFO> & list);
bool LineColorGetInfo(duint Address, LINECOLORINFO* info);

#endif // _LINECOLOR_H