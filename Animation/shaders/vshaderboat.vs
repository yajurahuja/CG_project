#version 410

layout (location=0) in vec3 vVertex;
layout (location = 1) in vec2 vt; // per-vertex texture co-ords

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;
uniform mat4 tboat;
uniform mat4 rboat;

out vec2 texture_coordinates;


void main(){
        texture_coordinates = vt;

    gl_Position = vProjection * vView * vModel * tboat * rboat * vec4(vVertex, 1.0);

}

