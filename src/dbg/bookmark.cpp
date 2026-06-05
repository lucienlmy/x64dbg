#include "bookmark.h"
#include "database_cb_batcher.h"

struct BookmarkSerializer : AddrInfoSerializer<BOOKMARKSINFO>
{
};

struct Bookmarks : AddrInfoHashMap<LockBookmarks, BOOKMARKSINFO, BookmarkSerializer>
{
    const char* jsonKey() const override
    {
        return "bookmarks";
    }
};

static Bookmarks bookmarks;

bool BookmarkSet(duint Address, bool Manual)
{
    BOOKMARKSINFO bookmark;
    if(!bookmarks.PrepareValue(bookmark, Address, Manual))
        return false;
    auto key = Bookmarks::VaKey(Address);
    if(bookmarks.Contains(key))
        return bookmarks.Delete(key);
    if(bookmarks.Add(bookmark))
    {
        DbCbNotifyBookmark(DbOperationType::Add, bookmark.modhash, bookmark.addr, Manual);
        return true;
    }
    return false;
}

bool BookmarkGet(duint Address)
{
    return bookmarks.Contains(Bookmarks::VaKey(Address));
}

bool BookmarkDelete(duint Address)
{
    BOOKMARKSINFO info;
    if(bookmarks.Get(Bookmarks::VaKey(Address),info))
    {
        if(bookmarks.Delete(Bookmarks::VaKey(Address)))
        {
            DbCbNotifyBookmark(DbOperationType::Remove, info.modhash, info.addr);

            return true;
        }
    }

    return false;
}

void BookmarkDelRange(duint Start, duint End, bool Manual)
{
    DbCallbackBatcher batcher;
    bookmarks.DeleteRange(Start, End, Manual, [](const BOOKMARKSINFO & bookmark)
    {
        DbCbNotifyBookmark(DbOperationType::Remove, bookmark.modhash, bookmark.addr);
    });
}

void BookmarkCacheSave(JSON Root)
{
    bookmarks.CacheSave(Root);
}

void BookmarkCacheLoad(JSON Root)
{
    bookmarks.CacheLoad(Root);
    bookmarks.CacheLoad(Root, "auto"); //legacy support
}

bool BookmarkEnum(BOOKMARKSINFO* List, size_t* Size)
{
    return bookmarks.Enum(List, Size);
}

void BookmarkClear()
{
    DbCallbackBatcher batcher;
    bookmarks.Clear([](const BOOKMARKSINFO & bookmark)
    {
        DbCbNotifyBookmark(DbOperationType::Remove, bookmark.modhash, bookmark.addr);
    });
}

void BookmarkGetList(std::vector<BOOKMARKSINFO> & list)
{
    bookmarks.GetList(list);
}

bool BookmarkGetInfo(duint Address, BOOKMARKSINFO* info)
{
    return bookmarks.GetInfo(Bookmarks::VaKey(Address), info);
}
