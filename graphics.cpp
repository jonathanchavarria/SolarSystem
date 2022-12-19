#include "graphics.h"

Graphics::Graphics()
{

}

Graphics::~Graphics()
{

}

bool Graphics::Initialize(int width, int height)
{
	// Used for the linux OS
#if !defined(__APPLE__) && !defined(MACOSX)
  // cout << glewGetString(GLEW_VERSION) << endl;
	glewExperimental = GL_TRUE;

	auto status = glewInit();

	// This is here to grab the error that comes from glew init.
	// This error is an GL_INVALID_ENUM that has no effects on the performance
	glGetError();

	//Check for error
	if (status != GLEW_OK)
	{
		std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
		return false;
	}
#endif



	// Init Camera
	m_camera = new Camera();
	if (!m_camera->Initialize(width, height))
	{
		printf("Camera Failed to Initialize\n");
		return false;
	}

	// Set up the shaders
	m_shader = new Shader();
	if (!m_shader->Initialize())
	{
		printf("Shader Failed to Initialize\n");
		return false;
	}

	// Add the vertex shader
	if (!m_shader->AddShader(GL_VERTEX_SHADER))
	{
		printf("Vertex Shader failed to Initialize\n");
		return false;
	}

	// Add the fragment shader
	if (!m_shader->AddShader(GL_FRAGMENT_SHADER))
	{
		printf("Fragment Shader failed to Initialize\n");
		return false;
	}

	// Connect the program
	if (!m_shader->Finalize())
	{
		printf("Program to Finalize\n");
		return false;
	}

	// Populate location bindings of the shader uniform/attribs
	if (!collectShPrLocs()) {
		printf("Some shader attribs not located!\n");
	}
	
	// Starship
	//m_ship = new Mesh(glm::vec3(2.0f, 3.0f, -5.0f), "assets\\SpaceShip-1.obj", "assets\\SpaceShip-1.png");

	// The Sun
	m_sun = new Sphere(64, "assets\\2k_sun.jpg", "assets\\Venus-n.jpg");

	m_mercury = new Sphere(48, "assets\\Mercury.jpg", "assets\\Mercury-n.jpg");

	m_venus = new Sphere(48, "assets\\Venus.jpg", "assets\\Venus-n.jpg");

	// The Earth
	m_earth = new Sphere(48, "assets\\2k_earth_daymap.jpg", "assets\\2k_earth_daymap-n.jpg");
	
	// The moon
	m_moon = new Sphere(48, "assets\\2k_moon.jpg", "assets\\2k_moon-n.jpg");

	m_mars = new Sphere(48, "assets\\Mars.jpg", "assets\\Mars-n.jpg");

	m_jupiter = new Sphere(48, "assets\\Jupiter.jpg", "assets\\Jupiter-n.jpg");

	m_saturn = new Sphere(48, "assets\\Saturn.jpg", "assets\\Saturn-n.jpg");

	m_uranus = new Sphere(48, "assets\\Uranus.jpg", "assets\\Uranus-n.jpg");

	m_neptune = new Sphere(48, "assets\\Neptune.jpg", "assets\\Neptune-n.jpg");

	m_ceres = new Sphere(48, "assets\\Ceres.jpg", "assets\\Ceres-n.jpg");

	m_eris = new Sphere(48, "assets\\Eris.jpg", "assets\\Eris-n.jpg");

	m_haumea = new Sphere(48, "assets\\Haumea.jpg", "assets\\Haumea-n.jpg");

	//m_ship = new Mesh(glm::vec3(2.0f, 3.0f, -5.0f), "assets\\SpaceShip-1.obj", "assets\\SpaceShip-1.png");


	innerBelt = new glm::mat4[amount];
	outerBelt = new glm::mat4[amount];
	srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
	float radius = 22.0;
	float radius2 = 47.0;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 model2 = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		float x2 = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		float z = cos(angle) * radius + displacement;
		float z2 = cos(angle) * radius2 + displacement;

		model = glm::translate(model, glm::vec3(x, y, z));
		model2 = glm::translate(model2, glm::vec3(x2, y, z2));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
		model = glm::scale(model, glm::vec3(scale));
		model2 = glm::scale(model2, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = static_cast<float>((rand() % 360));
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
		model2 = glm::rotate(model2, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
		// 4. now add to list of matrices
		innerBelt[i] = model;
		innerAsteroids.push_back(new Mesh(glm::vec3(2.0f, 3.0f, -5.0f), "assets\\rock.obj", "assets\\rock.png"));

		outerBelt[i] = model2;
		outerAsteroids.push_back(new Mesh(glm::vec3(2.0f, 3.0f, -5.0f), "assets\\rock.obj", "assets\\rock.png"));
	}



	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	return true;
}

void Graphics::HierarchicalUpdate2(double dt) {

	std::vector<float> speed, dist, rotSpeed, scale;
	glm::vec3 rotVector;
	glm::mat4 localTransform;
	// position of the sun	
	modelStack.push(glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0)));  // sun's coordinate
	localTransform = modelStack.top();		// The sun origin
	localTransform *= glm::rotate(glm::mat4(1.0f), (float)dt * 0.5f, glm::vec3(0.f, -1.f, 0.f));
	localTransform *= glm::scale(glm::vec3(1.25, 1.25, 1.25));

	
	if (m_sun != NULL)
		m_sun->Update(localTransform);

	speed = { 2, 2, 2 };
	dist = { 3. , 0., 3. };
	rotVector = { 0. , 1., 0. };
	rotSpeed = { 2., 1., 1. };
	scale = { .4,.4,.4 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_mercury != NULL)
		m_mercury->Update(localTransform);


	
	modelStack.pop();

	speed = { .8, .8, .8};
	dist = { 10. , 0., 10. };
	rotVector = { 0. , 1., 0. };
	rotSpeed = { 2., 1., 1. };
	scale = { .5,.5,.5 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_venus != NULL)
		m_venus->Update(localTransform);



	modelStack.pop();

	// position of the first planet
	speed = { .5, .5, .5 };
	dist = { 14. , 0., 14.};
	rotVector = { 0. , 1. , 0. };
	rotSpeed = { 1, 1., 1. };
	scale = { .75,.75,.75 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_earth != NULL)
		m_earth->Update(localTransform);


	// position of the first moon
	speed = { 1.5, 1.5, 1.5};
	dist = { 1.25, .50, 1.25 };
	rotVector = { 1.,1.,1. };
	rotSpeed = { .25, .25, .25 };
	scale = { .20f, .20f, .20f };
	localTransform = modelStack.top();
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store moon-planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_moon != NULL)
		m_moon->Update(localTransform);

	modelStack.pop();
	modelStack.pop();


	speed = { .94, .94, .94 };
	dist = { 18. , 0., 18. };
	rotVector = { 0. , 1., 0. };
	rotSpeed = { 1., 1., 1. };
	scale = { .45,.45,.45 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_mars != NULL)
		m_mars->Update(localTransform);

	modelStack.pop();

	speed = { 0.2, .2, .2 };
	dist = { 26. , 0., 26. };
	rotVector = { 0. , 1., 0. };
	rotSpeed = { 1., 1., 1. };
	scale = { 1.5,1.5,1.5 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_jupiter != NULL)
		m_jupiter->Update(localTransform);

	modelStack.pop();

	speed = { .12, .12, .12 };
	dist = { 34. , 0., 34. };
	rotVector = { 1., 1., 1. };
	scale = { 1.3,1.3,1.3 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_saturn != NULL)
		m_saturn->Update(localTransform);

	modelStack.pop();

	speed = { .09, .09, .09 };
	dist = { 38. , 0., 38. };
	rotVector = { 1., 1., 1. };
	scale = { 1.3,1.3,1.3 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_uranus != NULL)
		m_uranus->Update(localTransform);

	modelStack.pop();



	speed = { .07, .07, .07 };
	dist = { 44. , 0., 44. };
	rotVector = { 1., 1., 1. };
	scale = { 1.2,1.2,1.2 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_neptune != NULL)
		m_neptune->Update(localTransform);

	modelStack.pop();
	
	for (unsigned int i = 0; i < amount; i++)
	{
		speed = { 1, 1., 1. };
		dist = { 1, 1  , 1 };
		rotVector = { 1 , 1, 1 };
		rotSpeed = { 1, 1, 1. };
		scale = { .02,.02,.02 };
		glm::mat4 localTransform2;
		//localTransform = modelStack.top();
		localTransform = innerBelt[i];
		localTransform2 = outerBelt[i];

		localTransform *= glm::translate(glm::mat4(1.f),
			glm::vec3(sin(speed[0] * dt) * dist[0], cos(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
		localTransform2 *= glm::translate(glm::mat4(1.f),
			glm::vec3(sin(speed[0] * dt) * dist[0], cos(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));

		modelStack.push(localTransform);			// store planet-sun coordinate
		//localTransform *= glm::rotate(glm::mat4(1.f), -80.f, glm::vec3(1, 0, 0));
		localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
		localTransform2 *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);

		//localTransform = glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
		if (innerAsteroids[i] != NULL)
			innerAsteroids[i]->Update(localTransform);
		if (outerAsteroids[i] != NULL)
			outerAsteroids[i]->Update(localTransform2);

		modelStack.pop();

	}

	speed = { .75, .75, .75 };
	dist = { 20. , 2.5 , 20. };
	rotVector = { 1., 1., 1. };
	scale = { .40, .40, .40 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_ceres != NULL)
		m_ceres->Update(localTransform);

	modelStack.pop();

	speed = { .75, .75, .75 };
	dist = { 50. , 4.5 , 50. };
	rotVector = { 1., 1., 1. };
	scale = { .40, .40, .40 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_eris != NULL)
		m_eris->Update(localTransform);

	modelStack.pop();


	speed = { .55, .55, .55 };
	dist = { 45. , 2.5 , 45. };
	rotVector = { 1., 1., 1. };
	scale = { .40, .40, .40 };
	localTransform = modelStack.top();				// start with sun's coordinate
	localTransform *= glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2]));
	modelStack.push(localTransform);			// store planet-sun coordinate
	localTransform *= glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	localTransform *= glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
	if (m_haumea != NULL)
		m_haumea->Update(localTransform);

	modelStack.pop();







	 	// back to the planet coordinate


	modelStack.pop(); 	// back to the sun coordinate

	//modelStack.pop();	// empy stack


}

void Graphics::installLights() {
	currentLightPos = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 transformed = glm::vec3(m_camera->GetView() * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	globalAmbLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "globalAmbient");
	ambLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.ambient");
	diffLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.diffuse");
	specLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.specular");
	posLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.position");
	mambLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.ambient");
	mdiffLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.diffuse");
	mspecLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.specular");
	mshiLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.shininess");


	glProgramUniform4fv(m_shader->GetShaderProgram(), globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(m_shader->GetShaderProgram(), ambLoc, 1, lightAmbient);
	glProgramUniform4fv(m_shader->GetShaderProgram(), diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(m_shader->GetShaderProgram(), specLoc, 1, lightSpecular);
	glProgramUniform3fv(m_shader->GetShaderProgram(), posLoc, 1, lightPos);
	glProgramUniform4fv(m_shader->GetShaderProgram(), mambLoc, 1, matAmb);
	glProgramUniform4fv(m_shader->GetShaderProgram(), mdiffLoc, 1, matDif);
	glProgramUniform4fv(m_shader->GetShaderProgram(), mspecLoc, 1, matSpe);
	glProgramUniform1f(m_shader->GetShaderProgram(), mshiLoc, matShi);
}

void Graphics::ComputeTransforms(double dt, std::vector<float> speed, std::vector<float> dist, 
	std::vector<float> rotSpeed, glm::vec3 rotVector, std::vector<float> scale, glm::mat4& tmat, glm::mat4& rmat, glm::mat4& smat) {
	tmat = glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2])
	);
	rmat = glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	smat = glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
}

void Graphics::Render(double dt)
{
	//clear the screen
	//glClearColor(0.5, 0.2, 0.2, 1.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Start the correct program
	m_shader->Enable();


	// Send in the projection and view to the shader (stay the same while camera intrinsic(perspective) and extrinsic (view) parameters are the same
	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
	glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));
	
	

	installLights();

	for (unsigned int i = 0; i < amount; i++)
	{
		if (innerAsteroids[i]->hasTex) {
			glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(innerAsteroids[i]->GetModel()));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, innerAsteroids[i]->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			innerAsteroids[i]->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}
	for (unsigned int i = 0; i < amount; i++)
	{
		if (outerAsteroids[i]->hasTex) {
			glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(outerAsteroids[i]->GetModel()));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,outerAsteroids[i]->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			outerAsteroids[i]->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	if (m_sun != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sun->GetModel()));
		if (m_sun->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_sun->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sun->GetModel()));
		if (m_sun->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_sun->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_sun->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}


	if (m_mercury != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mercury->GetModel()));
		if (m_mercury->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mercury->GetModel()));
		if (m_mercury->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_mercury->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	if (m_venus != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_venus->GetModel()));
		//glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_camera->GetView() * m_earth->GetModel())))));
		if (m_venus->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_venus->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);
			
		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_venus->GetModel()));
		if (m_venus->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_venus->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_venus->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}
	
	
	if (m_earth != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_earth->GetModel()));
		//glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_camera->GetView() * m_earth->GetModel())))));
		if (m_earth->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_earth->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);
			//m_earth->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_earth->GetModel()));
		//glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_camera->GetView() * m_earth->GetModel())))));
		if (m_earth->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_earth->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_earth->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	// Render Moon
	if (m_moon != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_moon->GetModel()));
		if (m_moon->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_moon->GetModel()));
		if (m_moon->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_moon->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_moon->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}
	if (m_mars != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mars->GetModel()));
		if (m_mars->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mars->GetModel()));
		if (m_mars->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_mars->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_mars->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	if (m_jupiter != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_jupiter->GetModel()));
		if (m_jupiter->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_jupiter->GetModel()));
		if (m_jupiter->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_jupiter->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_jupiter->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	if (m_saturn != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_saturn->GetModel()));
		if (m_saturn->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_saturn->GetModel()));
		if (m_saturn->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_saturn->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_saturn->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	if (m_uranus != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_uranus->GetModel()));
		if (m_uranus->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_uranus->GetModel()));
		if (m_uranus->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_uranus->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_uranus->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}
	
	if (m_neptune != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_neptune->GetModel()));
		if (m_neptune->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_neptune->GetModel()));
		if (m_neptune->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_neptune->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_neptune->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}


	if (m_ceres != NULL) {
		GLuint hasN = m_shader->GetUniformLocation("hasNormalMap");
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_ceres->GetModel()));
		if (m_ceres->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mercury->getTextureID(false));
			GLuint sampler = m_shader->GetUniformLocation("samp0");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			glUniform1i(hasN, false);

		}
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_ceres->GetModel()));
		if (m_ceres->hasTex) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_ceres->getTextureID(true));
			GLuint sampler = m_shader->GetUniformLocation("samp1");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 1);
			glUniform1i(hasN, true);
			m_ceres->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	

	
	if (m_eris->hasTex) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_eris->GetModel()));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_eris->getTextureID(false));
		GLuint sampler = m_shader->GetUniformLocation("samp0");
		if (sampler == INVALID_UNIFORM_LOCATION)
		{
			printf("Sampler Not found not found\n");
		}
		glUniform1i(sampler, 0);
		m_eris->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	}

	
	if (m_haumea->hasTex) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_haumea->GetModel()));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_haumea->getTextureID(false));
		GLuint sampler = m_shader->GetUniformLocation("samp0");
		if (sampler == INVALID_UNIFORM_LOCATION)
		{
			printf("Sampler Not found not found\n");
		}
		glUniform1i(sampler, 0);
		m_haumea->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	}


	//timeFactor = ((float)dt); // uniform for the time factor

	



	

	// Get any errors from OpenGL
	auto error = glGetError();
	if (error != GL_NO_ERROR)
	{
		string val = ErrorString(error);
		std::cout << "Error initializing OpenGL! " << error << ", " << val << std::endl;
	}
}


bool Graphics::collectShPrLocs() {
	bool anyProblem = true;
	// Locate the projection matrix in the shader
	m_projectionMatrix = m_shader->GetUniformLocation("projectionMatrix");
	if (m_projectionMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_projectionMatrix not found\n");
		anyProblem = false;
	}

	// Locate the view matrix in the shader
	m_viewMatrix = m_shader->GetUniformLocation("viewMatrix");
	if (m_viewMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_viewMatrix not found\n");
		anyProblem = false;
	}

	// Locate the model matrix in the shader
	m_modelMatrix = m_shader->GetUniformLocation("modelMatrix");
	if (m_modelMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_modelMatrix not found\n");
		anyProblem = false;
	}

	m_normalMatrix = m_shader->GetUniformLocation("normMatrix");
	if (m_normalMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_normalMatrix not found\n");
		anyProblem = false;
	}

	// Locate the position vertex attribute
	m_positionAttrib = m_shader->GetAttribLocation("v_position");
	if (m_positionAttrib == -1)
	{
		printf("v_position attribute not found\n");
		anyProblem = false;
	}

	// Locate the color vertex attribute
	m_colorAttrib = m_shader->GetAttribLocation("v_color");
	if (m_colorAttrib == -1)
	{
		printf("v_color attribute not found\n");
		anyProblem = false;
	}

	// Locate the color vertex attribute
	m_tcAttrib = m_shader->GetAttribLocation("v_tc");
	if (m_tcAttrib == -1)
	{
		printf("v_texcoord attribute not found\n");
		anyProblem = false;
	}

	m_hasTexture = m_shader->GetUniformLocation("hasNormalMap");
	if (m_hasTexture == INVALID_UNIFORM_LOCATION) {
		printf("hasTexture uniform not found\n");
		anyProblem = false;
	}



	//globalAmbLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "globalAmbient");
	//ambLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.ambient");
	//diffLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.diffuse");
	//specLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.specular");
	//posLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "light.position");
	//mambLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.ambient");
	//mdiffLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.diffuse");
	//mspecLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.specular");
	//mshiLoc = glGetUniformLocation(m_shader->GetShaderProgram(), "material.shininess");


	//glProgramUniform4fv(m_shader->GetShaderProgram(), globalAmbLoc, 1, globalAmbient);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), ambLoc, 1, lightAmbient);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), diffLoc, 1, lightDiffuse);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), specLoc, 1, lightSpecular);
	//glProgramUniform3fv(m_shader->GetShaderProgram(), posLoc, 1, lightPos);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), mambLoc, 1, matAmb);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), mdiffLoc, 1, matDif);
	//glProgramUniform4fv(m_shader->GetShaderProgram(), mspecLoc, 1, matSpe);
	//glProgramUniform1f(m_shader->GetShaderProgram(), mshiLoc, matShi);
	return anyProblem;
}

std::string Graphics::ErrorString(GLenum error)
{
	if (error == GL_INVALID_ENUM)
	{
		return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
	}

	else if (error == GL_INVALID_VALUE)
	{
		return "GL_INVALID_VALUE: A numeric argument is out of range.";
	}

	else if (error == GL_INVALID_OPERATION)
	{
		return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
	}

	else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
	{
		return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
	}

	else if (error == GL_OUT_OF_MEMORY)
	{
		return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
	}
	else
	{
		return "None";
	}
}

