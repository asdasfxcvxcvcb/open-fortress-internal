#include "../Hooks.h"
#include "../../Features/Menu/Menu.h"
#include <d3d9.h>

typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

namespace Hooks
{
	namespace DirectX9_Reset
	{
		namespace
		{
			Reset_t Original = nullptr;
			
			HRESULT __stdcall Detour(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
			{
				F::Menu.OnDeviceLost();
				HRESULT result = Original(pDevice, pPresentationParameters);
				F::Menu.OnDeviceReset();
				return result;
			}
		}

		void Initialize()
		{
			// We'll hook this from the vftable later
		}
	}
}
