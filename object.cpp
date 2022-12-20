#include "object.h"

Object::Object()
{
	// Vertex Set Up
	setupVerticies();

	// Model Set Up
	angle = 0.0f;
	pivotLocation = glm::vec3(0.f, 0.f, 0.f);
	model = glm::translate(glm::mat4(1.0f), pivotLocation);

	// Buffer Set Up
	if (!InitBuffers()) {
		printf("Some buffers not initialized.\n");
	}

}

Object::Object(glm::vec3 pivot, const char* fname)
{
	// Vertex Set Up
	setupVerticies();

	// Model Set Up
	angle = 0.0f;
	pivotLocation = pivot;
	model = glm::translate(glm::mat4(1.0f), pivotLocation);

	// Buffer Set Up
	if (!InitBuffers()) {
		printf("Some buffers not initialized.\n");
	}
	hasTex = false;
	hasNorm = false;
	m_texture = new Texture(fname);
	if (m_texture)
		hasTex = true;
	else
		hasTex = false;

	m_normaltexture = new Texture(fname);
	if (m_normaltexture)
		hasNorm = true;
	else
		hasNorm = false;
}

Object::~Object()
{
	Vertices.clear();
	Indices.clear();
}

void Object::Update(glm::mat4 inmodel)
{
	model = inmodel;

}

glm::mat4 Object::GetModel()
{
	return model;
}

void Object::Render(GLint posAttribLoc, GLint colAttribLoc)
{

	glBindVertexArray(vao);

	// Enable vertex attibute arrays for each vertex attrib
	glEnableVertexAttribArray(posAttribLoc);
	glEnableVertexAttribArray(colAttribLoc);

	// Bind your VBO
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	// Set vertex attribute pointers to the load correct data
	glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(colAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));


	// Bind your Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	// Render
	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	// Disable vertex arrays
	glDisableVertexAttribArray(posAttribLoc);
	glDisableVertexAttribArray(colAttribLoc);
}

void Object::Render(GLint positionAttribLoc, GLint colorAttribLoc, GLint tcAttribLoc, GLint hasTex) {

	glBindVertexArray(vao);
	// Enable vertex attibute arrays for each vertex attrib
	glEnableVertexAttribArray(positionAttribLoc);
	glEnableVertexAttribArray(colorAttribLoc);
	glEnableVertexAttribArray(tcAttribLoc);

	// Bind your VBO
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	// Set vertex attribute pointers to the load correct data. Update here to load the correct attributes.
	glVertexAttribPointer(positionAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(colorAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(tcAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));


	// If has texture, set up texture unit(s): update here for texture rendering
	// If has texture, set up texture unit(s) Update here to activate and assign texture unit
	if (m_texture != NULL) {
		glUniform1i(hasTex, true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture->getTextureID());
	}
	else
		glUniform1i(hasTex, false);





	// Bind your Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	// Render
	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	// Disable vertex arrays
	glDisableVertexAttribArray(positionAttribLoc);
	glDisableVertexAttribArray(colorAttribLoc);
	glDisableVertexAttribArray(tcAttribLoc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
bool Object::InitBuffers() {

	// For OpenGL 3
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	return true;
}

void Object::setupVerticies() {
	Vertices = {
  {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.,0.}},
  {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.,0.}},
  {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.,0.}},
  {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {1.,0.}},
  {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}, {1.,0.}},
  {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.,0.}},
  {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.,0.}},
  {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.,0.}}
	};

	Indices = {
	  2, 3, 4,
	  8, 7, 6,
	  1, 5, 6,
	  2, 6, 7,
	  7, 8, 4,
	  1, 4, 8,
	  1, 2, 4,
	  5, 8, 6,
	  2, 1, 6,
	  3, 2, 7,
	  3, 7, 4,
	  5, 1, 8
	};

	// The index works at a 0th index
/*	for (unsigned int i = 0; i < Indices.size(); i++)
	{
		Vertices.push_back(Vertex(vertices[Indices[i]], normals[Indices[i]], texCoords[Indices[i]]));
		Indices.push_back(i);
	}
	*/
}


