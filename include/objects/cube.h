#ifndef CUBE_H
#define CUBE_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW

//#include <RigidBodies/RigidMain.h>
#include <vector>
struct FacePlane {
	glm::vec4 normal;
	std::vector<int> points = std::vector<int>(4, 0);
};

class Cube : public Entity  {

public:
	std::vector<RawModel> box_meshes;

	Cube(Shader* shader, glm::vec4 starting_pos, glm::vec3 color = glm::vec3(1, 1, 1), glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec3 rotation_axis = glm::vec3(0, 0, 0), float angle = 0) 
		: Entity(shader, starting_pos.xyz(), color, scale, rotation_axis, angle) {
		load_cube_map_texture();
		load_textures();
		load_model();
	}

	void create_rigid_body() {
		btCollisionShape* fallShape;
		fallShape = new btBoxShape(btVector3(0.5*scale.x, 0.5*scale.y, 0.5*scale.z));  
		fallShape->setMargin(0.10f);
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		fallShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
		this->rigidBody = new btRigidBody(fallRigidBodyCI);
	}

	void test_shadow_shader(Light *light, Shader *cube_map_shader, glm::mat4 projection, glm::mat4 view, glm::vec3 camera_position) {

		if (glIsTexture(cube_map) == GL_FALSE) {
			std::cerr << "Invalid cubemap texture ID" << std::endl;
		}


		cube_map_shader->use();
		cube_map_shader->setInt("skybox", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
		cube_map_shader->setVec3("cameraPos", camera_position);
		cube_map_shader->setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(5, 5, 5)));
		cube_map_shader->setMat4("projView", projection*view);
		// skybox cube
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:

	unsigned int cube_map;
	unsigned int texture;

	void load_cube_map_texture() {

		std::vector<std::string> faces
		{
			RESOURCES_PATH"textures/cubeMaps/skybox/right.jpg",
			RESOURCES_PATH"textures/cubeMaps/skybox/left.jpg",
			RESOURCES_PATH"textures/cubeMaps/skybox/top.jpg",
			RESOURCES_PATH"textures/cubeMaps/skybox/bottom.jpg",
			RESOURCES_PATH"textures/cubeMaps/skybox/front.jpg",
			RESOURCES_PATH"textures/cubeMaps/skybox/back.jpg"
		};
		loadCubemap(faces);
	}

	void load_textures() {
		texture = loader.load_texture(RESOURCES_PATH"textures/dirt.jpg");
			
	}

	void loadCubemap(std::vector<std::string> faces)
	{
		unsigned int cube_texture;
		glGenTextures(1, &cube_texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


		cube_map = cube_texture;

	}

	void form_face(float vertices[], float normals[], float tex[], int indices[], int triangle_index, int& indice_index, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, glm::vec4 normal) {
		int arr_index = triangle_index * 12;
		normals[arr_index++] = normal.x;
		normals[arr_index++] = normal.y;
		normals[arr_index++] = normal.z;
		normals[arr_index++] = normal.x;
		normals[arr_index++] = normal.y;
		normals[arr_index++] = normal.z;
		normals[arr_index++] = normal.x;
		normals[arr_index++] = normal.y;
		normals[arr_index++] = normal.z;
		normals[arr_index++] = normal.x;
		normals[arr_index++] = normal.y;
		normals[arr_index++] = normal.z;

		int tex_index = triangle_index * 8;
		tex[tex_index] = 0.0;
		tex[++tex_index] = 0.0;
		
		tex[++tex_index] = 0.0;
		tex[++tex_index] = 1.0;

		tex[++tex_index] = 1.0;
		tex[++tex_index] = 0.0;

		tex[++tex_index] = 1.0;
		tex[++tex_index] = 1.0;


		arr_index = triangle_index * 12;
		vertices[arr_index++] = p1.x;
		vertices[arr_index++] = p1.y;
		vertices[arr_index++] = p1.z;
		vertices[arr_index++] = p2.x;
		vertices[arr_index++] = p2.y;
		vertices[arr_index++] = p2.z;
		vertices[arr_index++] = p3.x;
		vertices[arr_index++] = p3.y;
		vertices[arr_index++] = p3.z;
		vertices[arr_index++] = p4.x;
		vertices[arr_index++] = p4.y;
		vertices[arr_index++] = p4.z;

		//first triangle
		int i_index = triangle_index * 4;

		//std::cout << "x1: " << vertices[i_index * 3] << "\n";
		//std::cout << "y1: " << vertices[i_index * 3 + 1] << "\n";
		indices[indice_index++] = i_index;
		indices[indice_index++] = i_index + 1;
		indices[indice_index++] = i_index + 2;
		
		//second triangle
		indices[indice_index++] = i_index + 2;
		indices[indice_index++] = i_index + 1;
		indices[indice_index++] = i_index + 3;
	}

	void load_model() {

		const int num_vertices = 3 * 4 * 6;
		const int num_textures = 2 * 4 * 6;
		const int num_indices = 6 * 6;
		float vertices[num_vertices];
		float normals[num_vertices];
		float tex[num_textures];
		
		int indices[num_indices];

		glm::vec4 p1 = glm::vec4(-0.5, -0.5, 0.5, 1);
		glm::vec4 p2 = glm::vec4(-0.5, 0.5, 0.5, 1);
		glm::vec4 p3 = glm::vec4(0.5, -0.5, 0.5, 1);
		glm::vec4 p4 = glm::vec4(0.5, 0.5, 0.5, 1);
		
		glm::vec4 p5 = glm::vec4(-0.5, -0.5, -0.5, 1);
		glm::vec4 p6 = glm::vec4(-0.5, 0.5, -0.5, 1);
		glm::vec4 p7 = glm::vec4(0.5, -0.5, -0.5, 1);
		glm::vec4 p8 = glm::vec4(0.5, 0.5, -0.5, 1);


		int indices_index = 0;
		//TODO: fix normals, they are fucked
		//front side
		form_face(vertices, normals, tex, indices, 0, indices_index, p1, p2, p3, p4, glm::vec4(0, 0, 1, 0));

		//left side  
		form_face(vertices, normals, tex, indices, 1, indices_index, p1, p2, p5, p6, glm::vec4(-1, 0, 0, 0));

		//right side
		form_face(vertices, normals, tex, indices, 2, indices_index, p3, p4, p7, p8, glm::vec4(1, 0, 0, 0));

		//back side
		// is z normal correct?
		form_face(vertices, normals, tex, indices, 3, indices_index, p5, p6, p7, p8, glm::vec4(0, 0, -1, 0));

		//top
		form_face(vertices, normals, tex, indices, 4, indices_index, p2, p6, p4, p8, glm::vec4(0, 1, 0, 0));

		//bottom
		form_face(vertices, normals, tex, indices, 5, indices_index, p1, p5, p3, p7, glm::vec4(0, -1, 0, 0));

		load_to_mesh(vertices, normals, tex, indices, sizeof(vertices)/3, sizeof(indices));
	}


};


#endif
