#include "../Hooks.h"
#include "../../Features/Menu/Menu.h"
#include <d3d9.h>

typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);

namespace Hooks
{
	namespace DirectX9_EndScene
	{
		namespace
		{
			EndScene_t Original = nullptr;
			
			HRESULT __stdcall Detour(IDirect3DDevice9* pDevice)
			{
				static bool bInit = false;
				if (!bInit)
				{
					F::Menu.Initialize(pDevice);
					bInit = true;
				}

				F::Menu.Render();

				return Original(pDevice);
			}
		}

		void Initialize()
		{
			// We'll hook this from the vftable later
		}
	}
}
