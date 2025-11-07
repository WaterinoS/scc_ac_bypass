#include "te-sdk.h"
#include <utility>
#include <d3d9.h>

static std::map<UINT32, int> checkAttempts;

struct MemoryCheck {
    UINT32 baseAddr;
    UINT8 expectedValue;
    const char* name;
};

static const MemoryCheck memoryChecks[] = {
    { 0x5E8606, 192, "[1] S0beit" },           // 0x06865E
    { 0x7487A8, 72,  "[2] CLEO" },             // 0xA88774
    { 0x4667DB, 192, "[3] CLEO" },             // 0xDB6746
    { 0x57B9FD, 68,  "[4] CLEO" },             // 0xFDB957
    { 0x58D552, 196, "[5] CLEO / MoonLoader" },// 0x52D558
    { 0x58FCE4, 64,  "[6] CLEO / MoonLoader" },// 0xE4FC58
    { 0x46A21B, 8,   "[7] CLEO" },             // 0x1BA246
    { 0x6FC5B0, 200, "SilentPatch" },          // 0xB0C56F
    { 0x5E85F9, 200, "SampFuncs" },            // 0xF9855E
    { 0x520191, 204, "[2] SampFuncs" },        // 0x910152
    { 0x6EFBC7, 196, "[2] S0beit" },           // 0xC7FB6E
    { 0x53C8F4, 132, "Modified VorbisFile.dll" },// 0xF4C853
    { 0x747EB4, 132, "UltraWH" },              // 0xB47E74
    { 0x522C24, 192, "Silent Aim" },           // 0x242C52
    { 0x743C60, 200, "Improved Deagle" },      // 0x603C74
    { 0x584D00, 132, "StealthRemastered" },    // 0x004D58
    { 0x522268, 132, "Sensfix.asi" },           // 0x682252
    { 0x53EA05, 0,   "Unknown Check" }
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

		int iVersion;
		byte byteMod;
		byte byteNameLen;
		char szNickName[32];
		uint32_t uiChallengeResponse;
		byte byteAuthBSLen;
		char pszAuthBullshit[64];
		char cver[32];
		uint8_t cverlen;

		(*static_cast<BitStream*>(ctx.bitStream)).Read(iVersion);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(byteMod);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(byteNameLen);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(szNickName, byteNameLen);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(uiChallengeResponse);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(byteAuthBSLen);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(pszAuthBullshit, byteAuthBSLen);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(cverlen);
		(*static_cast<BitStream*>(ctx.bitStream)).Read(cver, cverlen);

		auto size = static_cast<BitStream*>(ctx.bitStream)->GetNumberOfUnreadBits();
		unsigned char* bits = nullptr;
		if (size > 0)
		{
			bits = new unsigned char[size];
			(*static_cast<BitStream*>(ctx.bitStream)).ReadBits(bits, size);
		}

		// Modify values
        if ((!strcmp(cver, "0.3.7") && cverlen == 5) || (!strcmp(cver, "0.3.7-R1") && cverlen == 8))
        {
            memset(cver, 0, sizeof(cver));
            strcpy_s(cver, sizeof(cver), "0.3.7-R2");

			cverlen = static_cast<uint8_t>(strlen(cver));
            cver[cverlen] = '\0';
        }

		pszAuthBullshit[byteAuthBSLen] = '\0';
		szNickName[byteNameLen] = '\0';

		// Rewrite the bitstream
		BitStream* bs = static_cast<BitStream*>(ctx.bitStream);
		bs->ResetWritePointer();
		bs->Write(iVersion);
		bs->Write(byteMod);
		bs->Write(byteNameLen);
		bs->Write(szNickName, byteNameLen);
		bs->Write(uiChallengeResponse);
		bs->Write(byteAuthBSLen);
		bs->Write(pszAuthBullshit, byteAuthBSLen);
		bs->Write(cverlen);
		bs->Write(cver, cverlen);
		if (size > 0)
		{
			bs->WriteBits(bits, size);
            delete[] bits;
		}

        // MOBILE VERSION - NOT NEEDED
        //  uint16_t checksum = 0xBEEF;
        //  bs->Write(checksum);

		te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Spoofed client version to %s !", cver);
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

        switch (type)
        {
            case 0x5:
            {
                for (const auto& check : memoryChecks)
                {
                    // Offsets: baseAddr - offset => 0x0, 0x2, 0x4, 0x6, 0x8, 0xA, 0xC, 0xE, 0x10
                    if (address >= (check.baseAddr - 0x10) && address <= check.baseAddr)
                    {
                        response = check.expectedValue;
                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Matched %s: address=0x%X (base=0x%X, offset=0x%X)",
                            check.name, address, check.baseAddr, check.baseAddr - address);
                        break;
                    }
                }
                break;
            }
            case 0x45:
            {
                // Check if this is a SAMP.dll integrity check
                bool isSampCheck = false;
                for (const auto& sampAddr : sampAddresses)
                {
                    // Server randomizes offset (0x0, 0x2, 0x4, 0x6, 0x8, 0xA, 0xC, 0xE, 0x10)
                    if (address >= (sampAddr - 0x10) && address <= sampAddr)
                    {
                        isSampCheck = true;

                        // Track check attempts for this address
                        checkAttempts[address]++;

                        // Always respond with 192 (expected clean value)
                        response = 192;

                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] SAMP.dll check #%d: address=0x%X (base=0x%X) -> responding 192",
                            checkAttempts[address], address, sampAddr);
                        break;
                    }
                }

                // If not a known SAMP check
                if (!isSampCheck)
                {
                    //response = 192;
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Unknown 0x45 check: address=0x%X ..", address);
                }
                break;
            }
            /*case 0x47:
            {
				if (address == 0xCECECE && prev_response == 255) 
                {
					response = 254;
                }
                break;
			}
            case 0x48:
            {
                if (address == 0xDEDEDE && prev_response == 255)
                {
                    response = 254;
                }
                break;
            }*/
        }

        if (prev_response != response)
        {
			te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Spoofing SCC Response: type=0x%X, address=0x%X, response=%d (was %d)", type, address, response, prev_response);
			//printf("[SCC-AC-Bypass] Spoofing ClientCheckResponse: type=0x%X, address=0x%X, response=%d (was %d)\n", type, address, response, prev_response);
        }

        // Rewrite the response in the bitstream
        (*static_cast<BitStream*>(ctx.bitStream)).ResetWritePointer();
        (*static_cast<BitStream*>(ctx.bitStream)).Write(type);
        (*static_cast<BitStream*>(ctx.bitStream)).Write(address);
        (*static_cast<BitStream*>(ctx.bitStream)).Write(response);

        //te::sdk::LocalClient->SendRPC(ctx.rpcId, (static_cast<BitStream*>(ctx.bitStream)));
        //return false;
	}

    return true;
}

bool OnIncomingRPC(const te::sdk::RpcContext& ctx)
{
    if (ctx.rpcId == 103)
    {
        /*Parameters: UINT8 type, UINT32 address, UINT16 offset, UINT16 count*/

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

        switch (type)
        {
            case 0x5: // Memory integrity checks
            {
                for (const auto& check : memoryChecks)
                {
                    // Check if address is within range (baseAddr - 0x10 to baseAddr)
                    if (address >= (check.baseAddr - 0x10) && address <= check.baseAddr)
                    {
                        response = check.expectedValue;
                        shouldRespond = true;

                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming check for %s: address=0x%X (base=0x%X, offset=0x%X) -> responding %d",
                            check.name, address, check.baseAddr, check.baseAddr - address, response);
                        break;
                    }
                }

                break;
            }

            case 0x45: // SAMP.dll integrity checks
            {
                bool isSampCheck = false;
                for (const auto& sampAddr : sampAddresses)
                {
                    if (address >= (sampAddr - 0x10) && address <= sampAddr)
                    {
                        isSampCheck = true;

                        response = 192;
                        shouldRespond = true;

                        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming SAMP.dll check: address=0x%X (base=0x%X, offset=0x%X) -> responding 192",
                            address, sampAddr, sampAddr - address);
                        break;
                    }
                }

                /*
                if (!isSampCheck)
                {
                    te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming unknown 0x45 check: address=0x%X -> processing ..", address);
                }
                */
                break;
            }

            //case 0x47: // RPC integrity check
            //{
            //    if (address == 0xCECECE)
            //    {
            //        response = 254;
            //        shouldRespond = true;
            //        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming RPC check 0x47: address=0x%X -> responding 254", address);
            //    }
            //    break;
            //}

            //case 0x48: // RPC integrity check
            //{
            //    if (address == 0xDEDEDE)
            //    {
            //        response = 254;
            //        shouldRespond = true;
            //        te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Incoming RPC check 0x48: address=0x%X -> responding 254", address);
            //    }
            //    break;
            //}
        }

        // Send response back to server
        if (shouldRespond)
        {
            BitStream bsResponse;
            bsResponse.Write(type);
            bsResponse.Write(address);
            bsResponse.Write(response);

            te::sdk::LocalClient->SendRPC(103, &bsResponse);
            te::sdk::helper::logging::Log("[ SCC-AC-Bypass ] Sent response: type=0x%X, address=0x%X, response=%d", type, address, response);

            // Block original RPC from being processed
            return false;
        }
    }

    return true;
}

void Init()
{
	//printf("[TEST] Initializing RakNet hooks...\n");
    while (!te::sdk::InitRakNetHooks())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	//printf("[TEST] RakNet hooks initialized, registering callbacks...\n");
    te::sdk::RegisterRaknetCallback(HookType::OutgoingRpc, OnOutgoingRPC);
    te::sdk::RegisterRaknetCallback(HookType::IncomingRpc, OnIncomingRPC);

	//printf("[TEST] RakNet hooks initialized.\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        // CONSOLE
        {    	
            /*
                AllocConsole();
                FILE* f;
                freopen_s(&f, "CONOUT$", "w", stdout);
                freopen_s(&f, "CONOUT$", "w", stderr);
                freopen_s(&f, "CONIN$", "r", stdin);
            */
        }

        std::thread(Init).detach();
    }
    return TRUE;
}