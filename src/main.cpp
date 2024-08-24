
#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <math.h> 
#include <stb/stb_image.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>

#include <shaders/shader.h>
#include <scene/camera.h>
#include <scene/loader.h>
#include <objects/entity.h>

#include <scene/sceneManager.h>
#include <scene/physicsDebugger.h>
#include <scene/physicsManager.h>
#include <scene/light.h>
#include <objects/sphere.h>
#include <objects/cube.h>
#include <objects/plane.h>
#include <terrain/terrain.h>
#include <player/player.h>
#include <scene/skybox.h>

#include <objects/modelLoading/model.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
glm::vec3 processInput(GLFWwindow* window);
void simulate_physics(float time, int& nbFrames, PhysicsManager* physics_handler, float freq, float &accumulator);
void get_fps(int& frameCount, float currentTime, float& prev_time);

// settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(-25.0f, 10.0f, -5.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT,glm::vec3(0, 1, 0), 45.0f, 0.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastTime = 0.0f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -----------------------------

    // build and compile shaders
    // -------------------------
    Shader geometry_pass_shader(RESOURCES_PATH"shaders/deferredRendering/geometryPassShader.vs", RESOURCES_PATH"shaders/deferredRendering/geometryPassShader.fs");
    Shader model_geometry_pass_shader(RESOURCES_PATH"shaders/deferredRendering/modelGeometryPassShader.vs", RESOURCES_PATH"shaders/deferredRendering/modelGeometryPassShader.fs");
    Shader lighting_pass_shader(RESOURCES_PATH"shaders/deferredRendering/lightingPassShader.vs", RESOURCES_PATH"shaders/deferredRendering/lightingPassShader.fs");
    //Shader depth_lighting_shader(RESOURCES_PATH"shaders/shadowMapRendering/depthShader.vs", RESOURCES_PATH"shaders/shadowMapRendering/depthShader.fs", RESOURCES_PATH"shaders/shadowMapRendering/depthShader.gs");
    Shader shader(RESOURCES_PATH"shaders/defaultShader.vs", RESOURCES_PATH"shaders/defaultShader.fs");
    Shader skybox_shader(RESOURCES_PATH"shaders/cubeMap/skybox.vs", RESOURCES_PATH"shaders/cubeMap/skybox.fs");
 
    
    Frustum scene_frustum = Frustum(&camera);

    Sphere sphere = Sphere(&geometry_pass_shader,  glm::vec4(9, 3, 14, 1), glm::vec3(0.0f, 0.39f, 0.0f), 2);
    
    //Sphere sphere2 = Sphere(&geometry_pass_shader, glm::vec4(9.5, 7, 14, 1), glm::vec3(0.5f, 0.5f, 0.0f), 1);
    Cube cube = Cube(&geometry_pass_shader, glm::vec4(5, 18, 8, 1), glm::vec3(0.18f, 0.55f, 0.34f), glm::vec3(2, 2, 2));
    Cube cube2 = Cube(&geometry_pass_shader, glm::vec4(5, 15, 8, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(5, 1, 5));
    Cube cube3 = Cube(&geometry_pass_shader, glm::vec4(5, 7, 8, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(1, 5, 1));
    Cube cube4 = Cube(&geometry_pass_shader, glm::vec4(5, 7, 8, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(1, 10, 1));
    Cube cube5 = Cube(&geometry_pass_shader, glm::vec4(5, 5, 18, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(1, 10, 1));
    Cube cube6 = Cube(&geometry_pass_shader, glm::vec4(10, 5, 20, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(1, 10, 1));
    Cube cube7 = Cube(&geometry_pass_shader, glm::vec4(15, 5, 18, 1), glm::vec3(0.56f, 0.93f, 0.56f), glm::vec3(1, 10, 1));
    //Player  player = Player(&geometry_pass_shader, glm::vec4(40, 2, 40, 1), window);
    //Model model2(&geometry_pass_shader, RESOURCES_PATH"models/chair/chair.obj", glm::vec4(5, -1, 5, 1));
    Model model(&model_geometry_pass_shader, RESOURCES_PATH"models/backpack/backpack.obj", glm::vec4(5, 7, 5, 1));

    Sun sun = Sun(&shader, glm::vec3(0, 1000, 0), glm::vec3(0.6, 0.6, 0.4), 0.04f, 0.007f);

    LightManager light_manager= LightManager(&shader);
    light_manager.add_light(glm::vec3(5, 7, 12), glm::vec3(2.0, 0.0, 2.0), 0.04f, 0.007f);
    light_manager.add_sun(&sun);
    //light_manager.add_light(glm::vec3(5, 7, 10), glm::vec3(0.0, 1.0, 0.0), 0.04f, 0.007f);
    //light_manager.add_light(glm::vec3(10, 7, 5), glm::vec3(1.0, 0.0, 0.0), 0.04f, 0.007f);
    //light_manager.lights.push_back(&sun);

    Terrain* terrain = new Terrain(&geometry_pass_shader, &camera, &scene_frustum, glm::vec4(0.0f, -1.0f, 0.0f, 1.0), "Textures/height_map1.jpg", false);

    SceneRenderer scene_renderer = SceneRenderer(&lighting_pass_shader, &light_manager, &camera, SCR_WIDTH, SCR_HEIGHT);
    PhysicsManager physicsManager = PhysicsManager(&shader);

    
    physicsManager.add_object(&sphere);
    physicsManager.add_object(&cube);
    physicsManager.add_object(&cube2);
    physicsManager.add_object(&cube3);
    //physicsManager.add_object(&cube4);
    //physicsManager.add_object(&cube5);
    //physicsManager.add_object(&cube6);
    physicsManager.add_object(terrain);
    
    //physicsManager.add_object(&model);


    scene_renderer.add_deferred_entity(&model);
    scene_renderer.add_deferred_entity(terrain);
    scene_renderer.add_deferred_entity(&sphere);
    scene_renderer.add_deferred_entity(&cube);
    scene_renderer.add_deferred_entity(&cube2);
    scene_renderer.add_deferred_entity(&cube3);
    scene_renderer.add_deferred_entity(&cube5);
    scene_renderer.add_deferred_entity(&cube6);
    scene_renderer.add_deferred_entity(&cube7);
   // scene_renderer.add_deferred_entity(&model);
    scene_renderer.toggle_light_debugging(true);

    
    std::vector<std::string> faces
    {
        RESOURCES_PATH"textures/cubeMaps/skybox/right.jpg",
        RESOURCES_PATH"textures/cubeMaps/skybox/left.jpg",
        RESOURCES_PATH"textures/cubeMaps/skybox/top.jpg",
        RESOURCES_PATH"textures/cubeMaps/skybox/bottom.jpg",
        RESOURCES_PATH"textures/cubeMaps/skybox/front.jpg",
        RESOURCES_PATH"textures/cubeMaps/skybox/back.jpg"
    };
    
    glEnable(GL_DEPTH_TEST);


    light_manager.set_depth_maps(scene_renderer.deferred_entities);
    Skybox skyBox(&skybox_shader, faces);
    CubeMapDebugger cube_map_debugger(&geometry_pass_shader, glm::vec4(0, 5, 0, 1), light_manager.lights[0]->depthCubeMap);
    //CubeMapDebugger cube_map_debugger(&geometry_pass_shader, glm::vec4(0, 5, 0, 1), skybox.depthCubeMap);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // render loop
    // -----------
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int nbFrames = 0;
    lastTime = glfwGetTime();
    float prev_time = glfwGetTime();
    int frameCount = 0;
    float accumulator = 0;
    while (!glfwWindowShouldClose(window))
    {

        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 projection = camera.GetProjection();
        glm::mat4 view = camera.GetViewMatrix();
        get_fps(frameCount, currentTime, prev_time);
        //player.player_movement();
        
        simulate_physics(currentTime, nbFrames, &physicsManager, 0.01, accumulator);

        //physicsManager.simulation_loop(0.01);
        scene_frustum.update_visibility_planes();
          
        light_manager.set_depth_maps(scene_renderer.deferred_entities);
        //sun.set_depth_map(scene_renderer.deferred_entities);
        scene_renderer.render_scene();

        //cube_map_debugger.draw(projection, view, camera.Position);
        //skyBox.draw(projection, view);
                
        //physicsManager.debugging(projection, view, camera.Position);
        // per-frame time logic
        // --------------------
        // input
        // -----;
        sun.position = sun.position + processInput(window);


        lastTime = currentTime;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void get_fps(int& frameCount, float currentTime, float& prev_time) {
    float frameDeltaTime = currentTime - prev_time;
    frameCount++;
    if (frameDeltaTime >= 1.0) {
        // Calculate FPS
        double fps = (double)frameCount / frameDeltaTime;

        // Display FPS
        std::cout << "fps: " << fps << "\n";
        // Reset for next calculation

        prev_time = currentTime;
        frameCount = 0;
    }
}


void simulate_physics(float time, int& nbFrames, PhysicsManager* physicsManager, float freq, float &accumulator) {

    nbFrames++;
    float delta_time = time - lastTime;
    //lastTime = time;
    accumulator += delta_time;
    //int count = 0;
    //std::cout << "count: " << count++ << "\n";
    while (accumulator >= freq) {
        //std::cout << "acc: " << accumulator << "\n";
        physicsManager->simulation_loop(freq);

        nbFrames = 0;
        accumulator -= freq;
    }
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
glm::vec3 processInput(GLFWwindow* window)
{

    float movement_speed = deltaTime * 5.0;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.forward_mv = true;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)
        camera.forward_mv = false;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.back_mv = true;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)
        camera.back_mv = false;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.left_mv = true;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)
        camera.left_mv = false;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.right_mv = true;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
        camera.right_mv = false;

    if (camera.forward_mv)
        camera.ProcessKeyboard(FORWARD, movement_speed);
    if (camera.back_mv)
        camera.ProcessKeyboard(BACKWARD, movement_speed);
    if (camera.left_mv)
        camera.ProcessKeyboard(LEFT, movement_speed);
    if (camera.right_mv)
        camera.ProcessKeyboard(RIGHT, movement_speed);

    return glm::vec3(0, 0, 0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
