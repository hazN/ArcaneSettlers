//#include <glad/glad.h>
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "globalOpenGL.h"
#include "Animation.h"
#include "JSONPersitence.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3        (x,y,z)
#include <glm/vec4.hpp> // glm::vec4        (x,y,z,w)
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>  // File streaming thing (like cin, etc.)
#include <sstream>  // "string builder" type thing

// Some STL (Standard Template Library) things
#include <vector>           // aka a "smart array"

#include "globalThings.h"

#include "cShaderManager.h"
#include "cVAOManager/cVAOManager.h"
#include "cLightHelper.h"
#include "cVAOManager/c3DModelFileLoader.h"
#include "GUI.h"
#include "GameGUI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "cBasicTextureManager.h"
#include "GameObject/GameObject.h"

#include <Interface/iPhysicsFactory.h>
#include <Interface/SphereShape.h>
#include <Interface/BoxShape.h>
#include <Interface/CylinderShape.h>
#include <Interface/PlaneShape.h>
#include <Interface/TriangleMeshShape.h>
#include <physics/physx/PhysicsFactory.h>
#include <physics/physx/CharacterController.h>
#include "AnimationManager.h"
#include "PhysicsHelper.h"
#include <Interface/iCharacterController.h>
#include "quaternion_utils.h"
#include "IDGenerator.h"
#include "Colonist.h"
#include "TerrainManager.h"
#include "ColonistManager.h"
#include "EventSystem.h"
glm::vec3 g_cameraEye = glm::vec3(0.00f, 100, 0.001f);
glm::vec3 g_cameraTarget = glm::vec3(1.0f, 1.0f, 1.0f);
std::vector<GameObject*> gameObjects;
std::vector<GameObject*> randomBalls;
extern int ballIndex;
TerrainManager* terrainManager;
ColonistManager* colonistManager;
cBasicTextureManager* g_pTextureManager = NULL;

// Call back signatures here
void renderTransparentBuildingMesh();
void mouse_camera_update(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
glm::vec3 GetRayDirection(double mouseX, double mouseY, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int screenWidth, int screenHeight);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void DrawObject(cMeshObject* pCurrentMeshObject,
	glm::mat4x4 mat_PARENT_Model,               // The "parent's" model matrix
	GLuint shaderID,                            // ID for the current shader
	cBasicTextureManager* pTextureManager,
	cVAOManager* pVAOManager,
	GLint mModel_location,                      // Uniform location of mModel matrix
	GLint mModelInverseTransform_location);      // Uniform location of mView location
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}
float hp = 100;
// From here: https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats/5289624
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

bool LoadModelTypesIntoVAO(std::string fileTypesToLoadName,
	cVAOManager* pVAOManager,
	GLuint shaderID)
{
	std::ifstream modelTypeFile(fileTypesToLoadName.c_str());
	if (!modelTypeFile.is_open())
	{
		// Can't find that file
		return false;
	}

	// At this point, the file is open and ready for reading

	std::string PLYFileNameToLoad;
	std::string friendlyName;

	bool bKeepReadingFile = true;

	const unsigned int BUFFER_SIZE = 1000;  // 1000 characters
	char textBuffer[BUFFER_SIZE] = { 0 };  // Allocate AND clear (that's the {0} part)

	while (bKeepReadingFile)
	{
		modelTypeFile.getline(textBuffer, BUFFER_SIZE);

		PLYFileNameToLoad.clear();
		PLYFileNameToLoad.append(textBuffer);

		if (PLYFileNameToLoad == "EOF")
		{
			bKeepReadingFile = false;
			continue;
		}

		memset(textBuffer, 0, BUFFER_SIZE);
		modelTypeFile.getline(textBuffer, BUFFER_SIZE);
		friendlyName.clear();
		friendlyName.append(textBuffer);

		sModelDrawInfo motoDrawInfo;

		c3DModelFileLoader fileLoader;
		std::string errorText = "";

		std::string fileExtension = PLYFileNameToLoad.substr(PLYFileNameToLoad.length() - 3);
		bool isFBX = (fileExtension == "fbx" || fileExtension == "FBX");

		if (isFBX)
		{
			if (fileLoader.LoadFBXFile_Format_XYZ_N_RGBA_UV(PLYFileNameToLoad, motoDrawInfo, errorText))
			{
				std::cout << "Loaded the FBX file OK" << std::endl;
			}
			else
			{
				std::cout << errorText;
			}
		}
		else
		{
			if (fileLoader.LoadPLYFile_Format_XYZ_N_RGBA_UV(PLYFileNameToLoad, motoDrawInfo, errorText))
			{
				//std::cout << "Loaded the file OK" << std::endl;
			}
			else
			{
				std::cout << errorText;
			}
		}

		if (pVAOManager->LoadModelIntoVAO(friendlyName, motoDrawInfo, shaderID))
		{
			std::cout << "Loaded the " << friendlyName << " model" << std::endl;
		}
	}

	return true;
}

bool CreateObjects(std::string fileName)
{
	std::ifstream objectFile(fileName.c_str());
	if (!objectFile.is_open())
	{
		// Can't find that file
		return false;
	}

	// Basic variables
	std::string meshName;
	std::string friendlyName;
	glm::vec3 position;
	//glm::vec3 rotation;
	glm::quat qRotation;
	glm::vec3 scale;
	// Advanced
	bool useRGBA;
	glm::vec4 colour;
	bool isWireframe;
	bool doNotLight;
	// skip first line
	std::getline(objectFile, meshName);
	std::getline(objectFile, meshName);
	meshName = "";
	for (std::string line; std::getline(objectFile, line);)
	{
		std::istringstream in(line);
		std::string type;
		in >> type;
		if (type == "basic")
		{
			in >> meshName >> friendlyName >> position.x >> position.y >> position.z >> qRotation.x >> qRotation.y >> qRotation.z >> scale.x >> scale.y >> scale.z;
			cMeshObject* pObject = new cMeshObject();
			pObject->meshName = meshName;
			pObject->friendlyName = friendlyName;
			pObject->position = position;
			pObject->qRotation = qRotation;
			pObject->scaleXYZ = scale;
			g_pMeshObjects.push_back(pObject);
		}
		//else if (type == "advanced")
		//{
		//	in >> meshName >> friendlyName >> useRGBA >> colour.x >> colour.y >> colour.z >> colour.w >> isWireframe >> scale >> doNotLight;
		//	cMeshObject* pObject = new cMeshObject();
		//	pObject->meshName = meshName;
		//	pObject->friendlyName = friendlyName;
		//	pObject->bUse_RGBA_colour = useRGBA;
		//	pObject->RGBA_colour = colour;
		//	pObject->isWireframe = isWireframe;
		//	pObject->scale = 1.0f;
		//	pObject->bDoNotLight = doNotLight;
		//}
	}

	return true;
}
bool SaveTheVAOModelTypesToFile(std::string fileTypesToLoadName,
	cVAOManager* pVAOManager);

void DrawConcentricDebugLightObjects(void);

// HACK: These are the light spheres we will use for debug lighting
cMeshObject* pDebugSphere_1 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_2 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_3 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_4 = NULL;// = new cMeshObject();
cMeshObject* pDebugSphere_5 = NULL;// = new cMeshObject();
cMeshObject* renderBuildingMesh = NULL;

int main(int argc, char* argv[])
{
	//srand
	srand(time(NULL));

	std::cout << "starting up..." << std::endl;

	GLuint vertex_buffer = 0;

	GLuint shaderID = 0;

	GLint vpos_location = 0;
	GLint vcol_location = 0;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(1920, 1080, "Arcane Settlers - Hassan Assaf", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	std::cout << "Window created." << std::endl;

	glfwMakeContextCurrent(window);
	//   gladLoadGL( (GLADloadproc)glfwGetProcAddress );
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSwapInterval(1);

	// IMGUI
	//initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	//platform / render bindings
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 420"))
	{
		return 3;
	}

	// Create a shader thingy
	cShaderManager* pTheShaderManager = new cShaderManager();

	cShaderManager::cShader vertexShader01;
	cShaderManager::cShader fragmentShader01;

	vertexShader01.fileName = "assets/shaders/vertexShader01.glsl";
	fragmentShader01.fileName = "assets/shaders/fragmentShader01.glsl";

	if (!pTheShaderManager->createProgramFromFile("Shader_1", vertexShader01, fragmentShader01))
	{
		vertexShader01.fileName = "vertexShader01.glsl";
		fragmentShader01.fileName = "fragmentShader01.glsl";
		if (!pTheShaderManager->createProgramFromFile("Shader_1", vertexShader01, fragmentShader01))
		{
			std::cout << "Didn't compile shaders" << std::endl;
			std::string theLastError = pTheShaderManager->getLastError();
			std::cout << "Because: " << theLastError << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << "Compiled shader OK." << std::endl;
	}

	pTheShaderManager->useShaderProgram("Shader_1");

	shaderID = pTheShaderManager->getIDFromFriendlyName("Shader_1");

	glUseProgram(shaderID);

	::g_pTheLightManager = new cLightManager();

	cLightHelper* pLightHelper = new cLightHelper();

	// Set up the uniform variable (from the shader
	::g_pTheLightManager->LoadLightUniformLocations(shaderID);

	// Make this a spot light
//    vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
//                    // 0 = pointlight
//                    // 1 = spot light
//                    // 2 = directional light
	::g_pTheLightManager->vecTheLights[0].name = "MainLight";
	::g_pTheLightManager->vecTheLights[0].param1.x = 2.0f;
	::g_pTheLightManager->vecTheLights[0].position = glm::vec4(0.0f, 300.0f, -1000.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	// inner and outer angles
	::g_pTheLightManager->vecTheLights[0].atten = glm::vec4(0.1f, 0.001f, 0.0000001f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[0].param1.y = 10.0f;     // Degrees
	::g_pTheLightManager->vecTheLights[0].param1.z = 20.0f;     // Degrees
	::g_pTheLightManager->vecTheLights[0].TurnOn();

	// Make light #2 a directional light
	// BE CAREFUL about the direction and colour, since "colour" is really brightness.
	// (i.e. there NO attenuation)
	::g_pTheLightManager->vecTheLights[1].name = "MoonLight";
	::g_pTheLightManager->vecTheLights[1].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[1].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[1].TurnOn();
	//::g_pTheLightManager->vecTheLights[2].name = "notSure";

	::g_pTheLightManager->vecTheLights[3].name = "Torch2";
	::g_pTheLightManager->vecTheLights[3].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[3].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].diffuse = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[3].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[3].TurnOn();

	::g_pTheLightManager->vecTheLights[4].name = "Torch3";
	::g_pTheLightManager->vecTheLights[4].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[4].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[4].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//	::g_pTheLightManager->vecTheLights[4].TurnOn();

	::g_pTheLightManager->vecTheLights[5].name = "Torch4";
	::g_pTheLightManager->vecTheLights[5].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[5].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//	::g_pTheLightManager->vecTheLights[5].TurnOn();

	::g_pTheLightManager->vecTheLights[5].name = "Torch5";
	::g_pTheLightManager->vecTheLights[5].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[5].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[5].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[5].TurnOn();

	::g_pTheLightManager->vecTheLights[6].name = "Torch6";
	::g_pTheLightManager->vecTheLights[6].param1.x = 0.0f;  // 2 means directional
	::g_pTheLightManager->vecTheLights[6].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].atten = glm::vec4(0.1f, 0.00001f, 0.1f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].diffuse = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	::g_pTheLightManager->vecTheLights[6].specular = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//::g_pTheLightManager->vecTheLights[6].TurnOn();

	for (int i = 7; i < 16; i++)
	{
		::g_pTheLightManager->vecTheLights[i].param1.x = 1.0f;
		::g_pTheLightManager->vecTheLights[i].position = glm::vec4(0.0f, 10.0f, 0.0f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].atten = glm::vec4(0.1f, 0.001f, 0.1f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		::g_pTheLightManager->vecTheLights[i].direction = glm::vec4(0.f, 0.f, 1.f, 1.f);
		::g_pTheLightManager->vecTheLights[i].qDirection = glm::quat(glm::vec3(0));
		::g_pTheLightManager->vecTheLights[i].param1.y = 10.0f;     // Degrees
		::g_pTheLightManager->vecTheLights[i].param1.z = 20.0f;     // Degrees
	}
	::g_pTheLightManager->vecTheLights[7].name = "Beholder1L";
	::g_pTheLightManager->vecTheLights[8].name = "Beholder1M";
	::g_pTheLightManager->vecTheLights[9].name = "Beholder1R";
	::g_pTheLightManager->vecTheLights[10].name = "Beholder2L";
	::g_pTheLightManager->vecTheLights[11].name = "Beholder2M";
	::g_pTheLightManager->vecTheLights[12].name = "Beholder2R";
	::g_pTheLightManager->vecTheLights[13].name = "Beholder3L";
	::g_pTheLightManager->vecTheLights[14].name = "Beholder3M";
	::g_pTheLightManager->vecTheLights[15].name = "Beholder3R";

	// Load the models

	if (!LoadModelTypesIntoVAO("assets/PLYFilesToLoadIntoVAO.txt", pVAOManager, shaderID))
	{
		if (!LoadModelTypesIntoVAO("PLYFilesToLoadIntoVAO.txt", pVAOManager, shaderID))
		{
			std::cout << "Error: Unable to load list of models to load into VAO file" << std::endl;
		}
		// Do we exit here?
		// (How do we clean up stuff we've made, etc.)
	}//if (!LoadModelTypesIntoVAO...

// **************************************************************************************
// START OF: LOADING the file types into the VAO manager:
/*
*/
// END OF: LOADING the file types into the VAO manager
// **************************************************************************************

	// On the heap (we used new and there's a pointer)

	if (!CreateObjects("assets/createObjects.txt"))
	{
		if (!CreateObjects("createObjects.txt"))
		{
			std::cout << "Error: Unable to load list of objects to create" << std::endl;
		}
	}

	cMeshObject* pSkyBox = new cMeshObject();
	pSkyBox->meshName = "Skybox_Sphere";
	pSkyBox->friendlyName = "skybox";

	cMeshObject* pTerrain = new cMeshObject();
	pTerrain->meshName = "Terrain";
	pTerrain->friendlyName = "Terrain";
	pTerrain->bUse_RGBA_colour = false;
	pTerrain->position = glm::vec3(-128.f, -50.0f, -64.f);
	pTerrain->isWireframe = false;
	pTerrain->scaleXYZ = glm::vec3(1.f);
	pTerrain->textures[0] = "grass2.bmp";
	pTerrain->textureRatios[0] = 1.f;
	pTerrain->textureRatios[1] = 1.f;
	pTerrain->textureRatios[2] = 1.f;
	pTerrain->textureRatios[3] = 1.f;
	g_pMeshObjects.push_back(pTerrain);
	cMeshObject* pTerrainWireFrame = new cMeshObject();
	pTerrainWireFrame->meshName = "Terrain";
	pTerrainWireFrame->friendlyName = "TerrainWireframe";
	pTerrainWireFrame->bUse_RGBA_colour = false;
	pTerrainWireFrame->position = glm::vec3(-128.f, -49.9f, -64.f);
	pTerrainWireFrame->isWireframe = true;
	pTerrainWireFrame->scaleXYZ = glm::vec3(1.f);
	g_pMeshObjects.push_back(pTerrainWireFrame);

	//cMeshObject* pWarrior = new cMeshObject();
	//pWarrior->meshName = "Warrior";
	//pWarrior->friendlyName = "Warrior";
	//pWarrior->position = glm::vec3(0.f, 20.f, 0.f);
	//pWarrior->bUse_RGBA_colour = false;
	//pWarrior->scaleXYZ = glm::vec3(1.f);
	//pWarrior->setRotationFromEuler(glm::vec3(0.f, glm::radians(90.f), glm::radians(90.f)));
	//pWarrior->textures[0] = "Warrior_Texture.bmp";
	//pWarrior->textureRatios[0] = 1.f;
	//pWarrior->textureRatios[1] = 1.f;
	//pWarrior->textureRatios[2] = 1.f;
	//pWarrior->textureRatios[3] = 1.f;
	//cMeshObject* pWarriorSword = new cMeshObject();
	//pWarriorSword->meshName = "WarriorSword";
	//pWarriorSword->friendlyName = "WarriorSword";
	////pWarriorSword->position = glm::vec3(0.f, 25.f, 4.1f);
	//pWarriorSword->position = pWarrior->position + glm::vec3(0.f, 0.f, 0.f);
	//pWarriorSword->bUse_RGBA_colour = false;
	//pWarriorSword->scaleXYZ = glm::vec3(0.05f);
	//pWarriorSword->setRotationFromEuler(glm::vec3(0.f, 0.f, 0.f));
	//pWarriorSword->textures[0] = "Warrior_Sword_Texture.bmp";
	//pWarriorSword->textureRatios[0] = 1.f;
	//pWarriorSword->textureRatios[1] = 1.f;
	//pWarriorSword->textureRatios[2] = 1.f;
	//pWarriorSword->textureRatios[3] = 1.f;
	//g_pMeshObjects.push_back(pWarriorSword);
	//basic Terrain Ground 0 0 0 0 0 0 1
	// DEBUG SPHERES
	pDebugSphere_1 = new cMeshObject();
	pDebugSphere_1->meshName = "ISO_Sphere_1";
	pDebugSphere_1->friendlyName = "Debug_Sphere_1";
	pDebugSphere_1->bUse_RGBA_colour = true;
	pDebugSphere_1->RGBA_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_1->isWireframe = true;
	pDebugSphere_1->SetUniformScale(1);

	pDebugSphere_1->bDoNotLight = true;

	g_pMeshObjects.push_back(pDebugSphere_1);

	pDebugSphere_2 = new cMeshObject();
	pDebugSphere_2->meshName = "ISO_Sphere_1";
	pDebugSphere_2->friendlyName = "Debug_Sphere_2";
	pDebugSphere_2->bUse_RGBA_colour = true;
	pDebugSphere_2->RGBA_colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	pDebugSphere_2->isWireframe = true;
	pDebugSphere_2->SetUniformScale(1);
	pDebugSphere_2->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_2);

	pDebugSphere_3 = new cMeshObject();
	pDebugSphere_3->meshName = "ISO_Sphere_1";
	pDebugSphere_3->friendlyName = "Debug_Sphere_3";
	pDebugSphere_3->bUse_RGBA_colour = true;
	pDebugSphere_3->RGBA_colour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere_3->isWireframe = true;
	pDebugSphere_3->SetUniformScale(1);
	pDebugSphere_3->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_3);

	pDebugSphere_4 = new cMeshObject();
	pDebugSphere_4->meshName = "ISO_Sphere_1";
	pDebugSphere_4->friendlyName = "Debug_Sphere_4";
	pDebugSphere_4->bUse_RGBA_colour = true;
	pDebugSphere_4->RGBA_colour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_4->isWireframe = true;
	pDebugSphere_4->SetUniformScale(1);
	pDebugSphere_4->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_4);

	pDebugSphere_5 = new cMeshObject();
	pDebugSphere_5->meshName = "ISO_Sphere_1";
	pDebugSphere_5->friendlyName = "Debug_Sphere_5";
	pDebugSphere_5->bUse_RGBA_colour = true;
	pDebugSphere_5->RGBA_colour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	pDebugSphere_5->isWireframe = true;
	pDebugSphere_5->SetUniformScale(1);
	pDebugSphere_5->bDoNotLight = true;
	g_pMeshObjects.push_back(pDebugSphere_5);

	GLint mvp_location = glGetUniformLocation(shaderID, "MVP");       // program
	// uniform mat4 mModel;
	// uniform mat4 mView;
	// uniform mat4 mProjection;
	GLint mModel_location = glGetUniformLocation(shaderID, "mModel");
	GLint mView_location = glGetUniformLocation(shaderID, "mView");
	GLint mProjection_location = glGetUniformLocation(shaderID, "mProjection");
	// Need this for lighting
	GLint mModelInverseTransform_location = glGetUniformLocation(shaderID, "mModelInverseTranspose");

	// Textures
	::g_pTextureManager = new cBasicTextureManager();
	::g_pTextureManager->SetBasePath("assets/textures");
	::g_pTextureManager->Create2DTextureFromBMPFile("Dungeons_2_Texture_01_A.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("Dungeons_Texture_01.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("grass.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("carbon.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("Warrior_Texture.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("Cleric_Texture.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("RedCleric.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("grass2.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("Medieval_Texture.bmp");
	::g_pTextureManager->Create2DTextureFromBMPFile("woodTexture.bmp");
	// ICONS
	{
		::g_pTextureManager->SetBasePath("assets/icons");
		::g_pTextureManager->Create2DTextureFromBMPFile("Bread.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("cleric.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Coin.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Crystal.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Minerals.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("monk.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Paper.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("ranger.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("rogue.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Skull.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Stone.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("warrior.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("wizard.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Wood.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Workstation.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Forge.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("Anvil.bmp");
		::g_pTextureManager->Create2DTextureFromBMPFile("TrainingDummy.bmp");
		::g_pTextureManager->SetBasePath("assets/textures");
	}
	std::string errorString = "";
	if (::g_pTextureManager->CreateCubeTextureFromBMPFiles("TropicalSunnyDay",
		"TropicalSunnyDayRight2048.bmp", /* positive X */
		"TropicalSunnyDayLeft2048.bmp",  /* negative X */
		"TropicalSunnyDayDown2048.bmp",    /* positive Y */
		"TropicalSunnyDayUp2048.bmp",  /* negative Y */
		"TropicalSunnyDayBack2048.bmp",  /* positive Z */
		"TropicalSunnyDayFront2048.bmp", /* negative Z */
		true, errorString))
	{
		std::cout << "Loaded the tropical sunny day cube map OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR: Didn't load the tropical sunny day cube map. How sad." << std::endl;
		std::cout << "(because: " << errorString << std::endl;
	}
	std::cout.flush();
	// GUI
	ImGui::StyleColorsDark();
	GUI EditGUI;
	GameGUI gameGUI;

	float deltaTime = std::clock();
	float duration = 0;
	//bool increase = true;
	int increase = 1;
	//pVAOManager->Load();

	// START OF PHYSICS
	PhysicsHelper* physicsHelper = new PhysicsHelper();

	// Initialize physicsfactory, only non-interface call
	using namespace physics;
	_physicsFactory = new PhysicsFactory;
	rayCast = _physicsFactory->CreateRayCast();
	world = _physicsFactory->CreateWorld();;
	renderBuildingMesh = new cMeshObject();
	renderBuildingMesh->meshName = "Crate";
	renderBuildingMesh->friendlyName = "BuildingPreRender";
	renderBuildingMesh->bUse_RGBA_colour = true;
	renderBuildingMesh->RGBA_colour = glm::vec4(0.f, 1.f, 0.f, 0.5f);
	renderBuildingMesh->bIsVisible = false;
	renderBuildingMesh->scaleXYZ = glm::vec3(1.f);
	g_pMeshObjects.push_back(renderBuildingMesh);
	// LOAD TERRAIN
	buildingManager = new BuildingManager();
	{
		sModelDrawInfo terrainInfo;
		pVAOManager->FindDrawInfoByModelName("Terrain", terrainInfo);
		float resolution = 1.0f;
		//unsigned int gridWidth, gridDepth;
		//physicsHelper->getTerrainGridSize(terrainInfo, resolution, gridWidth, gridDepth);
		//std::vector<float> heightData = physicsHelper->generateHeightData(terrainInfo, gridWidth, gridDepth);
		std::vector<Vector3> vertices;
		for (size_t i = 0; i < terrainInfo.numberOfVertices; i++)
		{
			Vector3 v;
			v.x = terrainInfo.pVertices[i].x;
			v.y = terrainInfo.pVertices[i].y;
			v.z = terrainInfo.pVertices[i].z;
			vertices.push_back(v);
		}
		std::vector<unsigned int> indices;
		for (size_t i = 0; i < terrainInfo.numberOfIndices; i++)
		{
			indices.push_back(terrainInfo.pIndices[i]);
		}
		iShape* terrainShape = new TriangleMeshShape(vertices, indices);
		int id = IDGenerator::GenerateID();
		terrainShape->SetUserData(id);
		RigidBodyDesc terrainDesc;
		terrainDesc.isStatic = true;
		terrainDesc.position = Vector3(-128.0f, -50.0f, -64.0f);
		world->AddBody(_physicsFactory->CreateRigidBody(terrainDesc, terrainShape));
		// LOAD IN TERRAIN OBJECTS
		GameObject* goTerrain = new GameObject();
		goTerrain->id = id;
		goTerrain->mesh = pTerrain;
		terrainManager = new TerrainManager(goTerrain, &terrainInfo);
		const int maxObjects[3] = { 150,80,20 };
		terrainManager->placeObjectsOnTerrain(maxObjects);
		goMap.emplace(id, goTerrain);
	}
	// CRAFTING RECIPES
	{
		std::map<itemId, int> workstationRecipe;
		workstationRecipe.emplace(wood, 10);

		buildingRecipes.emplace(WORKSHOP, workstationRecipe);

		std::map<itemId, int> forgeRecipe;
		forgeRecipe.emplace(wood, 5);
		forgeRecipe.emplace(stone, 20);

		buildingRecipes.emplace(FORGE, forgeRecipe);

		std::map<itemId, int> anvilRecipe;
		anvilRecipe.emplace(stone, 10);
		anvilRecipe.emplace(ores, 10);

		buildingRecipes.emplace(ANVIL, anvilRecipe);

		std::map<itemId, int> trainingDummy;
		trainingDummy.emplace(wood, 5);

		buildingRecipes.emplace(DUMMY, trainingDummy);
	}

	const int NUMCOLONISTS = 3;
	colonistManager = new ColonistManager();
	for (int i = 0; i < NUMCOLONISTS; i++) {
		// Create colonist mesh
		cMeshObject* pColonistMesh = new cMeshObject();
		pColonistMesh->meshName = "Warrior";
		pColonistMesh->friendlyName = "Colonist" + std::to_string(i);
		pColonistMesh->position = glm::vec3(0.f, 20.f, 0.f);
		pColonistMesh->bUse_RGBA_colour = false;
		pColonistMesh->scaleXYZ = glm::vec3(1);
		pColonistMesh->setRotationFromEuler(glm::vec3(0.f, glm::radians(90.f), glm::radians(90.f)));
		pColonistMesh->textures[0] = "Warrior_Texture.bmp";
		pColonistMesh->textureRatios[0] = 1.f;
		pColonistMesh->textureRatios[1] = 1.f;
		pColonistMesh->textureRatios[2] = 1.f;
		pColonistMesh->textureRatios[3] = 1.f;
		// Add selection torus
		{
			cMeshObject* pTorus = new cMeshObject();
			pTorus->meshName = "Torus";
			pTorus->friendlyName = "Torus" + std::to_string(i);
			pTorus->position = glm::vec3(0.f, 20.f, 0.f);
			pTorus->bUse_RGBA_colour = true;
			pTorus->RGBA_colour = glm::vec4(0.f, 1.f, 1.f, 0.5f);
			pTorus->scaleXYZ = glm::vec3(1.f);
			//pTorus->setRotationFromEuler(glm::vec3(0.f, glm::radians(90.f), glm::radians(90.f)));
			pTorus->bIsVisible = false;
			pColonistMesh->vecChildMeshes.push_back(pTorus);
		}
		// Create GameObject
		GameObject* goColonist = new GameObject();
		goColonist->id = IDGenerator::GenerateID();
		goColonist->mesh = pColonistMesh;
		goColonist->mesh->scaleXYZ = glm::vec3(0.01f);
		goColonist->animCharacter = animationManager->CreateAnimatedCharacter("assets/models/Characters/riggedWarrior.fbx", goColonist, glm::vec3(0.01f));
		goColonist->animCharacter->SetAnimation(10);
		// Create Character controller
		iShape* cylinderShape = new CylinderShape(Vector3(0.7f, 2.f, 0.7f));
		cylinderShape->SetUserData(goColonist->id);
		glm::vec3 position = glm::vec3(i + 2.f, 1.f, i - 2.f);
		glm::quat rotation = glm::quat(glm::vec3(0));
		iCharacterController* playerCharacterController = _physicsFactory->CreateCharacterController(cylinderShape, position, rotation);
		world->AddCharacterController(playerCharacterController);
		playerCharacterController->SetGravity(Vector3(0.f, -9.81f, 0.f));
		goColonist->characterController = playerCharacterController;
		goVector.push_back(goColonist);
		goMap.emplace(goColonist->id, goColonist);
		// Add to Colonist Manager
		colonistManager->AddColonist(goColonist);
	}
	eventSystem = new EventSystem();
	float g_PrevTime = 0.f;
	g_cameraTarget = glm::vec3(0.f, 0, 0.f);
	g_cameraEye = glm::vec3(1.f, 150, 0.f);
	bool isKeyPressed = false;
	gPathFinder->calculateFlowfield(TerrainManager::worldToGridCoords(gDepot->mesh->position));
	GameGUI::addMessage("Welcome to Arcane Settlers!");
	GameGUI::addMessage("To begin either click on a unit, or drag a box to select multiple.");
	GameGUI::addMessage("Once you have done so, click on a resource like a tree to order them to chop it down.");
	GameGUI::addMessage("Colonists will drop their loot off at the depot once they are full, you can click on the Depot to view its contents.");
	GameGUI::addMessage("Enemies will raid soon so it's a good idea to prepare, create a workstation to open the other buildings, then make a Training Dummy for increased combat strength.");
	while (!glfwWindowShouldClose(window))
	{
		renderTransparentBuildingMesh();
		if (!gPause)
		{
			eventSystem->Update();
			colonistManager->Update();
			// Play random animation
			duration = (std::clock() - deltaTime) / (double)CLOCKS_PER_SEC;
			if (duration > 2.f)
			{
				deltaTime = std::clock();
				//goWarrior->animCharacter->SetAnimation(rand() % 14, 1.5f);
			}
			// Update animation manager(all animations)
			{
				double currTime = glfwGetTime();
				float elapsedTimeInSeconds = (float)(currTime - g_PrevTime);
				g_PrevTime = currTime;

				if (elapsedTimeInSeconds > 0.1f)
					elapsedTimeInSeconds = 0.1f;
				if (elapsedTimeInSeconds <= 0.f)
					elapsedTimeInSeconds = 0.001f;

				animationManager->UpdateAll(elapsedTimeInSeconds);
			}
			world->TimeStep(1.f);

			duration = (std::clock() - deltaTime) / (double)CLOCKS_PER_SEC;
			// Update each object to match its rigidbodies transform
			for (GameObject* go : gameObjects)
			{
				if (go->rigidBody)
				{
					go->mesh->position = go->rigidBody->GetGLMPosition();
					physics::Quaternion rot;
					go->rigidBody->GetRotation(rot);
					go->mesh->qRotation = glm::quat(rot.w, rot.x, rot.y, rot.z);
				}
			}
		}
		::g_pTheLightManager->CopyLightInformationToShader(shaderID);
		// Mouse Lookaround
		if (!menuMode && theEditMode != PHYSICS_TEST)
			mouse_camera_update(window);
		DrawConcentricDebugLightObjects();
		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

		// Different matview based on camera being used
		if (theEditMode == PHYSICS_TEST)
		{
			matView = glm::lookAt(::g_cameraEye,
				::g_cameraTarget,
				upVector);
		}
		else {
			matView = glm::lookAt(::g_cameraEye,
				::g_cameraEye + ::g_cameraTarget,
				upVector);
		}
		// Pass eye location to the shader
		// uniform vec4 eyeLocation;
		GLint eyeLocation_UniLoc = glGetUniformLocation(shaderID, "eyeLocation");

		glUniform4f(eyeLocation_UniLoc,
			::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);

		matProjection = glm::perspective(
			0.6f,       // Field of view (in degress, more or less 180)
			ratio,
			0.1f,       // Near plane (make this as LARGE as possible)
			10000.0f);   // Far plane (make this as SMALL as possible)
		// 6-8 digits of precision
		//
// Set once per scene (not per object)
		glUniformMatrix4fv(mView_location, 1, GL_FALSE, glm::value_ptr(matView));
		glUniformMatrix4fv(mProjection_location, 1, GL_FALSE, glm::value_ptr(matProjection));
		//    ____  _             _            __
		//   / ___|| |_ __ _ _ __| |_    ___  / _|  ___  ___ ___ _ __   ___
		//   \___ \| __/ _` | '__| __|  / _ \| |_  / __|/ __/ _ \ '_ \ / _ \
        //    ___) | || (_| | |  | |_  | (_) |  _| \__ \ (_|  __/ | | |  __/
		//   |____/ \__\__,_|_|   \__|  \___/|_|   |___/\___\___|_| |_|\___|
		//
		// We draw everything in our "scene"
		// In other words, go throug the vec_pMeshObjects container
		//  and draw each one of the objects
		// Separate from main draw so its easier to clear them
		for (GameObject* go : randomBalls)
		{
			// The parent's model matrix is set to the identity
			glm::mat4x4 matModel = glm::mat4x4(1.0f);

			// All the drawing code has been moved to the DrawObject function
			if (go->mesh == nullptr)
				continue;
			DrawObject(go->mesh,
				matModel,
				shaderID, ::g_pTextureManager,
				pVAOManager, mModel_location, mModelInverseTransform_location);
		}
		for (GameObject* go : goVector)
		{
			if (go == nullptr)
				continue;
			if (go->hasBones)
			{
				for (size_t i = 0; i < go->BoneModelMatrices.size(); i++)
				{
					std::string name = "BoneMatrices[" + std::to_string(i) + "]";
					GLint boneMatrix = glGetUniformLocation(shaderID, name.c_str());
					glUniformMatrix4fv(boneMatrix, 1, GL_FALSE, glm::value_ptr(go->BoneModelMatrices[i]));
					//std::string name = "BoneMatrices[" + std::to_string(i) + "]";
				}
				GLint bHasBones = glGetUniformLocation(shaderID, "bHasBones");
				if (go->hasBones)
				{
					glUniform1f(bHasBones, (GLfloat)GL_TRUE);
				}
				// The parent's model matrix is set to the identity
				glm::mat4x4 matModel = glm::mat4x4(1.0f);

				// All the drawing code has been moved to the DrawObject function
				if (go->mesh == nullptr)
					continue;

		
				DrawObject(go->mesh,
					matModel,
					shaderID, ::g_pTextureManager,
					pVAOManager, mModel_location, mModelInverseTransform_location);
				glUniform1f(bHasBones, (GLfloat)GL_FALSE);
				if (go->isSelected)
				{
					for (cMeshObject* mesh : go->mesh->vecChildMeshes)
					{
						mesh->position = go->mesh->position;
						mesh->position -= 0.1f;
						mesh->qRotation = go->mesh->qRotation;
						glm::mat4x4 mModel = glm::mat4x4(1.0f);
						DrawObject(mesh,
							mModel,
							shaderID, ::g_pTextureManager,
							pVAOManager, mModel_location, mModelInverseTransform_location);
					}
				}
			}
		}
		for (std::vector< cMeshObject* >::iterator itCurrentMesh = g_pMeshObjects.begin();
			itCurrentMesh != g_pMeshObjects.end();
			itCurrentMesh++)
		{
			cMeshObject* pCurrentMeshObject = *itCurrentMesh;        // * is the iterator access thing

			// Where do I put THIS??
			// i.e. if the object is invisble by the child objects ARE visible...?
			if (!pCurrentMeshObject->bIsVisible)
			{
				// Skip the rest of the loop
				continue;
			}
			// The parent's model matrix is set to the identity
			glm::mat4x4 matModel = glm::mat4x4(1.0f);

			// All the drawing code has been moved to the DrawObject function
			DrawObject(pCurrentMeshObject,
				matModel,
				shaderID, ::g_pTextureManager,
				pVAOManager, mModel_location, mModelInverseTransform_location);
		}//for ( unsigned int index

		// Draw the skybox
		{
			GLint bIsSkyboxObject_UL = glGetUniformLocation(shaderID, "bIsSkyboxObject");
			glUniform1f(bIsSkyboxObject_UL, (GLfloat)GL_TRUE);

			glm::mat4x4 matModel = glm::mat4x4(1.0f);

			// move skybox to the cameras location
			pSkyBox->position = ::g_cameraEye;

			// The scale of this large skybox needs to be smaller than the far plane
			//  of the projection matrix.
			// Maybe make it half the size
			// Here, our far plane is 10000.0f...
			pSkyBox->SetUniformScale(7500.0f);

			// All the drawing code has been moved to the DrawObject function
			DrawObject(pSkyBox,
				matModel,
				shaderID, ::g_pTextureManager,
				pVAOManager, mModel_location, mModelInverseTransform_location);

			// Turn this off
			glUniform1f(bIsSkyboxObject_UL, (GLfloat)GL_FALSE);
		}//for ( unsigned int index
		//    _____           _          __
		//   | ____|_ __   __| |   ___  / _|  ___  ___ ___ _ __   ___
		//   |  _| | '_ \ / _` |  / _ \| |_  / __|/ __/ _ \ '_ \ / _ \
        //   | |___| | | | (_| | | (_) |  _| \__ \ (_|  __/ | | |  __/
		//   |_____|_| |_|\__,_|  \___/|_|   |___/\___\___|_| |_|\___|
		//

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		EditGUI.render();
		gameGUI.render();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);

		// Set the window title
		//glfwSetWindowTitle(window, "Hey");

		//std::stringstream ssTitle;
		//ssTitle << "Camera (x,y,z): "
		//	<< ::g_cameraEye.x << ", "
		//	<< ::g_cameraEye.y << ", "
		//	<< ::g_cameraEye.z
		//	<< "Target (x,y,z): "
		//	<< ::g_cameraTarget.x << ", "
		//	<< ::g_cameraTarget.y << ", "
		//	<< ::g_cameraTarget.z
		//	<< "  Light #" << currentLight << " (xyz): "
		//	<< ::g_pTheLightManager->vecTheLights[currentLight].position.x << ", "
		//	<< ::g_pTheLightManager->vecTheLights[currentLight].position.y << ", "
		//	<< ::g_pTheLightManager->vecTheLights[currentLight].position.z
		//	<< " linear: " << ::g_pTheLightManager->vecTheLights[currentLight].atten.y
		//	<< " quad: " << ::g_pTheLightManager->vecTheLights[currentLight].atten.z;

		//std::string theText = ssTitle.str();
		std::string theText = "Arcane Settlers - Hassan Assaf";

		glfwSetWindowTitle(window, theText.c_str());
		// Or this:
		//std::string theText = ssTitle.str();
		//glfwSetWindowTitle(window, ssTitle.str().c_str() );
	}

	// Get rid of stuff
	delete pTheShaderManager;
	delete ::g_pTheLightManager;
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
void DrawConcentricDebugLightObjects(void)
{
	extern bool bEnableDebugLightingObjects;

	if (!bEnableDebugLightingObjects)
	{
		pDebugSphere_1->bIsVisible = false;
		pDebugSphere_2->bIsVisible = false;
		pDebugSphere_3->bIsVisible = false;
		pDebugSphere_4->bIsVisible = false;
		pDebugSphere_5->bIsVisible = false;
		return;
	}

	pDebugSphere_1->bIsVisible = false;
	pDebugSphere_2->bIsVisible = false;
	pDebugSphere_3->bIsVisible = false;
	pDebugSphere_4->bIsVisible = false;
	pDebugSphere_5->bIsVisible = false;

	cLightHelper theLightHelper;

	// Move the debug sphere to where the light #0 is
	pDebugSphere_1->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_2->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_3->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_4->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	pDebugSphere_5->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance75percent = theLightHelper.calcApproxDistFromAtten(
			0.75f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_2->SetUniformScale(distance75percent);
		pDebugSphere_2->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance50percent = theLightHelper.calcApproxDistFromAtten(
			0.50f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_3->SetUniformScale(distance50percent);
		pDebugSphere_3->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance25percent = theLightHelper.calcApproxDistFromAtten(
			0.25f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_4->SetUniformScale(distance25percent);
		pDebugSphere_4->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}

	{
		// Draw a bunch of concentric spheres at various "brightnesses"
		float distance5percent = theLightHelper.calcApproxDistFromAtten(
			0.05f,  // 75%
			0.001f,
			100000.0f,
			::g_pTheLightManager->vecTheLights[currentLight].atten.x,
			::g_pTheLightManager->vecTheLights[currentLight].atten.y,
			::g_pTheLightManager->vecTheLights[currentLight].atten.z);

		pDebugSphere_5->SetUniformScale(distance5percent);
		pDebugSphere_5->position = glm::vec3(::g_pTheLightManager->vecTheLights[currentLight].position);
	}
	return;
}

void renderTransparentBuildingMesh()
{
	if (selectedBuilding == BuildingType::NONE)
	{
		renderBuildingMesh->bIsVisible = false;
		return;
	}

	if (ImGui::GetIO().WantCaptureMouse)
		return;
	int width, height;
	double mouseX, mouseY;

	renderBuildingMesh->bIsVisible = true;

	glfwGetFramebufferSize(window, &width, &height);
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glm::ivec4 viewport(0, 0, width, height);
	glm::vec3 rayDirection = GetRayDirection(mouseX, mouseY, matView, matProjection, width, height);

	// Do raycast
	iRayCast::RayCastHit hit;
	if (rayCast->doRayCast(g_cameraEye, rayDirection, 5000, hit))
	{
		glm::vec3 normal;
		float terrainHeight;

		TerrainManager::getTerrainHeightAndNormal(hit.position, terrainHeight, normal);
		renderBuildingMesh->meshName = getBuildingMesh(selectedBuilding);
		renderBuildingMesh->position = hit.position;
		renderBuildingMesh->qRotation = glm::quatLookAt(-normal, glm::vec3(0.0f, 1.0f, 0.0f));
		if (selectedBuilding == DUMMY || selectedBuilding == FORGE || selectedBuilding == ANVIL)
		{
			renderBuildingMesh->qRotation *= glm::quat(glm::vec3(glm::radians(90.f), 0.f, 0.f));
		}
		float scale = getBuildingScale(selectedBuilding);
		renderBuildingMesh->scaleXYZ = glm::vec3(scale);
	}
}