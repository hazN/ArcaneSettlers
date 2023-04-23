#include "c3DModelFileLoader.h"
#include "..\globalThings.h"
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
extern glm::vec3 g_cameraEye;
extern glm::vec3 g_cameraTarget;
Assimp::Importer importer;
void CastToGLM(const aiMatrix4x4& in, glm::mat4& out)
{
	out = glm::mat4(in.a1, in.b1, in.c1, in.d1,
		in.a2, in.b2, in.c2, in.d2,
		in.a3, in.b3, in.c3, in.d3,
		in.a4, in.b4, in.c4, in.d4);
}
bool c3DModelFileLoader::LoadPLYFile_Format_XYZ_N_RGBA_UV(std::string filename,
	sModelDrawInfo& modelDrawInfo,
	std::string& errorText)
{
	errorText = "";     // Clear the error (i.e. no error)

	struct sVertex_XYZ_N_RGBA_UV
	{
		float x, y, z;
		float nx, ny, nz;
		//uchar red, green, blue, alpha;
		float red, green, blue, alpha;
		float texture_u, texture_v;
	};

	struct sTrianglePLY
	{
		// The 3 vertex index values from the ply file
		unsigned int vertexIndices[3];
	};

	// Array we will load into
	sVertex_XYZ_N_RGBA_UV* pTheModelArray = NULL;   // NULL, 0, nullptr
	sTrianglePLY* pTheModelTriangleArray = NULL;
	//    unsigned int numberOfvertices = 0;
	//    unsigned int numberOfTriangles = 0;

		// Start loading the file

	std::ifstream theFile(filename);
	if (!theFile.is_open())
	{
		//        std::cout << "Didn't open it!" << std::endl;
		errorText = "Can't open the file.";
		return false;
	}

	// Read an entire line
	char buffer[10000];
	theFile.getline(buffer, 10000);

	std::string theNextToken;

	// Scan for the word "vertex"
	while (theFile >> theNextToken)
	{
		if (theNextToken == "vertex")
		{
			break;
		}
	}
	//
	theFile >> modelDrawInfo.numberOfVertices;

	// Scan for the word "face"
	while (theFile >> theNextToken)
	{
		if (theNextToken == "face")
		{
			break;
		}
	}
	//
	theFile >> modelDrawInfo.numberOfTriangles;

	// Scan for the word "end_header"
	while (theFile >> theNextToken)
	{
		if (theNextToken == "end_header")
		{
			break;
		}
	}

	// Now we load the vertices
	// -0.036872 0.127727 0.00440925 0.2438786 0.9638011 -0.1077533 127 127 127 255 0.337485 0.8899501

	pTheModelArray = new sVertex_XYZ_N_RGBA_UV[modelDrawInfo.numberOfVertices];

	//    std::cout << "Loading";
	for (unsigned int count = 0; count != modelDrawInfo.numberOfVertices; count++)
	{
		theFile >> pTheModelArray[count].x;
		theFile >> pTheModelArray[count].y;
		theFile >> pTheModelArray[count].z;

		theFile >> pTheModelArray[count].nx;
		theFile >> pTheModelArray[count].ny;
		theFile >> pTheModelArray[count].nz;

		theFile >> pTheModelArray[count].red;
		theFile >> pTheModelArray[count].green;
		theFile >> pTheModelArray[count].blue;
		theFile >> pTheModelArray[count].alpha;

		theFile >> pTheModelArray[count].texture_u;
		theFile >> pTheModelArray[count].texture_v;

		//        if (count % 10000 == 0)
		//        {
		//            std::cout << ".";
		//        }
	}
	//    std::cout << "done" << std::endl;

		// Load the faces (or triangles)
	pTheModelTriangleArray = new sTrianglePLY[modelDrawInfo.numberOfTriangles];

	for (unsigned int count = 0; count != modelDrawInfo.numberOfTriangles; count++)
	{
		// 3 15393 15394 15395
		unsigned int discard = 0;
		theFile >> discard;

		theFile >> pTheModelTriangleArray[count].vertexIndices[0];
		theFile >> pTheModelTriangleArray[count].vertexIndices[1];
		theFile >> pTheModelTriangleArray[count].vertexIndices[2];
	}

	theFile.close();

	// This is now different because the vertex layout in the shader is different
	modelDrawInfo.pVertices = new sVertex_RGBA_XYZ_N_UV_T_BiN_Bones[modelDrawInfo.numberOfVertices];
	//    modelDrawInfo.pVertices = new sVertex[modelDrawInfo.numberOfVertices];

		// Now copy the information from the PLY infomation to the model draw info structure
	for (unsigned int index = 0; index != modelDrawInfo.numberOfVertices; index++)
	{
		// To The Shader                        From the file
		modelDrawInfo.pVertices[index].x = pTheModelArray[index].x;
		modelDrawInfo.pVertices[index].y = pTheModelArray[index].y;
		modelDrawInfo.pVertices[index].z = pTheModelArray[index].z;

		modelDrawInfo.pVertices[index].r = pTheModelArray[index].red;
		modelDrawInfo.pVertices[index].g = pTheModelArray[index].green;
		modelDrawInfo.pVertices[index].b = pTheModelArray[index].blue;

		// Copy the normal information we loaded, too! :)
		modelDrawInfo.pVertices[index].nx = pTheModelArray[index].nx;
		modelDrawInfo.pVertices[index].ny = pTheModelArray[index].ny;
		modelDrawInfo.pVertices[index].nz = pTheModelArray[index].nz;

		// Copy the texture coordinates we loaded
		modelDrawInfo.pVertices[index].u0 = pTheModelArray[index].texture_u;
		modelDrawInfo.pVertices[index].v0 = pTheModelArray[index].texture_v;
		// For now, I'll ignore the other two
	}

	modelDrawInfo.numberOfIndices = modelDrawInfo.numberOfTriangles * 3;

	// This is the "index" (or element) buffer
	modelDrawInfo.pIndices = new unsigned int[modelDrawInfo.numberOfIndices];

	unsigned int vertex_element_index_index = 0;

	for (unsigned int triangleIndex = 0; triangleIndex != modelDrawInfo.numberOfTriangles; triangleIndex++)
	{
		modelDrawInfo.pIndices[vertex_element_index_index + 0] = pTheModelTriangleArray[triangleIndex].vertexIndices[0];
		modelDrawInfo.pIndices[vertex_element_index_index + 1] = pTheModelTriangleArray[triangleIndex].vertexIndices[1];
		modelDrawInfo.pIndices[vertex_element_index_index + 2] = pTheModelTriangleArray[triangleIndex].vertexIndices[2];

		// Each +1 of the triangle index moves the "vertex element index" by 3
		// (3 vertices per triangle)
		vertex_element_index_index += 3;
	}

	delete[] pTheModelArray;
	delete[] pTheModelTriangleArray;

	return true;
}

bool c3DModelFileLoader::LoadFBXFile_Format_XYZ_N_RGBA_UV(std::string filename, sModelDrawInfo& modelDrawInfo, std::string& errorText)
{
	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights);

	// Check for errors
	if (!scene)
	{
		errorText = "Failed to load FBX file: " + filename;
		std::cout << errorText << std::endl;
		return false;
	}
	else std::cout << "Loaded FBX File: " << filename << std::endl;
	CharacterAnimationData animationData(scene);

	// Load mesh
	if (scene->HasMeshes())
	{
		unsigned int totalVertices = 0;
		unsigned int totalTriangles = 0;
		// Count total vertices/triangles for all meshes
		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			aiMesh* mesh = scene->mMeshes[meshIndex];
			std::cout << "Loading mesh " << mesh->mName.C_Str() << std::endl;
			totalVertices += mesh->mNumVertices;
			totalTriangles += mesh->mNumFaces;
		}
		modelDrawInfo.numberOfVertices = totalVertices;
		modelDrawInfo.numberOfTriangles = totalTriangles;
		modelDrawInfo.numberOfIndices = totalTriangles * 3;
		modelDrawInfo.pVertices = new sVertex_RGBA_XYZ_N_UV_T_BiN_Bones[modelDrawInfo.numberOfVertices];
		modelDrawInfo.pIndices = new unsigned int[modelDrawInfo.numberOfIndices];

		// Offsets for combining mesh
		unsigned int vertexOffset = 0;
		unsigned int indexOffset = 0;
		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			aiMesh* mesh = scene->mMeshes[meshIndex];
			unsigned int numVertices = mesh->mNumVertices;
			unsigned int numTriangles = mesh->mNumFaces;
			unsigned int numIndices = numTriangles * 3;

			// Create a bone reference map
			int totalWeights = 0;

			std::vector<BoneVertexData> boneVertexData;
			boneVertexData.resize(mesh->mNumVertices);
			int boneCount = 0;
			for (int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* bone = mesh->mBones[i];

				// Bone ids
				int boneIdx = 0;
				std::string boneName(bone->mName.data);

				std::map<std::string, int>::iterator it = animationData.boneNameToIdMap.find(boneName);
				if (it == animationData.boneNameToIdMap.end())
				{
					boneIdx = boneCount;
					boneCount++;
					BoneInfo bi;
					bi.mName = boneName;
					animationData.boneInfoVec.push_back(bi);

					CastToGLM(bone->mOffsetMatrix, animationData.boneInfoVec[boneIdx].mBoneOffset);
					animationData.boneNameToIdMap[boneName] = boneIdx;
				}
				else
				{
					boneIdx = animationData.boneNameToIdMap[boneName];
				}
				// Bone weights
				for (int weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
				{
					float weight = bone->mWeights[weightIdx].mWeight;
					int vertexId = bone->mWeights[weightIdx].mVertexId;
					boneVertexData[vertexId].AddBoneInfo(boneIdx, weight);
				}
			}

			// Loop through vertices
			for (unsigned int i = 0; i < numVertices; ++i)
			{
				unsigned int currentIndex = vertexOffset + i;

				// Vertices
				modelDrawInfo.pVertices[currentIndex].x = mesh->mVertices[i].x;
				modelDrawInfo.pVertices[currentIndex].y = mesh->mVertices[i].y;
				modelDrawInfo.pVertices[currentIndex].z = mesh->mVertices[i].z;
				modelDrawInfo.pVertices[currentIndex].w = 1.f;

				// Normals
				if (mesh->HasNormals())
				{
					modelDrawInfo.pVertices[currentIndex].nx = mesh->mNormals[i].x;
					modelDrawInfo.pVertices[currentIndex].ny = mesh->mNormals[i].y;
					modelDrawInfo.pVertices[currentIndex].nz = mesh->mNormals[i].z;
					modelDrawInfo.pVertices[currentIndex].nw = 0.f;
				}
				else
				{
					modelDrawInfo.pVertices[currentIndex].nx = 1.f;
					modelDrawInfo.pVertices[currentIndex].ny = 0.f;
					modelDrawInfo.pVertices[currentIndex].nz = 0.f;
					modelDrawInfo.pVertices[currentIndex].nw = 0.f;
				}

				// Texture coordinates
				if (mesh->HasTextureCoords(0))
				{
					modelDrawInfo.pVertices[currentIndex].u0 = mesh->mTextureCoords[0][i].x;
					modelDrawInfo.pVertices[currentIndex].v0 = mesh->mTextureCoords[0][i].y;
				}
				if (mesh->HasTextureCoords(1))
				{
					modelDrawInfo.pVertices[currentIndex].u1 = mesh->mTextureCoords[1][i].x;
					modelDrawInfo.pVertices[currentIndex].v1 = mesh->mTextureCoords[1][i].y;
				}
				// Colors
				if (mesh->mColors[0])
				{
					modelDrawInfo.pVertices[currentIndex].r = mesh->mColors[0][i].r;
					modelDrawInfo.pVertices[currentIndex].g = mesh->mColors[0][i].g;
					modelDrawInfo.pVertices[currentIndex].b = mesh->mColors[0][i].b;
					modelDrawInfo.pVertices[currentIndex].a = mesh->mColors[0][i].a;
				}
				// Use a BoneInformation Map to get bone info and store the values here
				BoneVertexData& bvd = boneVertexData[i];
				modelDrawInfo.pVertices[currentIndex].vBoneID[0] = bvd.mIds[0];
				modelDrawInfo.pVertices[currentIndex].vBoneID[1] = bvd.mIds[1];
				modelDrawInfo.pVertices[currentIndex].vBoneID[2] = bvd.mIds[2];
				modelDrawInfo.pVertices[currentIndex].vBoneID[3] = bvd.mIds[3];

				modelDrawInfo.pVertices[currentIndex].vBoneWeight[0] = bvd.mWeights[0];
				modelDrawInfo.pVertices[currentIndex].vBoneWeight[1] = bvd.mWeights[1];
				modelDrawInfo.pVertices[currentIndex].vBoneWeight[2] = bvd.mWeights[2];
				modelDrawInfo.pVertices[currentIndex].vBoneWeight[3] = bvd.mWeights[3];
			}
			// Load triangles
			for (unsigned int i = 0; i < numTriangles; ++i)
			{
				aiFace face = mesh->mFaces[i];
				modelDrawInfo.pIndices[indexOffset + i * 3] = vertexOffset + face.mIndices[0];
				modelDrawInfo.pIndices[indexOffset + i * 3 + 1] = vertexOffset + face.mIndices[1];
				modelDrawInfo.pIndices[indexOffset + i * 3 + 2] = vertexOffset + face.mIndices[2];
			}
			// Update offsets for the next mesh
			vertexOffset += mesh->mNumVertices;
			indexOffset += mesh->mNumFaces * 3;
		}
	}

	return true;
}

void cVAOManager::Load()
{
	int lightIndex = 0;
	int modelIndex = 0;
	std::ifstream saveFile("saveData.txt");
	if (!saveFile.is_open())
	{
		std::cout << "Load failed..." << std::endl;
	}
	else {
		std::string line;
		while (std::getline(saveFile, line))
		{
			std::istringstream in(line);
			std::string type;
			in >> type;
			if (type == "c")
			{
				std::getline(saveFile, line);
				std::istringstream in2(line);
				in2 >> ::g_cameraEye.x >> ::g_cameraEye.y >> ::g_cameraEye.z >> ::g_cameraTarget.x >> ::g_cameraTarget.y >> ::g_cameraTarget.z;
			}
			if (type == "l")
			{
				if (lightIndex < g_pTheLightManager->vecTheLights.size())
				{
					std::getline(saveFile, line);
					std::istringstream in2(line);
					in2 >> g_pTheLightManager->vecTheLights[lightIndex].position.x >> g_pTheLightManager->vecTheLights[lightIndex].position.y >> g_pTheLightManager->vecTheLights[lightIndex].position.z >> g_pTheLightManager->vecTheLights[lightIndex].position.w
						>> g_pTheLightManager->vecTheLights[lightIndex].diffuse.x >> g_pTheLightManager->vecTheLights[lightIndex].diffuse.y >> g_pTheLightManager->vecTheLights[lightIndex].diffuse.z >> g_pTheLightManager->vecTheLights[lightIndex].diffuse.w
						>> g_pTheLightManager->vecTheLights[lightIndex].specular.x >> g_pTheLightManager->vecTheLights[lightIndex].specular.y >> g_pTheLightManager->vecTheLights[lightIndex].specular.z >> g_pTheLightManager->vecTheLights[lightIndex].specular.w
						>> g_pTheLightManager->vecTheLights[lightIndex].atten.x >> g_pTheLightManager->vecTheLights[lightIndex].atten.y >> g_pTheLightManager->vecTheLights[lightIndex].atten.z >> g_pTheLightManager->vecTheLights[lightIndex].atten.w
						>> g_pTheLightManager->vecTheLights[lightIndex].direction.x >> g_pTheLightManager->vecTheLights[lightIndex].direction.y >> g_pTheLightManager->vecTheLights[lightIndex].direction.z >> g_pTheLightManager->vecTheLights[lightIndex].direction.w
						>> g_pTheLightManager->vecTheLights[lightIndex].param1.x >> g_pTheLightManager->vecTheLights[lightIndex].param1.y >> g_pTheLightManager->vecTheLights[lightIndex].param1.z >> g_pTheLightManager->vecTheLights[lightIndex].param1.w >> g_pTheLightManager->vecTheLights[lightIndex].param2.x;
					lightIndex++;
				}
			}
			else if (type == "o")
			{
				if (modelIndex < g_pMeshObjects.size())
				{
					std::getline(saveFile, line);
					std::istringstream in2(line);
					std::string meshname;
					in2 >> meshname;
					if (meshname == "Floor1" || meshname == "Wall1")
					{
						modelIndex++;
						continue;
					}
					g_pMeshObjects[modelIndex]->meshName = meshname;
					in2 >> g_pMeshObjects[modelIndex]->friendlyName >> g_pMeshObjects[modelIndex]->position.x >> g_pMeshObjects[modelIndex]->position.y >> g_pMeshObjects[modelIndex]->position.z
						>> g_pMeshObjects[modelIndex]->qRotation.x >> g_pMeshObjects[modelIndex]->qRotation.y >> g_pMeshObjects[modelIndex]->qRotation.z >> g_pMeshObjects[modelIndex]->qRotation.w >> g_pMeshObjects[modelIndex]->scaleXYZ.x >> g_pMeshObjects[modelIndex]->scaleXYZ.y >> g_pMeshObjects[modelIndex]->scaleXYZ.z >> g_pMeshObjects[modelIndex]->isWireframe
						>> g_pMeshObjects[modelIndex]->bUse_RGBA_colour >> g_pMeshObjects[modelIndex]->RGBA_colour.x >> g_pMeshObjects[modelIndex]->RGBA_colour.y >> g_pMeshObjects[modelIndex]->RGBA_colour.z >> g_pMeshObjects[modelIndex]->RGBA_colour.w
						>> g_pMeshObjects[modelIndex]->specular_colour_and_power.x >> g_pMeshObjects[modelIndex]->specular_colour_and_power.y >> g_pMeshObjects[modelIndex]->specular_colour_and_power.z >> g_pMeshObjects[modelIndex]->specular_colour_and_power.w
						>> g_pMeshObjects[modelIndex]->bDoNotLight >> g_pMeshObjects[modelIndex]->bIsVisible;
					modelIndex++;
				}
			}
		}
		std::cout << "Load successful!" << std::endl;
	}
}

void cVAOManager::AddObject(cMeshObject obj)
{
	std::ofstream saveFile;
	saveFile.open("assets/createObjects.txt", std::ios::app);
	saveFile << "basic " << obj.meshName << " " << obj.friendlyName << " 0 0 0 0 0 0 1\n";
	saveFile.close();
}
void cVAOManager::Save()
{
	std::ofstream saveFile;
	saveFile.open("saveData.txt");
	saveFile << "c" << std::endl << ::g_cameraEye.x << " " << ::g_cameraEye.y << " " << ::g_cameraEye.z << " " << ::g_cameraTarget.x << " " << ::g_cameraTarget.y << " " << ::g_cameraTarget.z << std::endl;
	for (cLight light : ::g_pTheLightManager->vecTheLights)
	{
		saveFile << "l" << std::endl << light.position.x << " " << light.position.y << " " << light.position.z << " " << light.position.w << " "
			<< light.diffuse.x << " " << light.diffuse.y << " " << light.diffuse.z << " " << light.diffuse.w << " "
			<< light.specular.x << " " << light.specular.y << " " << light.specular.z << " " << light.specular.w << " "
			<< light.atten.x << " " << light.atten.y << " " << light.atten.z << " " << light.atten.w << " "
			<< light.direction.x << " " << light.direction.y << " " << light.direction.z << " " << light.direction.w << " "
			<< light.param1.x << " " << light.param1.y << " " << light.param1.z << " " << light.param1.w << " "
			<< light.param2.x << " " << std::endl;
	}
	for (cMeshObject* object : g_pMeshObjects)
	{
		saveFile << "o" << std::endl << object->meshName << " " << object->friendlyName << " " << object->position.x << " " << object->position.y << " " << object->position.z << " "
			<< object->qRotation.x << " " << object->qRotation.y << " " << object->qRotation.z << " " << object->qRotation.w << " " << object->scaleXYZ.x << " " << object->scaleXYZ.y << " " << object->scaleXYZ.z << " " << object->isWireframe << " "
			<< object->bUse_RGBA_colour << " " << object->RGBA_colour.x << " " << object->RGBA_colour.y << " " << object->RGBA_colour.z << " " << object->RGBA_colour.w << " "
			<< object->specular_colour_and_power.x << " " << object->specular_colour_and_power.y << " " << object->specular_colour_and_power.z << " " << object->specular_colour_and_power.w << " "
			<< object->bDoNotLight << " " << object->bIsVisible << std::endl;
	}
	std::cout << "Save successful..." << std::endl;
	saveFile.close();
}