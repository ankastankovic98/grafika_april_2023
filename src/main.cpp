#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);
void setLights(Shader lightingShader, float currentFrame);
void renderQuad();

void drawCity(Shader modelShader, Model cityModel, Model stoneBridge, Model stonePlatformB);
void drawTrees(Shader modelShader, Model treeModel, int amount);

// settings
const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 900;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = false;
    glm::vec3 cityPosition = glm::vec3(0.0f);
    float cityScale = 1.0f;
    glm::vec3 bridgePossition = glm::vec3(0.0f);
    float bridgeScale = 0.5f;
    bool wireframe = false;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    glm::vec3 dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 dirLightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 dirLightDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 dirLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 pointLightPosition = glm::vec3(8.5f, 3.2f, 4.5f);
    glm::vec3 pointLightAmbient = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 pointLightDiffuse = glm::vec3(0.94f, 0.98f, 0.78f);
    glm::vec3 pointLightSpecular = glm::vec3(0.94f, 0.98f, 0.78f);

    bool hdr = true;
    float hdrExposure = 1.0f;
    float hdrGamma = 2.2f;
    bool bloom = false;
    int effectSelected = 0;

    bool cameraDebug = true;
    bool lightsDebug = false;

    void SaveToFile(std::string filename);
    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << cityPosition[0] << '\n'
        << cityPosition[1] << '\n'
        << cityPosition[2] << '\n'
        << cityScale << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << wireframe << '\n'
        << dirLightDirection[0] << '\n'
        << dirLightDirection[1] << '\n'
        << dirLightDirection[2] << '\n'
        << dirLightAmbient[0] << '\n'
        << dirLightAmbient[1] << '\n'
        << dirLightAmbient[2] << '\n'
        << dirLightDiffuse[0] << '\n'
        << dirLightDiffuse[1] << '\n'
        << dirLightDiffuse[2] << '\n'
        << dirLightSpecular[0] << '\n'
        << dirLightSpecular[1] << '\n'
        << dirLightSpecular[2] << '\n'
        << hdr << '\n'
        << hdrExposure << '\n'
        << hdrGamma << '\n'
        << bloom << "\n"
        << effectSelected << "\n"
        << cameraDebug << "\n"
        << lightsDebug << "\n"
        << bridgePossition[0] << '\n'
        << bridgePossition[1] << '\n'
        << bridgePossition[2] << '\n'
        << bridgeScale << '\n';

}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> cityPosition[0]
           >> cityPosition[1]
           >> cityPosition[2]
           >> cityScale
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> wireframe
           >> dirLightDirection[0]
           >> dirLightDirection[1]
           >> dirLightDirection[2]
           >> dirLightAmbient[0]
           >> dirLightAmbient[1]
           >> dirLightAmbient[2]
           >> dirLightDiffuse[0]
           >> dirLightDiffuse[1]
           >> dirLightDiffuse[2]
           >> dirLightSpecular[0]
           >> dirLightSpecular[1]
           >> dirLightSpecular[2]
           >> hdr
           >> hdrExposure
           >> hdrGamma
           >> bloom
           >> effectSelected
           >> cameraDebug
           >> lightsDebug
           >> bridgePossition[0]
           >> bridgePossition[1]
           >> bridgePossition[2]
           >> bridgeScale;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // Culling
    glCullFace(GL_FRONT);
    // Blending
    glDisable(GL_BLEND);

    // build and compile shaders
    // -------------------------
    Shader screenShader("resources/shaders/framebuffer.vs", "resources/shaders/framebuffer.fs");
    Shader ourShader("resources/shaders/cityShader.vs", "resources/shaders/cityShader.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");
    Shader instanceShader("resources/shaders/instanceShader.vs", "resources/shaders/instanceShader.fs");
    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    // load models
    // -----------
    Model cityModel("resources/objects/SH-Cartoon/SH-Cartoon.obj");
    cityModel.SetShaderTextureNamePrefix("material.");
    Model stoneBridge("resources/objects/Stone_Bridge_Obj/Stone Bridge_Obj.obj");
    stoneBridge.SetShaderTextureNamePrefix("material.");
    Model stonePlatformB("resources/objects/StonePlatform_Obj/StonePlatform_B.obj");
    stoneBridge.SetShaderTextureNamePrefix("material.");
    Model treeModel("resources/objects/Tree/Hand painted Tree.obj");
    treeModel.SetShaderTextureNamePrefix("material.");


    float skyboxVertices[] = {
            // positions
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,

            1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,

            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,

            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f
    };


    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };


    // Instancing
    unsigned int amount = 50;
    treeModel.Instantiate(amount);

    // Culling
    glFrontFace(GL_CW);

    // Quad VAO
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    // Load skybox textures
    vector<std::string> faces{
            FileSystem::getPath("resources/textures/skybox/space2/px.png"),
            FileSystem::getPath("resources/textures/skybox/space2/nx.png"),
            FileSystem::getPath("resources/textures/skybox/space2/py.png"),
            FileSystem::getPath("resources/textures/skybox/space2/ny.png"),
            FileSystem::getPath("resources/textures/skybox/space2/pz.png"),
            FileSystem::getPath("resources/textures/skybox/space2/nz.png")
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: RBO Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Pingpong Framebuffer not complete!" << std::endl;
    }
    // --------------------------------------------------------

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);
    blurShader.use();
    blurShader.setInt("image", 0);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if(programState->wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // bind to framebuffer and draw scene as we normally would to color texture
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        // make sure we clear the framebuffer's content
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float )SCR_WIDTH/(float )SCR_HEIGHT, 0.1f, 100.0f);
        // Skybox shader set
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // Draw skybox
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // view/projection transformations
        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();

        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 30.0f);
        setLights(ourShader, currentFrame);

        drawCity(ourShader, cityModel, stoneBridge, stonePlatformB);

        instanceShader.use();
        instanceShader.setMat4("projection", projection);
        instanceShader.setMat4("view", view);
        drawTrees(instanceShader, treeModel, amount);

        // Reset wireframe drawing so that it doesn't try to draw quads
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        // Clear all relevant buffers
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the quad plane on default framebuffer
        screenShader.use();
        screenShader.setInt("bloom", programState->bloom);
        screenShader.setInt("hdr", programState->hdr);
        screenShader.setFloat("exposure", programState->hdrExposure);
        screenShader.setFloat("gamma", programState->hdrGamma);
        screenShader.setInt("option", programState->effectSelected);
        // Bind bloom and non bloom
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        renderQuad();
        //glBindVertexArray(quadVAO);
        //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        //glDrawArrays(GL_TRIANGLES, 0, 6);


        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
void drawCity(Shader modelShader, Model cityModel, Model stoneBridge, Model stonePlatformB){

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, programState->cityPosition); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(programState->cityScale));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    cityModel.Draw(modelShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(programState->bridgePossition)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    stonePlatformB.Draw(modelShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(programState->bridgePossition + glm::vec3(-30.f, -5.0f, 0.0f))); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(programState->bridgeScale));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    stoneBridge.Draw(modelShader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(programState->bridgePossition + glm::vec3(30.f, -2.0f, 0.0f))); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(programState->bridgeScale));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    stoneBridge.Draw(modelShader);

}

void drawTrees(Shader modelShader, Model treeModel, int amount){

    treeModel.DrawInstanced(modelShader, amount);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
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
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Begin("Osnovno");
        ImGui::Text("Podesavanja");
        ImGui::Checkbox("Izmena svetla", &programState->lightsDebug);
        ImGui::Checkbox("Informacije o kameri", &programState->cameraDebug);

        ImGui::DragFloat3("Pozicija grada", (float*)&programState->cityPosition);
        ImGui::DragFloat("Velicina grada", &programState->cityScale, 0.05, 0.1, 20.0);

        ImGui::DragFloat3("Pozicija mosta", (float*)&programState->bridgePossition);
        ImGui::DragFloat("Velicina mosta", &programState->bridgeScale, 0.05, 0.1, 20.0);


        ImGui::Text("HDR");
        ImGui::Checkbox("HDR", &programState->hdr);
        if(programState->hdr){
            ImGui::SliderFloat("HDR Exposure", &programState->hdrExposure, 0.0f, 5.0f);
            ImGui::SliderFloat("HDR Gamma", &programState->hdrGamma, 0.0f, 5.0f);
            ImGui::Checkbox("Bloom", &programState->bloom);
        }
        ImGui::Text("Efekti");
        ImGui::Checkbox("Draw Wireframe", &programState->wireframe);
        ImGui::RadioButton("No Effect", &programState->effectSelected, 0);
        ImGui::RadioButton("Blur", &programState->effectSelected, 1);
        ImGui::RadioButton("Violet", &programState->effectSelected, 2);

        ImGui::End();
    }
    if(programState->lightsDebug){
        ImGui::Text("Directional light");
        ImGui::DragFloat3("Dir Direction", (float*)&programState->dirLightDirection, 0.05f, -1.0f, 1.0f);
        ImGui::DragFloat3("Dir Ambient", (float*)&programState->dirLightAmbient, 0.05f, -1.0f, 1.0f);
        ImGui::DragFloat3("Dir Diffuse", (float*)&programState->dirLightDiffuse, 0.05f, -1.0f, 1.0f);
        ImGui::DragFloat3("Dir Specular", (float*)&programState->dirLightSpecular, 0.05f, -1.0f, 1.0f);
    }

    if(programState->cameraDebug){
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
        programState->hdr = !programState->hdr;
        std::cout << "HDR - " << (programState->hdr  ? "ON" : "OFF") << '\n';
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
        programState->bloom = !programState->bloom;
        std::cout << "Bloom - " << (programState->bloom  ? "ON" : "OFF") << '\n';
    }
}

unsigned int loadCubemap(vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            // Load RGBA instead of RGB for .png cubemap textures
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setLights(Shader lightingShader, float currentFrame){
    //directional light
    lightingShader.setVec3("dirLight.direction", programState->dirLightDirection);
    lightingShader.setVec3("dirLight.ambient", programState->dirLightAmbient);
    lightingShader.setVec3("dirLight.diffuse", programState->dirLightDiffuse);
    lightingShader.setVec3("dirLight.specular", programState->dirLightSpecular);

    // point light blue
    lightingShader.setVec3("pointLights[0].position",  glm::vec3(5.0 * cos(currentFrame), 5.0f * sin(currentFrame), 2.0 * cos(currentFrame)));
    lightingShader.setVec3("pointLights[0].ambient", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    lightingShader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    lightingShader.setFloat("pointLights[0].constant", 0.05f);
    lightingShader.setFloat("pointLights[0].linear", 0.02f);
    lightingShader.setFloat("pointLights[0].quadratic", 0.001f);
    lightingShader.setVec3("pointLights[0].lightColor", glm::vec3(0.0, 0.0, 1.0f));

    // point light red
    lightingShader.setVec3("pointLights[1].position", glm::vec3 (-5.0 * sin(currentFrame), 3.0f * cos(currentFrame), 4.0 * sin(currentFrame)));
    lightingShader.setVec3("pointLights[1].ambient", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setVec3("pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    lightingShader.setVec3("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
    lightingShader.setFloat("pointLights[1].constant", 0.05f);
    lightingShader.setFloat("pointLights[1].linear", 0.02f);
    lightingShader.setFloat("pointLights[1].quadratic", 0.001f);
    lightingShader.setVec3("pointLights[1].lightColor", glm::vec3(1.0, 0.0, 0.0f));

    //spotlight
    lightingShader.setVec3("spotlight.position", glm::vec3 (0.0f, -5.0f, 0.0f));
    lightingShader.setVec3("spotlight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
    lightingShader.setFloat("spotlight.cutOff", glm::cos(glm::radians(2.5f)));
    lightingShader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(6.5f)));
    lightingShader.setFloat("spotlight.constant", 0.05f);
    lightingShader.setFloat("spotlight.linear", 0.0f);
    lightingShader.setFloat("spotlight.quadratic", 0.0f);
    lightingShader.setVec3("spotlight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
    lightingShader.setVec3("spotlight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
    lightingShader.setVec3("spotlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
}
