#version 410

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
out vec4 frag_color;
int shiftx;

void main() {
        vec4 texel = texture(basic_texture, texture_coordinates.xy + vec2(0.0, 0.01 * sin(texture_coordinates.x*shiftx/2000)));
        if(texel.a < 0.2)
            discard;
	//frag_color = vec4(texture_coordinates, 0.0, 1.0);
	frag_color = texel;
}
