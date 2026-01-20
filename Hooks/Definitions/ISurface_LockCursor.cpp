#include "../Hooks.h"
#include "../../Features/Menu/Menu.h"

DEFINE_HOOK(ISurface_LockCursor, void, __fastcall, void* ecx, void* edx)
{
	// If menu is open, unlock cursor instead of locking it
	if (F::Menu.IsOpen())
	{
		I::MatSystemSurface->UnlockCursor();
		return;
	}

	// Otherwise, call original
	Func.Original<FN>()(ecx, edx);
}
