#include "Entry.h"

void CGlobal_Entry::Load()
{
	printf("[Entry] Initializing offsets...\n");
	U::Offsets.Initialize();

	//Interfaces
	{
		printf("[Entry] Getting interfaces...\n");
		
		I::BaseClientDLL = U::Interface.Get<IBaseClientDLL*>("client.dll", "VClient017");
		printf("[Entry] BaseClientDLL: %p\n", I::BaseClientDLL);
		
		I::ClientEntityList = U::Interface.Get<IClientEntityList*>("client.dll", "VClientEntityList003");
		printf("[Entry] ClientEntityList: %p\n", I::ClientEntityList);
		
		I::GameMovement = U::Interface.Get<IGameMovement*>("client.dll", "GameMovement001");
		printf("[Entry] GameMovement: %p\n", I::GameMovement);
		
		I::Prediction = U::Interface.Get<CPrediction*>("client.dll", "VClientPrediction001");
		printf("[Entry] Prediction: %p\n", I::Prediction);

		I::EngineClient = U::Interface.Get<IVEngineClient013*>("engine.dll", "VEngineClient013");
		printf("[Entry] EngineClient: %p\n", I::EngineClient);
		
		I::EngineVGui = U::Interface.Get<IEngineVGui*>("engine.dll", "VEngineVGui001");
		printf("[Entry] EngineVGui: %p\n", I::EngineVGui);
		
		I::RenderView = U::Interface.Get<IVRenderView*>("engine.dll", "VEngineRenderView014");
		printf("[Entry] RenderView: %p\n", I::RenderView);
		
		I::GameEventManager = U::Interface.Get<IGameEventManager2*>("engine.dll", "GAMEEVENTSMANAGER002");
		printf("[Entry] GameEventManager: %p\n", I::GameEventManager);
		
		I::EngineTraceClient = U::Interface.Get<IEngineTrace*>("engine.dll", "EngineTraceClient003");
		printf("[Entry] EngineTraceClient: %p\n", I::EngineTraceClient);
		
		I::ModelInfoClient = U::Interface.Get<IVModelInfoClient*>("engine.dll", "VModelInfoClient006");
		printf("[Entry] ModelInfoClient: %p\n", I::ModelInfoClient);

		I::MatSystemSurface = U::Interface.Get<IMatSystemSurface*>("vguimatsurface.dll", "VGUI_Surface030");
		printf("[Entry] MatSystemSurface: %p\n", I::MatSystemSurface);

		I::Cvar = U::Interface.Get<ICvar*>("vstdlib.dll", "VEngineCvar004");
		printf("[Entry] Cvar: %p\n", I::Cvar);

		//Other shenanigans
		{
			printf("[Entry] Getting other interfaces...\n");
			
			if (U::Offsets.m_dwIInput != 0x0) {
				I::Input = **reinterpret_cast<IInput***>(U::Offsets.m_dwIInput + 0x2);
				printf("[Entry] Input: %p\n", I::Input);
			} else {
				printf("[Entry] Input: FAILED (m_dwIInput not found)\n");
			}

			if (U::Offsets.m_dwGlobalVars != 0x0) {
				I::GlobalVarsBase = *reinterpret_cast<CGlobalVarsBase**>(U::Offsets.m_dwGlobalVars + 0x8);
				printf("[Entry] GlobalVarsBase: %p\n", I::GlobalVarsBase);
			} else {
				printf("[Entry] GlobalVarsBase: FAILED (m_dwGlobalVars not found)\n");
			}

			if (U::Offsets.m_dwClientState != 0x0) {
				I::ClientState = *reinterpret_cast<CClientState**>(U::Offsets.m_dwClientState + 0x1);
				printf("[Entry] ClientState: %p\n", I::ClientState);
			} else {
				printf("[Entry] ClientState: FAILED (m_dwClientState not found)\n");
			}
		}
	}

	printf("[Entry] Initializing DrawManager...\n");
	H::Draw.Initialize();

	printf("[Entry] Initializing Hooks...\n");
	G::Hooks.Initialize();

	printf("[Entry] Load complete!\n");
	
	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 15, 150, 150, 255 }, "[Only Fortress] CGlobal_Entry::Load() finished!\n");
}