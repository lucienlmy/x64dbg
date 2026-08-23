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
    if(mOperations.empty())
        return;

    mOpList.resize(mOperations.size());
    for(size_t i = 0; i < mOperations.size(); i++)
        mOpList[i] = &mOperations[i];

    PLUG_CB_DBOPERATION info;
    info.operations = mOpList.data();
    info.count = mOpList.size();
    info.batchId = mBatchId;

    // Do not let reentrant database callbacks append to the batch being delivered.
    auto activeBatcher = DbCallbackBatcher::tActiveBatcher;
    if(activeBatcher == this)
        DbCallbackBatcher::tActiveBatcher = mPrevious;

    plugincbcall(mLoading ? CB_DBLOADOPERATION : CB_DBOPERATION, &info);

    if(activeBatcher == this)
        DbCallbackBatcher::tActiveBatcher = activeBatcher;

    mOperations.clear();
    mOpList.clear();
    mStrings.clear();
}
