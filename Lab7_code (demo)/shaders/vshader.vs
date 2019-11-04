#version 330 core

layout (location=0) in vec3 vVertex;
//layout (location=1) in vec3 vNormal;
//layout (location=2) in vec3 vColor;
uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;


void main(void) {
    //vec3 position_vertex = vec3(vModel * vec4(vVertex, 1.0));
    gl_Position = vProjection * vView * vModel * vec4(vVertex, 1.0);


}

