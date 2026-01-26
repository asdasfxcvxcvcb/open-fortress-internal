#pragma once

class CUserCmd;
class C_BasePlayer;

class CSequenceFreezing
{
public:
	void Run(C_BasePlayer* pLocal, CUserCmd* pCmd);
};

namespace F { inline CSequenceFreezing SequenceFreezing; }
