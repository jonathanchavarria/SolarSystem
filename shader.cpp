#include "shader.h"

Shader::Shader()
{
  m_shaderProg = 0;
}

Shader::~Shader()
{
  for (std::vector<GLuint>::iterator it = m_shaderObjList.begin() ; it != m_shaderObjList.end() ; it++)
  {
    glDeleteShader(*it);
  }

  if (m_shaderProg != 0)
  {
    glDeleteProgram(m_shaderProg);
    m_shaderProg = 0;
  }
}

bool Shader::Initialize()
{
  m_shaderProg = glCreateProgram();

  if (m_shaderProg == 0) 
  {
    std::cerr << "Error creating shader program\n";
    return false;
  }

  return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool Shader::AddShader(GLenum ShaderType)
{
  std::string s;

  if(ShaderType == GL_VERTEX_SHADER)
  {
      s = "#version 460\n \
          \
          layout (location = 0) in vec3 v_position; \
          layout (location = 1) in vec3 v_color; \
          layout (location = 2) in vec2 v_tc;  \
           \
          out vec3 varNorm; \
          out vec3 varLdir; \
          out vec3 varPos; \
          out vec2 tc; \
            \
          struct PositionalLight{\
            vec4 ambient;\
            vec4 diffuse;\
            vec4 spec;\
            vec3 position;\
           };\
          struct Material{\
            vec4 ambient;\
            vec4 diffuse;\
            vec4 spec;\
            float shininess;\
           };\
           \
          layout (binding=0) uniform sampler2D samp0;\
          layout (binding = 1) uniform sampler2D samp1;\
            \
          uniform vec4 GlobalAmbient; \
          uniform PositionalLight light; \
          uniform Material material; \
          uniform mat4 projectionMatrix; \
          uniform mat4 viewMatrix; \
          uniform mat4 modelMatrix; \
          uniform mat3 normMatrix; \
           \
          void main(void) \
          { \
            vec4 v = vec4(v_position, 1.0); \
            gl_Position = (projectionMatrix * viewMatrix * modelMatrix) * v; \
            tc = v_tc; \
            varPos = (viewMatrix * modelMatrix * vec4(v_position, 1.0f)).xyz; \
            varLdir = light.position-varPos;\
            varNorm = normMatrix*v_color;\
          } \
          ";
  }
  else if(ShaderType == GL_FRAGMENT_SHADER)
  {
      s = "#version 460\n \
          in vec3 varNorm; \
          in vec3 varLdir; \
          in vec3 varPos; \
          in vec2 tc; \
          out vec4 frag_color; \
            \
            layout (binding=0) uniform sampler2D samp0;\
            layout (binding = 1) uniform sampler2D samp1;\
            \
          struct PositionalLight{\
            vec4 ambient;\
            vec4 diffuse;\
            vec4 spec;\
            vec3 position;\
           };\
          struct Material{\
            vec4 ambient;\
            vec4 diffuse;\
            vec4 spec;\
            float shininess;\
           };\
           \
          uniform vec4 GlobalAmbient; \
          uniform PositionalLight light; \
          uniform Material material; \
          uniform mat4 projectionMatrix; \
          uniform mat4 viewMatrix; \
          uniform mat4 modelMatrix; \
          uniform mat3 normMatrix; \
          uniform bool hasNormalMap; \
          \
           \
            void main(void) \
            { \
            vec3 L = normalize(varLdir); \
            vec3 N;\
            if (hasNormalMap) \
                N = normalize(varNorm + texture(samp1, tc).xyz * 2 - 1); \
            else \
                N = normalize(varNorm); \
            vec3 V = normalize(-varPos); \
            vec3 R = normalize(reflect(-L, N)); \
            float cosTheta = dot(L,N); \
            float cosPhi = dot(R,V); \
            vec3 amb = ((GlobalAmbient) + (texture(samp0,tc)*light.ambient * material.ambient)/1).xyz;\
            vec3 dif = light.diffuse.xyz * material.diffuse.xyz * texture(samp0,tc).xyz * max(0.0, cosTheta); \
            vec3 spc = light.spec.xyz * material.spec.xyz * pow(max(0.0,cosPhi), material.shininess); \
            frag_color = vec4(amb+dif+spc,1);\
          } \
          "; 
  }
  // gl_Position = (projectionMatrix * viewMatrix * modelMatrix) * v; 
  GLuint ShaderObj = glCreateShader(ShaderType);

  if (ShaderObj == 0) 
  {
    std::cerr << "Error creating shader type " << ShaderType << std::endl;
    return false;
  }

  // Save the shader object - will be deleted in the destructor
  m_shaderObjList.push_back(ShaderObj);

  const GLchar* p[1];
  p[0] = s.c_str();
  GLint Lengths[1] = { (GLint)s.size() };

  glShaderSource(ShaderObj, 1, p, Lengths);

  glCompileShader(ShaderObj);

  GLint success;
  glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

  if (!success) 
  {
    GLchar InfoLog[1024];
    glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
    std::cerr << "Error compiling: " << InfoLog << std::endl;
    return false;
  }

  glAttachShader(m_shaderProg, ShaderObj);

  return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Shader::Finalize()
{
  GLint Success = 0;
  GLchar ErrorLog[1024] = { 0 };

  glLinkProgram(m_shaderProg);

  glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
  if (Success == 0)
  {
    glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
    std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
    return false;
  }

  glValidateProgram(m_shaderProg);
  glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
  if (!Success)
  {
    glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
    std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
    return false;
  }

  // Delete the intermediate shader objects that have been added to the program
  for (std::vector<GLuint>::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++)
  {
    glDeleteShader(*it);
  }

  m_shaderObjList.clear();

  return true;
}


void Shader::Enable()
{
    glUseProgram(m_shaderProg);
}


GLint Shader::GetUniformLocation(const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if (Location == -1) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}

GLint Shader::GetAttribLocation(const char* pAttribName)
{
    GLuint Location = glGetAttribLocation(m_shaderProg, pAttribName);

    if (Location == -1) {
        fprintf(stderr, "Warning! Unable to get the location of attribute '%s'\n", pAttribName);
    }

    return Location;
}

GLuint Shader::GetShaderProgram() {
    return m_shaderProg;
}
