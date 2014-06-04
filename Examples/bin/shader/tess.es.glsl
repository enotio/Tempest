#version 410 core
layout(triangles, equal_spacing) in;

uniform mat4 mvpMatrix;
uniform sampler2D heightMap;

// from control shader
in VertexES {
  vec2 texcoord;
  vec3 normal;
  } vertes[];

// to fragment shader
out VertexFS {
  vec2 texcoord;
  vec3 normal;
  } vertfs;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2) {
  return gl_TessCoord.x*v0 + gl_TessCoord.y*v1 + gl_TessCoord.z*v2;
  }

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2) {
  return gl_TessCoord.x*v0 + gl_TessCoord.y*v1 + gl_TessCoord.z*v2;
  }

void main(void) {
  vertfs.texcoord = interpolate2D(vertes[0].texcoord, vertes[1].texcoord, vertes[2].texcoord);
  vertfs.normal   = normalize(interpolate3D(vertes[0].normal, vertes[1].normal, vertes[2].normal));

  float h = texture(heightMap, vertfs.texcoord).r;
  vec4 dpos = vec4(vertfs.normal, 0) * ( h / 4.0);
  vec4 pos  = vec4(interpolate3D( gl_in[0].gl_Position.xyz,
                                  gl_in[1].gl_Position.xyz,
                                  gl_in[2].gl_Position.xyz), 1.0) + dpos;
  gl_Position = mvpMatrix *  pos;
  }
