#include "init.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "engine.hpp"
#include "particle.hpp"
#include "primitive.hpp"
#include "obj.hpp"

#include "tiny_obj_loader.h"

#include <utility>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef _WIN32
// Ask hybrid-graphics laptops to run this on the discrete GPU instead of the integrated one
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

bool HOLD = false;

// Command line options
struct Options{
    bool benchmark = false;     // --benchmark: sweep cloth size x substeps, write a CSV
    std::string csv = "bench/scaling.csv";
    std::string recordDir;      // --record <dir>: run the default scene and save frames as PNGs
    int frames = 200;           // timed frames per benchmark config / frames to record
};
Options parseArgs(int argc, char** argv);

using Clock = std::chrono::steady_clock;
double elapsedMs(Clock::time_point a, Clock::time_point b){
    return std::chrono::duration<double, std::milli>(b - a).count();
}

// const glm::vec3 clearColor = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 clearColor = glm::vec3(0.541f, 0.898f, 1.0f);

// Window parameters
int height = 600;
int width = 800;

// Model, view, projection matrices
glm::mat4 model = glm::mat4(1.0f);

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Camera position in world space
glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f); // Camera up vector
float Yaw = -90.0f; // Camera yaw
float Pitch = 0.0f; // Camera pitch
bool Perspective = true; // Camera projection type
Camera camera(cameraPos, Yaw, Pitch, Up, Perspective);
double lastX = width / 2.0f;
double lastY = height / 2.0f;
bool firstMouse = true;

// Physics parameters
float gravity = -.98f;
float delta_time = 0.0166f; // 60 fps
glm::mat4 geometricToPhysical{glm::mat4(1.0f)};

// Callback functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
std::vector<Primitive> Scene;

void drawPrimitive(Shader& program, Primitive& prim);
int runBenchmark(GLFWwindow* window, Shader& program, Primitive& ground, const Options& opts);
void saveFrame(GLFWwindow* window, const std::string& path);


Primitive makeCube(glm::mat4 inital_trans, float inverse_mass = 1.0f, float stretching_coeff = 1.0f){

    // Get cube vertices
    std::vector<float> vertices = {
    // front
    -0.5f, -0.5f,  0.5f, // 0
    0.5f, -0.5f,  0.5f, // 1
    0.5f,  0.5f,  0.5f, // 2
    -0.5f,  0.5f,  0.5f, // 3
    // back
    -0.5f, -0.5f, -0.5f, // 4
    0.5f, -0.5f, -0.5f, // 5
    0.5f,  0.5f, -0.5f, // 6
    -0.5f,  0.5f, -0.5f, // 7
    };

    // Define the indices for the cube
    std::vector<unsigned int> indices = {
        // front
        0, 1, 2,
        0, 2, 3,
        // right
        1, 5, 6,
        1, 6, 2,
        // back
        7, 6, 5,
        7, 5, 4,
        // left
        4, 0, 3,
        4, 3, 7,
        // bottom
        4, 5, 1,
        4, 1, 0,
        // top
        3, 2, 6,
        6, 7, 3
    };

    // Apply the transformation to the vertices
    for(int i = 0; i < vertices.size(); i+=3){
        glm::vec4 temp = glm::vec4(vertices[i], vertices[i+1], vertices[i+2], 1.0f);
        temp = inital_trans * temp;
        vertices[i] = temp.x;
        vertices[i+1] = temp.y;
        vertices[i+2] = temp.z;
    }

    // Create the particles
    std::vector<Particle> particles;
    for (int i = 0; i < vertices.size(); i+=3){
        Particle p;
        p.position = glm::vec3(vertices[i], vertices[i+1], vertices[i+2]);
        p.old_position = p.position;
        p.velocity = glm::vec3(0.0f, 0.f, 0.0f);
        p.inverse_mass = inverse_mass;
        particles.push_back(p);

    }

    // Make a primitive
    Primitive Cube = Primitive(particles, indices, stretching_coeff, true, 0.0f, false);

    return Cube;
}

// A rectangular cloth of nx x ny particles hanging in the x-y plane, pinned at its two top corners.
// Same physical size as the default cloth, so only the resolution changes between benchmark configs.
Primitive makeClothGrid(int nx, int ny){
    const float x0 = -0.4f, x1 = 0.4f, yTop = 1.15f, yBottom = 0.35f;
    std::vector<Particle> particles;
    for(int r = 0; r < ny; r++){
        for(int c = 0; c < nx; c++){
            float x = x0 + (x1 - x0) * c / (nx - 1);
            float y = yTop - (yTop - yBottom) * r / (ny - 1);
            particles.push_back(Particle(glm::vec3(x, y, 0.0f)));
        }
    }
    particles[0].inverse_mass = 0.0f;      // top left
    particles[nx - 1].inverse_mass = 0.0f; // top right

    // Two counter-clockwise triangles per grid cell
    std::vector<unsigned int> indices;
    for(int r = 0; r + 1 < ny; r++){
        for(int c = 0; c + 1 < nx; c++){
            unsigned int tl = r * nx + c, tr = tl + 1, bl = tl + nx, br = bl + 1;
            indices.insert(indices.end(), {tl, bl, tr, tr, bl, br});
        }
    }
    // Same settings as the default cloth in main()
    return Primitive(particles, indices, 1.f, true, 1.f, true);
}

int main(int argc, char** argv){
    Options opts = parseArgs(argc, argv);

    GLFWwindow* window = init::setupWindow(height, width);
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Set the mouse callback function
    glfwSetCursorPosCallback(window, mouse_callback);

    // Set the screen resizing callback function
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

// std::cout << "Loading the obj file" << std::endl;
//     std::string inputfile = "./models/bunny.obj";
//     tinyobj::attrib_t attrib;
//     std::vector<tinyobj::shape_t> shapes;
//     std::string warn;
//     std::string err;
//     bool ret = tinyobj::LoadObj(&attrib, &shapes, NULL, &warn, &err, inputfile.c_str());
//     if (!warn.empty()) {
//         std::cout << warn << std::endl;
//     }
//     if (!err.empty()) {
//         std::cerr << err << std::endl;
//     }
//     if(!ret){
//         std::cout << "Failed to load the obj file" << std::endl;
//         return 1;
//     }
//     std::cout << "Loaded the obj file" << std::endl;

//     // Make the vertices
//     std::vector<float> obj_vertices;
//     for (int i = 0; i < attrib.vertices.size(); i++){
//         obj_vertices.push_back(attrib.vertices[i]);
//     }
//     // Make the indices
//     std::vector<unsigned int> obj_indices;
//     for (int i = 0; i < shapes[0].mesh.indices.size(); i++){
//         obj_indices.push_back(shapes[0].mesh.indices[i].vertex_index);
//     }

//     // Make the box from obj.hpp
//     std::vector<Particle> particles;
//     for(int i = 0; i < obj_vertices.size(); i+=3){
//         Particle p;
//         p.position = glm::vec3(obj_vertices[i], obj_vertices[i+1], obj_vertices[i+2]);
//         p.old_position = p.position;
//         p.velocity = glm::vec3(0.0f, 0.f, 0.0f);
//         p.inverse_mass = 1.f;
//         particles.push_back(p);
//     }

//     Primitive Box = Primitive(particles, obj_indices, 1.f);

    // Make the cloth from obj.hpp
    std::vector<Particle> particles_cloth;
    // std::vector<float> cloth_vertices;
    // std::vector<unsigned int> cloth_indices;

    //Make cloth from obj.hpp
    // std::pair<std::vector<float>, std::vector<unsigned int>> cloth = createCloth(.5f, 1.f, 10, 20);
    // cloth_vertices = cloth.first;
    // cloth_indices = cloth.second;


    for(int i = 0; i < cloth_vertices.size(); i+=3){
        Particle p;
        p.position = glm::vec3(cloth_vertices[i], cloth_vertices[i+1], cloth_vertices[i+2]);
        p.old_position = p.position;
        p.velocity = glm::vec3(0.0f, 0.f, 0.0f);
        if(i > cloth_vertices.size()/2){
            p.velocity = glm::vec3(.1f, 0.f, 1.0f);
        }
        p.inverse_mass = 1.f;
        particles_cloth.push_back(p);
    }


    // particles_cloth[9].inverse_mass = 0.0f;

    // particles_cloth[9].inverse_mass = 0.0f;
    // particles_cloth[9].velocity = glm::vec3(0.0f, 10.0f, 0.0f);



    // Find the top right and top left corner 
    int top_right = 0;
    int top_left = 0;
    for(int i = 0; i < particles_cloth.size(); i++){
        if(particles_cloth[i].position.x > particles_cloth[top_right].position.x){
            top_right = i;
        }
        if(particles_cloth[i].position.x == particles_cloth[top_right].position.x && particles_cloth[i].position.y > particles_cloth[top_right].position.y){
            top_right = i;
        }
        if(particles_cloth[i].position.x < particles_cloth[top_left].position.x){
            top_left = i;
        }
        if(particles_cloth[i].position.x == particles_cloth[top_left].position.x && particles_cloth[i].position.y > particles_cloth[top_left].position.y){
            top_left = i;
        }

    }
    particles_cloth[top_right].inverse_mass = 0.0f;
    particles_cloth[top_right].velocity = glm::vec3(0.0f, 0.0f, 0.0f);

    particles_cloth[top_left].inverse_mass = 0.0f;
    particles_cloth[top_left].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    

    Primitive Cloth = Primitive(particles_cloth, cloth_indices, 1.f, true, 1.f, true);
    Cloth.ambient = glm::vec4(178.f/255.f, 115.f/255.f, 37.f/255.f, 1.0f);
    Cloth.ambient_strength = 0.539f;
    Cloth.diffuse = glm::vec4(183.f/255.f, 101.f/255.f, 101.f/255.f, 1.0f);
    Cloth.diffuse_strength = 0.8f;
    // Scene.push_back(Cloth);

    // Translate the vertices to the +10 y
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::translate(rotation, glm::vec3(0.0f, 1.f, 0.f));
    rotation = glm::rotate(rotation, glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Make a primitive
    // Primitive Cube = Primitive(particles, indices);
    Primitive Cube = makeCube(rotation, 1.0f, .01f);
    // Scene.push_back(Cube);
        model = glm::translate(model, glm::vec3(0.0f, -1.f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, .01f, 10.f));
    Primitive Ground = makeCube(model, 0.0f, 0.0f);
    Ground.ambient = glm::vec4(0.2f, 1.f, 0.2f, 1.0f);
    Ground.ambient_strength = 0.4f;
    Ground.diffuse = glm::vec4(0.2f, 1.f, 0.2f, 1.0f);
    Ground.diffuse_strength = 0.5f;

    Scene.push_back(Ground);
    // std::cout << "particles.size(): " << particles.size() << std::endl;

    // Frametime
    float last_time = 0.0f;
    float deltaTime = 0.0f;

    // Add a shader
    Shader program("./shaders/shader.vert", "./shaders/shader.frag");
  
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    //Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    int scale = 1;

    if(opts.benchmark){
        int status = runBenchmark(window, program, Ground, opts);
        init::cleanup(window);
        return status;
    }

    bool start_sim = false;
    bool recording = !opts.recordDir.empty();
    int recordedFrames = 0;
    if(recording){
        start_sim = true;
        glfwSwapInterval(0);
    }

    int substeps = 5;
    int index = 0;

    // Per-frame timings shown in the UI. GPU timer queries are double buffered so that
    // reading last frame's result never stalls the pipeline.
    unsigned int timerQueries[2];
    glGenQueries(2, timerQueries);
    int queryFrame = 0;
    double solverMs = 0.0, renderCpuMs = 0.0, gpuDrawMs = 0.0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // If the spacebar is pressed, start the simulation
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            start_sim = true;
            std::cout << "Spacebar pressed" << std::endl;
        }

        // Process input
        camera.processKeyboardInput(window);

        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = 0.0166; // 1/60
        last_time = currentFrame;

        // Add substeps
        Cloth.timings = PrimitiveTimings();
        Ground.timings = PrimitiveTimings();
        auto solveStart = Clock::now();
        if(start_sim){
            float substep_dt = deltaTime / substeps;
            for (int i = 0; i < substeps; i++){
                // Scene[0].update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                // Cube.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                // Ground.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                Cloth.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
                // Box.update(substep_dt, glm::vec3(0.0f, gravity, 0.0f));
            }
            // Cube.update(deltaTime, glm::vec3(0.0f, gravity, 0.0f));
        }
        solverMs = elapsedMs(solveStart, Clock::now());
        // listen for a ctrl event
        if (io.KeyCtrl){
            std::cout << "ctrl pressed" << std::endl;
            //Close the window
            glfwSetWindowShouldClose(window, true);
        }

        // Render fps
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("FPS");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Particles: %zu  Constraints: %zu", Cloth.numParticles(), Cloth.numConstraints());
            ImGui::Text("Solver (CPU): %.3f ms", solverMs);
            ImGui::Text("Normals + upload (CPU): %.3f ms", renderCpuMs);
            ImGui::Text("Draw (GPU): %.3f ms", gpuDrawMs);

                // Add a slider for substeps
            // ImGui::Begin("Substeps");
            ImGui::SliderInt("Substeps", &substeps, 1, 10);
            ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);
            ImGui::End();
        }
   
        // Color for each object in the scene
        {
            ImGui::Begin("Color");
            ImGui::SliderInt("Object", &index, 0, Scene.size()-1);
            ImGui::ColorEdit4("Ambient", glm::value_ptr(Cloth.ambient));
            ImGui::ColorEdit4("Diffuse", glm::value_ptr(Cloth.diffuse));
            ImGui::ColorEdit4("Specular", glm::value_ptr(Cloth.specular));
            ImGui::SliderFloat("Shininess", &Cloth.shininess, 0.0f, 256.0f);
            ImGui::SliderFloat("Ambient Strength", &Cloth.ambient_strength, 0.0f, 1.0f);
            ImGui::SliderFloat("Diffuse Strength", &Cloth.diffuse_strength, 0.0f, 1.0f);
            ImGui::SliderFloat("Specular Strength", &Cloth.specular_strength, 0.0f, 1.0f);
            ImGui::End();
        }
        
        // ImGui::End();
        

        

        // Rendering commands here
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Render the cube
        glBeginQuery(GL_TIME_ELAPSED, timerQueries[queryFrame % 2]);
        program.use();
        model = glm::mat4(1.0f);
        // model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        program.setMat4("model", model);
        program.setMat4("view", camera.getViewMatrix());
        program.setMat4("projection", camera.getProjectionMatrix(height, width));
        program.setVec4("ambientColor", Cloth.ambient);
        program.setVec4("diffuseColor", Cloth.diffuse);
        program.setVec4("specularColor", Cloth.specular);
        program.setFloat("shininess", Cloth.shininess);
        program.setFloat("ambientStrength", Cloth.ambient_strength);
        program.setFloat("diffuseStrength", Cloth.diffuse_strength);
        program.setFloat("specularStrength", Cloth.specular_strength);
        program.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 0.0f));
        program.setVec3("viewPos", camera.getCameraPos());
        // Scene[0].render();
        // Cube.render();
        Cloth.render();
        // Box.render();

        //Make a ground plane with cube
        // glBindVertexArray(g_VAO);
        model = glm::mat4(1.0f);

        program.setMat4("model", model);
        program.setVec4("ambientColor", Scene[0].ambient);
        program.setVec4("diffuseColor", Scene[0].diffuse);
        program.setVec4("specularColor", Scene[0].specular);
        program.setFloat("shininess", Scene[0].shininess);
        program.setFloat("ambientStrength", Scene[0].ambient_strength);
        program.setFloat("diffuseStrength", Scene[0].diffuse_strength);
        program.setFloat("specularStrength", Scene[0].specular_strength);
        program.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 0.0f));
        program.setVec3("viewPos", camera.getCameraPos());
        // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // Scene[1].render();
        Ground.render();
        glEndQuery(GL_TIME_ELAPSED);

        renderCpuMs = Cloth.timings.normalsMs + Cloth.timings.uploadMs + Ground.timings.normalsMs + Ground.timings.uploadMs;
        if(queryFrame > 0){
            unsigned int prev = timerQueries[(queryFrame + 1) % 2];
            GLint available = 0;
            glGetQueryObjectiv(prev, GL_QUERY_RESULT_AVAILABLE, &available);
            if(available){
                GLuint64 ns = 0;
                glGetQueryObjectui64v(prev, GL_QUERY_RESULT, &ns);
                gpuDrawMs = ns / 1.0e6;
            }
        }
        queryFrame++;

        // Make a dot at the center of the screen
        // program.setVec4("color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        // glPointSize(10.0f);
        // glDrawArrays(GL_POINTS, 0, 1);

     
        // Render imgui
        ImGui::Render();
        // std::cout << "looping" << std::endl;
        if(recording){
            // Save every other frame (30 fps at the fixed 1/60 s simulation step), without the UI
            if(queryFrame % 2 == 0){
                char name[32];
                snprintf(name, sizeof(name), "/frame_%04d.png", recordedFrames++);
                saveFrame(window, opts.recordDir + name);
            }
            if(queryFrame >= opts.frames){
                glfwSetWindowShouldClose(window, true);
            }
        }
        else{
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
		glfwSwapBuffers(window);

    }

    glDeleteQueries(2, timerQueries);
    init::cleanup(window);

    // Release cloth vertices and indices
    cloth_vertices.clear();
    cloth_indices.clear();

    return 0;
}


// Callback function for mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    // std::cout << "xpos: " << xpos << " ypos: " << ypos << std::endl;
    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;
    camera.processMouseInput(window, xoffset, yoffset);
}

// Callback function for screen resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    width = width;
    height = height;
    glViewport(0, 0, width, height);
}

Options parseArgs(int argc, char** argv){
    Options opts;
    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];
        bool hasValue = i + 1 < argc;
        if(arg == "--benchmark") opts.benchmark = true;
        else if(arg == "--csv" && hasValue) opts.csv = argv[++i];
        else if(arg == "--record" && hasValue) opts.recordDir = argv[++i];
        else if(arg == "--frames" && hasValue) opts.frames = std::stoi(argv[++i]);
        else{
            std::cerr << "Unknown or incomplete argument: " << arg << "\n"
                      << "Usage: PBDynamics [--benchmark [--csv file] [--frames N]] [--record dir [--frames N]]" << std::endl;
            exit(1);
        }
    }
    return opts;
}

// Set the material uniforms of a primitive and draw it (same uniforms as the main loop)
void drawPrimitive(Shader& program, Primitive& prim){
    program.setMat4("model", glm::mat4(1.0f));
    program.setVec4("ambientColor", prim.ambient);
    program.setVec4("diffuseColor", prim.diffuse);
    program.setVec4("specularColor", prim.specular);
    program.setFloat("shininess", prim.shininess);
    program.setFloat("ambientStrength", prim.ambient_strength);
    program.setFloat("diffuseStrength", prim.diffuse_strength);
    program.setFloat("specularStrength", prim.specular_strength);
    prim.render();
}

// Read back the current back buffer and write it as a PNG
void saveFrame(GLFWwindow* window, const std::string& path){
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    std::vector<unsigned char> pixels(w * h * 3), flipped(w * h * 3);
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    // OpenGL's origin is bottom-left; PNG rows go top-down
    for(int y = 0; y < h; y++){
        std::copy(pixels.begin() + y * w * 3, pixels.begin() + (y + 1) * w * 3, flipped.begin() + (h - 1 - y) * w * 3);
    }
    stbi_write_png(path.c_str(), w, h, 3, flipped.data(), w * 3);
}

double mean(const std::vector<double>& v){
    double s = 0.0;
    for(double x : v) s += x;
    return s / v.size();
}

double percentile(std::vector<double> v, double p){
    std::sort(v.begin(), v.end());
    size_t idx = (size_t)std::ceil(p / 100.0 * v.size()) - 1;
    return v[std::min(idx, v.size() - 1)];
}

// Sweeps cloth resolution x substeps with the same fixed time step as the interactive mode.
// For every frame it records:
//   - solver: CPU wall time of all substeps (split into integrate and constraint solve)
//   - normals / upload: CPU time to recompute normals and re-upload the vertex buffer
//   - gpu draw: GL_TIME_ELAPSED timer query around the cloth + ground draw calls
//   - frame: wall time from the start of the simulation step until glFinish() returns
int runBenchmark(GLFWwindow* window, Shader& program, Primitive& ground, const Options& opts){
    const int gridSizes[] = {16, 32, 64, 128, 256};
    const int substepCounts[] = {1, 5, 10};
    const int warmup = 20;
    const float dt = 0.0166f;

    std::ofstream csv(opts.csv);
    if(!csv){
        std::cerr << "Could not open " << opts.csv << " for writing" << std::endl;
        return 1;
    }
    csv << "grid,particles,triangles,constraints,substeps,frames,"
           "solver_mean_ms,solver_p50_ms,solver_p95_ms,integrate_mean_ms,constraints_mean_ms,"
           "normals_mean_ms,upload_mean_ms,gpu_draw_mean_ms,frame_mean_ms,frame_p50_ms,frame_p95_ms\n";

    glfwSwapInterval(0);
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    unsigned int query;
    glGenQueries(1, &query);

    for(int n : gridSizes){
        Primitive cloth = makeClothGrid(n, n);
        cloth.ambient = glm::vec4(178.f/255.f, 115.f/255.f, 37.f/255.f, 1.0f);
        cloth.ambient_strength = 0.539f;
        cloth.diffuse = glm::vec4(183.f/255.f, 101.f/255.f, 101.f/255.f, 1.0f);
        cloth.diffuse_strength = 0.8f;

        for(int substeps : substepCounts){
            std::vector<double> solver, integrate, constraints, normals, upload, gpu, frame;
            for(int f = 0; f < warmup + opts.frames; f++){
                glfwPollEvents();
                cloth.timings = PrimitiveTimings();
                ground.timings = PrimitiveTimings();

                auto t0 = Clock::now();
                for(int i = 0; i < substeps; i++){
                    cloth.update(dt / substeps, glm::vec3(0.0f, gravity, 0.0f));
                }
                auto t1 = Clock::now();

                glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                program.use();
                program.setMat4("view", camera.getViewMatrix());
                program.setMat4("projection", camera.getProjectionMatrix(height, width));
                program.setVec3("lightPos", glm::vec3(1.0f, 1.0f, 0.0f));
                program.setVec3("viewPos", camera.getCameraPos());
                glBeginQuery(GL_TIME_ELAPSED, query);
                drawPrimitive(program, cloth);
                drawPrimitive(program, ground);
                glEndQuery(GL_TIME_ELAPSED);
                glFinish();
                auto t2 = Clock::now();

                GLuint64 ns = 0;
                glGetQueryObjectui64v(query, GL_QUERY_RESULT, &ns);
                glfwSwapBuffers(window); // show progress; not part of the timed region

                if(f < warmup) continue;
                solver.push_back(elapsedMs(t0, t1));
                integrate.push_back(cloth.timings.integrateMs);
                constraints.push_back(cloth.timings.constraintsMs);
                normals.push_back(cloth.timings.normalsMs + ground.timings.normalsMs);
                upload.push_back(cloth.timings.uploadMs + ground.timings.uploadMs);
                gpu.push_back(ns / 1.0e6);
                frame.push_back(elapsedMs(t0, t2));
            }

            char row[512];
            snprintf(row, sizeof(row), "%d,%zu,%zu,%zu,%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f",
                     n, cloth.numParticles(), cloth.numTriangles(), cloth.numConstraints(), substeps, opts.frames,
                     mean(solver), percentile(solver, 50), percentile(solver, 95), mean(integrate), mean(constraints),
                     mean(normals), mean(upload), mean(gpu), mean(frame), percentile(frame, 50), percentile(frame, 95));
            csv << row << "\n";
            std::cout << row << std::endl;
        }
    }
    std::cout << "renderer: " << glGetString(GL_RENDERER) << std::endl;
    glDeleteQueries(1, &query);
    return 0;
}
