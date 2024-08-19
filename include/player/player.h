#ifndef PLAYER_H
#define PLAYER_H

#include <objects/entity.h>

class Player : public Entity  {

public:

	GLFWwindow* window;

	Player(Shader* shader, glm::vec4 position, GLFWwindow* window) : Entity(shader, position.xyz()) {
		this->window = window;
	}

	virtual void create_rigid_body() {
		btCollisionShape* fallShape = new btCapsuleShape(1, 2);
		//fallShape = new btBoxShape(btVector3(, 0.5 * scale.y, 0.5 * scale.z));
		btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
		fallShape->setMargin(0.10f);
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		fallShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
		this->rigidBody = new btRigidBody(fallRigidBodyCI);
		this->rigidBody->setAngularFactor(btVector3(0, 0, 0));
		this->rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	void player_movement() {
		
		// if the player cannot move than this method does not do anything
		if (this->rigidBody == nullptr)
			return;

		btVector3 movementVelocity = btVector3(0, 0, 0) + btVector3(0, this->rigidBody->getLinearVelocity().y(), 0);
		bool button_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			movementVelocity = movementVelocity + btVector3(10, 0, 0);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			movementVelocity = movementVelocity + btVector3(-10, 0, 0);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			movementVelocity = movementVelocity + btVector3(0, 0, -10);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			movementVelocity = movementVelocity + btVector3(0, 0, 10);
		}
		
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			this->rigidBody->applyCentralForce(btVector3(0, 50, 0));
		}


		
		this->rigidBody->setLinearVelocity(movementVelocity);

	}

};

#endif