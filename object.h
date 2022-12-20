#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"
#include "Texture.h"
class Object
{
public:
    Object();
    Object(glm::vec3 pivot, const char* fname);

    ~Object();
    void Update(glm::mat4 model);
    void Render(GLint posAttrib, GLint colAttrib);
    void Render(GLint positionAttribLoc, GLint colorAttribLoc, GLint tcAttribLoc, GLint hasTex);

    glm::mat4 GetModel();
    std::vector<glm::vec2> getTexCoords();
    bool InitBuffers();
    void setupVerticies();
    GLuint getTextureID(bool normal) {
        if (normal)
            return m_normaltexture->getTextureID();
        else
            return m_texture->getTextureID();
    }
    bool hasTex;
    bool hasNorm;
private:
    glm::vec3 pivotLocation;
    glm::mat4 model;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;
    std::vector<glm::vec2> texCoords;
    //bool loadTexture(const char* textFile, int textureType);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    GLuint vao;

    float angle;
    Texture* m_texture;
    Texture* m_normaltexture;
};

#endif /* OBJECT_H */
