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

    void Add(DbOperation & op);
    void Flush();

    static DbCallbackBatcher* Get();
    static thread_local DbCallbackBatcher* tActiveBatcher; // per thread batch object

private:
    std::vector<DbOperation> mOperations;
    std::vector<std::string> mStrings;
    DbCallbackBatcher* mPrevious;
    bool mOwner; // if a batcher is defined in a thread where there's already a batcher on the stack, this is set to false
};

void DbCbNotify(DbOperation & op);

#endif // _DATABASE_BATCHER_H
