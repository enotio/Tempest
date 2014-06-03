#version 410 core
layout(triangles, equal_spacing) in;

uniform mat4 modelViewProjectionMatrix;

uniform sampler2D heightmap;

// from control shader
in VertexES
{
  vec3 position;
  vec2 texcoord;
  vec3 normal;
}
vertes[];

// to fragment shader
out VertexFS
{
  vec3 position;
  vec2 texcoord;
  vec3 normal;
} vertfs;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
  return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
  return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main(void)
{
  vertfs.position = interpolate3D(vertes[0].position, vertes[1].position, vertes[2].position);
  vertfs.texcoord = interpolate2D(vertes[0].texcoord, vertes[1].texcoord, vertes[2].texcoord);
  vertfs.normal = normalize(interpolate3D(vertes[0].normal, vertes[1].normal, vertes[2].normal));

  gl_Position = modelViewProjectionMatrix * (
                  vec4(interpolate3D(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz), 1.0) -
          vec4(vertfs.normal, 1) * (texture(heightmap, vertfs.texcoord) / 4.0)
          );
}
