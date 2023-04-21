#include "cMeshObject.h"

int cMeshObject::tid = 0;

cMeshObject::cMeshObject()
{
	this->position = glm::vec3(0.0f);
	this->qRotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
	this->SetUniformScale(1.0f);
	this->isWireframe = false;
	this->RGBA_colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	this->bUse_RGBA_colour = false;
	this->specular_colour_and_power = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	this->bDoNotLight = false;
	this->bIsVisible = true;
	this->textureRatios[0] = 0.0f;
	this->textureRatios[1] = 0.0f;
	this->textureRatios[2] = 0.0f;
	this->textureRatios[3] = 0.0f;
	this->textureRatios[4] = 0.0f;
	this->textureRatios[5] = 0.0f;
	this->textureRatios[6] = 0.0f;
	this->textureRatios[7] = 0.0f;
	id = tid++;
}

void cMeshObject::setRotationFromEuler(glm::vec3 newEulerAngleXYZ)
{
	this->qRotation = glm::quat(newEulerAngleXYZ);
}

void cMeshObject::adjustRoationAngleFromEuler(glm::vec3 EulerAngleXYZ_Adjust)
{
	this->qRotation *= glm::quat(EulerAngleXYZ_Adjust);
}

void cMeshObject::SetUniformScale(float newScale)
{
	this->scaleXYZ = glm::vec3(newScale);
}

cMeshObject* cMeshObject::findObjectByFriendlyName(std::string nameToFind, bool bSearchChildren /*=true*/)
{
	for (std::vector< cMeshObject* >::iterator itCurrentMesh = this->vecChildMeshes.begin();
		itCurrentMesh != this->vecChildMeshes.end();
		itCurrentMesh++)
	{
		cMeshObject* pCurrentMesh = *itCurrentMesh;

		// Check if its the mesh, if so return it
		if (pCurrentMesh->friendlyName == nameToFind)
			return pCurrentMesh;

		// Otherwise search its children as well
		cMeshObject* pChildMesh = pCurrentMesh->findObjectByFriendlyName(nameToFind, bSearchChildren);

		// Check if child is the mesh, if so return it
		if (pChildMesh)
			return pChildMesh;
	}
	// Failed to find the mesh
	return NULL;
}
