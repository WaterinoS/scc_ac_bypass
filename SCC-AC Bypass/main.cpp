#include "te-sdk.h"
#include <utility>
#include <d3d9.h>

static std::map<UINT32, int> checkAttempts;

struct MemoryCheck {
    UINT8 type;
    UINT32 baseAddr;
    UINT8 expectedValue;
    const char* name;
};

static const MemoryCheck memoryChecks[] = {
    { 0x5, 0x5E8606, 192, "[1] S0beit" },           // 0x06865E
    { 0x5, 0x7487A8, 72,  "[2] CLEO" },             // 0xA88774
    { 0x5, 0x4667DB, 192, "[3] CLEO" },             // 0xDB6746
    { 0x5, 0x57B9FD, 68,  "[4] CLEO" },             // 0xFDB957
    { 0x5, 0x58D552, 196, "[5] CLEO / MoonLoader" },// 0x52D558
    { 0x5, 0x58FCE4, 64,  "[6] CLEO / MoonLoader" },// 0xE4FC58
    { 0x5, 0x46A21B, 8,   "[7] CLEO" },             // 0x1BA246
    { 0x5, 0x6FC5B0, 200, "SilentPatch" },          // 0xB0C56F
    { 0x5, 0x5E85F9, 200, "SampFuncs" },            // 0xF9855E
    { 0x5, 0x520191, 204, "[2] SampFuncs" },        // 0x910152
    { 0x5, 0x6EFBC7, 196, "[2] S0beit" },           // 0xC7FB6E
    { 0x5, 0x53C8F4, 128, "Modified VorbisFile.dll" },// 0xF4C853
    { 0x5, 0x747EB4, 132, "UltraWH" },              // 0xB47E74
    { 0x5, 0x522C24, 192, "Silent Aim" },           // 0x242C52
    { 0x5, 0x743C60, 200, "Improved Deagle" },      // 0x603C74
    { 0x5, 0x584D00, 132, "StealthRemastered" },    // 0x004D58
    { 0x5, 0x522268, 132, "Sensfix.asi" },          // 0x682252
    { 0x5, 0x53EA05, 0,   "Unknown Check" },

    /* gtat.pro:7777 */
    { 0x5, 0x468D59, 0,   "Init Check 1" },
    { 0x5, 0x4B3FC0, 8,   "Init Check 2" },
    { 0x5, 0x521500, 4,   "Init Check 3" },
    { 0x5, 0x712E40, 8,   "Init Check 4" },
    { 0x5, 0x53BEE0, 0,   "Init Check 5" },
    { 0x5, 0x73A530, 204, "Init Check 6" },
    { 0x5, 0x73B550, 140, "Init Check 7" },
    { 0x5, 0x73FB10, 68,  "Init Check 8" },
    { 0x5, 0x5231A6, 72,  "Init Check 9" },
    { 0x5, 0x52322D, 76,  "Init Check 10" },
    { 0x5, 0x5109C5, 128, "Init Check 11" },
    { 0x5, 0x5233BA, 8,   "Init Check 12" },
    { 0x5, 0x6FF420, 204, "Init Check 13" },

    { 0x5, 0x463CC5, 200, "Memory Check 1" },       // detected: 192 -> undetected: 200
    { 0x5, 0x463CC9, 72,  "Memory Check 2" },       // detected: 192 -> undetected: 72
    { 0x5, 0x4641C6, 72,  "Memory Check 3" },       // detected: 128 -> undetected: 72

    { 0x5, 0x7428E6, 4,   "Check 14" },
    { 0x5, 0x7428B0, 76,  "Check 15" },
    { 0x5, 0x740450, 64,  "Check 16" },
    { 0x5, 0x73306D, 8,   "Check 17" },
    { 0x5, 0x745BD1, 4,   "Check 18" },

    { 0x5, 0x540746, 64,  "Texture Check 1" },
    { 0x5, 0x540732, 64,  "Texture Check 2" },
    { 0x5, 0x540737, 64,  "Texture Check 3" },
    { 0x5, 0x540733, 64,  "Texture Check 4" },
    { 0x5, 0x54075C, 64,  "Texture Check 5" },
    { 0x5, 0x5407CA, 64,  "Texture Check 6" },
    { 0x5, 0x540734, 64,  "Texture Check 7" },
    { 0x5, 0x540770, 64,  "Texture Check 8" },
    { 0x5, 0x5407ED, 64,  "Texture Check 9" },

    { 0x5, 0x466722, 192, "Code Integrity 1" },     // detected: 200 -> undetected: 192
    { 0x5, 0x4667B5, 192, "Code Integrity 2" },     // detected: 200 -> undetected: 192
    { 0x5, 0x46679A, 192, "Code Integrity 3" },     // detected: 200 -> undetected: 192
    { 0x5, 0x466739, 192, "Code Integrity 4" },     // detected: 200 -> undetected: 192
    { 0x5, 0x46676D, 192, "Code Integrity 5" },     // detected: 200 -> undetected: 192
    { 0x5, 0x4667A4, 192, "Code Integrity 6" },     // detected: 200 -> undetected: 192
    { 0x5, 0x4667C8, 192, "Code Integrity 7" },     // detected: 200 -> undetected: 192
    { 0x5, 0x46679E, 192, "Code Integrity 8" },     // detected: 200 -> undetected: 192
    { 0x5, 0x46675B, 192, "Code Integrity 9" },     // detected: 200 -> undetected: 192

    { 0x5, 0x748DA3, 192, "Final Check 1" },
    { 0x5, 0x68A50E, 200, "Final Check 2" },
    { 0x5, 0x549652, 200, "Final Check 3" },

    { 0x45, 0x6C580, 0, "SAMP.dll Integrity" },
    { 0x48, 0x0, 0, "Unknown Type 0x48" },
    { 0x2, 0x0, 1, "Additional Check Type 0x2" }
};

// SAMP.dll addresses for different client versions
static const UINT32 sampAddresses[] = {
    0x41B04,  // 0.3.DL-R1
    0x42044,  // 0.3.7-R5
    0x41FF4,  // 0.3.7-R4
    0x41904   // 0.3.7-R3
};

bool OnOutgoingRPC(const te::sdk::RpcContext& ctx)
{
    if (ctx.rpcId == 25/*ClientJoin*/)
    {
        te::sdk::helper::samp::AddChatMessage("[ #TE ] SCC Anticheat Bypass by WaterSmoke Loaded !", D3DCOLOR_XRGB(0, 0xFF, 0));
	}
    else if (ctx.rpcId == 103/*ClientCheckResponse*/)
    {
        /*Parameters: UINT8 type, UINT32 address, UINT8 response*/
        UINT8 type = 0;
        UINT32 address = 0;
        UINT8 prev_response = 0;

        (*static_cast<BitStream*>(ctx.bitStream)).Read(type);
        (*static_cast<BitStream*>(ctx.bitStream)).Read(address);
        (*static_cast<BitStream*>(ctx.bitStream)).Read(prev_response);

        UINT8 response = prev_response;
        bool matched = false;

        for (const auto& check : memoryChecks)
        {
            if (check.type != type) continue;

            if (type == 0x48 || type == 0x2)
            {
                response = check.expectedValue;
                matched = true;
                te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Matched %s: type=0x%X, address=0x%X -> responding %d",
                    check.name, type, address, response);
                break;
            }
            else if (type == 0x45)
            {
                bool isSampCheck = false;
                for (const auto& sampAddr : sampAddresses)
                {
                    if (address >= (sampAddr - 0x10) && address <= sampAddr)
                    {
                        isSampCheck = true;
                        response = 192;

                        checkAttempts[address]++;
                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] SAMP.dll check #%d: address=0x%X (base=0x%X) -> responding 192",
                            checkAttempts[address], address, sampAddr);
                        break;
                    }
                }

                if (isSampCheck)
                {
                    matched = true;
                    break;
                }

                if (address == check.baseAddr)
                {
                    response = check.expectedValue;
                    matched = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Matched %s: address=0x%X -> responding %d",
                        check.name, address, response);
                    break;
                }
            }
            else if (type == 0x5)
            {
                if (address == check.baseAddr)
                {
                    response = check.expectedValue;
                    matched = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Matched check for %s: address=0x%X (exact match) -> responding %d",
                        check.name, address, response);
                    break;
                }
            }
        }

        // If type 0x5 and no exact match found, check with offset
        if (type == 0x5 && !matched)
        {
            for (const auto& check : memoryChecks)
            {
                if (check.type == 0x5 && address >= (check.baseAddr - 0x10) && address < check.baseAddr)
                {
                    response = check.expectedValue;
                    matched = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Matched %s: address=0x%X (base=0x%X, offset=0x%X) -> responding %d",
                        check.name, address, check.baseAddr, check.baseAddr - address, response);
                    break;
                }
            }
        }

        if (prev_response != response)
        {
            te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Spoofing SCC Response: type=0x%X, address=0x%X, response=%d (was %d)", type, address, response, prev_response);
        }

        // Rewrite the response in the bitstream
        (*static_cast<BitStream*>(ctx.bitStream)).ResetWritePointer();
        (*static_cast<BitStream*>(ctx.bitStream)).Write(type);
        (*static_cast<BitStream*>(ctx.bitStream)).Write(address);
        (*static_cast<BitStream*>(ctx.bitStream)).Write(response);
    }

    return true;
}

bool OnIncomingRPC(const te::sdk::RpcContext& ctx)
{
    if (ctx.rpcId == 103)
    {
        UINT8 type = 0;
        UINT32 address = 0;
        UINT16 offset = 0;
        UINT16 count = 0;

        (*static_cast<BitStream*>(ctx.bitStream)).Read(type);
        (*static_cast<BitStream*>(ctx.bitStream)).Read(address);
        (*static_cast<BitStream*>(ctx.bitStream)).Read(offset);
        (*static_cast<BitStream*>(ctx.bitStream)).Read(count);

        UINT8 response = 0;
        bool shouldRespond = false;

        for (const auto& check : memoryChecks)
        {
            if (check.type != type) continue;

            if (type == 0x48 || type == 0x2)
            {
                response = check.expectedValue;
                shouldRespond = true;
                te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming %s: type=0x%X, address=0x%X -> responding %d",
                    check.name, type, address, response);
                break;
            }
            else if (type == 0x45)
            {
                for (const auto& sampAddr : sampAddresses)
                {
                    if (address >= (sampAddr - 0x10) && address <= sampAddr)
                    {
                        response = 192;
                        shouldRespond = true;
                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming SAMP.dll check: address=0x%X (base=0x%X) -> responding 192",
                            address, sampAddr);
                        break;
                    }
                }
                if (shouldRespond) break;

                if (address == check.baseAddr)
                {
                    response = check.expectedValue;
                    shouldRespond = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming %s: address=0x%X -> responding %d",
                        check.name, address, response);
                    break;
                }
            }
            else if (type == 0x5)
            {
                if (address == check.baseAddr)
                {
                    response = check.expectedValue;
                    shouldRespond = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming check for %s: address=0x%X (exact match) -> responding %d",
                        check.name, address, response);
                    break;
                }
            }
        }

        if (type == 0x5 && !shouldRespond)
        {
            for (const auto& check : memoryChecks)
            {
                if (check.type == 0x5 && address >= (check.baseAddr - 0x10) && address < check.baseAddr)
                {
                    response = check.expectedValue;
                    shouldRespond = true;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming check for %s: address=0x%X (base=0x%X, offset=0x%X) -> responding %d",
                        check.name, address, check.baseAddr, check.baseAddr - address, response);
                    break;
                }
            }
        }

        if (shouldRespond)
        {
            BitStream bsResponse;
            bsResponse.Write(type);
            bsResponse.Write(address);
            bsResponse.Write(response);

            te::sdk::LocalClient->SendRPC(103, &bsResponse);
            te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Sent response: type=0x%X, address=0x%X, response=%d", type, address, response);

            return false;
        }
    }

    return true;
}

void Init()
{
    while (!te::sdk::InitRakNetHooks())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
	}

    te::sdk::RegisterRaknetCallback(HookType::OutgoingRpc, OnOutgoingRPC);
    te::sdk::RegisterRaknetCallback(HookType::IncomingRpc, OnIncomingRPC);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        std::thread(Init).detach();
    }
    return TRUE;
}