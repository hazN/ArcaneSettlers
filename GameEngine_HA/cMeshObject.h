#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

class cMeshObject
{
public:
	cMeshObject();
	int id;
	std::string meshName; // Mesh name, usually shared name of the model
	std::string friendlyName; // Name for this specific object
	glm::vec3 position;    
	glm::quat qRotation;
	void setRotationFromEuler(glm::vec3 newEulerAngleXYZ);
	void adjustRoationAngleFromEuler(glm::vec3 EulerAngleXYZ_Adjust);
	glm::vec3 scaleXYZ;
	void SetUniformScale(float newScale);
	bool isWireframe;	
	glm::vec4 RGBA_colour; // Diffuse Colour, W = Alpha
	bool bUse_RGBA_colour; // Use RGBA_colour over the texture
	glm::vec4 specular_colour_and_power; // Specular Colour, W = Power
	bool bDoNotLight;
	bool bIsVisible;
	std::string textures[8]; // Name of each texture, "TextureName.bmp"
	float textureRatios[8]; // Weight of each texture, 0.f to 1.f
	std::vector< cMeshObject* > vecChildMeshes;
	cMeshObject* findObjectByFriendlyName(std::string name, bool bSearchChildren = true);
private:
	static int tid;
};