#version 450 core
out vec4 fragColor;
in vec2 normCoord;
in vec2 fragCoord;
uniform vec4 accent_color;
void main() {
	vec2 c = vec2(0.285, 0.01);
	vec2 z = normCoord;
	int max_iter = 500;
	int iter = 0;
	while (length(z) < 4.0 && iter < max_iter) {
		float xtmp = z.x * z.x - z.y * z.y;
		z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
		iter++;
	}
	float map[8][8] = {
		{0.0, 0.5, 0.125, 0.625, 0.03125, 0.53125, 0.15625, 0.65625},
		{0.75, 0.25, 0.875, 0.375, 0.78125, 0.28125, 0.90625, 0.40625},
		{0.1875, 0.6875, 0.0625, 0.5625, 0.21875, 0.71875, 0.09375, 0.59375},
		{0.9375, 0.4375, 0.8125, 0.3125, 0.96875, 0.46875, 0.84375, 0.34375},
		{0.046875, 0.546875, 0.171875, 0.671875, 0.015625, 0.515625, 0.140625, 0.640625},
		{0.796875, 0.296875, 0.921875, 0.421875, 0.765625, 0.265625, 0.890625, 0.390625},
		{0.234375, 0.734375, 0.109375, 0.609375, 0.203125, 0.703125, 0.078125, 0.578125},
		{0.984375, 0.484375, 0.859375, 0.359375, 0.953125, 0.453125, 0.828125, 0.328125}
	};
	float color = 1.0;
	if (iter != max_iter) {
		float iter = iter + 1 - log(log(length(z))) / log(2);
		color = log(float(iter)) / log(float(max_iter));
		int x = int(mod(fragCoord.x, 8.0));
		int y = int(mod(fragCoord.y, 8.0));
		color += map[y][x] * 0.05;
	}
	fragColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), accent_color, color);
}