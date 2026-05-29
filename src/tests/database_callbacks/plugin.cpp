#define WIN32_NO_STATUS
#include <Windows.h>

#include <array>
#include <atomic>
#include <cstring>
#include <string>

#include "_plugins.h"
#include "bridgemain.h"
#include "database.h"

struct TestOperation
{
    DbOperation operation;
    std::string text; // keep text alive
    bool bulk;
};

namespace
{
    int gPluginHandle = 0;
    std::vector<TestOperation> operations;

    void resetState()
    {
        operations.clear();
    }

    void cbPlugin(CBTYPE cbType, void* callbackInfo)
    {
        if(cbType == CB_INITDEBUG)
        {
            resetState();
            return;
        }

        if(cbType == CB_DBOPERATION)
        {
            PLUG_CB_DBOPERATION * info = (PLUG_CB_DBOPERATION*) callbackInfo;

            for(size_t i = 0; i < info->count; i ++)
            {
                DbOperation & operation = info->operations[i];
                std::string saved = "";

                if(operation.text && (operation.itemType == DbItemType::Comment || operation.itemType == DbItemType::Label))
                {
                    saved = std::string(operation.text);
                }

                bool is_bulk = info->count > 1;
                operations.push_back({operation, saved, is_bulk});
            }

            return;
        }
    }

    duint evalExpr(const char* expr)
    {
        return expr ? DbgValFromString(expr) : 0;
    }

    bool cbReset(int, char**)
    {
        resetState();
        return _plugin_testassert(true, "state reset");
    }

    static bool getOpType(char s, DbOperationType & type)
    {
        if(s == 'a')
            type = DbOperationType::Add;
        else if(s == 'r')
            type = DbOperationType::Remove;
        else
            return false;

        return true;
    }

    static bool getItemType(char s, DbItemType & type)
    {
        if(s == 'f')
            type = DbItemType::Function;
        else if(s == 'l')
            type = DbItemType::Label;
        else if(s == 'c')
            type = DbItemType::Comment;
        else if(s == 'b')
            type = DbItemType::Bookmark;
        else if(s == 'p')
            type = DbItemType::Loop;
        else if(s == 'g')
            type = DbItemType::Argument;
        else
            return false;

        return true;
    }

    bool cbAssertLastOperation(int argc, char** argv)
    {
        if(argc < 6)
            return false;

        if(!_plugin_testassert(operations.size() > 0, "no operations in memory"))
            return false;

        TestOperation & last = operations.back();

        const duint modhash = evalExpr(argv[1]);
        if(!_plugin_testassert(modhash == last.operation.modhash, "modhash passed to callback doesn't match (%llu != %llu)", last.operation.modhash, modhash))
            return false;

        const duint target = evalExpr(argv[2]);
        if(!_plugin_testassert(target != 0, "failed to resolve target expression '%s'", argv[2]))
            return false;

        DbItemType itemType;
        if(!_plugin_testassert(strlen(argv[3]) && getItemType(argv[3][0], itemType), "failed to resolve item type '%s'", argv[3]))
            return false;

        if(!_plugin_testassert(last.operation.itemType == itemType, "item type passed to callback doesn't match (%d != %d)", last.operation.itemType, itemType))
            return false;

        DbOperationType opType;
        if(!_plugin_testassert(strlen(argv[4]) && getOpType(argv[4][0], opType), "failed to resolve op type '%s'", argv[4]))
            return false;

        if(!_plugin_testassert(last.operation.opType == opType, "operation type passed to callback doesn't match (%d != %d)", last.operation.opType, opType))
            return false;

        bool bulk = (bool) evalExpr(argv[5]);

        const char* text = NULL;
        if((itemType == DbItemType::Label || itemType == DbItemType::Comment) && argc >= 7)
        {
            text = argv[6];
            if(!_plugin_testassert(last.text == text, "text passed to callback doesn't match (%s != %s)", last.text.c_str(), text))
                return false;
        }

        if((itemType == DbItemType::Function || itemType == DbItemType::Argument || itemType == DbItemType::Loop) && argc >= 7)
        {
            const duint end = evalExpr(argv[6]);
            if(!_plugin_testassert(last.operation.end == end, "end passed to callback doesn't match (%llu != %llu)", last.operation.end, end))
                return false;
        }

        if(itemType == DbItemType::Loop && argc >= 8)
        {
            const int depth = (int) evalExpr(argv[7]);
            if(!_plugin_testassert(last.operation.depth == depth, "depth passed to callback doesn't match (%d != %d)", last.operation.depth, depth))
                return false;
        }

        if(!_plugin_testassert(last.operation.address == target, "address passed to callback doesn't match (%llu != %llu)", last.operation.address, target))
            return false;

        if(!_plugin_testassert(last.bulk == bulk, "bulk passed to callback doesn't match (%d != %d)", last.bulk, bulk))
            return false;
        return true;
    }
}

extern "C" __declspec(dllexport) bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = 1;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, sizeof(initStruct->pluginName), X64DBG_TEST_NAME, _TRUNCATE);
    gPluginHandle = initStruct->pluginHandle;
    _plugin_registercallback(gPluginHandle, CB_DBOPERATION, cbPlugin);
    _plugin_registercallback(gPluginHandle, CB_INITDEBUG, cbPlugin);
    _plugin_registercommand(gPluginHandle, "dbreset", cbReset, false);
    _plugin_registercommand(gPluginHandle, "assertlastop", cbAssertLastOperation, false);
    return true;
}

extern "C" __declspec(dllexport) void plugstop()
{
}

extern "C" __declspec(dllexport) void plugsetup(PLUG_SETUPSTRUCT*)
{
}
