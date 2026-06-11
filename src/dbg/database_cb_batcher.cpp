#include "database_cb_batcher.h"

thread_local DbCallbackBatcher* DbCallbackBatcher::tActiveBatcher = nullptr;

DbCallbackBatcher::DbCallbackBatcher()
{
    mPrevious = DbCallbackBatcher::Get();
    mOwner = (mPrevious == nullptr);

    if(mOwner)
    {
        DbCallbackBatcher::tActiveBatcher = this;

        mOperations.reserve(DB_MAX_CB_BATCH_SIZE);
        mStrings.reserve(DB_MAX_CB_BATCH_SIZE);
    }
}

DbCallbackBatcher::~DbCallbackBatcher()
{
    if(mOwner)
    {
        this->Flush();
        DbCallbackBatcher::tActiveBatcher = mPrevious;
    }
}

void DbCallbackBatcher::Add(DbOperation & op)
{
    auto batcher = DbCallbackBatcher::Get();

    if(!mOwner && batcher != nullptr)
    {
        return batcher->Add(op);
    }

    if(op.text != nullptr && (op.itemType == DbItemType::Comment || op.itemType == DbItemType::Label))  // save c string in temporary string vector which will be cleared on flush
    {
        mStrings.emplace_back(op.text);
        op.text = mStrings.back().c_str();
    }

    mOperations.emplace_back(op);

    if(mOperations.size() >= DB_MAX_CB_BATCH_SIZE)
        this->Flush();
}

void DbCallbackBatcher::Flush()
{
    if(mOperations.empty())
        return;

    PLUG_CB_DBOPERATION info;
    info.operations = mOperations.data();
    info.count = mOperations.size();

    plugincbcall(CB_DBOPERATION, &info);

    mOperations.clear();
    mStrings.clear();
}

DbCallbackBatcher* DbCallbackBatcher::Get()
{
    return DbCallbackBatcher::tActiveBatcher;
}

void DbCbNotify(DbOperation & op)
{
    auto batcher = DbCallbackBatcher::Get();

    if(batcher == nullptr)
    {
        PLUG_CB_DBOPERATION info;
        info.operations = & op;
        info.count = 1;

        plugincbcall(CB_DBOPERATION, &info);

        return;
    }

    batcher->Add(op);
}