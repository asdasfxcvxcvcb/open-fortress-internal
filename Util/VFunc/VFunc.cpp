#include "VFunc.h"

#pragma warning(push)
#pragma warning(disable: 4311 4302 4312) // Disable pointer truncation warnings for 32-bit

void**& CUtil_VFunc::Get_VTable(void* inst, const unsigned int offset)
{
	return *reinterpret_cast<void***>(reinterpret_cast<unsigned int>(inst) + offset);
}

const void** CUtil_VFunc::Get_VTable(const void* inst, const unsigned int offset)
{
	return *reinterpret_cast<const void***>(reinterpret_cast<unsigned int>(inst) + offset);
}

#pragma warning(pop)