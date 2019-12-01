#version 410

layout (location=0) in vec3 vVertex;
layout (location = 1) in vec2 vt; // per-vertex texture co-ords

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform float Time;

out vec2 texture_coordinates;
out float shiftx;


void main(){
        vec2 pos = vt;
        shiftx = Time;
	texture_coordinates = vt;
        gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);

}

