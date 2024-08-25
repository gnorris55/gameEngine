#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

//#include <SceneManagement/SceneRenderer.h> 
#include <objects/entity.h>
#include <scene/light.h>

class SceneRenderer {


public:
	
	std::vector<Entity *> static_deferred_entities;
	std::vector<Entity *> dynamic_deferred_entities;
	std::vector<Entity *> deferred_entities;
	std::vector<Entity*> forward_entities;
	LightManager* light_manager;

	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec, gLightNormal, gTangent, gBitangent;
	unsigned int quadVAO = 0;
	int screen_height, screen_width;
	bool light_debugging = false;

	Shader *lighting_pass_shader;
	Camera *camera;

	SceneRenderer(Shader *lighting_pass_shader, LightManager *light_manager, Camera* camera, int screen_width, int screen_height) {
		this->camera = camera;
		this->screen_width = screen_width;
		this->screen_height = screen_height;
		this->lighting_pass_shader = lighting_pass_shader;
		this->light_manager = light_manager;
		create_gTextures();
	}

	void toggle_light_debugging(bool debugging) {
		light_debugging = debugging;
	}

	void render_scene() {
		//light_manager->render_depth_maps(deferred_entities, forward_entities);
		deferred_rendering();
		forward_rendering();
	}

	void add_deferred_entity(Entity *new_entity, bool is_static = false) {
		deferred_entities.push_back(new_entity);
		if (is_static)
			static_deferred_entities.push_back(new_entity);
		else
			dynamic_deferred_entities.push_back(new_entity);
	}
	
	void add_forward_entity(Entity *new_entity) {
		forward_entities.push_back(new_entity);
	}

	void update_static_shadow_maps() {
		light_manager->sun->set_depth_map(static_deferred_entities);
		light_manager->set_depth_maps(static_deferred_entities);
	}



private:


	void deferred_rendering() {


		// create shadow map for all entities
		// TODO: prebake in shadow map for static objects
		//if (light_manager->sun != nullptr)
			//light_manager->sun->set_depth_map(deferred_entities);

        glViewport(0, 0, screen_width, screen_height);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (Entity* entity : deferred_entities) {
			entity->draw(camera->GetProjection(), camera->GetViewMatrix(), camera->Position);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// lighting pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lighting_pass_shader->use();
		lighting_pass_shader->setInt("gPosition", 0);
		lighting_pass_shader->setInt("gNormal", 1);
		lighting_pass_shader->setInt("gAlbedoSpec", 2);
		lighting_pass_shader->setInt("gLightNormal", 3);
		lighting_pass_shader->setInt("gTangent", 4);
		lighting_pass_shader->setInt("gBitangent", 5);
		lighting_pass_shader->setInt("directionalShadowMap", 6);
		lighting_pass_shader->setInt("omniDirectionalShadowMap", 7);
	

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gLightNormal);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, gTangent);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, gBitangent);

		define_lights();
		lighting_pass_shader->setVec3("viewPos", camera->Position);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	void forward_rendering() {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. render lights on top of scene
		// --------------------------------
		for (Entity * entity : forward_entities){
			entity->draw(camera->GetProjection(), camera->GetViewMatrix(), camera->Position);
		}
		if (light_debugging) {
			for (Light* light : light_manager->lights) {
				light->draw(camera->GetProjection(), camera->GetViewMatrix(), camera->Position);
			}
		}
		if (light_manager->sun != nullptr) {
			light_manager->sun->draw(camera->GetProjection(), camera->GetViewMatrix(), camera->Position);
		}
	}

	void define_lights() {
		unsigned int texture_unit = 7;

		lighting_pass_shader->setInt("shadowMap", texture_unit);
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light_manager->lights[0]->depthCubeMap);

		for (int i = 0; i < light_manager->lights.size(); i++) {

			Light* curr_light = light_manager->lights[i];
			lighting_pass_shader->setFloat("far_plane", curr_light->far_plane);

			lighting_pass_shader->setVec3("lights[" + std::to_string(i) + "].Position", curr_light->position);
			lighting_pass_shader->setVec3("lights["+ std::to_string(i) +"].Color", curr_light->color);
			// update attenuation parameters and calculate radius
			const float constant = curr_light->constant; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = curr_light->linear_constant;
			const float quadratic = curr_light->quadratic_constant;
			lighting_pass_shader->setFloat("lights[" + std::to_string(i) + "].Linear", linear);
			lighting_pass_shader->setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
			// then calculate radius of light volume/sphere
			const float maxBrightness = std::fmaxf(std::fmaxf(1, 1), 1);
			float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			lighting_pass_shader->setFloat("lights[" + std::to_string(i) + "].Radius", radius);
			
			lighting_pass_shader->setInt("shadowMap"+std::to_string(i+1), texture_unit);
			glActiveTexture(GL_TEXTURE0 + texture_unit);
			glBindTexture(GL_TEXTURE_CUBE_MAP, light_manager->lights[i]->depthCubeMap);

			texture_unit++;
		}
		if (light_manager->sun != nullptr) {
					
			Sun* sun = light_manager->sun;
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, sun->depth_texture);

			lighting_pass_shader->setMat4("lightSpaceMatrix", sun->lightSpaceMatrix);
			lighting_pass_shader->setVec3("sunLight.Position", sun->position);
			lighting_pass_shader->setVec3("sunLight.Color", sun->color);
		}

	}


	void create_render_quad() {

		unsigned int quadVBO;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	
	void create_gTexture(unsigned int &gTexture, GLenum attachment, GLint tex_size) {
		glGenTextures(1, &gTexture);
		glBindTexture(GL_TEXTURE_2D, gTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, tex_size, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, gTexture, 0);
	}

	void create_gTextures() {
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		create_gTexture(gPosition, GL_COLOR_ATTACHMENT0, GL_RGBA16F);
		create_gTexture(gNormal, GL_COLOR_ATTACHMENT1, GL_RGBA16F);
		create_gTexture(gAlbedoSpec, GL_COLOR_ATTACHMENT2, GL_RGBA);
		create_gTexture(gLightNormal, GL_COLOR_ATTACHMENT3, GL_RGBA16F);
		create_gTexture(gTangent, GL_COLOR_ATTACHMENT4, GL_RGBA16F);
		create_gTexture(gBitangent, GL_COLOR_ATTACHMENT5, GL_RGBA16F);

		unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(6, attachments);
		// create and attach depth buffer (renderbuffer)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		create_render_quad();

		lighting_pass_shader->use();
		lighting_pass_shader->setInt("gPosition", 0);
		lighting_pass_shader->setInt("gNormal", 1);
		lighting_pass_shader->setInt("gAlbedoSpec", 2);
		lighting_pass_shader->setInt("gLightNormal", 3);
		lighting_pass_shader->setInt("gTangent", 4);
		lighting_pass_shader->setInt("gBitangent", 5);
		lighting_pass_shader->setInt("directionalShadowMap", 6);
		lighting_pass_shader->setInt("omniDirectionalShadowMap", 7);

	}
	



};

#endif