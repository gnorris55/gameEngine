#ifndef SPHERE_H
#define SPHERE_H

#include <objects/entity.h>


class Sphere : public Entity {


public:
	
	float radius;

	Sphere(Shader* shader, glm::vec4 starting_position,glm::vec3 color = glm::vec3(1, 1, 1), float radius = 1, glm::vec3 rotation_axis = glm::vec3(0, 0, 0), float angle = 0) : Entity(shader, starting_position.xyz(), color, glm::vec3(radius, radius, radius), rotation_axis, angle) {
		this->radius = radius;
		load_model();
	}

	void create_rigid_body() {
		btCollisionShape* fallShape = new btSphereShape(radius);  // Box of size 1x1x1
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		fallShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
		this->rigidBody = new btRigidBody(fallRigidBodyCI);
	}

	
private:
	
	void load_model() {
		//this is a test
		const int lat_segment = 60, lon_segment = 60;
		float vertices[lat_segment*lon_segment*3];
		float normals[lat_segment * lon_segment * 3];
		float tex[lat_segment * lon_segment * 2];
		int indices[lat_segment*lon_segment*6];
		create_mesh(vertices, normals, tex, indices, lon_segment, lat_segment);
		load_to_mesh(vertices, normals, tex, indices, sizeof(vertices)/3, sizeof(indices));
		load_textures();
	}


	void load_textures() {
		
		this->diffuse_texture = loader.load_texture(RESOURCES_PATH"textures/green.jpg");
		this->specular_texture = loader.load_texture(RESOURCES_PATH"textures/white.jpg");
	}

	void create_mesh(float vertices[], float normals[], float tex[], int indices[], int lon, int lat) {
		for (int i = 0; i < lat; i++) {
			float theta = glm::radians(180.0f) * (float)i / lat;
			float sin_theta = sin(theta);
			float cos_theta = cos(theta);

			for (int j = 0; j < lon; j++) {
				float phi = glm::radians(360.0f) * (float)j / lon;
				float sin_phi = sin(phi);
				float cos_phi = cos(phi);

				float x = sin_theta * cos_phi;
				float y = sin_theta * sin_phi;
				float z = cos_theta;
				
				float u = (float) j / lon;
				float v = (float) i/ lat;
				
				vertices[(lat * i + j)*3] = x;
				vertices[(lat * i + j)*3 + 1] = y;
				vertices[(lat * i + j)*3 + 2] = z;
				
				normals[(lat * i + j)*3] = x;
				normals[(lat * i + j)*3 + 1] = y;
				normals[(lat * i + j)*3 + 2] = z;

				tex[(lat * i + j)*2] = u;
				tex[(lat * i + j)*2 + 1] = v;

			}
		}
		//std::cout << "loading indices\n";
		for (int i = 0; i < lat; i++) {
			for (int j = 0; j < lon; j++) {
				float first = i * lat + j;
				float second = (i+1)*lat + j;
				
				indices[(i * lat + j) * 6] = first;
				indices[(i * lat + j) * 6 + 1] = second;
				indices[(i * lat + j) * 6 + 2] = first + 1;

				indices[(i * lat + j) * 6 + 3] = second;
				indices[(i * lat + j) * 6 + 4] = first + 1;
				indices[(i * lat + j) * 6 + 5] = second + 1;
			}
		}

	}

};


#endif
