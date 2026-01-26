#include "SequenceFreezing.h"
#include "../Vars.h"
#include "../../SDK/Interfaces/IVEngineClient.h"
#include "../../SDK/Interfaces/INetChannel.h"
#include "../../SDK/Entities/C_BasePlayer.h"

void CSequenceFreezing::Run(C_BasePlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::SequenceFreezing::Enabled)
		return;

	if (!pLocal)
		return;

	// Check if console is visible
	if (I::EngineClient->Con_IsVisible())
		return;

	// Check if connected to a server
	if (!I::EngineClient->IsConnected())
		return;

	// Check if alive using deadflag (like ESP does)
	if (pLocal->deadflag())
		return;

	// Get the net channel
	INetChannel* pNetChan = reinterpret_cast<INetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	// Manipulate the outgoing sequence number
	int& nOutSequenceNr = pNetChan->GetOutSequenceNr();
	nOutSequenceNr += Vars::SequenceFreezing::Amount;
}
