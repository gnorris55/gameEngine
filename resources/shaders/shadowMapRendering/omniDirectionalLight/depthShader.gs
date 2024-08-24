#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;


// the view projection of the light faces
// the projections are all perpective
uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void emitFace(mat4 m) {
	// for each triangle it renders it in each perpective
	for  (int j = 0; j < 3; ++j) {
		FragPos = gl_in[j].gl_Position;
		gl_Position = m * FragPos;
		EmitVertex();
	}

	EndPrimitive();
}

void main() {
	gl_Layer = 0;
	emitFace(shadowMatrices[0]);

	gl_Layer = 1;
	emitFace(shadowMatrices[1]);

	gl_Layer = 2;
	emitFace(shadowMatrices[2]);

	gl_Layer = 3;
	emitFace(shadowMatrices[3]);

	gl_Layer = 4;
	emitFace(shadowMatrices[4]);

	gl_Layer = 5;
	emitFace(shadowMatrices[5]);
}