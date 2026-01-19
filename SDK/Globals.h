#pragma once

// Global state variables for cheat features
namespace G
{
	inline bool bSilentAngles = false;   // Silent aim - restore view angles after command
	inline bool bPSilentAngles = false;  // Perfect silent aim - choke packet then restore
	inline bool bAimbotActive = false;   // True when aimbot is actively targeting an enemy
}
