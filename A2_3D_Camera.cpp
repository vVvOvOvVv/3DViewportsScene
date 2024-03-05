#include "Camera.h"
#include "SimpleModel.h"
#include "Texture.h"
#include "utilities.h"
#include <glm/fwd.hpp>


// global variables
// settings
unsigned int gWindowWidth = 800, gWindowHeight = 800;
const float gCamRotateSensitivity = 0.1f;

// scene content
ShaderProgram gShader;	// shader program object
GLuint gVBO1 = 0,
	   gVBO2 = 0,
	   gVBO3 = 0;		// vertex buffer object identifiers
GLuint gVAO1 = 0,
	   gVAO2 = 0,
	   gVAO3 = 0;		// vertex array object identifiers

map<string, Camera> gCamera;				// camera objects
map<string, mat4> gModelMatrix;				// object's matrix   

// UI variables
// framerate/time
float gFrameRate = 60.0f,
gFrameTime = 1 / gFrameRate;
// animation toggle
bool gAnimToggle = false; 
// tranformation sensitivities
const float gRotateSensitivity = 1.0f; 
// camera controls
float gPrevYaw, gYaw = 0, gPrevPitch, gPitch = 0;

Light gLight;						// light properties
map<string, Material> gMaterials;	// material properties
Texture gCubeEnvMap;				// cube environment map - object
map<string, Texture> gTextures;		// texture map for walls and floors
SimpleModel gModel;					// scene object model

// function initialise scene and render settings
static void init(GLFWwindow* window) {
	// set the color the color buffer should be cleared to
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);	// enable depth buffer test

	// compile and link a vertex and fragment shader pair
	gShader.compileAndLink("lightingAndTexture.vert", "pointLightTexture.frag");

	// initialise view matrices
	// top right
	gCamera["Top Right"].setViewMatrix(vec3(0.0f, 5.0f, 0.1f),
		vec3(0.0f, 0.0f, 0.0f));
	// bot left
	gCamera["Bot Left"].setViewMatrix(vec3(0.0f, 0.5f, 0.5f),
		vec3(0.0f, 0.5f, -1.0f));
	// bot right
	gCamera["Bot Right"].setViewMatrix(vec3(0.0f, 1.0f, 0.9f),
		vec3(0.0f, 0.25f, 0.0f));
	// main
	gCamera["Main"].setViewMatrix(vec3(0.0f, 0.0f, 2.0f),
		vec3(0.0f, 0.0f, 0.0f));

	// aspect ratio
	float aspectRatio = static_cast<float>(gWindowWidth) / gWindowHeight; 
	// initialise projection matrices
	// ortho - top down
	gCamera["Top Right"].setProjMatrix(ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f));
	// ortho - front
	gCamera["Bot Left"].setProjMatrix(ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f));
	// perspective
	gCamera["Bot Right"].setProjMatrix(perspective(radians(60.0f), aspectRatio, 0.1f, 10.0f));
	// main
	gCamera["Main"].setProjMatrix(ortho(0.0f, static_cast<float>(gWindowWidth), 0.0f,
		static_cast<float>(gWindowHeight), 0.1f, 10.0f));

	// initialise model matrices
	gModelMatrix["Env"] = mat4(1.0f);
	gModelMatrix["Ring"] = translate(vec3(0.0f, 0.5f, 0.0f))
		* scale(vec3(0.3f, 0.3f, 0.3f))
		* rotate(radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

	// initialise point light properties  
	gLight.pos = vec3(0.0f, 1.0f, 0.0f);
	gLight.La = vec3(0.3f);
	gLight.Ld = vec3(1.0f);
	gLight.Ls = vec3(1.0f);
	gLight.att = vec3(1.0f, 0.0f, 0.0f);

	// initialise material properties ===============================
	gMaterials["General"].Ka = vec3(0.25f, 0.21f, 0.21f);
	gMaterials["General"].Kd = vec3(1.0f, 0.83f, 0.83f);
	gMaterials["General"].Ks = vec3(0.3f, 0.3f, 0.3f);
	gMaterials["General"].shininess = 11.3f;

	gMaterials["Wall"].Ka = glm::vec3(0.2f);
	gMaterials["Wall"].Kd = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterials["Wall"].Ks = glm::vec3(0.2f, 0.7f, 1.0f);
	gMaterials["Wall"].shininess = 40.0f;
	// ==============================================================

	// load textures ================================================
	// cube environment map
	gCubeEnvMap.generate("./images/cm_front.bmp", "./images/cm_back.bmp",
		"./images/cm_left.bmp", "./images/cm_right.bmp",
		"./images/cm_top.bmp", "./images/cm_bottom.bmp");
	// floor texture
	gTextures["Floor"].generate("./images/check.bmp"); 
	// load texture and normal map for walls
	gTextures["Stone"].generate("./images/Fieldstone.bmp");
	gTextures["StoneNormalMap"].generate("./images/FieldstoneBumpDOT3.bmp");
	// painting texture 
	gTextures["Painting"].generate("./images/smile.bmp");
	// =============================================================
	
	// load model
	gModel.loadModel("./models/torus.obj"); // CHANGE BACK

	// vertex positions, normals and texture coordinates ===========
	//  for floor and painting
	vector<GLfloat> texturedVertices = {
		/* ---------------------------------------------------
		NOTE TO SELF: MAKE SURE THE ORDER MATCHES THE STRUCT
		--------------------------------------------------- */ 
		// floor  -------------------------------------------
		-1.0f, 0.0f, 1.0f,	// vertex 0: position
		0.0f, 1.0f, 0.0f,	// vertex 0: normal
		0.0f, 0.0f,			// vertex 0: texture coordinate 

		1.0f, 0.0f, 1.0f,	// vertex 1: position
		0.0f, 1.0f, 0.0f,	// vertex 1: normal
		4.0f, 0.0f,			// vertex 1: texture coordinate 

		-1.0f, 0.0f, -1.0f,	// vertex 2: position 
		0.0f, 1.0f, 0.0f,	// vertex 2: normal
		0.0f, 4.0f,			// vertex 2: texture coordinate 

		1.0f, 0.0f, -1.0f,	// vertex 3: position
		0.0f, 1.0f, 0.0f,	// vertex 3: normal
		4.0f, 4.0f,			// vertex 3: texture coordinate 
		// painting -----------------------------------------
		-0.25f, 0.25f, -0.99f,	// vertex 4: position 
		0.0f, 1.0f, 0.0f,		// vertex 4: normal
		0.0f, 0.0f,				// vertex 4: texture coordinate 

		0.25f, 0.25f, -0.99f,	// vertex 5: position 
		0.0f, 1.0f, 0.0f,		// vertex 5: normal
		1.0f, 0.0f,				// vertex 5: texture coordinate 

		-0.25f, 0.75f, -0.99f,	// vertex 6: position
		0.0f, 1.0f, 0.0f,		// vertex 6: normal
		0.0f, 1.0f,				// vertex 6: texture coordinate 

		0.25f, 0.75f, -0.99f,	// vertex 7: position
		0.0f, 1.0f, 0.0f,		// vertex 7: normal
		1.0f, 1.0f,				// vertex 7: texture coordinate 
	};
	// for walls
	vector<GLfloat> wallVertices = { 
		// close wall
		-1.0f, 0.0f, 1.0f,	// vertex 0: position
		0.0f, 1.0f, 0.0f,	// vertex 0: normal
		3.0f, 0.0f,			// vertex 0: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 0: tangent

		1.0f, 0.0f, 1.0f,	// vertex 1: position 
		0.0f, 1.0f, 0.0f,	// vertex 1: normal
		0.0f, 0.0f,			// vertex 1: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 1: tangent

		-1.0f, 1.0f, 1.0f,	// vertex 2: position 
		0.0f, 1.0f, 0.0f,	// vertex 2: normal
		3.0f, 3.0f,			// vertex 2: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 2: tangent

		1.0f, 1.0f, 1.0f,	// vertex 3: position 
		0.0f, 1.0f, 0.0f,	// vertex 3: normal
		0.0f, 3.0f,			// vertex 3: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 3: tangent
		// left wall
		-1.0f, 0.0f, 1.0f,	// vertex 4: position 
		0.0f, 1.0f, 0.0f,	// vertex 4: normal
		0.0f, 0.0f,			// vertex 4: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 4: tangent

		-1.0f, 0.0f, -1.0f,	// vertex 5: position 
		0.0f, 1.0f, 0.0f,	// vertex 5: normal
		3.0f, 0.0f,			// vertex 5: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 5: tangent

		-1.0f, 1.0f, 1.0f,	// vertex 6: position
		0.0f, 1.0f, 0.0f,	// vertex 6: normal
		0.0f, 3.0f,			// vertex 6: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 6: tangent

		-1.0f, 1.0f, -1.0f,	// vertex 7: position
		0.0f, 1.0f, 0.0f,	// vertex 7: normal
		3.0f, 3.0f,			// vertex 7: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 7: tangent
		// close wall
		-1.0f, 0.0f, -1.0f,	// vertex 8: position 
		0.0f, 1.0f, 0.0f,	// vertex 8: normal
		0.0f, 0.0f,			// vertex 8: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 8: tangent

		1.0f, 0.0f, -1.0f,	// vertex 9: position 
		0.0f, 1.0f, 0.0f,	// vertex 9: normal
		3.0f, 0.0f,			// vertex 9: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 9: tangent

		-1.0f, 1.0f, -1.0f,	// vertex 10: position
		0.0f, 1.0f, 0.0f,	// vertex 10: normal
		0.0f, 3.0f,			// vertex 10: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 10: tangent

		1.0f, 1.0f, -1.0f,	// vertex 11: position
		0.0f, 1.0f, 0.0f,	// vertex 11: normal
		3.0f, 3.0f,			// vertex 11: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 11: tangent
		// right wall
		1.0f, 0.0f, -1.0f,	// vertex 12: position 
		0.0f, 1.0f, 0.0f,	// vertex 12: normal
		0.0f, 0.0f,			// vertex 12: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 12: tangent

		1.0f, 0.0f, 1.0f,	// vertex 13: position 
		0.0f, 1.0f, 0.0f,	// vertex 13: normal
		3.0f, 0.0f,			// vertex 13: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 13: tangent

		1.0f, 1.0f, -1.0f,	// vertex 14: position
		0.0f, 1.0f, 0.0f,	// vertex 14: normal
		0.0f, 3.0f,			// vertex 14: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 14: tangent

		1.0f, 1.0f, 1.0f,	// vertex 15: position
		0.0f, 1.0f, 0.0f,	// vertex 15: normal
		3.0f, 3.0f,			// vertex 15: texture coordinate
		1.0f, 0.0f, 0.0f,	// vertex 15: tangent
	}; 
	// vertices for the lines (main view)
	vector<GLfloat> lineVertices = {
		0.0f, 400.0f, 1.5f,		// line 1 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 0: colour
		800.0f, 400.0f, 1.5f,	// line 1 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 1 vertex 1: colour

		400.0f, 0.0f, 1.5f,		// line 2 vertex 0: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 0: colour
		400.0f, 800.0f, 1.5f,	// line 2 vertex 1: position
		1.0f, 1.0f, 1.0f,		// line 2 vertex 1: colour
	}; 
	// ============================================================
	// create VBO - for the floor and painting
	glGenBuffers(1, &gVBO1);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* texturedVertices.size(), &texturedVertices[0], GL_STATIC_DRAW);

	// floor and painting
	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO1);			// generate unused VAO identifier
	glBindVertexArray(gVAO1);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO1);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, texCoord)));		// specify format of texture coordinate data 

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2); 
	
	// create VBO - for the walls
	glGenBuffers(1, &gVBO2);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * wallVertices.size(), &wallVertices[0], GL_STATIC_DRAW);

	// walls
	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO2);			// generate unused VAO identifier
	glBindVertexArray(gVAO2);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2);	// bind the VBO

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, position)));		// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, normal)));		// specify format of colour data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, texCoord)));		// specify format of texture coordinate data
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTanTex),
		reinterpret_cast<void*>(offsetof(VertexNormTanTex, tangent)));		// specify format of tangent data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind the VAO

	// lines --------------------------------------
	// create VBO
	glGenBuffers(1, &gVBO3);					// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* lineVertices.size(), &lineVertices[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data 
	glGenVertexArrays(1, &gVAO3);			// generate unused VAO identifier
	glBindVertexArray(gVAO3);				// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3);	// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));	// specify format of position data
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));		// specify format of colour data

	glEnableVertexAttribArray(0);	// enable vertex attributes
	glEnableVertexAttribArray(4);

	glBindVertexArray(0); // Unbind the VAO
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// cursor movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	// pass mouse data to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// pass mouse data to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

// error callback function
static void error_callback(int error, const char* description) {
	std::cerr << description << std::endl;	// output error description
}

// create and populate tweak bar elements
TwBar* create_UI(const string name = "Interface") {
	// create a tweak bar
	TwBar* twBar = TwNewBar(name.c_str());

	// give tweak bar the size of graphics window
	TwWindowSize(gWindowWidth, gWindowHeight);
	TwDefine(" TW_HELP visible=false ");	// disable help menu
	TwDefine(" GLOBAL fontsize=3 ");		// set large font size
	TwDefine(" Main label='Controls' refresh=0.02 text=light size='220 250' ");

	// frame stat entries
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT,
		&gFrameRate, " group='Frame Statistics' precision=2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT,
		&gFrameTime, " group='Frame Statistics' ");

	// animation toggle
	TwAddVarRW(twBar, "Toggle", TW_TYPE_BOOL16,
		&gAnimToggle, " group='Animation' ");

	// light controls
		// 'room' is 2x2 square, keep light close to cube (except y-coord)
	TwAddVarRW(twBar, "Position X", TW_TYPE_FLOAT, &gLight.pos.x, 
		" group='Light' min=-2.0 max=2.0 step=0.01 ");
	TwAddVarRW(twBar, "Position Y", TW_TYPE_FLOAT, &gLight.pos.y, 
		" group='Light' min=0.0 max=5.0 step=0.01 ");
	TwAddVarRW(twBar, "Position Z", TW_TYPE_FLOAT, &gLight.pos.z, 
		" group='Light' min=-2.0 max=2.0 step=0.01 "); 

	// camera controls
	TwAddVarRW(twBar, "Yaw", TW_TYPE_FLOAT, &gYaw, // look left/right
		" group='Camera' precision=2 min=-45.00 max=45.00 step=0.01 ");
	TwAddVarRW(twBar, "Pitch", TW_TYPE_FLOAT, &gPitch, // look up/down
		" group='Camera' precision=2 min=-45.00 max=45.00 step=0.01 ");  

	return twBar;
}

static void update_scene(GLFWwindow* window) {
	// to rotate the object
	float rotateAngle = 0.0f; 

	// rotate obj if animation is toggled
	if (gAnimToggle)  
		rotateAngle += gRotateSensitivity * gFrameTime;
	// translate obj - update gModelMatrix
	gModelMatrix["Ring"] *= rotate(rotateAngle, vec3(0.0f, 0.0f, 1.0f));

	// update camera angle
	float deltaYaw = radians((gPrevYaw - gYaw) * gCamRotateSensitivity * gFrameRate),
		deltaPitch = radians((gPrevPitch - gPitch) * gCamRotateSensitivity * gFrameRate);

	//gCamera["Top Right"].updateRotation(deltaYaw, deltaPitch);
	//gCamera["Bot Left"].updateRotation(deltaYaw, deltaPitch);
	gCamera["Bot Right"].updateRotation(deltaYaw, deltaPitch);
	// update camera direction (tilt)
		// no need to move camera, just tilt => 0.0f 
	//gCamera["Top Right"].update(0.0f, 0.0f);
	//gCamera["Bot Left"].update(0.0f, 0.0f);
	gCamera["Bot Right"].update(0.0f, 0.0f);

	gPrevPitch = gPitch;
	gPrevYaw = gYaw;
}

// draw ring
void draw_object(string view) {
	// set viewing position
	gShader.setUniform("uViewpoint", gCamera[view].getPosition());

	/// calculate matrices
	mat4 MVP = gCamera[view].getProjMatrix()
		* gCamera[view].getViewMatrix()
		* gModelMatrix["Ring"];
	mat3 normalMatrix = mat3(transpose(inverse(gModelMatrix["Ring"])));

	// set material properties
	gShader.setUniform("uMaterial.Ks", gMaterials["General"].Ks);
	gShader.setUniform("uMaterial.Ka", gMaterials["General"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["General"].Kd);
	gShader.setUniform("uMaterial.shininess", gMaterials["General"].shininess);

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["Ring"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);
	gShader.setUniform("uColorSet", 0);

	// set texture
	glActiveTexture(GL_TEXTURE0);
	gCubeEnvMap.bind(); 
	gModel.drawModel();  
}

// walls and floor
void draw_env(string view) { 
	// set viewing position
	gShader.setUniform("uViewpoint", gCamera[view].getPosition());

	mat4 MVP = gCamera[view].getProjMatrix()
		* gCamera[view].getViewMatrix()
		* gModelMatrix["Env"];
	mat3 normalMatrix = mat3(transpose(inverse(gModelMatrix["Env"])));

	// set uniform variables
	gShader.setUniform("uModelViewProjectionMatrix", MVP);
	gShader.setUniform("uModelMatrix", gModelMatrix["Env"]);
	gShader.setUniform("uNormalMatrix", normalMatrix);
	gShader.setUniform("uColorSet", 1);

	// set material properties
	gShader.setUniform("uMaterial.Ks", gMaterials["General"].Ks);
	gShader.setUniform("uMaterial.Ka", gMaterials["General"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["General"].Kd);
	gShader.setUniform("uMaterial.shininess", gMaterials["General"].shininess);

	glActiveTexture(GL_TEXTURE1);
	gTextures["Floor"].bind();

	glBindVertexArray(gVAO1);			// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices    

	gShader.setUniform("uColorSet", 3);
	glActiveTexture(GL_TEXTURE4);
	gTextures["Painting"].bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);	// render the vertices  

	// set material properties - walls
	gShader.setUniform("uMaterial.Ka", gMaterials["Wall"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["Wall"].Kd);
	gShader.setUniform("uMaterial.Ks", gMaterials["Wall"].Ks);
	gShader.setUniform("uMaterial.shininess", gMaterials["Wall"].shininess);

	gShader.setUniform("uColorSet", 2);
	glActiveTexture(GL_TEXTURE2);
	gTextures["Stone"].bind();
	glActiveTexture(GL_TEXTURE3); 
	gTextures["StoneNormalMap"].bind();

	glBindVertexArray(gVAO2);			// make VAO active
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);	// render the vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);	// render the vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);	// render the vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);	// render the vertices    
}

// function to render the scene
static void render_scene() {
	// clear colour buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the shaders associated with the shader program
	gShader.use();

	// set light properties 
	gShader.setUniform("uLight.pos", gLight.pos);
	gShader.setUniform("uLight.La", gLight.La);
	gShader.setUniform("uLight.Ld", gLight.Ld);
	gShader.setUniform("uLight.Ls", gLight.Ls);
	gShader.setUniform("uLight.att", gLight.att);

	gShader.setUniform("uEnvironmentSampler", 0);
	gShader.setUniform("uTextureSampler", 1);
	gShader.setUniform("uTextureSampler2", 2);
	gShader.setUniform("uNormalSampler", 3);
	gShader.setUniform("uTextureSampler3", 4);

	/* ==========================================
	*	DRAW VIEWPORTS
	========================================== */  
	// top right ================================
	glViewport(400, 400, 400, 400);

	draw_object("Top Right");

	draw_env("Top Right");

	// bottom left ==============================
	glViewport(0, 0, 400, 400);

	draw_object("Bot Left");

	draw_env("Bot Left"); 

	// bottom right =============================
	glViewport(400, 0, 400, 400);

	draw_object("Bot Right");

	draw_env("Bot Right");

	// main =====================================
	glViewport(0, 0, 800, 800);

	// set material properties
	gShader.setUniform("uMaterial.Ks", gMaterials["General"].Ks);
	gShader.setUniform("uMaterial.Ka", gMaterials["General"].Ka);
	gShader.setUniform("uMaterial.Kd", gMaterials["General"].Kd);
	gShader.setUniform("uMaterial.shininess", gMaterials["General"].shininess);

	// set viewing position
	gShader.setUniform("uViewpoint", gCamera["Main"].getPosition());

	// draw lines
	glBindVertexArray(gVAO3);				// make VAO for lines active
	gShader.setUniform("uColorSet", 4);
	// use the main orthographic projection matrix to set model-view-project matrix
	mat4 MVP = gCamera["Main"].getProjMatrix() * gCamera["Main"].getViewMatrix();
	// set uniform model transformation matrix 
	gShader.setUniform("uModelViewProjectionMatrix", MVP); 
	glDrawArrays(GL_LINES, 0, 4);	// display the lines

	/* ========================================== 
	========================================== */

	// flush the graphics pipeline
	glFlush();
}

int main(void) {
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, 
		"3DViewportsScene", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// initialise scene and render settings
	init(window);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);	// update the scene  

		render_scene();			// render the scene

		// set polygon render mode to fill
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// clean up
	glDeleteBuffers(1, &gVBO1);
	glDeleteBuffers(1, &gVBO2);
	glDeleteBuffers(1, &gVBO3);
	glDeleteVertexArrays(1, &gVAO1);
	glDeleteVertexArrays(1, &gVAO2);
	glDeleteVertexArrays(1, &gVAO3);

	// delete and uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
