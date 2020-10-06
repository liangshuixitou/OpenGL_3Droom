/**************************************************************************

Author: 李辉阳

StudentID: 161820230

Date: 2020-09-16

Description: Computer Graphics Creating a 3D Room 

**************************************************************************/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>

/*************************************************************** 
 * 回调函数专区
 * 1. 窗口大小改变
 * 2. 主循环中进行处理的主函数
 * 3. 键盘监听函数
 * 4. 鼠标按键监听函数
 * 5. 鼠标移动监听函数
 * 6. 鼠标滚轮监听函数
****************************************************************/
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// ------------------------------------------------------------------------
void processInput(GLFWwindow *window);
// ------------------------------------------------------------------------
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
// ------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
// ------------------------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// ------------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// ------------------------------------------------------------------------
// 纹理加载函数
// funct: 将纹理路径作为参数传入，可返回sample2D类型的纹理地址
// ------------------------------------------------------------------------
unsigned int loadTexture(const char *path);

// ------------------------------------------------------------------------
// 绕任意轴旋转函数
// funct: 给定旋转轴上任意两点以及旋转角度，即可给出旋转矩阵
// ------------------------------------------------------------------------
glm::mat4 RotateArbitraryLine(glm::vec3 v1, glm::vec3 v2, float theta);

// ------------------------------------------------------------------------
// 绑定VAO函数
// funct: 给对应的顶点数组绑定对应的VAO
// ------------------------------------------------------------------------
void bindVAO(unsigned int *VBO, unsigned int *VAO, float vertices[], int size);

// ------------------------------------------------------------------------
// 着色器Shader类
// 将GLSL语言书写的着色器代码进行对象封装，并进行使用
// ------------------------------------------------------------------------
class Shader
{
public:
    unsigned int ID;
    // 着色器构造函数
    // ------------------------------------------------------------------------
    Shader(const char *vertexSource, const char *fragmentSource)
    {

        // 编译着色器代码
        unsigned int vertexShader, fragmentShader;

        // 顶点着色器
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        // 片段着色器
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        // 着色器程序
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // 将已经连接的着色器闪出
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    // 激活着色器
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // 几种不同的设置GLSL代码中uniform变量的函数
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, glm::vec3 value) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

private:
    // 检查编译着色器源代码过程中的编译出错问题
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

// 初始屏幕大小
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// 基础顶点着色器代码
// ------------------------------------------------------------------------
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 vPos;"
                                 "layout (location = 1) in vec3 vNormal;"
                                 "layout (location = 2) in vec2 vTexCoord;"
                                 "out vec3 Normal;"
                                 "out vec3 FragPos;"
                                 "out vec2 TexCoords;"
                                 "uniform mat4 model;"
                                 "uniform mat4 view;"
                                 "uniform mat4 projection;"
                                 "void main()"
                                 "{"
                                 "   gl_Position = projection * view * model * vec4(vPos, 1.0f);"
                                 "   FragPos = vec3(model * vec4(vPos, 1.0f));"
                                 "   Normal = mat3(transpose(inverse(model))) * vNormal;"
                                 "   TexCoords = vTexCoord;"
                                 "}\0";

// 灯光顶点着色器代码
// ------------------------------------------------------------------------
const char *lightVertexShaderSource = "#version 330 core\n"
                                      "layout (location = 0) in vec3 vPos;"
                                      "uniform mat4 model;"
                                      "uniform mat4 view;"
                                      "uniform mat4 projection;"
                                      "void main()"
                                      "{"
                                      "   gl_Position = projection * view * model * vec4(vPos, 1.0f);"
                                      "}\0";

// 基础片段着色器代码
// ------------------------------------------------------------------------
const char *fragmentShaderSource = "#version 330 core\n"
                                   "in vec3 Normal;\n"
                                   "in vec3 FragPos;\n"
                                   "in vec2 TexCoords;\n"
                                   "out vec4 FragColor;\n"
                                   "struct Material {\n"
                                   "   sampler2D diffuse;\n"
                                   "   sampler2D specular;\n"
                                   "   float shininess;\n"
                                   "};\n"
                                   "struct DirLight {\n"
                                   "    vec3 direction;\n"
                                   "    vec3 ambient;\n"
                                   "    vec3 diffuse;\n"
                                   "    vec3 specular;\n"
                                   "};\n"
                                   "struct PointLight {\n"
                                   "    vec3 position;\n"
                                   "    float constant;\n"
                                   "    float linear;\n"
                                   "    float quadratic;\n"
                                   "    vec3 ambient;\n"
                                   "    vec3 diffuse;\n"
                                   "    vec3 specular;\n"
                                   "};\n"
                                   "struct SpotLight {\n"
                                   "    vec3 position;\n"
                                   "    vec3 direction;\n"
                                   "    float innerCutOff;\n"
                                   "    float outerCutOff;\n"
                                   "    float constant;\n"
                                   "    float linear;\n"
                                   "    float quadratic;\n"
                                   "    vec3 ambient;\n"
                                   "    vec3 diffuse;\n"
                                   "    vec3 specular;\n"
                                   "};\n"
                                   "uniform DirLight dirLight;\n"
                                   "uniform SpotLight spotLight;\n"
                                   "#define NR_POINT_LIGHTS 6\n"
                                   "uniform PointLight pointLights[NR_POINT_LIGHTS];\n"
                                   "uniform Material material;\n"
                                   "uniform vec3 viewPos;\n"
                                   "vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)\n"
                                   "{\n"
                                   "    vec3 lightDir = normalize(-light.direction);\n"
                                   "    // 漫反射着色\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // 镜面光着色\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // 合并结果\n"
                                   "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
                                   "    return (ambient + diffuse + specular);\n"
                                   "}\n"
                                   "vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
                                   "{\n"
                                   "    vec3 lightDir = normalize(light.position - fragPos);\n"
                                   "    // 漫反射着色\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // 镜面光着色\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // 衰减\n"
                                   "    float distance = length(light.position - fragPos);\n"
                                   "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
                                   "    // 合并结果\n"
                                   "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
                                   "    ambient *= attenuation;\n"
                                   "    diffuse *= attenuation;\n"
                                   "    specular *= attenuation;\n"
                                   "    return (ambient + diffuse + specular);\n"
                                   "}\n"
                                   "vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
                                   "{\n"
                                   "    vec3 lightDir = normalize(light.position - fragPos);\n"
                                   "    // 漫反射着色\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // 镜面光着色\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // 衰减\n"
                                   "    float distance = length(light.position - fragPos);\n"
                                   "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
                                   "    // 聚光\n"
                                   "    float theta = dot(lightDir, normalize(-light.direction));\n"
                                   "    float epsilon = light.innerCutOff - light.outerCutOff;\n"
                                   "    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
                                   "    // 合并结果\n"
                                   "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
                                   "    ambient *= attenuation;\n"
                                   "    diffuse *= attenuation * intensity;\n"
                                   "    specular *= attenuation * intensity;\n"
                                   "    return (ambient + diffuse + specular);\n"
                                   "}\n"
                                   "void main()\n"
                                   "{"
                                   "    // 属性\n"
                                   "    vec3 norm = normalize(Normal);\n"
                                   "    vec3 viewDir = normalize(viewPos - FragPos);\n"
                                   "    // 定向光照\n"
                                   "    // vec3 result = CalcDirLight(dirLight, norm, viewDir);\n"
                                   "    vec3 result = vec3(0.0f);\n"
                                   "    // 点光源\n"
                                   "    for(int i = 0; i < NR_POINT_LIGHTS; i++)\n"
                                   "        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
                                   "    // 聚光\n"
                                   "    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);\n"
                                   "    FragColor = vec4(result, 1.0f);\n"
                                   "}\0";

// 灯光片段着色器代码
const char *lightFragmentShaderSource = "#version 330 core\n"
                                        "out vec4 FragColor;"
                                        "uniform vec3 lightColor;"
                                        "void main()"
                                        "{"
                                        "   FragColor = vec4(lightColor, 1.0f);"
                                        "}\0";

// ------------------------------------------------------------------------
// 为了方便监听流程事件，我们再此定义一些全局变量来设置环境中的一些参数
// ------------------------------------------------------------------------

// 鼠标移动灵敏度
// ------------------------------------------------------------------------
const float sensitivity = 0.05;

// 1. 摄像机移动速率
// 2. 摄像机位置
// 3. 摄像机朝向
// 4. 摄像机正上方向
// ------------------------------------------------------------------------
const float speed = 3.0f;
// ------------------------------------------------------------------------
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
// ------------------------------------------------------------------------
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
// ------------------------------------------------------------------------
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 1. 当前帧与上一帧的时间差
// 2. 上一帧的时间
// ------------------------------------------------------------------------
float deltaTime = 0.0f;
// ------------------------------------------------------------------------
float lastFrame = 0.0f;

// 1. 鼠标初始位置
// 2. 全局摄像机角度
// 3. 鼠标第一次记录的时候
// ------------------------------------------------------------------------
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
// ------------------------------------------------------------------------
float yaw = -90.0f, pitch = 0;
// ------------------------------------------------------------------------
int firstMouse = 1;

// 透视投影矩阵视角，滚轮调整摄像机角度
// ------------------------------------------------------------------------
float fov = 45.0f;

// 是否可以上下飞行
// ------------------------------------------------------------------------
bool isFPS = 1;

// 1. 手电筒开关
// 2. 手电筒关照强度
// ------------------------------------------------------------------------
bool enSpotLight = false;
// ------------------------------------------------------------------------
glm::vec3 spotLight = glm::vec3(1.0f, 1.0f, 1.0f);

// 点光源亮度
// ------------------------------------------------------------------------
float brightness = 0.6;

// 板凳拖出来的距离
// ------------------------------------------------------------------------
float chairOutDis = 0.0;

// 风扇速度(一共5档)
// ------------------------------------------------------------------------
float fanSpeed = 1.0f;
float lastTime = 0.0f;

// 切换画(一共5幅画)
// ------------------------------------------------------------------------
int pictureIndex = 0;

// 1. 升降门的旋转角度
// 2. 升降门的旋转方向
// ------------------------------------------------------------------------
float doorRotate = 0.0f;
// ------------------------------------------------------------------------
bool doorFont = 1;

// 1. 枕头旋转角度
// 2. 枕头旋转方向
// ------------------------------------------------------------------------
float pillowRotate = 0.0f;
// ------------------------------------------------------------------------
bool pillowFont = 1;

int main()
{
    // glfw: 初始化操作
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 苹果操作系统所特有的操作，可忽略
    // ------------------------------
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw: 创建窗口
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Room", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 注册回调函数
    // ---------------------------------------
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: 加载底层OpenGL函数指针
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 配置全局变量
    // 开启深度测试
    // 不开启Gamma校正
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    // 创建Shader类(封装好的着色器程序)
    Shader *cubeProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *lightProgram = new Shader(lightVertexShaderSource, lightFragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *floorProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *wallProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *innerWallProgram = new Shader(vertexShaderSource, fragmentShaderSource);

    // 立方体顶点数据
    // ------------------------------------------------------------------
    float vertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    // 地板顶点数据
    // ------------------------------------------------------------------
    float floorVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 15.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 15.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 15.0f};

    // 外墙壁顶点数据
    // ------------------------------------------------------------------
    float wallVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 3.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 3.0f, 3.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 3.0f, 3.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 3.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 3.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 3.0f, 3.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 3.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 3.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 3.0f};

    // 内墙壁顶点数据
    // ------------------------------------------------------------------
    float innerWallVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 3.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 3.0f, 3.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 3.0f, 3.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 3.0f, 0.0f,

        0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 0.0f,
        0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 3.0f,
        0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 3.0f,
        0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 3.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 3.0f,
        0.5f, 0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 3.0f, 3.0f,
        0.5f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 3.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 3.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 3.0f};

    // 房间内装饰画顶点数据
    // ------------------------------------------------------------------
    float pictureVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // 门顶点数据
    // ------------------------------------------------------------------
    float doorVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // 门内面顶点数据(转换了法线方向)
    // ------------------------------------------------------------------
    float doorBackVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};

    // 门顶部边框顶点数据
    // ------------------------------------------------------------------
    float doorTopFrameVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, -0.5f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, -0.5f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // 门顶部边框内面顶点数据
    // ------------------------------------------------------------------
    float doorTopFrameBackVertices[] = {
        // ---- 位置 ----    ---- 法向 ----     --- 纹理 ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, -0.5f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, -0.5f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};

    // 房屋四周箱子位置
    // ----------------------------
    glm::vec3 cubePositions[] = {
        glm::vec3(8.0f, 1.8f, 8.0f),
        glm::vec3(-8.0f, 1.8f, -8.0f),
        glm::vec3(-8.0f, 1.8f, 8.0f),
        glm::vec3(8.0f, 1.8f, -8.0f)};

    // 整个世界中所有灯光位置
    // ----------------------------
    glm::vec3 pointLightPositions[] = {
        // 室内吊灯
        // ----------------------------
        glm::vec3(0.0f, 2.8f, 0.0f),

        // 四周悬浮灯
        // ----------------------------
        glm::vec3(8.0f, 2.8f, 8.0f),
        glm::vec3(-8.0f, 2.8f, -8.0f),
        glm::vec3(-8.0f, 2.8f, 8.0f),
        glm::vec3(8.0f, 2.8f, -8.0f),

        // 门口小悬浮灯
        // ----------------------------
        glm::vec3(3.2f, 1.8f, 5.0f),

        // 最外圈装饰灯
        // ----------------------------
        glm::vec3(6.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, -6.0f + 6.0f, -30.0f),
        glm::vec3(-6.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, 6.0f + 6.0f, -30.0f),
        glm::vec3(3.0f, 5.196f + 6.0f, -30.0f),
        glm::vec3(5.196f, 3.0f + 6.0f, -30.0f),
        glm::vec3(-3.0f, -5.196f + 6.0f, -30.0f),
        glm::vec3(-5.196f, -3.0f + 6.0f, -30.0f),
        glm::vec3(-3.0f, 5.196f + 6.0f, -30.0f),
        glm::vec3(-5.196f, 3.0f + 6.0f, -30.0f),
        glm::vec3(3.0f, -5.196f + 6.0f, -30.0f),
        glm::vec3(5.196f, -3.0f + 6.0f, -30.0f),

        // 次外圈装饰灯
        // ----------------------------
        glm::vec3(4.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, -4.0f + 6.0f, -30.0f),
        glm::vec3(-4.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, 4.0f + 6.0f, -30.0f),
        glm::vec3(2.828f, 2.828f + 6.0f, -30.0f),
        glm::vec3(-2.828f, 2.828f + 6.0f, -30.0f),
        glm::vec3(2.828f, -2.828f + 6.0f, -30.0f),
        glm::vec3(-2.828f, -2.828f + 6.0f, -30.0f),

        // 次次外圈装饰灯
        // ----------------------------
        glm::vec3(2.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, -2.0f + 6.0f, -30.0f),
        glm::vec3(-2.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, 2.0f + 6.0f, -30.0f),

        // 最内圈装饰灯
        // ----------------------------
        glm::vec3(0.0f, 6.0f, -30.0f)};

    // 房屋中灯光体积变换
    // ----------------------------
    glm::vec3 pointLightVolumes[] = {
        glm::vec3(0.4f, 0.2f, 0.4f)};

    // 风扇扇叶位置变换
    // ----------------------------
    glm::vec3 fanPositions[] = {
        glm::vec3(0.6f, 2.8f, 0.0f),
        glm::vec3(0.0f, 2.8f, 0.6f),
        glm::vec3(-0.6f, 2.8f, 0.0f),
        glm::vec3(0.0f, 2.8f, -0.6f)};

    // 风扇扇叶体积变换
    // ----------------------------
    glm::vec3 fanVolumes[] = {
        glm::vec3(0.8f, 0.03f, 0.3f),
        glm::vec3(0.3f, 0.03f, 0.8f),
        glm::vec3(0.8f, 0.03f, 0.3f),
        glm::vec3(0.3f, 0.03f, 0.8f)};

    // 桌子木板位置变换
    // ----------------------------
    glm::vec3 tablePositions[] = {
        glm::vec3(3.35f, 0.5f, 0.0f),
        glm::vec3(3.35f, 0.0f, 0.9f),
        glm::vec3(3.35f, 0.0f, -0.9f)};

    // 桌子木板体积变换
    // ----------------------------
    glm::vec3 tableVolumes[] = {
        glm::vec3(1.2f, 0.05f, 1.75f),
        glm::vec3(1.2f, 1.05f, 0.05f),
        glm::vec3(1.2f, 1.05f, 0.05f)};

    // 椅子木板位置变换
    // ----------------------------
    glm::vec3 chairPositions[] = {
        // 凳子板面
        // ----------------------------
        glm::vec3(2.8f, 0.18f, -0.3f),

        // 背后两根
        // ----------------------------
        glm::vec3(2.425f, 0.15f, -0.625f),
        glm::vec3(2.425f, 0.15f, 0.025f),

        // 前面两根
        // ----------------------------
        glm::vec3(3.175f, -0.16f, -0.625f),
        glm::vec3(3.175f, -0.16f, 0.025f),

        // 底下左右两根
        // ----------------------------
        glm::vec3(2.8f, -0.18f, -0.625f),
        glm::vec3(2.8f, -0.18f, 0.025f),

        // 前面底下一根
        // ----------------------------
        glm::vec3(3.175f, -0.18f, -0.3f),

        // 靠背两根
        // ----------------------------
        glm::vec3(2.425f, 0.925f, -0.3f),
        glm::vec3(2.425f, 0.7f, -0.3f)};

    // 椅子木板体积变换
    // 与上面的位置变换一一对应
    // ----------------------------
    glm::vec3 chairVolumes[] = {
        glm::vec3(0.8f, 0.05f, 0.7f),
        glm::vec3(0.05f, 1.6f, 0.05f),
        glm::vec3(0.05f, 1.6f, 0.05f),
        glm::vec3(0.05f, 0.68f, 0.05f),
        glm::vec3(0.05f, 0.68f, 0.05f),
        glm::vec3(0.8f, 0.05f, 0.05f),
        glm::vec3(0.8f, 0.05f, 0.05f),
        glm::vec3(0.05f, 0.05f, 0.7f),
        glm::vec3(0.05f, 0.05f, 0.7f),
        glm::vec3(0.05f, 0.05f, 0.7f)};

    // 床木板位置变换
    // ----------------------------
    glm::vec3 bedPositions[] = {
        glm::vec3(-2.59f, -0.3f, -0.7f),
        glm::vec3(-3.9f, 0.00f, -0.7f)};

    // 椅子木板体积变换
    // ----------------------------
    glm::vec3 bedVolumes[] = {
        glm::vec3(2.8f, 0.4f, 1.8f),
        glm::vec3(0.1f, 1.2f, 1.8f)};

    // 绑定立方体顶点属性数据
    // --------------------------
    unsigned int cubeVBO, cubeVAO;
    bindVAO(&cubeVBO, &cubeVAO, vertices, sizeof(vertices));

    // 绑定灯光顶点属性数据
    // --------------------------
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 绑定地板顶点属性数据
    // --------------------------
    unsigned int floorVBO, floorVAO;
    bindVAO(&floorVBO, &floorVAO, floorVertices, sizeof(floorVertices));

    // 绑定墙壁顶点属性数据
    // --------------------------
    unsigned int wallVBO, wallVAO;
    bindVAO(&wallVBO, &wallVAO, wallVertices, sizeof(wallVertices));

    // 绑定内墙壁顶点属性数据
    // --------------------------
    unsigned int innerWallVBO, innerWallVAO;
    bindVAO(&innerWallVBO, &innerWallVAO, innerWallVertices, sizeof(innerWallVertices));

    // 绑定图片顶点属性数据
    // --------------------------
    unsigned int pictureVBO, pictureVAO;
    bindVAO(&pictureVBO, &pictureVAO, pictureVertices, sizeof(pictureVertices));

    // 绑定门顶点属性数据
    // --------------------------
    unsigned int doorVBO, doorVAO;
    bindVAO(&doorVBO, &doorVAO, doorVertices, sizeof(doorVertices));

    // 绑定门背面顶点属性数据
    // --------------------------
    unsigned int doorBackVBO, doorBackVAO;
    bindVAO(&doorBackVBO, &doorBackVAO, doorBackVertices, sizeof(doorBackVertices));

    // 绑定门框上部顶点属性数据
    // --------------------------
    unsigned int doorTopFrameVBO, doorTopFrameVAO;
    bindVAO(&doorTopFrameVBO, &doorTopFrameVAO, doorTopFrameVertices, sizeof(doorTopFrameVertices));

    // 绑定门框上部背面顶点属性数据
    // --------------------------
    unsigned int doorTopFrameBackVBO, doorTopFrameBackVAO;
    bindVAO(&doorTopFrameBackVBO, &doorTopFrameBackVAO, doorTopFrameBackVertices, sizeof(doorTopFrameBackVertices));

    // 加载纹理
    // ----------------------------------------------------------------------------
    cubeProgram->use();
    unsigned int texture1 = loadTexture("resources\\textures\\container2.png");
    unsigned int texture2 = loadTexture("resources\\textures\\container2_specular.png");
    unsigned int tableTexture = loadTexture("resources\\textures\\table.jpg");
    unsigned int pillowTexture = loadTexture("resources\\textures\\pillow.jpg");
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    floorProgram->use();
    unsigned int floorTexture = loadTexture("resources\\textures\\floor2.jpg");
    unsigned int pictureFrameTexture = loadTexture("resources\\textures\\frame.jpg");
    unsigned int pictureTexture1 = loadTexture("resources\\textures\\picture1.jpg");
    unsigned int pictureTexture2 = loadTexture("resources\\textures\\picture2.jpg");
    unsigned int pictureTexture3 = loadTexture("resources\\textures\\picture3.jpg");
    unsigned int pictureTexture4 = loadTexture("resources\\textures\\picture4.jpg");
    unsigned int pictureTexture5 = loadTexture("resources\\textures\\picture5.jpg");
    unsigned int doorTexture = loadTexture("resources\\textures\\door.jpg");
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    wallProgram->use();
    unsigned int wallTexture = loadTexture("resources\\textures\\brickwall.jpg");
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    innerWallProgram->use();
    unsigned int innerWallTexture = loadTexture("resources\\textures\\innerwall2.jpg");
    // ----------------------------------------------------------------------------

    // 事件主循环
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // 计算摄像机两帧之间的时间
        // ------------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 计算风扇速率
        // ------------------------
        lastTime = lastTime + deltaTime * fanSpeed;

        // 对于输入事件的处理
        // ------------------------
        processInput(window);

        // 激活纹理绑定纹理到对应数字区
        // -----------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, innerWallTexture);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, tableTexture);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, pictureFrameTexture);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, pictureTexture1);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, pictureTexture2);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, pictureTexture3);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, pictureTexture4);
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, pictureTexture5);
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, doorTexture);
        glActiveTexture(GL_TEXTURE14);
        glBindTexture(GL_TEXTURE_2D, pillowTexture);

        // 循环清空
        // --------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 启动立方体加载程序
        // -----------------------------
        cubeProgram->use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        // 配置手电筒光源
        // -----------------------------
        cubeProgram->setVec3("spotLight.position", cameraPos);
        cubeProgram->setVec3("spotLight.direction", cameraFront);
        cubeProgram->setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        cubeProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        cubeProgram->setFloat("spotLight.constant", 1.0f);
        cubeProgram->setFloat("spotLight.linear", 0.09f);
        cubeProgram->setFloat("spotLight.quadratic", 0.02f);
        cubeProgram->setVec3("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        cubeProgram->setVec3("spotLight.diffuse", spotLight);
        cubeProgram->setVec3("spotLight.specular", spotLight);

        // 配置直照光源
        // -----------------------------
        cubeProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        cubeProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        cubeProgram->setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        cubeProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 配置点光源
        // -----------------------------
        for (int i = 0; i < 6; ++i)
        {
            std::string index = std::to_string(i);
            cubeProgram->setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
            cubeProgram->setFloat("pointLights[" + index + "].constant", 1.0f);
            cubeProgram->setFloat("pointLights[" + index + "].linear", 0.09f);
            cubeProgram->setFloat("pointLights[" + index + "].quadratic", 0.02f);
            cubeProgram->setVec3("pointLights[" + index + "].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
            cubeProgram->setVec3("pointLights[" + index + "].diffuse", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness * 0.5));
            cubeProgram->setVec3("pointLights[" + index + "].specular", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));
        }

        // 配置材质以及观察角
        // -----------------------------
        cubeProgram->setVec3("viewPos", cameraPos);
        cubeProgram->setInt("material.diffuse", 0);
        cubeProgram->setInt("material.specular", 1);
        cubeProgram->setFloat("material.shininess", 64.0f);

        // 配置模型变换矩阵, 观察矩阵, 透视投影矩阵
        // ----------------------------------------
        cubeProgram->setMat4("model", model);
        cubeProgram->setMat4("view", view);
        cubeProgram->setMat4("projection", projection);

        // 绘制周围四个木箱子
        // -----------------------------
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 4; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制桌子
        // -----------------------------
        cubeProgram->setInt("material.diffuse", 5);
        cubeProgram->setInt("material.specular", 5);

        for (int i = 0; i < 3; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, tablePositions[i]);
            model = glm::scale(model, tableVolumes[i]); // Make it a smaller cube
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制椅子
        // -----------------------------
        for (int i = 0; i < 10; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-0.7f * chairOutDis, 0.0f, 0.0f));
            model = glm::translate(model, chairPositions[i]);
            model = glm::scale(model, chairVolumes[i]); // Make it a smaller cube
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制床
        // -----------------------------
        for (int i = 0; i < 2; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, bedPositions[i]);
            model = glm::scale(model, bedVolumes[i]); // Make it a smaller cube
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 绘制枕头
        // -----------------------------
        cubeProgram->setInt("material.diffuse", 14);
        cubeProgram->setInt("material.specular", 14);
        model = glm::mat4(1.0f);
        model = RotateArbitraryLine(glm::vec3(-3.625f, -0.1f, 0.0f), glm::vec3(-3.625f, -0.1f, 1.0f), pillowRotate);
        model = glm::translate(model, glm::vec3(-3.7f, -0.025f, -0.7f));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.6f)); // Make it a smaller cube
        cubeProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 绘制吊扇扇叶
        // -----------------------------
        glBindVertexArray(cubeVAO);
        cubeProgram->setInt("material.diffuse", 4);
        cubeProgram->setInt("material.specular", 4);
        glm::mat4 route;
        for (int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, fanPositions[i]);
            model = glm::scale(model, fanVolumes[i]); // Make it a smaller cube
            route = RotateArbitraryLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.1f, 0.0f), lastTime);
            model = route * model;
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 启动灯光加载程序
        // -----------------------------
        lightProgram->use();

        // 配置模型变换矩阵, 观察矩阵, 透视投影矩阵
        // ----------------------------------------
        model = glm::mat4(1.0f);
        lightProgram->setMat4("view", view);
        lightProgram->setMat4("projection", projection);
        lightProgram->setVec3("lightColor", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));

        // 绘制灯光
        // ----------------------------------------
        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i <= 30; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            glm::mat4 route;

            // 绘制周围四个灯光
            // ----------------------------------------
            if (i > 0 && i < 6)
            {
                if (i != 5)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            }

            // 绘制吊灯
            // ----------------------------------------
            else if (i == 0)
            {
                model = glm::scale(model, pointLightVolumes[i]); // Make it a smaller cube
                route = RotateArbitraryLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.1f, 0.0f), lastTime);
                model = route * model;
            }

            // 绘制外圈装饰灯光
            // ----------------------------------------
            else if (i > 5 && i < 18)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), lastFrame);
                model = route * model;
            }

            // 绘制次外圈装饰灯光
            // ----------------------------------------
            else if (i >= 18 && i < 26)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), -0.8 * lastFrame);
                model = route * model;
            }

            // 绘制次次外圈装饰灯光
            // ----------------------------------------
            else if (i >= 26 && i < 30)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), 0.6 * lastFrame);
                model = route * model;
            }

            // 绘制中心装饰灯光
            // ----------------------------------------
            else if (i == 30)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), -0.4 * lastFrame);
                model = route * model;
            }

            lightProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // 启动平面加载程序
        // ----------------------------------------
        floorProgram->use();

        glBindVertexArray(floorVAO);

        // 绑定手电筒灯光源
        // ----------------------------------------
        floorProgram->setVec3("spotLight.position", cameraPos);
        floorProgram->setVec3("spotLight.direction", cameraFront);
        floorProgram->setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        floorProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        floorProgram->setFloat("spotLight.constant", 1.0f);
        floorProgram->setFloat("spotLight.linear", 0.09f);
        floorProgram->setFloat("spotLight.quadratic", 0.02f);
        floorProgram->setVec3("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        floorProgram->setVec3("spotLight.diffuse", spotLight);
        floorProgram->setVec3("spotLight.specular", spotLight);

        // 绑定直射光灯光源
        // ----------------------------------------
        floorProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        floorProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        floorProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        floorProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 绑定点光源
        // ----------------------------------------
        for (int i = 0; i < 6; ++i)
        {
            std::string index = std::to_string(i);
            floorProgram->setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
            floorProgram->setFloat("pointLights[" + index + "].constant", 1.0f);
            floorProgram->setFloat("pointLights[" + index + "].linear", 0.09f);
            floorProgram->setFloat("pointLights[" + index + "].quadratic", 0.02f);
            floorProgram->setVec3("pointLights[" + index + "].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
            floorProgram->setVec3("pointLights[" + index + "].diffuse", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness * 0.5));
            floorProgram->setVec3("pointLights[" + index + "].specular", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));
        }

        // 配置模型变换矩阵, 观察矩阵, 透视投影矩阵
        // ----------------------------------------
        floorProgram->setVec3("viewPos", cameraPos);
        floorProgram->setInt("material.diffuse", 2);
        floorProgram->setInt("material.specular", 2);
        floorProgram->setFloat("material.shininess", 4.0f);

        // 绘制青草地面
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 14.5f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f));

        floorProgram->setMat4("model", model);
        floorProgram->setMat4("view", view);
        floorProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制屋内图片
        // ----------------------------------------
        glBindVertexArray(pictureVAO);
        floorProgram->setInt("material.diffuse", pictureIndex + 7);
        floorProgram->setInt("material.specular", pictureIndex + 7);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.3f, -3.4f));
        model = glm::scale(model, glm::vec3(3.2f, 1.8f, 1.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制图片边框
        // ----------------------------------------
        floorProgram->setInt("material.diffuse", 6);
        floorProgram->setInt("material.specular", 6);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.3f, -3.4005f));
        model = glm::scale(model, glm::vec3(3.65f, 2.15f, 1.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门
        // ----------------------------------------
        glBindVertexArray(doorVAO);
        floorProgram->setInt("material.diffuse", 12);
        floorProgram->setInt("material.specular", 12);

        model = glm::mat4(1.0f);
        model = RotateArbitraryLine(glm::vec3(0.0f, 2.0f, 4.0f), glm::vec3(1.0f, 2.0f, 4.0f), doorRotate);
        model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
        model = glm::scale(model, glm::vec3(5.0f, 2.5f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门背面
        // ----------------------------------------
        glBindVertexArray(doorBackVAO);
        floorProgram->setInt("material.diffuse", 12);
        floorProgram->setInt("material.specular", 12);

        model = glm::mat4(1.0f);
        model = RotateArbitraryLine(glm::vec3(0.0f, 2.0f, 4.0f), glm::vec3(1.0f, 2.0f, 4.0f), doorRotate);
        model = glm::translate(model, glm::vec3(0.0f, 0.75f, -0.0001f));
        model = glm::scale(model, glm::vec3(5.0f, 2.5f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门边框
        // ----------------------------------------
        glBindVertexArray(doorVAO);
        floorProgram->setInt("material.diffuse", 3);
        floorProgram->setInt("material.specular", 3);

        // 绘制门左边框
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.25f, 0.5, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门右边框
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.25f, 0.5, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门边框内面
        // ----------------------------------------
        glBindVertexArray(doorBackVAO);
        floorProgram->setInt("material.diffuse", 3);
        floorProgram->setInt("material.specular", 3);

        // 绘制左边框内面
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.25f, 0.5, -0.0001f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制右边框内面
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.25f, 0.5, -0.0001f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门上边框
        // ----------------------------------------
        glBindVertexArray(doorTopFrameVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.5f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 1.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 绘制门上边框内面
        // ----------------------------------------
        glBindVertexArray(doorTopFrameBackVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.5f, -0.0001f));
        model = glm::scale(model, glm::vec3(8.0f, 1.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 启动墙壁加载程序
        // ----------------------------------------
        wallProgram->use();

        glBindVertexArray(wallVAO);

        // 配置手电筒光源
        // ----------------------------------------
        wallProgram->setVec3("spotLight.position", cameraPos);
        wallProgram->setVec3("spotLight.direction", cameraFront);
        wallProgram->setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        wallProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        wallProgram->setFloat("spotLight.constant", 1.0f);
        wallProgram->setFloat("spotLight.linear", 0.09f);
        wallProgram->setFloat("spotLight.quadratic", 0.02f);
        wallProgram->setVec3("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        wallProgram->setVec3("spotLight.diffuse", spotLight);
        wallProgram->setVec3("spotLight.specular", spotLight);

        // 配置直射光源
        // ----------------------------------------
        wallProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        wallProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        wallProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        wallProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 配置点光源
        // ----------------------------------------
        for (int i = 0; i < 6; ++i)
        {
            std::string index = std::to_string(i);
            wallProgram->setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
            wallProgram->setFloat("pointLights[" + index + "].constant", 1.0f);
            wallProgram->setFloat("pointLights[" + index + "].linear", 0.09f);
            wallProgram->setFloat("pointLights[" + index + "].quadratic", 0.02f);
            wallProgram->setVec3("pointLights[" + index + "].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
            wallProgram->setVec3("pointLights[" + index + "].diffuse", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness * 0.5));
            wallProgram->setVec3("pointLights[" + index + "].specular", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));
        }

        // 配置模型变换矩阵, 观察矩阵, 透视投影矩阵
        // ----------------------------------------
        wallProgram->setVec3("viewPos", cameraPos);
        wallProgram->setInt("material.diffuse", 3);
        wallProgram->setInt("material.specular", 3);
        wallProgram->setFloat("material.shininess", 4.0f);

        // 绘制外墙面
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 4.0f, 8.0f));

        wallProgram->setMat4("model", model);
        wallProgram->setMat4("view", view);
        wallProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // 启动内墙壁加载程序
        // ----------------------------------------
        innerWallProgram->use();

        glBindVertexArray(innerWallVAO);

        // 配置手电筒光源
        // ----------------------------------------
        innerWallProgram->setVec3("spotLight.position", cameraPos);
        innerWallProgram->setVec3("spotLight.direction", cameraFront);
        innerWallProgram->setFloat("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        innerWallProgram->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        innerWallProgram->setFloat("spotLight.constant", 1.0f);
        innerWallProgram->setFloat("spotLight.linear", 0.09f);
        innerWallProgram->setFloat("spotLight.quadratic", 0.02f);
        innerWallProgram->setVec3("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        innerWallProgram->setVec3("spotLight.diffuse", spotLight * glm::vec3(0.5));
        innerWallProgram->setVec3("spotLight.specular", spotLight * glm::vec3(0.5));

        // 配置直射光源
        // ----------------------------------------
        innerWallProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        innerWallProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        innerWallProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        innerWallProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // 配置点光源
        // ----------------------------------------
        for (int i = 0; i < 6; ++i)
        {
            std::string index = std::to_string(i);
            innerWallProgram->setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
            innerWallProgram->setFloat("pointLights[" + index + "].constant", 1.0f);
            innerWallProgram->setFloat("pointLights[" + index + "].linear", 0.09f);
            innerWallProgram->setFloat("pointLights[" + index + "].quadratic", 0.02f);
            innerWallProgram->setVec3("pointLights[" + index + "].ambient", glm::vec3(0.1f, 0.1f, 0.1f));
            innerWallProgram->setVec3("pointLights[" + index + "].diffuse", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness * 0.5));
            innerWallProgram->setVec3("pointLights[" + index + "].specular", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));
        }

        // 配置模型变换矩阵, 观察矩阵, 透视投影矩阵
        // ----------------------------------------
        innerWallProgram->setVec3("viewPos", cameraPos);
        innerWallProgram->setInt("material.diffuse", 4);
        innerWallProgram->setInt("material.specular", 4);
        innerWallProgram->setFloat("material.shininess", 4.0f);

        // 绘制硅藻泥内墙面
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(7.9f, 3.95f, 7.9f));

        innerWallProgram->setMat4("model", model);
        innerWallProgram->setMat4("view", view);
        innerWallProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // glfw: 交换缓冲区进行刷新
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &floorVAO);
    glDeleteVertexArrays(1, &wallVAO);
    glDeleteVertexArrays(1, &innerWallVAO);
    glDeleteVertexArrays(1, &pictureVAO);
    glDeleteVertexArrays(1, &doorVAO);
    glDeleteVertexArrays(1, &doorBackVAO);
    glDeleteVertexArrays(1, &doorTopFrameVAO);
    glDeleteVertexArrays(1, &doorTopFrameBackVAO);

    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &innerWallVBO);
    glDeleteBuffers(1, &pictureVBO);
    glDeleteBuffers(1, &doorVBO);
    glDeleteBuffers(1, &doorBackVBO);
    glDeleteBuffers(1, &doorTopFrameVBO);
    glDeleteBuffers(1, &doorTopFrameBackVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

/**
 * @method 键盘监听回调函数
 * @description 鉴于案件切换功能以及键盘扫描，使用释放按键事件作为trigger触发事件
 */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // 切换飞行模式
    // -------------------------------------------------
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        isFPS = !isFPS;
        if (isFPS)
            std::cout << "降落~" << std::endl;
        else
            std::cout << "芜湖起飞！！！" << std::endl;
    }

    // 切换图片
    // -------------------------------------------------
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
    {
        pictureIndex = (pictureIndex + 1) % 5;
        std::cout << "下一张图片" << std::endl;
    }
    if (key == GLFW_KEY_C && action == GLFW_RELEASE)
    {
        pictureIndex = (pictureIndex == 0) ? 4 : pictureIndex - 1;
        std::cout << "上一张图片" << std::endl;
    }

    // 开启仓库门
    // -------------------------------------------------
    if (key == GLFW_KEY_X && action == GLFW_RELEASE)
    {
        doorFont = !doorFont;
        if (doorFont)
            std::cout << "仓库门开启" << std::endl;
        else
            std::cout << "仓库们关闭" << std::endl;
    }

    // 切换枕头翻滚
    // -------------------------------------------------
    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
        pillowFont = !pillowFont;
        std::cout << "枕头给我滚！！！" << std::endl;
    }
}

/**
 * @method 鼠标按键监听函数
 * @description 监听鼠标按键, 左键切换手电筒灯光(开关)
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        enSpotLight = !enSpotLight;
        if (enSpotLight)
            std::cout << "开启手电筒" << std::endl;
        else
            std::cout << "关闭手电筒" << std::endl;
    }
}

/**
 * @method 主循环中事件处理函数
 * @description 用于处理每次循环中的事件流
 */
void processInput(GLFWwindow *window)
{

    // 退出窗口
    // ---------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 摄像机移动
    // W 前进
    // S 后退
    // A 左移
    // D 右移
    // left shift 加速
    // -------------------------------------------------------------------
    float cameraSpeed = deltaTime * speed; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    // UP 上调灯光亮度
    // -------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && brightness <= 1.0)
        brightness += 0.002;

    // DOWN 下调灯光亮度
    // -------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && brightness >= 0.0)
        brightness -= 0.002;

    // Q 向右移动椅子
    // --------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && chairOutDis <= 1.0)
        chairOutDis += 0.003;

    // E 向左移动椅子
    // --------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && chairOutDis >= 0.0)
        chairOutDis -= 0.003;

    // 给风扇调速
    // 1 2 3 4 5 风速档位
    // 0 风扇停止
    // ----------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (fanSpeed != 1.0f)
            std::cout << "呼~" << std::endl;
        fanSpeed = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (fanSpeed != 2.0f)
            std::cout << "呼呼~~" << std::endl;
        fanSpeed = 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (fanSpeed != 3.0f)
            std::cout << "呼呼呼~~~" << std::endl;
        fanSpeed = 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (fanSpeed != 4.0f)
            std::cout << "呼呼呼呼~~~~" << std::endl;
        fanSpeed = 4.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        if (fanSpeed != 5.0f)
            std::cout << "呼呼呼呼呼~~~~~" << std::endl;
        fanSpeed = 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        if (fanSpeed != 0.0f)
            std::cout << "www~" << std::endl;
        fanSpeed = 0.0f;
    }

    // 处理按键监听函数中的事件
    // ------------------------
    if (isFPS)
        cameraPos.y = 0.7f;
    if (enSpotLight)
        spotLight = glm::vec3(1.0f, 1.0f, 1.0f);
    else
        spotLight = glm::vec3(0.0f, 0.0f, 0.0f);

    if (doorRotate < 0.0 && doorFont)
    {
        doorRotate += deltaTime * 0.8;
    }
    else if (doorRotate > -1.57 && !doorFont)
    {
        doorRotate -= deltaTime * 0.8;
    }

    if (pillowRotate < 0.0 && pillowFont)
    {
        pillowRotate += deltaTime * 3.0;
    }
    else if (pillowRotate > -1.57 && !pillowFont)
    {
        pillowRotate -= deltaTime * 3.0;
    }
}

/**
 * @method 鼠标监听事件
 * @description 用欧拉角来处理摄像机的视角问题
 */
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

/**
 * @method 处理滚轮监听事件
 * @description 调整摄像机的视域问题
 */
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * @method 纹理加载函数
 * @description 将纹理路径作为参数传入，可返回sample2D类型的纹理地址
 */
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

/**
 * @method 绕任意轴旋转函数
 * @description 给定旋转轴上任意两点以及旋转角度，即可给出旋转矩阵
 */
glm::mat4 RotateArbitraryLine(glm::vec3 v1, glm::vec3 v2, float theta)
{
    glm::mat4 rmatrix;
    float a = v1.x;
    float b = v1.y;
    float c = v1.z;

    glm::vec3 p1 = v2 - v1;
    glm::vec3 p = glm::normalize(p1);

    float u = p.x;
    float v = p.y;
    float w = p.z;

    float uu = u * u;
    float uv = u * v;
    float uw = u * w;
    float vv = v * v;
    float vw = v * w;
    float ww = w * w;
    float au = a * u;
    float av = a * v;
    float aw = a * w;
    float bu = b * u;
    float bv = b * v;
    float bw = b * w;
    float cu = c * u;
    float cv = c * v;
    float cw = c * w;

    float costheta = glm::cos(theta);
    float sintheta = glm::sin(theta);

    rmatrix[0][0] = uu + (vv + ww) * costheta;
    rmatrix[0][1] = uv * (1 - costheta) + w * sintheta;
    rmatrix[0][2] = uw * (1 - costheta) - v * sintheta;
    rmatrix[0][3] = 0;

    rmatrix[1][0] = uv * (1 - costheta) - w * sintheta;
    rmatrix[1][1] = vv + (uu + ww) * costheta;
    rmatrix[1][2] = vw * (1 - costheta) + u * sintheta;
    rmatrix[1][3] = 0;

    rmatrix[2][0] = uw * (1 - costheta) + v * sintheta;
    rmatrix[2][1] = vw * (1 - costheta) - u * sintheta;
    rmatrix[2][2] = ww + (uu + vv) * costheta;
    rmatrix[2][3] = 0;

    rmatrix[3][0] = (a * (vv + ww) - u * (bv + cw)) * (1 - costheta) + (bw - cv) * sintheta;
    rmatrix[3][1] = (b * (uu + ww) - v * (au + cw)) * (1 - costheta) + (cu - aw) * sintheta;
    rmatrix[3][2] = (c * (uu + vv) - w * (au + bv)) * (1 - costheta) + (av - bu) * sintheta;
    rmatrix[3][3] = 1;

    return rmatrix;
}

/**
 * @method 绑定VAO函数
 * @description 给对应的顶点数组绑定对应的VAO
 */
void bindVAO(unsigned int *VBO, unsigned int *VAO, float vertices[], int size)
{
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // 绑定位置属性
    // --------------------------------------------------------------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 绑定法线属性
    // --------------------------------------------------------------------------
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 绑定纹理属性
    // --------------------------------------------------------------------------
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}
