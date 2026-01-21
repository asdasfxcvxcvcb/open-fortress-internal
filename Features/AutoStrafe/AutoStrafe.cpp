#include "AutoStrafe.h"
#include "../Vars.h"
#include <cmath>

void CAutoStrafe::Run(C_TFPlayer* pLocal, CUserCmd* cmd)
{
	if (!Vars::Misc::AutoStrafe)
		return;

	if (!pLocal)
		return;

	// Only strafe when in air
	if (pLocal->m_fFlags() & FL_ONGROUND)
		return;

	// Get current movement input
	const float flForwardMove = cmd->forwardmove;
	const float flSideMove = cmd->sidemove;

	// Only strafe if there's actual movement input
	if (flForwardMove == 0.0f && flSideMove == 0.0f)
		return;

	// Get view direction vectors
	Vector vForward = {}, vRight = {};
	U::Math.AngleVectors(cmd->viewangles, &vForward, &vRight, nullptr);

	vForward.z = vRight.z = 0.0f;
	vForward.Normalize();
	vRight.Normalize();

	// Calculate wish direction (where player wants to move based on input)
	Vector vWishVel = {
		(vForward.x * flForwardMove) + (vRight.x * flSideMove),
		(vForward.y * flForwardMove) + (vRight.y * flSideMove),
		0.0f
	};
	
	Vector vWishDir = {};
	U::Math.VectorAngles(vWishVel, vWishDir);

	// Calculate current velocity direction
	Vector vCurDir = {};
	U::Math.VectorAngles(pLocal->m_vecVelocity(), vCurDir);

	// Calculate angle delta between wish and current direction
	float flDirDelta = U::Math.NormalizeAngle(vWishDir.y - vCurDir.y);
	
	// Check max delta - don't strafe if direction change is too large
	if (fabsf(flDirDelta) > Vars::Misc::AutoStrafeMaxDelta)
		return;
	
	// Apply turn scale (remapped from 0.1-1.0 to 0.9-1.0 for better control)
	const float flTurnScale = 0.9f + (Vars::Misc::AutoStrafeTurnRate * 0.1f);
	
	// Calculate rotation: ±90° (perpendicular to velocity) + delta correction
	const float flRotation = DEG2RAD((flDirDelta > 0.0f ? -90.0f : 90.0f) + (flDirDelta * flTurnScale));

	// Apply rotation matrix to movement
	const float flCosRot = cosf(flRotation);
	const float flSinRot = sinf(flRotation);

	cmd->forwardmove = (flCosRot * flForwardMove) - (flSinRot * flSideMove);
	cmd->sidemove = (flSinRot * flForwardMove) + (flCosRot * flSideMove);
}
