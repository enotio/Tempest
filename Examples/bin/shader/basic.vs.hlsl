struct VS_INPUT {
  float3 Position   : POSITION;
  float2 Texture    : TEXCOORD0;
  };

struct VS_OUTPUT {
  float4 Position   : POSITION;
  float2 Texture    : TEXCOORD0;
  };

float4x4 mvpMatrix;

VS_OUTPUT main( in VS_INPUT In ) {
  VS_OUTPUT vs;

  vs.Position = mul( float4(In.Position, 1.0),  mvpMatrix );
  vs.Texture  = float2( In.Texture.x, 1.0-In.Texture.y );

  return vs;
  }
