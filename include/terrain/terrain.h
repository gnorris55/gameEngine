#ifndef TERRAIN_H
#define TERRAIN_H

#include <terrain/renderTileTree.h>
#include <terrain/renderTile.h>
#include <shaders/shader_c.h>
#include <algorithm>

class Terrain : public Entity {

public:


	Shader* shader;
	Shader* grass_shader;
	Frustum* frustum;
	ComputeShader* clipping_shader;
	Camera* camera;
	Renderer renderer;
	glm::vec4 position;
	glm::mat4 model;
	std::vector<RawModel> terrain_mesh;
	std::vector<std::vector<RenderTile*>> render_tiles;
	std::vector<float> height_map;
	bool has_grass;

	unsigned int texture;
	unsigned int height_map_tex;


	RenderTileTree *render_tile_tree;

	Terrain(Shader* shader, Camera *camera, Frustum *frustum, glm::vec4 position, const char* terrain_height_name, bool has_grass = true) : Entity(shader, position.xyz()) {
		
		this->shader = shader;
		this->grass_shader = grass_shader;
		this->frustum = frustum;
		this->frustum = frustum;
		this->camera = camera;
		this->position = position;
		this->has_grass = has_grass;
		this->model = glm::transpose(glm::translate(glm::mat4(1.0f), position.xyz()));
		
		load_textures(terrain_height_name);
		createTerrainMesh();

		Shader* grass_shader = new Shader(RESOURCES_PATH"shaders/grass/grassShader.vs", RESOURCES_PATH"shaders/grass/grassShader.fs", RESOURCES_PATH"shaders/grass/grassShader.gs");
		ComputeShader* clipping_shader = new ComputeShader(RESOURCES_PATH"shaders/grass/clippingComputeShader.cs");
		if (has_grass) render_tile_tree = new RenderTileTree(grass_shader, clipping_shader, model, 128, 64, position, height_map);

	}

	~Terrain() {
		for (int i = 0; i < render_tiles.size(); i++) {
			for (RenderTile* renderTile : render_tiles[i]) {
				delete renderTile;
			}
		}
		delete render_tile_tree;
	}

	void create_rigid_body() {

		float *heights = height_map.data();


		float min_height = -10.0;
		float max_height = 10.0;

		btScalar y = 1;

		btHeightfieldTerrainShape* terrainShape = new btHeightfieldTerrainShape(65, 65, heights, 1, min_height, max_height, 1, PHY_FLOAT, false);

		btVector3 terrainScaling(2.0f, 1.0f, 2.0f); // Scale the terrain (e.g., make it wider)
		terrainShape->setLocalScaling(terrainScaling);
		//terrainShape->setMargin(0.05f); // Set collision margin

		std::cout << "pos: " << glm::to_string(position) << "\n";
		// 4. Create a rigid body for the terrain
		btTransform terrainTransform;
		terrainTransform.setIdentity();
		terrainTransform.setOrigin(btVector3(position.x+64, position.y, position.z+64));

		btDefaultMotionState* terrainMotionState = new btDefaultMotionState(terrainTransform);
		btRigidBody::btRigidBodyConstructionInfo terrainRbInfo(
			0.0f,  // Mass (0 for static objects)
			terrainMotionState,
			terrainShape,
			btVector3(0, 0, 0)  // Inertia (not needed for static objects)
		);

		this->rigidBody = new btRigidBody(terrainRbInfo);
		

	}

	
	void draw(glm::mat4 proj, glm::mat4 view, glm::vec3 view_pos) {

		float time = static_cast<float>(glfwGetTime());

		shader->use();
		shader->setInt("texture_diffuse1", 0);
		shader->setInt("texture_specular1", 1);
		shader->setInt("texture_normal1", 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		shader->setFloat("texScalar", 40.0f);
		RawModel temp_mesh = terrain_mesh[0];
		shader->setVec3("viewPos", view_pos);
		shader->setMat4("projView", proj*view);
		shader->setBool("hasTexture", true);
		shader->setMat4("model", model);
		shader->setVec3("objectColor", glm::vec3(0.2f, 0.2f, 0.2f));

		renderer.render(temp_mesh, GL_TRIANGLES);
		
		shader->setFloat("texScalar", 1.0f);

		if (has_grass) render_tile_tree->draw(time, proj, view, view_pos, camera->get_view_dir(), frustum);
	}

	void draw_depth_buffer(Shader* depth_shader) {
		depth_shader->setMat4("model", transform_matrix);
		renderer.render(terrain_mesh[0], GL_TRIANGLES);
	}



	
private:
	const int TERRAIN_SIZE = 128;
	const int SQUARE_SIZE = 18;
	const int NUM_DIVISION = 64;
	glm::vec3 points[65][65];
	glm::vec2 globalTexCoord[65][65];
	Loader loader;

	void createTerrainMesh() {
	
		//const int num_vertices = 73728;
		const int num_vertices = 65*65*3*6;
		const int num_tex = 65*65*2*6;
		float vertices[num_vertices];
		float normals[num_vertices];
		float tex_coord[num_tex];

		setPoints();
		mapVertices(vertices, normals, tex_coord);

		RawModel temp_rawModel = loader.loadToVAOTexture(vertices, normals, tex_coord, sizeof(vertices) - (((NUM_DIVISION+1) * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE), sizeof(tex_coord) - (((NUM_DIVISION+1) * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE));
		terrain_mesh.push_back(temp_rawModel);

	}

	void load_textures(const char* terrain_heights_name) {
		texture = loader.load_texture(RESOURCES_PATH"textures/ground.jpg");
		height_map_tex = loader.load_texture(RESOURCES_PATH"textures/height_map1.jpg");
	}
		
	glm::vec3 get_vec3_from_texture_array(int i, int j, float tex_array[]) {
		float winds[65 * 65*3];
		glm::vec3 rtn_vec;
		rtn_vec.x = tex_array[(i * 64 + j) * 3];
		rtn_vec.y = tex_array[(i * 64 + j) * 3 + 1];
		rtn_vec.z = tex_array[(i * 64 + j) * 3 + 2];

		return rtn_vec;
	}

	
	void mapVertices(float vertices[], float normals[], float tex_coord[]) {
		int vertex = 0;
		int normalsVertex = 0;
		int texVertex = 0;
		for (int i = 0; i <= NUM_DIVISION; i++) {
			for (int j = 0; j <= NUM_DIVISION; j++) {
				if ((j != NUM_DIVISION && i != NUM_DIVISION) || (j == NUM_DIVISION && i == NUM_DIVISION)) {
					glm::vec2 inputTex[] = {globalTexCoord[i][j] , globalTexCoord[i][j + 1], globalTexCoord[i + 1][j], globalTexCoord[i + 1][j + 1]};
					glm::vec3 inputPoints[] = { points[i][j], points[i][j + 1], points[i + 1][j], points[i + 1][j + 1] };
					mapSquare(vertices, normals, tex_coord, &vertex, &normalsVertex, &texVertex, inputPoints, inputTex, glm::vec3(1.0f, 1.0f, 1.0f));

				}

			}

		}
	}

	void setPoints() {

		float height_map[65 * 65];
		glBindTexture(GL_TEXTURE_2D, height_map_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, height_map);
		

		float posX, posY, posZ;
		for (int i = 0; i <= NUM_DIVISION; i++) {
			for (int j = 0; j <= NUM_DIVISION; j++) {
				height_map[i * 65 + j] *= 3;
				//posX = (float)j / ((float)NUM_DIVISION - 1) * TERRAIN_SIZE;
				posX = j * 2;
				posY = height_map[i * 65 + j];
				this->height_map.push_back(height_map[i * 65 + j]);
				posZ = i * 2;
				points[j][i] = glm::vec3(posX, posY, posZ);

				glm::vec2 tempTexCoord;
				tempTexCoord.x = (float)j / ((float)NUM_DIVISION);
				tempTexCoord.y = (float)i / ((float)NUM_DIVISION);
				globalTexCoord[j][i] = tempTexCoord;
			}

		}

	}

	void mapSquare(float vertices[], float normals[], float tex[], int* vertex, int* normalsVertex, int* texVertex, glm::vec3 points[], glm::vec2 textureCoord[], glm::vec3 normalDir) {
		//first triangle
		vertexToElement(vertices, vertex, points[0]);
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[2]);

		glm::vec3 normalVec = calculateNormals(points[0], points[1], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);

		texToElement(tex, texVertex, textureCoord[0]);
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[2]);
		
		//second triangle
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[3]);
		vertexToElement(vertices, vertex, points[2]);

		normalVec = calculateNormals(points[1], points[3], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[3]);
		texToElement(tex, texVertex, textureCoord[2]);
	}

	void vertexToElement(float vertices[], int* vertex, glm::vec3 vector) {
		vertices[*vertex] = vector.x;
		vertices[*vertex + 1] = vector.y;
		vertices[*vertex + 2] = vector.z;
		*vertex += 3;
	}
	
	void texToElement(float tex[], int* vertex, glm::vec2 vector) {
		tex[*vertex] = vector.x;
		tex[*vertex + 1] = vector.y;
		*vertex += 2;
	}

	glm::vec3 calculateNormals(glm::vec3 vectorA, glm::vec3 vectorB, glm::vec3 vectorC) {
		return glm::cross(vectorB - vectorA, vectorC - vectorA);
	}

};

#endif