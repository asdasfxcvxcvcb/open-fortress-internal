#include "../Hooks.h"
#include "../../Features/Menu/ImGuiMenu.h"

DEFINE_HOOK(ISurface_LockCursor, void, __fastcall, void* ecx, void* edx)
{
	// If menu is open, unlock cursor instead of locking it
	if (F::ImGuiMenu.IsOpen())
	{
		I::MatSystemSurface->UnlockCursor();
		return;
	}

	// Otherwise, call original
	Func.Original<FN>()(ecx, edx);
}
