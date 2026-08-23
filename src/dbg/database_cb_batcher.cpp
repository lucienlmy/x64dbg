#include "database_cb_batcher.h"
#include <atomic>
#include <utility>
#include "_plugins.h"
#include "plugin_loader.h"

thread_local DbCallbackBatcher* DbCallbackBatcher::tActiveBatcher = nullptr;

namespace
{
    struct PendingDbCallback
    {
        explicit PendingDbCallback(bool loading, uint32_t batchId)
            : loading(loading), batchId(batchId)
        {
        }

        void add(const DbOperation & operation)
        {
            auto copy = operation;
            if((copy.itemType == DbItemTypeComment || copy.itemType == DbItemTypeLabel) && copy.text != nullptr)
            {
                strings.emplace_back(copy.text);
                copy.text = strings.back().c_str();
            }
            operations.emplace_back(copy);
        }

        void deliver()
        {
            auto string = strings.begin();
            for(auto & operation : operations)
            {
                if((operation.itemType == DbItemTypeComment || operation.itemType == DbItemTypeLabel) && operation.text != nullptr)
                {
                    ASSERT_ALWAYS(string != strings.end());
                    operation.text = string++->c_str();
                }
            }

            std::vector<const DbOperation*> operationList;
            operationList.reserve(operations.size());
            for(const auto & operation : operations)
                operationList.push_back(&operation);

            PLUG_CB_DBOPERATION info;
            info.operations = operationList.data();
            info.count = operationList.size();
            info.batchId = batchId;
            plugincbcall(loading ? CB_DBLOADOPERATION : CB_DBOPERATION, &info);
        }

        std::vector<DbOperation> operations;
        std::deque<std::string> strings;
        bool loading;
        uint32_t batchId;
    };

    std::recursive_mutex dbCallbackMutex;
    thread_local size_t dbCallbackOperationDepth = 0;
    thread_local DbCallbackOperation* dbCallbackOperationOwner = nullptr;
    bool deliveringDbCallbacks = false;
    std::deque<PendingDbCallback> pendingDbCallbacks;

    void drainDbCallbacks()
    {
        std::unique_lock<std::recursive_mutex> lock(dbCallbackMutex);
        if(deliveringDbCallbacks)
            return;

        deliveringDbCallbacks = true;
        while(!pendingDbCallbacks.empty())
        {
            auto current = std::move(pendingDbCallbacks.front());
            pendingDbCallbacks.pop_front();
            lock.unlock();
            current.deliver();
            lock.lock();
        }
        deliveringDbCallbacks = false;
    }

    void deliverDbCallback(PendingDbCallback callback)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(dbCallbackMutex);
            pendingDbCallbacks.emplace_back(std::move(callback));
        }
        if(dbCallbackOperationDepth == 0)
            drainDbCallbacks();
    }
}

DbCallbackOperation::DbCallbackOperation()
{
    mPreviousOwner = dbCallbackOperationOwner;
    mOwner = dbCallbackOperationDepth++ == 0;
    if(mOwner)
    {
        dbCallbackOperationOwner = this;
        mLock = std::unique_lock<std::recursive_mutex>(dbCallbackMutex);
    }
}

DbCallbackOperation::~DbCallbackOperation()
{
    ASSERT_ALWAYS(dbCallbackOperationDepth > 0);
    dbCallbackOperationDepth--;
    if(mOwner)
    {
        ASSERT_ALWAYS(dbCallbackOperationDepth == 0);
        dbCallbackOperationOwner = mPreviousOwner;
        mLock.unlock();
        drainDbCallbacks();
    }
}

void DbCallbackOperation::DrainAndRelock()
{
    if(!mOwner)
    {
        ASSERT_ALWAYS(dbCallbackOperationOwner != nullptr);
        dbCallbackOperationOwner->DrainAndRelock();
        return;
    }

    ASSERT_ALWAYS(mLock.owns_lock());
    const auto operationDepth = dbCallbackOperationDepth;
    dbCallbackOperationDepth = 0;
    mLock.unlock();
    drainDbCallbacks();
    mLock.lock();
    dbCallbackOperationDepth = operationDepth;
}

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
        PendingDbCallback callback(loading, 0);
        callback.add(op);
        deliverDbCallback(std::move(callback));
    }
}

void DbCallbackBatcher::add(DbOperation & op, bool loading)
{
    ASSERT_ALWAYS(loading == mLoading);

    // NOTE: bad, we already paid the DbOperation construction
    if(!mActive)
        return;

    if((op.itemType == DbItemTypeComment || op.itemType == DbItemTypeLabel) && op.text != nullptr) // save c string in temporary string vector which will be cleared on flush
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

    PendingDbCallback callback(mLoading, mBatchId);
    callback.operations.reserve(mOperations.size());
    for(const auto & operation : mOperations)
        callback.add(operation);

    mOperations.clear();
    mStrings.clear();

    // A callback can mutate the database. Queue those nested notifications and
    // deliver them only after every plugin has received this older batch.
    auto activeBatcher = DbCallbackBatcher::tActiveBatcher;
    if(activeBatcher == this)
        DbCallbackBatcher::tActiveBatcher = mPrevious;

    deliverDbCallback(std::move(callback));
    mOperation.DrainAndRelock();

    if(activeBatcher == this)
        DbCallbackBatcher::tActiveBatcher = activeBatcher;
}
