#pragma comment(lib, "rpcrt4.lib")
#include <rpc.h>
#include <Windows.h>
#include <iostream>
#include "Utils.h"

std::string Utils::generateUUID()
{
    UUID uuid;
    UuidCreate(&uuid);
    char* str;
    UuidToStringA(&uuid, (RPC_CSTR*)&str);
    return str;
    //RpcStringFreeA((RPC_CSTR*)&str);
}