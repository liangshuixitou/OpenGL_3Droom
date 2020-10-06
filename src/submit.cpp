/**************************************************************************

Author: �����

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
 * �ص�����ר��
 * 1. ���ڴ�С�ı�
 * 2. ��ѭ���н��д����������
 * 3. ���̼�������
 * 4. ��갴����������
 * 5. ����ƶ���������
 * 6. �����ּ�������
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
// ������غ���
// funct: ������·����Ϊ�������룬�ɷ���sample2D���͵������ַ
// ------------------------------------------------------------------------
unsigned int loadTexture(const char *path);

// ------------------------------------------------------------------------
// ����������ת����
// funct: ������ת�������������Լ���ת�Ƕȣ����ɸ�����ת����
// ------------------------------------------------------------------------
glm::mat4 RotateArbitraryLine(glm::vec3 v1, glm::vec3 v2, float theta);

// ------------------------------------------------------------------------
// ��VAO����
// funct: ����Ӧ�Ķ�������󶨶�Ӧ��VAO
// ------------------------------------------------------------------------
void bindVAO(unsigned int *VBO, unsigned int *VAO, float vertices[], int size);

// ------------------------------------------------------------------------
// ��ɫ��Shader��
// ��GLSL������д����ɫ��������ж����װ��������ʹ��
// ------------------------------------------------------------------------
class Shader
{
public:
    unsigned int ID;
    // ��ɫ�����캯��
    // ------------------------------------------------------------------------
    Shader(const char *vertexSource, const char *fragmentSource)
    {

        // ������ɫ������
        unsigned int vertexShader, fragmentShader;

        // ������ɫ��
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        // Ƭ����ɫ��
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        // ��ɫ������
        ID = glCreateProgram();
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // ���Ѿ����ӵ���ɫ������
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    // ������ɫ��
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // ���ֲ�ͬ������GLSL������uniform�����ĺ���
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
    // ��������ɫ��Դ��������еı����������
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

// ��ʼ��Ļ��С
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// ����������ɫ������
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

// �ƹⶥ����ɫ������
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

// ����Ƭ����ɫ������
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
                                   "    // ��������ɫ\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // �������ɫ\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // �ϲ����\n"
                                   "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
                                   "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
                                   "    return (ambient + diffuse + specular);\n"
                                   "}\n"
                                   "vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)\n"
                                   "{\n"
                                   "    vec3 lightDir = normalize(light.position - fragPos);\n"
                                   "    // ��������ɫ\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // �������ɫ\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // ˥��\n"
                                   "    float distance = length(light.position - fragPos);\n"
                                   "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
                                   "    // �ϲ����\n"
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
                                   "    // ��������ɫ\n"
                                   "    float diff = max(dot(normal, lightDir), 0.0);\n"
                                   "    // �������ɫ\n"
                                   "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                                   "    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
                                   "    // ˥��\n"
                                   "    float distance = length(light.position - fragPos);\n"
                                   "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
                                   "    // �۹�\n"
                                   "    float theta = dot(lightDir, normalize(-light.direction));\n"
                                   "    float epsilon = light.innerCutOff - light.outerCutOff;\n"
                                   "    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
                                   "    // �ϲ����\n"
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
                                   "    // ����\n"
                                   "    vec3 norm = normalize(Normal);\n"
                                   "    vec3 viewDir = normalize(viewPos - FragPos);\n"
                                   "    // �������\n"
                                   "    // vec3 result = CalcDirLight(dirLight, norm, viewDir);\n"
                                   "    vec3 result = vec3(0.0f);\n"
                                   "    // ���Դ\n"
                                   "    for(int i = 0; i < NR_POINT_LIGHTS; i++)\n"
                                   "        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
                                   "    // �۹�\n"
                                   "    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);\n"
                                   "    FragColor = vec4(result, 1.0f);\n"
                                   "}\0";

// �ƹ�Ƭ����ɫ������
const char *lightFragmentShaderSource = "#version 330 core\n"
                                        "out vec4 FragColor;"
                                        "uniform vec3 lightColor;"
                                        "void main()"
                                        "{"
                                        "   FragColor = vec4(lightColor, 1.0f);"
                                        "}\0";

// ------------------------------------------------------------------------
// Ϊ�˷�����������¼��������ٴ˶���һЩȫ�ֱ��������û����е�һЩ����
// ------------------------------------------------------------------------

// ����ƶ�������
// ------------------------------------------------------------------------
const float sensitivity = 0.05;

// 1. ������ƶ�����
// 2. �����λ��
// 3. ���������
// 4. ��������Ϸ���
// ------------------------------------------------------------------------
const float speed = 3.0f;
// ------------------------------------------------------------------------
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
// ------------------------------------------------------------------------
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
// ------------------------------------------------------------------------
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 1. ��ǰ֡����һ֡��ʱ���
// 2. ��һ֡��ʱ��
// ------------------------------------------------------------------------
float deltaTime = 0.0f;
// ------------------------------------------------------------------------
float lastFrame = 0.0f;

// 1. ����ʼλ��
// 2. ȫ��������Ƕ�
// 3. ����һ�μ�¼��ʱ��
// ------------------------------------------------------------------------
float lastX = SCR_WIDTH / 2, lastY = SCR_HEIGHT / 2;
// ------------------------------------------------------------------------
float yaw = -90.0f, pitch = 0;
// ------------------------------------------------------------------------
int firstMouse = 1;

// ͸��ͶӰ�����ӽǣ����ֵ���������Ƕ�
// ------------------------------------------------------------------------
float fov = 45.0f;

// �Ƿ�������·���
// ------------------------------------------------------------------------
bool isFPS = 1;

// 1. �ֵ�Ͳ����
// 2. �ֵ�Ͳ����ǿ��
// ------------------------------------------------------------------------
bool enSpotLight = false;
// ------------------------------------------------------------------------
glm::vec3 spotLight = glm::vec3(1.0f, 1.0f, 1.0f);

// ���Դ����
// ------------------------------------------------------------------------
float brightness = 0.6;

// ����ϳ����ľ���
// ------------------------------------------------------------------------
float chairOutDis = 0.0;

// �����ٶ�(һ��5��)
// ------------------------------------------------------------------------
float fanSpeed = 1.0f;
float lastTime = 0.0f;

// �л���(һ��5����)
// ------------------------------------------------------------------------
int pictureIndex = 0;

// 1. �����ŵ���ת�Ƕ�
// 2. �����ŵ���ת����
// ------------------------------------------------------------------------
float doorRotate = 0.0f;
// ------------------------------------------------------------------------
bool doorFont = 1;

// 1. ��ͷ��ת�Ƕ�
// 2. ��ͷ��ת����
// ------------------------------------------------------------------------
float pillowRotate = 0.0f;
// ------------------------------------------------------------------------
bool pillowFont = 1;

int main()
{
    // glfw: ��ʼ������
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ƻ������ϵͳ�����еĲ������ɺ���
    // ------------------------------
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw: ��������
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Room", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ע��ص�����
    // ---------------------------------------
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: ���صײ�OpenGL����ָ��
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ����ȫ�ֱ���
    // ������Ȳ���
    // ������GammaУ��
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    // ����Shader��(��װ�õ���ɫ������)
    Shader *cubeProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *lightProgram = new Shader(lightVertexShaderSource, lightFragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *floorProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *wallProgram = new Shader(vertexShaderSource, fragmentShaderSource);
    // ------------------------------------------------------------------------
    Shader *innerWallProgram = new Shader(vertexShaderSource, fragmentShaderSource);

    // �����嶥������
    // ------------------------------------------------------------------
    float vertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
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

    // �ذ嶥������
    // ------------------------------------------------------------------
    float floorVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 15.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 15.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 15.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 15.0f};

    // ��ǽ�ڶ�������
    // ------------------------------------------------------------------
    float wallVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
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

    // ��ǽ�ڶ�������
    // ------------------------------------------------------------------
    float innerWallVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
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

    // ������װ�λ���������
    // ------------------------------------------------------------------
    float pictureVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // �Ŷ�������
    // ------------------------------------------------------------------
    float doorVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // �����涥������(ת���˷��߷���)
    // ------------------------------------------------------------------
    float doorBackVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};

    // �Ŷ����߿򶥵�����
    // ------------------------------------------------------------------
    float doorTopFrameVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, -0.5f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 4.0f, -0.5f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

    // �Ŷ����߿����涥������
    // ------------------------------------------------------------------
    float doorTopFrameBackVertices[] = {
        // ---- λ�� ----    ---- ���� ----     --- ���� ---
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, -0.5f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 4.0f, -0.5f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, -0.5f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};

    // ������������λ��
    // ----------------------------
    glm::vec3 cubePositions[] = {
        glm::vec3(8.0f, 1.8f, 8.0f),
        glm::vec3(-8.0f, 1.8f, -8.0f),
        glm::vec3(-8.0f, 1.8f, 8.0f),
        glm::vec3(8.0f, 1.8f, -8.0f)};

    // �������������еƹ�λ��
    // ----------------------------
    glm::vec3 pointLightPositions[] = {
        // ���ڵ���
        // ----------------------------
        glm::vec3(0.0f, 2.8f, 0.0f),

        // ����������
        // ----------------------------
        glm::vec3(8.0f, 2.8f, 8.0f),
        glm::vec3(-8.0f, 2.8f, -8.0f),
        glm::vec3(-8.0f, 2.8f, 8.0f),
        glm::vec3(8.0f, 2.8f, -8.0f),

        // �ſ�С������
        // ----------------------------
        glm::vec3(3.2f, 1.8f, 5.0f),

        // ����Ȧװ�ε�
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

        // ����Ȧװ�ε�
        // ----------------------------
        glm::vec3(4.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, -4.0f + 6.0f, -30.0f),
        glm::vec3(-4.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, 4.0f + 6.0f, -30.0f),
        glm::vec3(2.828f, 2.828f + 6.0f, -30.0f),
        glm::vec3(-2.828f, 2.828f + 6.0f, -30.0f),
        glm::vec3(2.828f, -2.828f + 6.0f, -30.0f),
        glm::vec3(-2.828f, -2.828f + 6.0f, -30.0f),

        // �δ���Ȧװ�ε�
        // ----------------------------
        glm::vec3(2.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, -2.0f + 6.0f, -30.0f),
        glm::vec3(-2.0f, 0.0f + 6.0f, -30.0f),
        glm::vec3(0.0f, 2.0f + 6.0f, -30.0f),

        // ����Ȧװ�ε�
        // ----------------------------
        glm::vec3(0.0f, 6.0f, -30.0f)};

    // �����еƹ�����任
    // ----------------------------
    glm::vec3 pointLightVolumes[] = {
        glm::vec3(0.4f, 0.2f, 0.4f)};

    // ������Ҷλ�ñ任
    // ----------------------------
    glm::vec3 fanPositions[] = {
        glm::vec3(0.6f, 2.8f, 0.0f),
        glm::vec3(0.0f, 2.8f, 0.6f),
        glm::vec3(-0.6f, 2.8f, 0.0f),
        glm::vec3(0.0f, 2.8f, -0.6f)};

    // ������Ҷ����任
    // ----------------------------
    glm::vec3 fanVolumes[] = {
        glm::vec3(0.8f, 0.03f, 0.3f),
        glm::vec3(0.3f, 0.03f, 0.8f),
        glm::vec3(0.8f, 0.03f, 0.3f),
        glm::vec3(0.3f, 0.03f, 0.8f)};

    // ����ľ��λ�ñ任
    // ----------------------------
    glm::vec3 tablePositions[] = {
        glm::vec3(3.35f, 0.5f, 0.0f),
        glm::vec3(3.35f, 0.0f, 0.9f),
        glm::vec3(3.35f, 0.0f, -0.9f)};

    // ����ľ������任
    // ----------------------------
    glm::vec3 tableVolumes[] = {
        glm::vec3(1.2f, 0.05f, 1.75f),
        glm::vec3(1.2f, 1.05f, 0.05f),
        glm::vec3(1.2f, 1.05f, 0.05f)};

    // ����ľ��λ�ñ任
    // ----------------------------
    glm::vec3 chairPositions[] = {
        // ���Ӱ���
        // ----------------------------
        glm::vec3(2.8f, 0.18f, -0.3f),

        // ��������
        // ----------------------------
        glm::vec3(2.425f, 0.15f, -0.625f),
        glm::vec3(2.425f, 0.15f, 0.025f),

        // ǰ������
        // ----------------------------
        glm::vec3(3.175f, -0.16f, -0.625f),
        glm::vec3(3.175f, -0.16f, 0.025f),

        // ������������
        // ----------------------------
        glm::vec3(2.8f, -0.18f, -0.625f),
        glm::vec3(2.8f, -0.18f, 0.025f),

        // ǰ�����һ��
        // ----------------------------
        glm::vec3(3.175f, -0.18f, -0.3f),

        // ��������
        // ----------------------------
        glm::vec3(2.425f, 0.925f, -0.3f),
        glm::vec3(2.425f, 0.7f, -0.3f)};

    // ����ľ������任
    // �������λ�ñ任һһ��Ӧ
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

    // ��ľ��λ�ñ任
    // ----------------------------
    glm::vec3 bedPositions[] = {
        glm::vec3(-2.59f, -0.3f, -0.7f),
        glm::vec3(-3.9f, 0.00f, -0.7f)};

    // ����ľ������任
    // ----------------------------
    glm::vec3 bedVolumes[] = {
        glm::vec3(2.8f, 0.4f, 1.8f),
        glm::vec3(0.1f, 1.2f, 1.8f)};

    // �������嶥����������
    // --------------------------
    unsigned int cubeVBO, cubeVAO;
    bindVAO(&cubeVBO, &cubeVAO, vertices, sizeof(vertices));

    // �󶨵ƹⶥ����������
    // --------------------------
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // �󶨵ذ嶥����������
    // --------------------------
    unsigned int floorVBO, floorVAO;
    bindVAO(&floorVBO, &floorVAO, floorVertices, sizeof(floorVertices));

    // ��ǽ�ڶ�����������
    // --------------------------
    unsigned int wallVBO, wallVAO;
    bindVAO(&wallVBO, &wallVAO, wallVertices, sizeof(wallVertices));

    // ����ǽ�ڶ�����������
    // --------------------------
    unsigned int innerWallVBO, innerWallVAO;
    bindVAO(&innerWallVBO, &innerWallVAO, innerWallVertices, sizeof(innerWallVertices));

    // ��ͼƬ������������
    // --------------------------
    unsigned int pictureVBO, pictureVAO;
    bindVAO(&pictureVBO, &pictureVAO, pictureVertices, sizeof(pictureVertices));

    // ���Ŷ�����������
    // --------------------------
    unsigned int doorVBO, doorVAO;
    bindVAO(&doorVBO, &doorVAO, doorVertices, sizeof(doorVertices));

    // ���ű��涥����������
    // --------------------------
    unsigned int doorBackVBO, doorBackVAO;
    bindVAO(&doorBackVBO, &doorBackVAO, doorBackVertices, sizeof(doorBackVertices));

    // ���ſ��ϲ�������������
    // --------------------------
    unsigned int doorTopFrameVBO, doorTopFrameVAO;
    bindVAO(&doorTopFrameVBO, &doorTopFrameVAO, doorTopFrameVertices, sizeof(doorTopFrameVertices));

    // ���ſ��ϲ����涥����������
    // --------------------------
    unsigned int doorTopFrameBackVBO, doorTopFrameBackVAO;
    bindVAO(&doorTopFrameBackVBO, &doorTopFrameBackVAO, doorTopFrameBackVertices, sizeof(doorTopFrameBackVertices));

    // ��������
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

    // �¼���ѭ��
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // �����������֮֡���ʱ��
        // ------------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // �����������
        // ------------------------
        lastTime = lastTime + deltaTime * fanSpeed;

        // ���������¼��Ĵ���
        // ------------------------
        processInput(window);

        // ���������������Ӧ������
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

        // ѭ�����
        // --------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ������������س���
        // -----------------------------
        cubeProgram->use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        // �����ֵ�Ͳ��Դ
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

        // ����ֱ�չ�Դ
        // -----------------------------
        cubeProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        cubeProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        cubeProgram->setVec3("dirLight.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        cubeProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // ���õ��Դ
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

        // ���ò����Լ��۲��
        // -----------------------------
        cubeProgram->setVec3("viewPos", cameraPos);
        cubeProgram->setInt("material.diffuse", 0);
        cubeProgram->setInt("material.specular", 1);
        cubeProgram->setFloat("material.shininess", 64.0f);

        // ����ģ�ͱ任����, �۲����, ͸��ͶӰ����
        // ----------------------------------------
        cubeProgram->setMat4("model", model);
        cubeProgram->setMat4("view", view);
        cubeProgram->setMat4("projection", projection);

        // ������Χ�ĸ�ľ����
        // -----------------------------
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 4; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ��������
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

        // ��������
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

        // ���ƴ�
        // -----------------------------
        for (int i = 0; i < 2; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, bedPositions[i]);
            model = glm::scale(model, bedVolumes[i]); // Make it a smaller cube
            cubeProgram->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ������ͷ
        // -----------------------------
        cubeProgram->setInt("material.diffuse", 14);
        cubeProgram->setInt("material.specular", 14);
        model = glm::mat4(1.0f);
        model = RotateArbitraryLine(glm::vec3(-3.625f, -0.1f, 0.0f), glm::vec3(-3.625f, -0.1f, 1.0f), pillowRotate);
        model = glm::translate(model, glm::vec3(-3.7f, -0.025f, -0.7f));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.6f)); // Make it a smaller cube
        cubeProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ���Ƶ�����Ҷ
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

        // �����ƹ���س���
        // -----------------------------
        lightProgram->use();

        // ����ģ�ͱ任����, �۲����, ͸��ͶӰ����
        // ----------------------------------------
        model = glm::mat4(1.0f);
        lightProgram->setMat4("view", view);
        lightProgram->setMat4("projection", projection);
        lightProgram->setVec3("lightColor", glm::vec3(0.9648f, 0.9297f, 0.8359f) * glm::vec3(brightness));

        // ���Ƶƹ�
        // ----------------------------------------
        glBindVertexArray(lightVAO);
        for (unsigned int i = 0; i <= 30; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            glm::mat4 route;

            // ������Χ�ĸ��ƹ�
            // ----------------------------------------
            if (i > 0 && i < 6)
            {
                if (i != 5)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
                model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            }

            // ���Ƶ���
            // ----------------------------------------
            else if (i == 0)
            {
                model = glm::scale(model, pointLightVolumes[i]); // Make it a smaller cube
                route = RotateArbitraryLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.1f, 0.0f), lastTime);
                model = route * model;
            }

            // ������Ȧװ�εƹ�
            // ----------------------------------------
            else if (i > 5 && i < 18)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), lastFrame);
                model = route * model;
            }

            // ���ƴ���Ȧװ�εƹ�
            // ----------------------------------------
            else if (i >= 18 && i < 26)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), -0.8 * lastFrame);
                model = route * model;
            }

            // ���ƴδ���Ȧװ�εƹ�
            // ----------------------------------------
            else if (i >= 26 && i < 30)
            {
                model = glm::scale(model, glm::vec3(0.4f));
                route = RotateArbitraryLine(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, 1.0f), 0.6 * lastFrame);
                model = route * model;
            }

            // ��������װ�εƹ�
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

        // ����ƽ����س���
        // ----------------------------------------
        floorProgram->use();

        glBindVertexArray(floorVAO);

        // ���ֵ�Ͳ�ƹ�Դ
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

        // ��ֱ���ƹ�Դ
        // ----------------------------------------
        floorProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        floorProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        floorProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        floorProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // �󶨵��Դ
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

        // ����ģ�ͱ任����, �۲����, ͸��ͶӰ����
        // ----------------------------------------
        floorProgram->setVec3("viewPos", cameraPos);
        floorProgram->setInt("material.diffuse", 2);
        floorProgram->setInt("material.specular", 2);
        floorProgram->setFloat("material.shininess", 4.0f);

        // ������ݵ���
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 14.5f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f));

        floorProgram->setMat4("model", model);
        floorProgram->setMat4("view", view);
        floorProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ��������ͼƬ
        // ----------------------------------------
        glBindVertexArray(pictureVAO);
        floorProgram->setInt("material.diffuse", pictureIndex + 7);
        floorProgram->setInt("material.specular", pictureIndex + 7);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.3f, -3.4f));
        model = glm::scale(model, glm::vec3(3.2f, 1.8f, 1.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ����ͼƬ�߿�
        // ----------------------------------------
        floorProgram->setInt("material.diffuse", 6);
        floorProgram->setInt("material.specular", 6);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.3f, -3.4005f));
        model = glm::scale(model, glm::vec3(3.65f, 2.15f, 1.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ������
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

        // �����ű���
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

        // �����ű߿�
        // ----------------------------------------
        glBindVertexArray(doorVAO);
        floorProgram->setInt("material.diffuse", 3);
        floorProgram->setInt("material.specular", 3);

        // ��������߿�
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.25f, 0.5, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // �������ұ߿�
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.25f, 0.5, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // �����ű߿�����
        // ----------------------------------------
        glBindVertexArray(doorBackVAO);
        floorProgram->setInt("material.diffuse", 3);
        floorProgram->setInt("material.specular", 3);

        // ������߿�����
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.25f, 0.5, -0.0001f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // �����ұ߿�����
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.25f, 0.5, -0.0001f));
        model = glm::scale(model, glm::vec3(1.5f, 3.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // �������ϱ߿�
        // ----------------------------------------
        glBindVertexArray(doorTopFrameVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.5f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 1.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // �������ϱ߿�����
        // ----------------------------------------
        glBindVertexArray(doorTopFrameBackVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.5f, -0.0001f));
        model = glm::scale(model, glm::vec3(8.0f, 1.0f, 8.0f));
        floorProgram->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ����ǽ�ڼ��س���
        // ----------------------------------------
        wallProgram->use();

        glBindVertexArray(wallVAO);

        // �����ֵ�Ͳ��Դ
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

        // ����ֱ���Դ
        // ----------------------------------------
        wallProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        wallProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        wallProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        wallProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // ���õ��Դ
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

        // ����ģ�ͱ任����, �۲����, ͸��ͶӰ����
        // ----------------------------------------
        wallProgram->setVec3("viewPos", cameraPos);
        wallProgram->setInt("material.diffuse", 3);
        wallProgram->setInt("material.specular", 3);
        wallProgram->setFloat("material.shininess", 4.0f);

        // ������ǽ��
        // ----------------------------------------
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 4.0f, 8.0f));

        wallProgram->setMat4("model", model);
        wallProgram->setMat4("view", view);
        wallProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // ������ǽ�ڼ��س���
        // ----------------------------------------
        innerWallProgram->use();

        glBindVertexArray(innerWallVAO);

        // �����ֵ�Ͳ��Դ
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

        // ����ֱ���Դ
        // ----------------------------------------
        innerWallProgram->setVec3("dirLight.direction", glm::vec3(0.0f, 1.0f, 0.0f));
        innerWallProgram->setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        innerWallProgram->setVec3("dirLight.diffuse", glm::vec3(0.9f, 0.9f, 0.9f));
        innerWallProgram->setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

        // ���õ��Դ
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

        // ����ģ�ͱ任����, �۲����, ͸��ͶӰ����
        // ----------------------------------------
        innerWallProgram->setVec3("viewPos", cameraPos);
        innerWallProgram->setInt("material.diffuse", 4);
        innerWallProgram->setInt("material.specular", 4);
        innerWallProgram->setFloat("material.shininess", 4.0f);

        // ���ƹ�������ǽ��
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(7.9f, 3.95f, 7.9f));

        innerWallProgram->setMat4("model", model);
        innerWallProgram->setMat4("view", view);
        innerWallProgram->setMat4("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        // glfw: ��������������ˢ��
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
 * @method ���̼����ص�����
 * @description ���ڰ����л������Լ�����ɨ�裬ʹ���ͷŰ����¼���Ϊtrigger�����¼�
 */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // �л�����ģʽ
    // -------------------------------------------------
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        isFPS = !isFPS;
        if (isFPS)
            std::cout << "����~" << std::endl;
        else
            std::cout << "�ߺ���ɣ�����" << std::endl;
    }

    // �л�ͼƬ
    // -------------------------------------------------
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
    {
        pictureIndex = (pictureIndex + 1) % 5;
        std::cout << "��һ��ͼƬ" << std::endl;
    }
    if (key == GLFW_KEY_C && action == GLFW_RELEASE)
    {
        pictureIndex = (pictureIndex == 0) ? 4 : pictureIndex - 1;
        std::cout << "��һ��ͼƬ" << std::endl;
    }

    // �����ֿ���
    // -------------------------------------------------
    if (key == GLFW_KEY_X && action == GLFW_RELEASE)
    {
        doorFont = !doorFont;
        if (doorFont)
            std::cout << "�ֿ��ſ���" << std::endl;
        else
            std::cout << "�ֿ��ǹر�" << std::endl;
    }

    // �л���ͷ����
    // -------------------------------------------------
    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
        pillowFont = !pillowFont;
        std::cout << "��ͷ���ҹ�������" << std::endl;
    }
}

/**
 * @method ��갴����������
 * @description ������갴��, ����л��ֵ�Ͳ�ƹ�(����)
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        enSpotLight = !enSpotLight;
        if (enSpotLight)
            std::cout << "�����ֵ�Ͳ" << std::endl;
        else
            std::cout << "�ر��ֵ�Ͳ" << std::endl;
    }
}

/**
 * @method ��ѭ�����¼�������
 * @description ���ڴ���ÿ��ѭ���е��¼���
 */
void processInput(GLFWwindow *window)
{

    // �˳�����
    // ---------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // ������ƶ�
    // W ǰ��
    // S ����
    // A ����
    // D ����
    // left shift ����
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

    // UP �ϵ��ƹ�����
    // -------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && brightness <= 1.0)
        brightness += 0.002;

    // DOWN �µ��ƹ�����
    // -------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && brightness >= 0.0)
        brightness -= 0.002;

    // Q �����ƶ�����
    // --------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && chairOutDis <= 1.0)
        chairOutDis += 0.003;

    // E �����ƶ�����
    // --------------------------------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && chairOutDis >= 0.0)
        chairOutDis -= 0.003;

    // �����ȵ���
    // 1 2 3 4 5 ���ٵ�λ
    // 0 ����ֹͣ
    // ----------------------------------------------
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (fanSpeed != 1.0f)
            std::cout << "��~" << std::endl;
        fanSpeed = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (fanSpeed != 2.0f)
            std::cout << "����~~" << std::endl;
        fanSpeed = 2.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (fanSpeed != 3.0f)
            std::cout << "������~~~" << std::endl;
        fanSpeed = 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (fanSpeed != 4.0f)
            std::cout << "��������~~~~" << std::endl;
        fanSpeed = 4.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        if (fanSpeed != 5.0f)
            std::cout << "����������~~~~~" << std::endl;
        fanSpeed = 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        if (fanSpeed != 0.0f)
            std::cout << "www~" << std::endl;
        fanSpeed = 0.0f;
    }

    // ���������������е��¼�
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
 * @method �������¼�
 * @description ��ŷ������������������ӽ�����
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
 * @method ������ּ����¼�
 * @description �������������������
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
 * @method ������غ���
 * @description ������·����Ϊ�������룬�ɷ���sample2D���͵������ַ
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
 * @method ����������ת����
 * @description ������ת�������������Լ���ת�Ƕȣ����ɸ�����ת����
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
 * @method ��VAO����
 * @description ����Ӧ�Ķ�������󶨶�Ӧ��VAO
 */
void bindVAO(unsigned int *VBO, unsigned int *VAO, float vertices[], int size)
{
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // ��λ������
    // --------------------------------------------------------------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // �󶨷�������
    // --------------------------------------------------------------------------
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ����������
    // --------------------------------------------------------------------------
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}
