#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include <glm/gtx/rotate_vector.hpp>
class Plane : public Entity {

public:

	glm::vec4 position;
	glm::vec4 normal;
	std::vector<glm::vec4> plane_points = std::vector<glm::vec4>(4);
	std::vector<RawModel> plane_mesh;

	Plane(Shader *shader, glm::vec4 position, glm::vec4 normal, glm::vec3 color = glm::vec3(1, 1, 1)) : Entity(shader, position.xyz()) {
		this->position = position;
		this->normal = glm::normalize(normal);
		create_mesh();
	}

	void draw(glm::mat4 projection, glm::mat4 view, glm::vec3 camera_position) override {

		glm::mat4 model = glm::transpose(glm::translate(glm::mat4(1.0f), position.xyz()));
		shader->use();
		shader->setVec3("viewPos", camera_position);
		shader->setMat4("model", this->transform_matrix);
		shader->setMat4("projView", projection*view);
		shader->setVec3("objectColor", glm::vec3(0.5, 0.5, 0.5));
		shader->setBool("hasTexture", false);

		renderer.render(plane_mesh[0], GL_TRIANGLES);
	}

	float get_height_from_plane(glm::vec4 point_position) {
		glm::vec4 difference = point_position - this->position;
		return glm::dot(difference.xyz(), normal.xyz());
	}

	void create_rigid_body() override {
		btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0.0, 1, 0), 0);
		btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
		this->rigidBody = new btRigidBody(groundRigidBodyCI);
	}

private:
	Loader loader;
	Renderer renderer;

	void form_triangle(float arr[], float normals[], int triangle_index, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 normal) {
		int arr_index = triangle_index * 9;
		normals[arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;


		arr_index = triangle_index * 9;
		arr[arr_index] = p1.x;
		arr[++arr_index] = p1.y;
		arr[++arr_index] = p1.z;
		arr[++arr_index] = p2.x;
		arr[++arr_index] = p2.y;
		arr[++arr_index] = p2.z;
		arr[++arr_index] = p3.x;
		arr[++arr_index] = p3.y;
		arr[++arr_index] = p3.z;
	}

	void create_mesh() {


		glm::vec3 x1 = glm::normalize(normal.xyz()-glm::vec3(-1.0, 0.0, -1.0));
		glm::vec3 x2 = glm::normalize(glm::cross(x1, normal.xyz()));
		glm::vec3 x3 = glm::normalize(glm::cross(x2, normal.xyz()));

		std::cout << glm::to_string(normal) << "\n";
		glm::mat4 rot_mat = glm::orientation(normal.xyz(), glm::vec3(0.0, 1.0, 0.0));
		std::cout << glm::to_string(rot_mat) << "\n";
		//glm::mat4 model = glm::mat4(1.0f)*rot_mat;
		//model = (glm::translate(model, position.xyz()));
		glm::mat4 model = glm::scale(model, glm::vec3(1000, 1, 1000));
		//model = glm::transpose(model);

		glm::vec4 p1 = glm::vec4(-1.0, 0.0, -1.0, 1.0);
		glm::vec4 p2 = glm::vec4(-1.0, 0.0,  1.0, 1.0);
		glm::vec4 p3 = glm::vec4(1.0, 0.0, 1.0, 1.0);
		glm::vec4 p4 = glm::vec4(1.0, 0.0, -1.0, 1.0);

		plane_points[0] = p1;
		plane_points[1] = p2;
		plane_points[2] = p3;
		plane_points[3] = p4;

		float plane_vertices[6 * 3];
		float normals[6 * 3];
		
		form_triangle(plane_vertices, normals, 0, p1, p2, p3, normal);
		form_triangle(plane_vertices, normals, 1, p1, p3, p4, normal);
		
		plane_mesh.push_back(loader.loadToVAO(plane_vertices, normals, (18) * sizeof(float)));
	}


};



#endif