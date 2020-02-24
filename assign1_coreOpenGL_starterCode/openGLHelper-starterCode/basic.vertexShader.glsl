#version 150

in vec3 position;
in vec4 color;

in vec3 position_lf;
in vec3 position_rt;
in vec3 position_tp;
in vec3 position_bt;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform int mode;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  if(mode==1 || mode==2 || mode==3){
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
      col = color;
  }
  else if(mode == 4){
      gl_Position = projectionMatrix * modelViewMatrix * vec4((position_bt+position_tp+position_lf+position_rt)/4,1.0f);
	  col = color;
  }
}