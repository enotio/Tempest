#version 410 core
#define id gl_InvocationID

layout(vertices = 3) out;
// from vertex shader
in VertexVS {
  vec2 texcoord;
  vec3 normal;
  } vertvs[];

// to evaluation shader
out VertexES {
  vec2 texcoord;
  vec3 normal;
  } vertes[];

void main(void) {
  const int inner = 64;
  const int outer = 64;
  if( 0 == id ){
    gl_TessLevelInner[0] = inner;
    gl_TessLevelInner[1] = inner;

    gl_TessLevelOuter[0] = outer;
    gl_TessLevelOuter[1] = outer;
    gl_TessLevelOuter[2] = outer;
    gl_TessLevelOuter[3] = outer;
    }

  vertes[id].texcoord    = vertvs[id].texcoord;
  vertes[id].normal      = vertvs[id].normal;
  gl_out[id].gl_Position = gl_in[id].gl_Position;
  }
