#pragma once

#include "DrawManager/DrawManager.h"

class CTraceFilterHitscan : public ITraceFilter
{
public:
	C_BaseEntity* pSkip = nullptr;

	bool ShouldHitEntity(IHandleEntity* pHandleEntity, int contentsMask) override
	{
		if (!pHandleEntity)
			return false;

		C_BaseEntity* pEntity = reinterpret_cast<C_BaseEntity*>(pHandleEntity);

		if (pEntity == pSkip)
			return false;

		return true;
	}

	TraceType_t GetTraceType() const override
	{
		return TRACE_EVERYTHING;
	}
};

class CUtil_Trace
{
public:
	void TraceRay(const Vector& vStart, const Vector& vEnd, unsigned int nMask, ITraceFilter* pFilter, trace_t* pTrace)
	{
		Ray_t ray;
		ray.Init(vStart, vEnd);
		I::EngineTraceClient->TraceRay(ray, nMask, pFilter, pTrace);
	}
};

namespace U { inline CUtil_Trace Trace; }
