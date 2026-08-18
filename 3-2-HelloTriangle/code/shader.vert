// SPDX-FileCopyrightText: 2022-2026 PCJohn (Jan Pečiva, peciva@fit.vut.cz)
//
// SPDX-License-Identifier: MIT-0

#version 450

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out vec3 outColor;


// color primaries
vec2 positions[] =
	vec2[](
		vec2( 0.433,  0.250),
		vec2( 0.000, -0.500),
		vec2(-0.433,  0.250)
	);

vec3 colors[3] =
	vec3[](
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0)
	);


void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	outColor = colors[gl_VertexIndex];
}
