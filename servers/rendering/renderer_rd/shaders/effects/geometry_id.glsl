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
layout(set = 0, binding = 0) uniform usampler2D source_id;

layout(push_constant, std430) uniform Params {
	vec2 internal_size;
	vec2 target_size;
} params;

vec3 id_to_color(uint id) {
	if (id == 0u) {
		return vec3(0.0);
	}
	uint h = id * 747796405u + 2891336453u;
	h = ((h >> ((h >> 28u) + 4u)) ^ h) * 277803737u;
	h = (h >> 22u) ^ h;
	return vec3(float((h) & 0xFFu), float((h >> 8) & 0xFFu), float((h >> 16) & 0xFFu)) / 255.0;
}

void main() {
	vec2 internal_coord = (gl_FragCoord.xy / params.target_size) * params.internal_size;
	ivec2 coord = clamp(ivec2(internal_coord), ivec2(0), ivec2(params.internal_size) - ivec2(1));
	uint id = texelFetch(source_id, coord, 0).r;
	frag_color = vec4(id_to_color(id), 1.0);
}
