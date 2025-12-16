#include "shader.h"

std::string Shader::read_file(const char* filename) {
    std::ifstream in;
    std::stringstream buffer;
    std::string shaderCode;

    try {
        in.open(filename);
        buffer << in.rdbuf();
        in.close();
        shaderCode = buffer.str();

    } catch(std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FAILED TO READ " << filename << std::endl;
    }

    return shaderCode;
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {

    // Retrieve the vertex and fragment source code from filepath
    std::string vertexCode = read_file(vertexPath);
    std::string fragmentCode = read_file(fragmentPath);
    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Compile shaders
    unsigned int vertexShader, fragmentShader;
    int success;
    char infoLog[512];
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);                // create shader
    glShaderSource(vertexShader, 1, &vertexSource, NULL);           // attach shader source code to shader
    glCompileShader(vertexShader);                                  // compile shader
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);            // create shader
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);       // attach shader source code to shader
    glCompileShader(fragmentShader);                                // compile shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // Link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    
    // check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(ID);
}
void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const {         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) const { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
