#include "database_cb_batcher.h"

thread_local DbCallbackBatcher* DbCallbackBatcher::tActiveBatcher = nullptr;

DbCallbackBatcher::DbCallbackBatcher()
{
    mPrevious = tActiveBatcher;
    tActiveBatcher = this;

    mOperations.reserve(DB_MAX_CB_BATCH_SIZE);
    mStrings.reserve(DB_MAX_CB_BATCH_SIZE);
}

DbCallbackBatcher::~DbCallbackBatcher()
{
    this->Flush();
    tActiveBatcher = mPrevious;
}

void DbCallbackBatcher::Add(DbOperation op, const char* text)
{
    if(text != nullptr)    // save c string in temporary string vector which will be cleared on flush
    {
        mStrings.emplace_back(text);
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
    return tActiveBatcher;
}

void DbCbNotifyLabel(DbOperationType opType, duint modhash, duint address, const char* text, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Label;
    op.opType = opType;
    op.address = address;
    op.text = text;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op, text);
}

void DbCbNotifyComment(DbOperationType opType, duint modhash, duint address, const char* text, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Comment;
    op.opType = opType;
    op.address = address;
    op.text = text;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op, text);
}

void DbCbNotifyBookmark(DbOperationType opType, duint modhash, duint address, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Bookmark;
    op.opType = opType;
    op.address = address;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op);
}

void DbCbNotifyFunction(DbOperationType opType, duint modhash, duint start, duint end, duint instructioncount, duint parent, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Function;
    op.opType = opType;
    op.address = start;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.end = end;
        op.instructioncount = instructioncount;
        op.parent = parent;
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op);
}

void DbCbNotifyArgument(DbOperationType opType, duint modhash, duint start, duint end, duint instructioncount, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Argument;
    op.opType = opType;
    op.address = start;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.end = end;
        op.instructioncount = instructioncount;
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op);
}

void DbCbNotifyLoop(DbOperationType opType, duint modhash, duint start, duint end, duint instructioncount, duint parent, int depth, bool manual)
{
    auto batcher = DbCallbackBatcher::Get();

    DbOperation op = {};
    op.itemType = DbItemType::Loop;
    op.opType = opType;
    op.address = start;
    op.modhash = modhash;

    if(opType == DbOperationType::Add)    // only add callbacks have data assigned
    {
        op.end = end;
        op.instructioncount = instructioncount;
        op.parent = parent;
        op.depth = depth;
        op.manual = manual;
    }

    if(batcher == nullptr || manual)
    {
        PLUG_CB_DBOPERATION callbackInfo = { & op, 1 };
        plugincbcall(CB_DBOPERATION, &callbackInfo);

        return;
    }

    batcher->Add(op);
}
