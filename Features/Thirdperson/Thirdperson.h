#pragma once

#include "../../SDK/SDK.h"

class CThirdperson
{
public:
	void Run(CViewSetup* pSetup);
};

namespace F { inline CThirdperson Thirdperson; }
