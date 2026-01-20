#pragma once

#include "../Math/Math.h"

class CUtil_Pattern
{
public:
	uintptr_t Find(const char* const szModule, const char* const szPattern);

private:
	uintptr_t FindPattern(const uintptr_t dwAddress, const uintptr_t dwLen, const char* const szPattern);
};

namespace U { inline CUtil_Pattern Pattern; }