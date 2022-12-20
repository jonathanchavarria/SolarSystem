#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <stack>
using namespace std;

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "sphere.h"
#include "mesh.h"



#define numVBOs 2;
#define numIBs 2;


class Graphics
{
  public:
    Graphics();
    ~Graphics();
    bool Initialize(int width, int height);
    void HierarchicalUpdate2(double dt);
    void Render(double dt);

    Camera* getCamera() { return m_camera; }

  private:
    std::string ErrorString(GLenum error);

    bool collectShPrLocs();
    void ComputeTransforms (double dt, std::vector<float> speed, std::vector<float> dist,
        std::vector<float> rotSpeed, glm::vec3 rotVector, std::vector<float> scale, 
        glm::mat4& tmat, glm::mat4& rmat, glm::mat4& smat);

    stack<glm::mat4> modelStack;

    Camera *m_camera;
    Shader *m_shader;

    GLint m_projectionMatrix;
    GLint m_viewMatrix;
    GLint m_modelMatrix;
    GLint m_positionAttrib;
    GLint m_colorAttrib;
    GLint m_tcAttrib;
    GLint m_hasTexture;
   

    GLint m_normalMatrix;

    GLint m_tfLoc;

    Sphere* m_sun;
    Sphere* m_mercury;
    Sphere* m_venus;
    Sphere* m_earth;
    Sphere* m_moon;
    Sphere* m_mars;
    Sphere* m_jupiter;
    Sphere* m_saturn;
    Sphere* m_uranus;
    Sphere* m_neptune;

    Sphere* m_ceres;
    Sphere* m_eris;
    Sphere* m_haumea;
    const static unsigned int amount = 40;
    vector<Mesh*> innerAsteroids; 
    vector<Mesh*> outerAsteroids;
    void installLights();

    Mesh* m_mesh;
    Object* m_skyBox;

    GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
    float globalAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float lightAmbient[4] = { .15f, .15f, .15f, .15f };
    float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float lightSpecular[4] = { .20f, .15f, .10f, 1.0f };
    float lightPos[3] = { 0.0, 0.0, 0.0 };

    
    glm::mat4* innerBelt;
    glm::mat4* outerBelt;
   

    glm::vec3 currentLightPos;


    float matAmb[4] = { 3.20, 3.20, 3.20, 1.0 };
    float matDif[4] = { 1.0, 1.0, 1.0, 1.0 };
    float matSpe[4] = { 5.0, 5.0, 5.0, 1.0 };
    float matShi = 2.0;
};

#endif /* GRAPHICS_H */
