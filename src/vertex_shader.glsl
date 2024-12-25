#version 450 core
out vec2 normCoord;
out vec2 fragCoord;
uniform vec2 resolution;
void main() {
	vec4 positions[] = {
		vec4(-1.0, -1.0, 0.0, 1.0),
		vec4( 3.0, -1.0, 0.0, 1.0),
		vec4(-1.0,  3.0, 0.0, 1.0)
	};
	normCoord = vec2(
		positions[gl_VertexID].x * resolution.x / resolution.y,
		positions[gl_VertexID].y) * vec2(1.3, 1.3);
	fragCoord = (positions[gl_VertexID].xy + 1.0) * 0.5 * resolution;
	gl_Position = positions[gl_VertexID];
}