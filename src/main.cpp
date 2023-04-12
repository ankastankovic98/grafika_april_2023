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
void setLights(Shader lightingShader);

void drawCity(Shader modelShader, Model cityModel);
void drawTrees(Shader modelShader, Model treeModel, int amount);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

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
    glm::vec3 treeOnePosition = glm::vec3(1.0f);
    float treeOneScale = 0.10f;
    glm::vec3 treeTwoPosition = glm::vec3(1.0f);
    float treeTwoScale = 0.10f;
    bool wireframe = false;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

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
        << treeOnePosition[0] << '\n'
        << treeOnePosition[1] << '\n'
        << treeOnePosition[2] << '\n'
        << treeOneScale << '\n'
        << treeTwoPosition[0] << '\n'
        << treeTwoPosition[1] << '\n'
        << treeTwoPosition[2] << '\n'
        << treeTwoScale << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << wireframe << '\n';
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
           >> treeOnePosition[0]
           >> treeOnePosition[1]
           >> treeOnePosition[2]
           >> treeOneScale
           >> treeTwoPosition[0]
           >> treeTwoPosition[1]
           >> treeTwoPosition[2]
           >> treeTwoScale
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z
           >> pointLight.constant
           >> pointLight.linear
           >> pointLight.quadratic
           >> wireframe;
    }
}

ProgramState *programState;

void drawBackpack(Shader modelShader, Model backpackModel);

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
    Shader ourShader("resources/shaders/cityShader.vs", "resources/shaders/cityShader.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");
    Shader instanceShader("resources/shaders/instanceShader.vs", "resources/shaders/instanceShader.fs");
    // load models
    // -----------

    //Model ourModel("resources/objects/SH-Cartoon/SH-Cartoon.obj");
    Model cityModel("resources/objects/SH-Cartoon/SH-Cartoon.obj");
    cityModel.SetShaderTextureNamePrefix("material.");

    Model treeModel("resources/objects/Tree/Hand painted Tree.obj");
    treeModel.SetShaderTextureNamePrefix("material.");
    // Instancing
    unsigned int amount = 50;
    treeModel.Instantiate(amount);

    float skyboxVertices[] = {
            // positions
            -2.0f, -2.0f, -2.0f,
            -2.0f,  2.0f, -2.0f,
            2.0f, -2.0f, -2.0f,
            2.0f, -2.0f, -2.0f,
            -2.0f,  2.0f, -2.0f,
            2.0f,  2.0f, -2.0f,

            -2.0f, -2.0f, -2.0f,
            -2.0f, -2.0f,  2.0f,
            -2.0f,  2.0f, -2.0f,
            -2.0f,  2.0f, -2.0f,
            -2.0f, -2.0f,  2.0f,
            -2.0f,  2.0f,  2.0f,

            2.0f, -2.0f,  2.0f,
            2.0f, -2.0f, -2.0f,
            2.0f,  2.0f,  2.0f,
            2.0f,  2.0f,  2.0f,
            2.0f, -2.0f, -2.0f,
            2.0f,  2.0f, -2.0f,

            -2.0f,  2.0f,  2.0f,
            -2.0f, -2.0f,  2.0f,
            2.0f,  2.0f,  2.0f,
            2.0f,  2.0f,  2.0f,
            -2.0f, -2.0f,  2.0f,
            2.0f, -2.0f,  2.0f,

            2.0f,  2.0f, -2.0f,
            -2.0f,  2.0f, -2.0f,
            2.0f,  2.0f,  2.0f,
            2.0f,  2.0f,  2.0f,
            -2.0f,  2.0f, -2.0f,
            -2.0f,  2.0f,  2.0f,

            -2.0f, -2.0f,  2.0f,
            -2.0f, -2.0f, -2.0f,
            2.0f, -2.0f, -2.0f,
            2.0f, -2.0f, -2.0f,
            2.0f, -2.0f,  2.0f,
            -2.0f, -2.0f,  2.0f
    };

    glFrontFace(GL_CW);

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

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);


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
        setLights(ourShader);

        drawCity(ourShader, cityModel);

        instanceShader.use();
        instanceShader.setMat4("projection", projection);
        instanceShader.setMat4("view", view);
        setLights(instanceShader); // todo
        drawTrees(instanceShader, treeModel, amount);


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
void drawCity(Shader modelShader, Model cityModel){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, programState->cityPosition); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(programState->cityScale));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
    cityModel.Draw(modelShader);
}

void drawTrees(Shader modelShader, Model treeModel, int amount){

    treeModel.DrawInstanced(modelShader, amount);
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
        ImGui::Begin("Setup window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        ImGui::DragFloat3("City position", (float*)&programState->cityPosition);
        ImGui::DragFloat("City scale", &programState->cityScale, 0.05, 0.1, 20.0);

        ImGui::DragFloat3("Tree one position", (float*)&programState->treeOnePosition);
        ImGui::DragFloat("Tree one scale", &programState->treeOneScale, 0.05, 0.1, 20.0);

        ImGui::DragFloat3("Tree two position", (float*)&programState->treeTwoPosition);
        ImGui::DragFloat("Tree two scale", &programState->treeTwoScale, 0.05, 0.1, 20.0);


        ImGui::Text("Effects");
        ImGui::Checkbox("Draw Wireframe", &programState->wireframe);

        ImGui::End();
    }

    {
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

void setLights(Shader lightingShader){
    lightingShader.setVec3("dirLight.direction", glm::vec3 (-2.2f, -10.0f, -0.3f));
    lightingShader.setVec3("dirLight.ambient", glm::vec3(0.4f, 0.4f, 0.4f));
    lightingShader.setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    lightingShader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
}
