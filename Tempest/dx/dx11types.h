#ifndef DX11TYPES_H
#define DX11TYPES_H

#ifndef _MSC_VER
#define __in
#define __out
#define __in_opt
#define __out_opt
#define __in_ecount_opt(X)
#define __out_ecount_opt(X)
#define __in_ecount(X)
#define __out_ecount(X)
#define __in_bcount(X)
#define __out_bcount(X)
#define __in_bcount_opt(X)
#define __out_bcount_opt(X)
#define __in_opt
#define __inout
#define __inout_opt
#define __in_ecount_part_opt(X,Y)
#define __out_ecount_part_opt(X,Y)
#endif
#include <D3D11.h>

namespace Tempest{
  struct DX11Texture{
    ID3D11Texture2D*          texture = 0;
    ID3D11RenderTargetView*   rt      = 0;
    ID3D11DepthStencilView*   depth   = 0;
    int                       rtMip   = -1;
    ID3D11ShaderResourceView* view    = 0;
    };
  }

#endif // DX11TYPES_H
