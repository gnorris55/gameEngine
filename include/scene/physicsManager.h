#ifndef BULLET_PHYSICS_H
#define BULLET_PHYSICS_H

#include <objects/sphere.h>
#include <objects/cube.h>
#include <objects/entity.h>
#include <scene/physicsDebugger.h>
#include <iostream>

class PhysicsManager {


public:


	PhysicsManager(Shader *shader) {
		init(shader);

	}
	
    ~PhysicsManager() {

	}

    void add_object(Entity *object) {
        objects.push_back(object);
        object->create_rigid_body();
        dynamicsWorld->addRigidBody(object->get_rigid_body());
    }
	
    void init(Shader *shader){
        // 1. Initialize Bullet
        
        btBroadphaseInterface* broadphase = new btDbvtBroadphase();
        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
        
        
        // 2. Create the physics world
        this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        dynamicsWorld->setGravity(btVector3(0, -9.81, 0));  // Set gravity

        physics_debugger = new PhysicsDebugger(shader);
        physics_debugger->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints);
        dynamicsWorld->setDebugDrawer(physics_debugger);

    }

    void simulation_loop(double time_step, bool debugging = false) {
        // 5. Simulation loop
        dynamicsWorld->stepSimulation(time_step, 1);


        if (debugging)
            dynamicsWorld->debugDrawWorld();


        for (int i = 0; i < objects.size(); i++) {
            btTransform trans;
            btRigidBody* curr_body = objects[i]->get_rigid_body();
            curr_body->getMotionState()->getWorldTransform(trans);
            btVector3 linear = trans.getOrigin();
            btQuaternion angular = trans.getRotation();
            
            float openGLMatrix[16];
            trans.getOpenGLMatrix(openGLMatrix);

            objects[i]->update_transform(openGLMatrix);
        }
    }


    void debugging(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_view) {
        physics_debugger->draw_scene(proj, view, camera_view);
    }

private:
    std::vector<Entity *> objects;
    btDiscreteDynamicsWorld* dynamicsWorld;
    PhysicsDebugger* physics_debugger;

};



#endif