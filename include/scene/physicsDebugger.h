#ifndef PHYSICS_DEBUGGER_H
#define PHYSICS_DEBUGGER_H

#include <shaders/shader.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/string_cast.hpp>
#include <scene/loader.h>

class PhysicsDebugger : public btIDebugDraw {

    Shader* shader;
	int m_debugMode;
    unsigned int VAO = 0;
    Loader loader;

public:


    std::vector<float> points;
	PhysicsDebugger(Shader *shader) : m_debugMode(DBG_DrawWireframe), shader(shader) {}

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
        
        //std::cout << "point b: " << glm::to_string(glm::vec3(from.getX(), from.getY(), from.getZ())) <<" \n";
        //std::cout << "we are trying to draw a point\n";

        points.push_back(from.getX());
        points.push_back(from.getY());
        points.push_back(from.getZ());
        points.push_back(to.getX());
        points.push_back(to.getY());
        points.push_back(to.getZ());
        //glEnableVertexArrayAttrib(0);
        glBindVertexArray(0); // Unbind the VAO
        glUseProgram(0);


        glBegin(GL_TRIANGLES);
        //glLineWidth(5.0f);
        glColor3f(1.0f, 1.0f, 1.0f);   // Set color to red
        glVertex3f(0.0f, 0.0f, 0.0f);  // Start point of the line
        glVertex3f(1.0f, 1.0f, 1.0f);  // End point of the line
        glEnd();

        //glColor3f(color.getX(), color.getY(), color.getZ());
        //glBegin(GL_LINES);
        //glVertex3f(from.getX(), from.getY(), from.getZ());
        //glVertex3f(to.getX(), to.getY(), to.getZ());
        //glEnd();
    }

    void draw_scene(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_view) {
      
        float *vertices = points.data();
              
        this->VAO = loader.createVAO();
        loader.storeDataInAttributeList(0, points.size()*sizeof(float), 3, vertices);
       
        shader->use();
        shader->setMat4("projView", proj * view);
        shader->setMat4("model", glm::mat4(1.0f));
        shader->setVec3("objectColor", glm::vec3(0.0, 1.0, 0.0));
        shader->setBool("hasTexture", false);
      

        glBindVertexArray(this->VAO);
        //glDrawArrays(GL_LINES, 0, points.size()/3.0f);
        glDrawArrays(GL_LINES, 0, points.size()/3.0f);
        glBindVertexArray(0);
       
        glDeleteVertexArrays(1, &VAO);
        points.clear();
    }

    virtual void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
        //std::cout << "we are trying to draw a point\n";
        glColor3f(color.getX(), color.getY(), color.getZ());
        glBegin(GL_POINTS);
        glVertex3f(pointOnB.getX(), pointOnB.getY(), pointOnB.getZ());
        glEnd();

               //glColor3f(color.getX(), color.getY(), color.getZ());
        //glBegin(GL_POINTS);
        //glVertex3f(pointOnB.getX(), pointOnB.getY(), pointOnB.getZ());
        //glEnd();
        
        btVector3 to = pointOnB + normalOnB * distance;
        drawLine(pointOnB, to, color);
    }

    virtual void reportErrorWarning(const char* warningString) {
        std::cerr << warningString << std::endl;
    }

    virtual void draw3dText(const btVector3& location, const char* textString) {
        // You can implement text drawing here if needed
    }

    virtual void setDebugMode(int debugMode) {
        m_debugMode = debugMode;
    }

    virtual int getDebugMode() const {
        return m_debugMode;
    }

};

#endif