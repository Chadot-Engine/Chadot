#[vertex]
#version 450

layout(location = 0) out vec2 uv;

void main() {
	vec2 base_arr[3] = vec2[](vec2(-1.0, -1.0), vec2(-1.0, 3.0), vec2(3.0, -1.0));
	vec2 uv_arr[3] = vec2[](vec2(0.0, 0.0), vec2(0.0, 2.0), vec2(2.0, 0.0));
	gl_Position = vec4(base_arr[gl_VertexIndex], 0.0, 1.0);
	uv = uv_arr[gl_VertexIndex];
}

#[fragment]
#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;
layout(set = 0, binding = 0) uniform sampler2D source_tangent;

layout(push_constant, std430) uniform Params {
	vec2 internal_size;
	vec2 target_size;
} params;

void main() {
	vec2 internal_coord = (gl_FragCoord.xy / params.target_size) * params.internal_size;
	ivec2 coord = clamp(ivec2(internal_coord), ivec2(0), ivec2(params.internal_size) - ivec2(1));
	vec4 t = texelFetch(source_tangent, coord, 0);
	if (t.w == 0.0) {
		frag_color = vec4(0.5, 0.5, 0.5, 1.0); // Error Rate
	} else {
		frag_color = vec4(t.xyz * 0.5 + 0.5, 1.0);
	}
}
