cbuffer ConstantBuffer : register(b0) {
  matrix mvpMatrix;
  }

struct VS_OUTPUT {
  float4 pos      : SV_POSITION;
  float2 texCoord : TEXCOORD0;
  };

VS_OUTPUT main( float3 pos      : POSITION,
                float2 texCoord : TEXCOORD ) {
  VS_OUTPUT output;
  output.pos = mul( mvpMatrix, float4(pos, 1.0) );
  output.texCoord = float2(texCoord.x, 1.0 - texCoord.y);
  return output;
  }
