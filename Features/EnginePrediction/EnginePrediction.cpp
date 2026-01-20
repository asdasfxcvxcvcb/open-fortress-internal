#include "EnginePrediction.h"

void CFeatures_EnginePrediction::Start(C_TFPlayer* pLocal, CUserCmd* cmd)
{
	m_fOldCurrentTime = I::GlobalVarsBase->curtime;
	m_fOldFrameTime = I::GlobalVarsBase->frametime;
	m_nOldTickCount = I::GlobalVarsBase->tickcount;

	bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	bOldInPrediction = I::Prediction->m_bInPrediction;

	const int nOldTickBase = pLocal->m_nTickBase();

	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;

	I::Prediction->RunCommand(pLocal, cmd, I::MoveHelper);

	pLocal->m_nTickBase() = nOldTickBase;
}

void CFeatures_EnginePrediction::Finish(C_TFPlayer* pLocal)
{
	I::GlobalVarsBase->curtime   = m_fOldCurrentTime;
	I::GlobalVarsBase->frametime = m_fOldFrameTime;
	I::GlobalVarsBase->tickcount = m_nOldTickCount;

	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;

}

int CFeatures_EnginePrediction::GetTickbase(C_TFPlayer* pLocal, CUserCmd* cmd)
{
	static int       s_nTick    = 0;
	static CUserCmd* s_pLastCmd = nullptr;

	if (cmd)
	{
		if (!s_pLastCmd || s_pLastCmd->hasbeenpredicted)
			s_nTick = pLocal->m_nTickBase();
		else
			s_nTick++;

		s_pLastCmd = cmd;
	}

	return s_nTick;
}