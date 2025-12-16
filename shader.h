#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  

class Shader {
    std::string read_file(const char* filename);

public:
    unsigned int ID;                                                // shader program ID
  
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    void use();                                                     // use/activate the shader
    
    // utility functions for uniform vars
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
};
  
#endif
