#include "Entry.h"
#include "../Features/Menu/ImGuiMenu.h"
#include "../Features/Config/Config.h"

void CGlobal_Entry::Load()
{
	U::Offsets.Initialize();

	//Interfaces
	{
		I::BaseClientDLL = U::Interface.Get<IBaseClientDLL*>("client.dll", "VClient017");
		I::ClientEntityList = U::Interface.Get<IClientEntityList*>("client.dll", "VClientEntityList003");
		I::GameMovement = U::Interface.Get<IGameMovement*>("client.dll", "GameMovement001");
		I::Prediction = U::Interface.Get<CPrediction*>("client.dll", "VClientPrediction001");
		I::EngineClient = U::Interface.Get<IVEngineClient013*>("engine.dll", "VEngineClient013");
		I::EngineVGui = U::Interface.Get<IEngineVGui*>("engine.dll", "VEngineVGui001");
		I::RenderView = U::Interface.Get<IVRenderView*>("engine.dll", "VEngineRenderView014");
		I::GameEventManager = U::Interface.Get<IGameEventManager2*>("engine.dll", "GAMEEVENTSMANAGER002");
		I::EngineTraceClient = U::Interface.Get<IEngineTrace*>("engine.dll", "EngineTraceClient003");
		I::ModelInfoClient = U::Interface.Get<IVModelInfoClient*>("engine.dll", "VModelInfoClient006");
		I::MatSystemSurface = U::Interface.Get<IMatSystemSurface*>("vguimatsurface.dll", "VGUI_Surface030");
		I::Cvar = U::Interface.Get<ICvar*>("vstdlib.dll", "VEngineCvar004");

		//Other shenanigans
		{
			if (U::Offsets.m_dwIInput != 0x0)
				I::Input = **reinterpret_cast<IInput***>(U::Offsets.m_dwIInput + 0x2);

			if (U::Offsets.m_dwGlobalVars != 0x0)
				I::GlobalVarsBase = *reinterpret_cast<CGlobalVarsBase**>(U::Offsets.m_dwGlobalVars + 0x8);

			if (U::Offsets.m_dwClientState != 0x0)
				I::ClientState = *reinterpret_cast<CClientState**>(U::Offsets.m_dwClientState + 0x1);

			if (U::Offsets.m_dwClientModeShared != 0x0)
				I::ClientModeShared = *reinterpret_cast<CClientModeShared**>(U::Offsets.m_dwClientModeShared + 0x1);
		}
	}

	H::Draw.Initialize();
	G::Hooks.Initialize();
	g_Config.LoadConfig();
	
	if (I::Cvar)
	{
		I::Cvar->ConsoleColorPrintf({ 15, 150, 150, 255 }, "[Necromancer] Loaded and bloated, haha JK, this open fartess cheat was made by blizzman\n");
		I::Cvar->ConsoleColorPrintf({ 255, 255, 0, 255 }, "[Necromancer] Press INSERT to open menu\n");
	}
}

void CGlobal_Entry::Unload()
{
	try
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 150, 0, 255 }, "[Necromancer] Unloading...\n");
	}
	catch (...) {}

	try
	{
		F::ImGuiMenu.Shutdown();
	}
	catch (...) {}

	Sleep(100);

	try
	{
		MH_DisableHook(MH_ALL_HOOKS);
	}
	catch (...) {}
	
	Sleep(100);
	
	try
	{
		MH_Uninitialize();
	}
	catch (...) {}
	
	try
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 15, 150, 150, 255 }, "[Necromancer] Unloaded. enjoy being a retarded legit\n");
	}
	catch (...) {}
}