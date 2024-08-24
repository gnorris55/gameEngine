#ifndef SKYBOX_H
#define SKYBOX_H

class Skybox {

public:
    
	unsigned int texture;
    unsigned int VAO;
    Loader loader;
    Shader* shader;

	Skybox(Shader *shader, std::vector<std::string> faces) {
        this->shader = shader;
        load_cube_map(faces);
	}

	void draw(glm::mat4 projection, glm::mat4 view) {
        
        glDepthFunc(GL_LEQUAL);
        shader->use();
        
        shader->setMat4("projection", projection);

        glm::mat4 cubeView = glm::mat4(glm::mat3(view));
        shader->setMat4("view", cubeView);

        glBindVertexArray(VAO);

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(VAO);

        glDepthFunc(GL_LESS);

	}

	void load_cube_map(std::vector<std::string> faces) {
        
        
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        unsigned int skyboxVBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        texture = loader.load_cube_map(faces);
        shader->setInt("skybox", 0);
	}
};


#endif