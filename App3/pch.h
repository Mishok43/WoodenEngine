#pragma once

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <pix.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <agile.h>
#include <concrt.h>
#include <string>
#include "Common\d3dx12.h"
#include <sstream>
#include <Windows.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#define DBOUT( s , s2 )            \
{                             \
   std::ostringstream os_;    \
   os_ << s << ": " << s2 << "\n";        \
   OutputDebugStringA( os_.str().c_str() );  \
}


using Microsoft::WRL::ComPtr;


