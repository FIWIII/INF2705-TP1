#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>               
#include <glm/vec3.hpp>              

using namespace gl;

class Model
{
public:
    void load(const char* path);
    
    ~Model();
    
    void draw() const;

private:
    GLuint vao_{0}, vbo_{0}, ebo_{0};
    GLsizei count_{0};
};

