#ifndef LIGHT_H
#define LIGHT_H

class Light : public Entity  {

public:

	glm::vec3 position;
	glm::vec3 color;

	float linear_constant, quadratic_constant;
	float constant = 1;
	unsigned int VAO;
    unsigned int num_indices;
    unsigned int depthCubeMap;

	
	Light(Shader *shader, glm::vec3 position, glm::vec3 color, float linear, float quadratic, unsigned int VAO = 0, unsigned int num_indices = 0) : Entity(shader, position) {

		this->position = position;
		this->color = color;
		this->linear_constant = linear;
		this->quadratic_constant = quadratic;
        this->depth_shader = depth_shader;
        this->VAO = VAO;
        this->num_indices = num_indices;
        //create_cube();
	}

	void draw(glm::mat4 proj, glm::mat4 view, glm::vec3 camera_position) override {
        
        shader->use();
        shader->setMat4("projView", proj * view);
        shader->setMat4("model", glm::transpose(glm::translate(glm::mat4(1.0f), position)));
        shader->setVec3("objectColor", color);
        shader->setBool("hasTexture", false);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


private:

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;

    Shader* depth_shader;

    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);


	};

class LightManager {

public:

    Shader *shader;
    Shader *depth_shader;
    std::vector<Light*> lights;

    LightManager(Shader* shader) : shader(shader) {
        load_model();
    }

    ~LightManager() {
        //for (Light *light : lights)
            //delete light;
    }

    void add_light(glm::vec3 position, glm::vec3 color, float linear, float quadratic) {
        Light* temp_light = new Light(shader, position, color, linear, quadratic, VAO, num_indices);
        lights.push_back(temp_light);
    }

private:

    unsigned int VAO;
    unsigned int num_indices;
    Loader loader;

    void load_model() {
        //this is a test
        const int lat_segment = 4, lon_segment = 4;
        float vertices[lat_segment * lon_segment * 3];
        float normals[lat_segment * lon_segment * 3];
        float tex[lat_segment * lon_segment * 2];
        int indices[lat_segment * lon_segment * 6];
        create_mesh(vertices, normals, tex, indices, lon_segment, lat_segment);
        load_to_mesh(vertices, normals, tex, indices, sizeof(vertices) / 3.0f, sizeof(indices));
        //load_textures();
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

    void create_mesh(float vertices[], float normals[], float tex[], int indices[], int lon, int lat) {
        for (int i = 0; i < lat; i++) {
            float theta = glm::radians(360.0f) * (float)i / lat;
            float sin_theta = sin(theta);
            float cos_theta = cos(theta);

            for (int j = 0; j < lon; j++) {
                float phi = glm::radians(360.0f) * (float)j / lon;
                float sin_phi = sin(phi);
                float cos_phi = cos(phi);

                float x = sin_theta * cos_phi;
                float y = sin_theta * sin_phi;
                float z = cos_theta;

                float u = (float)j / lon;
                float v = (float)i / lat;

                vertices[(lat * i + j) * 3] = x;
                vertices[(lat * i + j) * 3 + 1] = y;
                vertices[(lat * i + j) * 3 + 2] = z;

                normals[(lat * i + j) * 3] = x;
                normals[(lat * i + j) * 3 + 1] = y;
                normals[(lat * i + j) * 3 + 2] = z;

                tex[(lat * i + j) * 2] = u;
                tex[(lat * i + j) * 2 + 1] = v;

            }
        }

        for (int i = 0; i < lat; i++) {
            for (int j = 0; j < lon; j++) {
                float first = i * lat + j;
                float second = (i + 1) * lat + j;

                indices[(i * lat + j) * 6] = first;
                indices[(i * lat + j) * 6 + 1] = second;
                indices[(i * lat + j) * 6 + 2] = first + 1;

                indices[(i * lat + j) * 6 + 3] = second;
                indices[(i * lat + j) * 6 + 4] = first + 1;
                indices[(i * lat + j) * 6 + 5] = second + 1;
            }
        }
    }


private:


};

class Sun : public Light {


public:

    unsigned int depth_texture;
    Shader *depth_shader;
    Shader* entity_shader;
    Shader* FBO_debugger_shader;
    glm::mat4 lightSpaceMatrix;


    Sun(Shader* shader, glm::vec3 position, glm::vec3 color, float linear, float quadratic) : Light(shader, position, color, linear, quadratic) {
        
        create_shaders();
        create_FBO();
        create_render_quad();
    }

    ~Sun() {
        glDeleteShader(FBO_debugger_shader->ID);
        glDeleteShader(entity_shader->ID);
        glDeleteShader(depth_shader->ID);
    }

   
    void set_depth_map(std::vector<Entity *> entities, bool debug = false) {
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        lightView = glm::lookAt(glm::vec3(position.x, 20, position.z), glm::vec3(position.x, 0.0, position.z+7), glm::vec3(0.0, 1.0, 0.0));
        //lightView = glm::lookAt(position, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        this->lightSpaceMatrix = lightProjection * lightView;
        
        // render scene from light's point of view
        glCullFace(GL_FRONT);
        depth_shader->use();
        depth_shader->setMat4("lightSpaceMatrix", this->lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        for (Entity* entity : entities) {

            entity->draw_depth_buffer(depth_shader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
        if (debug)
            draw_debugger();
    }


    void draw_debugger() {

        FBO_debugger_shader->use();
        FBO_debugger_shader->setFloat("near_plane", near_plane);
        FBO_debugger_shader->setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depth_texture);

        glBindVertexArray(render_quad_VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

private:
    unsigned int render_quad_VAO;
    unsigned int FBO;
    const float SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    float near_plane = 1.0f, far_plane = 50.0f;

    void create_shaders() {
        depth_shader = new Shader(RESOURCES_PATH"shaders/shadowMapRendering/directionalLight/depthShader.vs", RESOURCES_PATH"shaders/shadowMapRendering/directionalLight/depthShader.fs");
        entity_shader = new Shader(RESOURCES_PATH"shaders/shadowMapRendering/directionalLight/depthShader.vs", RESOURCES_PATH"shaders/shadowMapRendering/directionalLight/depthShader.fs");
        FBO_debugger_shader = new Shader(RESOURCES_PATH"shaders/debugging/FBOShader.vs", RESOURCES_PATH"shaders/debugging/FBOShader.fs");
    }

    void create_FBO() {
        glGenFramebuffers(1, &FBO);
        // create depth texture
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
       
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    void create_render_quad()
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        unsigned int quadVBO;
        glGenVertexArrays(1, &render_quad_VAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(render_quad_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


    }





};


#endif