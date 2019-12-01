#version 410

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
out vec4 frag_color;

void main() {
	vec4 texel = texture(basic_texture, texture_coordinates);
        if(texel.a < 0.1)
            discard;
	//frag_color = vec4(texture_coordinates, 0.0, 1.0);
	frag_color = texel;
}
