#pragma once

#include <deque>
#include <mutex>
#include "_plugins.h"

class DbCallbackOperation
{
public:
    DbCallbackOperation();
    ~DbCallbackOperation();

    void DrainAndRelock();

    DbCallbackOperation(const DbCallbackOperation &) = delete;
    DbCallbackOperation & operator=(const DbCallbackOperation &) = delete;

private:
    std::unique_lock<std::recursive_mutex> mLock;
    DbCallbackOperation* mPreviousOwner = nullptr;
    bool mOwner = false;
};

class DbCallbackBatcher
{
public:
    explicit DbCallbackBatcher(bool loading = false);
    ~DbCallbackBatcher();

    static bool IsActive(bool loading = false);
    static void Add(DbOperation & op, bool loading = false);

private:
    static thread_local DbCallbackBatcher* tActiveBatcher; // per thread batch object

    DbCallbackOperation mOperation;

    void add(DbOperation & op, bool loading);
    void flush();

    std::vector<DbOperation> mOperations;
    std::deque<std::string> mStrings;
    DbCallbackBatcher* mPrevious = nullptr;
    uint32_t mBatchId = 0;
    bool mLoading = false;
    bool mActive = false;
    bool mOwner = false; // if a batcher is defined in a thread where there's already a batcher on the stack, this is set to false
};
