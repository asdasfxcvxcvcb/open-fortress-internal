#pragma once

#include "../../SDK/SDK.h"

class CAutoStrafe
{
public:
	void Run(C_TFPlayer* pLocal, CUserCmd* cmd);
};

namespace F { inline CAutoStrafe AutoStrafe; }
