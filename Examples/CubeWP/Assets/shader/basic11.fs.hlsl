struct VS_OUTPUT {
  float4 pos      : SV_POSITION;
  float2 texCoord : TEXCOORD0;
  };

Texture2D xtexture : register(t0);
SamplerState samp {
  Filter   = MIN_MAG_MIP_LINEAR;
  AddressU = Wrap;
  AddressV = Wrap;
  };

float4 main( VS_OUTPUT input ) : SV_TARGET {
  return xtexture.Sample(samp,input.texCoord);
  }
