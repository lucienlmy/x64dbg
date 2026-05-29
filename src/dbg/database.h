#ifndef _DATABASE_H
#define _DATABASE_H

#include "_global.h"

enum class DbLoadSaveType
{
    CommandLine,
    DebugData,
    All
};

enum class DbItemType
{
    Function,
    Label,
    Comment,
    Bookmark,
    Loop,
    Argument
};

enum class DbOperationType
{
    Add,
    Remove
};

typedef struct DbOperation
{
    DbItemType itemType;
    DbOperationType opType;
    bool manual;
    union
    {
        struct
        {
            const char* text; // comments, labels
        };

        struct
        {
            duint end, instructioncount, parent; // functions, arguments, loops
            int depth; // loops only (nesting depth); unused by functions/arguments
        };
    };
    duint address, modhash;
} DbOperation;

void DbSave(DbLoadSaveType saveType, const char* dbfile = nullptr, bool disablecompression = false);
void DbLoad(DbLoadSaveType loadType, const char* dbfile = nullptr);
void DbClose();
void DbClear(bool terminating = false);
void DbSetPath(const char* Directory, const char* ModulePath);
bool DbCheckHash(duint currentHash);
duint DbGetHash();

#endif // _DATABASE_H
