#include "database_cb_batcher.h"
#include <atomic>
#include "_plugins.h"
#include "plugin_loader.h"

thread_local DbCallbackBatcher* DbCallbackBatcher::tActiveBatcher = nullptr;

DbCallbackBatcher::DbCallbackBatcher(bool loading)
    : mLoading(loading)
{
    mPrevious = tActiveBatcher;
    mOwner = (mPrevious == nullptr);

    if(mOwner)
    {
        static std::atomic_uint32_t gBatchId(1);
        mBatchId = gBatchId++;
        mActive = IsActive(loading);
        tActiveBatcher = this;
    }
}

DbCallbackBatcher::~DbCallbackBatcher()
{
    if(mOwner)
    {
        this->flush();
        DbCallbackBatcher::tActiveBatcher = mPrevious;
    }
}

bool DbCallbackBatcher::IsActive(bool loading)
{
    if(tActiveBatcher)
    {
        ASSERT_ALWAYS(loading == tActiveBatcher->mLoading);
        return tActiveBatcher->mActive;
    }
    return !plugincbempty(loading ? CB_DBLOADOPERATION : CB_DBOPERATION);
}

void DbCallbackBatcher::Add(DbOperation & op, bool loading)
{
    if(tActiveBatcher != nullptr)
    {
        tActiveBatcher->add(op, loading);
    }
    else
    {
        PLUG_CB_DBOPERATION info;
        const DbOperation* opList = &op;

        info.operations = &opList;
        info.count = 1;
        info.batchId = 0;

        plugincbcall(loading ? CB_DBLOADOPERATION : CB_DBOPERATION, &info);
    }
}

void DbCallbackBatcher::add(DbOperation & op, bool loading)
{
    ASSERT_ALWAYS(loading == mLoading);

    // NOTE: bad, we already paid the DbOperation construction
    if(!mActive)
        return;

    if(op.text != nullptr && (op.itemType == DbItemTypeComment || op.itemType == DbItemTypeLabel)) // save c string in temporary string vector which will be cleared on flush
    {
        mStrings.emplace_back(op.text);
        op.text = mStrings.back().c_str();
    }

    mOperations.emplace_back(op);

    // Flush if we exceed the batch size threshold
    if(mOperations.size() >= 8096)
    {
        flush();
    }
}

void DbCallbackBatcher::flush()
{
    if(mFlushing)
        return;

    mFlushing = true;
    while(!mOperations.empty())
    {
        std::vector<DbOperation> operations;
        std::deque<std::string> strings;
        operations.swap(mOperations);
        strings.swap(mStrings);

        std::vector<const DbOperation*> opList(operations.size());
        for(size_t i = 0; i < operations.size(); i++)
            opList[i] = &operations[i];

        PLUG_CB_DBOPERATION info;
        info.operations = opList.data();
        info.count = opList.size();
        info.batchId = mBatchId;

        // Reentrant operations append to the now-empty member containers and
        // are delivered by the next iteration after this callback completes.
        plugincbcall(mLoading ? CB_DBLOADOPERATION : CB_DBOPERATION, &info);
    }
    mFlushing = false;
}
