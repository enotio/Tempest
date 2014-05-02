struct PS_INPUT {
  float4 Position   : POSITION;
  float2 Texture    : TEXCOORD0;
  };

struct PS_OUTPUT {
  float4 Color   : COLOR0;
  };

sampler2D xtexture;

PS_OUTPUT main( in PS_INPUT vs ) {
  PS_OUTPUT fs;

  fs.Color = tex2D(xtexture, vs.Texture);

  return fs;
  }
