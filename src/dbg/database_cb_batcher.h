#ifndef _DATABASE_BATCHER_H
#define _DATABASE_BATCHER_H

#include "plugin_loader.h"
#include "database.h"

#define DB_MAX_CB_BATCH_SIZE ( 8096 ) // max number of operations in a single plugin callback

class DbCallbackBatcher
{
public:
    DbCallbackBatcher();
    ~DbCallbackBatcher();

    void Add(DbOperation op, const char* text = nullptr);
    void Flush();

    static DbCallbackBatcher* Get();
    static thread_local DbCallbackBatcher* tActiveBatcher; // per thread batch object

private:
    std::vector<DbOperation> mOperations;
    std::vector<std::string> mStrings;
    DbCallbackBatcher* mPrevious;
    bool mOwner; // if a batcher is defined in a thread where there's already a batcher on the stack, this is set to false
};

void DbCbNotifyLabel(DbOperationType opType, duint modhash, duint address, const char* text = nullptr, bool manual = false);
void DbCbNotifyComment(DbOperationType opType, duint modhash, duint address, const char* text = nullptr, bool manual = false);
void DbCbNotifyBookmark(DbOperationType opType, duint modhash, duint address, bool manual = false);
void DbCbNotifyFunction(DbOperationType opType, duint modhash, duint start, duint end = 0, duint instructioncount = 0, duint parent = 0, bool manual = false);
void DbCbNotifyArgument(DbOperationType opType, duint modhash, duint start, duint end = 0, duint instructioncount = 0, bool manual = false);
void DbCbNotifyLoop(DbOperationType opType, duint modhash, duint start, duint end = 0, duint instructioncount = 0, duint parent = 0, int depth = 0, bool manual = false);

#endif // _DATABASE_BATCHER_H
