#ifndef ENTITY_H
#define ENTITY_H
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

class Entity {


public:
	
	Shader* geometry_shader;
	void set_transform(glm::vec3 translate, glm::vec3 rotation_axis, glm::vec3 scale, float angle) {

		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::transpose(glm::translate(model, translate));
		model = glm::translate(model, translate);
		model = glm::scale(model, scale);

		if (angle != 0) 
			model = glm::rotate(model, angle, rotation_axis);

		this->transform_matrix = glm::transpose(model);

	}

	void update_transform(float matrix[16]) {
		glm::mat4 new_transform = glm::mat4(matrix[0], matrix[1], matrix[2], matrix[3],
			matrix[4], matrix[5], matrix[6], matrix[7],
			matrix[8], matrix[9], matrix[10], matrix[11],
			matrix[12], matrix[13], matrix[14], matrix[15]);
		new_transform = glm::scale(new_transform, scale);
		this->transform_matrix = glm::transpose(new_transform);
	}


	virtual void draw(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_position) {

		shader->use();

		shader->setInt("diffuse_texture1", 0);
		shader->setInt("specular_texture1", 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_texture);
		
		//std::cout << "tMaxtrix: " << glm::to_string(transform_matrix) << "\n";

		shader->setMat4("model", this->transform_matrix);
		shader->setMat4("projView", proj * view);
		shader->setVec3("viewPos", camera_position);
		shader->setMat4("projView", proj * view);
		shader->setVec3("objectColor", color);
		shader->setBool("hasTexture", false);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_LINES, num_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO);
	}

	void draw_geometry(){
	
		geometry_shader->use();
		geometry_shader->setMat4("model", this->transform_matrix);
		
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO);


	}

	virtual void draw_depth_buffer(Shader* depth_shader) {
		//std::cout << glm::to_string(transform_matrix) << "\n";
		//std::cout << depth_shader->ID << "\n";
		depth_shader->setMat4("model", transform_matrix);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	btRigidBody* get_rigid_body() {
		return this->rigidBody;
	}

	unsigned int get_VAO() {
		return VAO;
	}
	
	unsigned int get_indices() {
		return num_indices;
	}

	glm::mat4 get_tranform_matrix() {
		return transform_matrix;
	}

	virtual void create_rigid_body() {
		
	}

protected:


	Shader *shader;
	Loader loader;
	std::vector<Entity *> children;
	glm::mat4 transform_matrix;

	unsigned int VAO;
	unsigned int num_indices = 0;
	unsigned int specular_texture, diffuse_texture;

	glm::vec3 position;
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 color;

	btRigidBody* rigidBody = nullptr;

	Entity(Shader *shader, glm::vec3 starting_position, glm::vec3 color = glm::vec3(1, 1, 1), glm::vec3 scale = glm::vec3(1, 1, 1), glm::vec3 rotation_axis = glm::vec3(0, 0, 0), float angle = 0) 
		: position(starting_position), color(color), scale(scale), rotation(glm::quat(rotation_axis.x, rotation_axis.y, rotation_axis.z, angle)) {
		this->shader = shader;
		//this->geometry_shader = new Shader(RESOURCES_PATH"shaders/geometryShader.vs", RESOURCES_PATH"shaders/geometryShader.fs");
		set_transform(starting_position, rotation_axis, scale, angle);
	}

	
	void load_to_mesh(float vertices[], float normals[], float texture_coord[], int indices[], int num_vertices, int num_indices) {
		this->VAO = loader.createVAO();
		this->num_indices = num_indices;
		loader.bindIndicesBuffer(indices, num_indices);
		loader.storeDataInAttributeList(0, num_vertices * 3, 3, vertices);
		loader.storeDataInAttributeList(1, num_vertices * 3, 3, normals);
		loader.storeDataInTexCoordList(2, num_vertices * 2, 2, texture_coord);
		glBindVertexArray(0);
	}

};


#endif