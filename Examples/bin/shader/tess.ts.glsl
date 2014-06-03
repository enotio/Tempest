#version 410

layout ( vertices = 3 ) out;

#define level 4;

void main (void) {
  // copy current vertex to output
  gl_out [gl_InvocationID].gl_Position = gl_in [gl_InvocationID].gl_Position;

  if ( gl_InvocationID == 0 ) {
    gl_TessLevelInner[0] = level;

    gl_TessLevelOuter[0] = level;
    gl_TessLevelOuter[1] = level;
    gl_TessLevelOuter[2] = level;
    }
  }
