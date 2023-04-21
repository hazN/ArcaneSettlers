#include "globalOpenGL.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cMeshObject.h"
#include "cBasicTextureManager.h"
#include "cVAOManager/cVAOManager.h"
#include "cVAOManager/sModelDrawInfo.h"
#include <iostream>

void DrawObject(cMeshObject* pCurrentMeshObject,
	glm::mat4x4 mat_PARENT_Model,               // The "parent's" model matrix
	GLuint shaderID,                            // ID for the current shader
	cBasicTextureManager* pTextureManager,
	cVAOManager* pVAOManager,
	GLint mModel_location,                      // Uniform location of mModel matrix
	GLint mModelInverseTransform_location,      // Uniform location of mView location
	const std::vector<glm::mat4>& instanceModelMatrices = std::vector<glm::mat4>()) // Instancing matrices, if its empty then dont use instancing
{
	// Don't draw any "back facing" triangles
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	// Turn on depth buffer test at draw time
	glEnable(GL_DEPTH_TEST);

	// This texture assignment is the same steps as with any other texture
	std::string texture7Name = pCurrentMeshObject->textures[7];
	GLuint texture07Number = pTextureManager->getTextureIDFromName(texture7Name);
	GLuint texture07Unit = 7;			// Texture unit go from 0 to 79
	glActiveTexture(texture07Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture07Number);
	GLint texture7_UL = glGetUniformLocation(shaderID, "texture7");
	glUniform1i(texture7_UL, texture07Unit);

	glm::mat4x4 matModel = mat_PARENT_Model;
	// Move the object
	glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f),
		pCurrentMeshObject->position);
	// Rotate the object
	glm::mat4 matQRotation = glm::mat4(pCurrentMeshObject->qRotation);

	// Scale the object
	glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentMeshObject->scaleXYZ.x,
			pCurrentMeshObject->scaleXYZ.y,
			pCurrentMeshObject->scaleXYZ.z));
	// Applying all these transformations to the model matrix
	matModel = matModel * matTranslation;
	matModel = matModel * matQRotation;
	matModel = matModel * matScale;

	glUniformMatrix4fv(mModel_location, 1, GL_FALSE, glm::value_ptr(matModel));
	// Inverse transpose of a 4x4 matrix removes the right column and lower row
	// Leaving only the rotation (the upper left 3x3 matrix values)
	glm::mat4 mModelInverseTransform = glm::inverse(glm::transpose(matModel));
	glUniformMatrix4fv(mModelInverseTransform_location, 1, GL_FALSE, glm::value_ptr(mModelInverseTransform));

	// Wireframe
	if (pCurrentMeshObject->isWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);      // GL_POINT, GL_LINE, GL_FILL
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Setting the colour in the shader
	GLint RGBA_Colour_ULocID = glGetUniformLocation(shaderID, "RGBA_Colour");

	// Turn on alpha transparency for everything
	glEnable(GL_BLEND);
	// Basic alpha transparency: 0.5 --> 0.5 what was already on the colour buffer + 0.5 of this object being drawn
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform4f(RGBA_Colour_ULocID,
		pCurrentMeshObject->RGBA_colour.r,
		pCurrentMeshObject->RGBA_colour.g,
		pCurrentMeshObject->RGBA_colour.b,
		pCurrentMeshObject->RGBA_colour.w);

	GLint bUseRGBA_Colour_ULocID = glGetUniformLocation(shaderID, "bUseRGBA_Colour");

	if (pCurrentMeshObject->bUse_RGBA_colour)
	{
		glUniform1f(bUseRGBA_Colour_ULocID, (GLfloat)GL_TRUE);
	}
	else
	{
		glUniform1f(bUseRGBA_Colour_ULocID, (GLfloat)GL_FALSE);
	}

	// Copy specular object colour and power.
	GLint specularColour_ULocID = glGetUniformLocation(shaderID, "specularColour");

	glUniform4f(specularColour_ULocID,
		pCurrentMeshObject->specular_colour_and_power.r,
		pCurrentMeshObject->specular_colour_and_power.g,
		pCurrentMeshObject->specular_colour_and_power.b,
		pCurrentMeshObject->specular_colour_and_power.w);

	GLint bDoNotLight_Colour_ULocID = glGetUniformLocation(shaderID, "bDoNotLight");

	if (pCurrentMeshObject->bDoNotLight)
	{
		glUniform1f(bDoNotLight_Colour_ULocID, (GLfloat)GL_TRUE);
	}
	else
	{
		glUniform1f(bDoNotLight_Colour_ULocID, (GLfloat)GL_FALSE);
	}

	// Set up the textures on this model
	std::string texture0Name = pCurrentMeshObject->textures[0];
	std::string texture1Name = pCurrentMeshObject->textures[1];

	GLuint texture00Number = pTextureManager->getTextureIDFromName(texture0Name);

	// Choose the texture Unit I want
	GLuint texture00Unit = 0;			// Texture unit go from 0 to 79
	glActiveTexture(texture00Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984

	// Bind the texture to the active texture unit
	glBindTexture(GL_TEXTURE_2D, texture00Number);

	GLint texture0_UL = glGetUniformLocation(shaderID, "texture0");
	glUniform1i(texture0_UL, texture00Unit);

	GLuint texture01Number = pTextureManager->getTextureIDFromName(texture1Name);
	GLuint texture01Unit = 1;			// Texture unit go from 0 to 79
	glActiveTexture(texture01Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_2D, texture01Number);
	GLint texture1_UL = glGetUniformLocation(shaderID, "texture1");
	glUniform1i(texture1_UL, texture01Unit);

	GLint texRatio_0_3 = glGetUniformLocation(shaderID, "texRatio_0_3");
	glUniform4f(texRatio_0_3,
		pCurrentMeshObject->textureRatios[0],
		pCurrentMeshObject->textureRatios[1],
		pCurrentMeshObject->textureRatios[2],
		pCurrentMeshObject->textureRatios[3]);

	GLuint cubeMapTextureNumber = pTextureManager->getTextureIDFromName("TropicalSunnyDay");
	GLuint texture30Unit = 30;			// Texture unit go from 0 to 79
	glActiveTexture(texture30Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureNumber);
	GLint skyboxTexture_UL = glGetUniformLocation(shaderID, "skyboxTexture");
	glUniform1i(skyboxTexture_UL, texture30Unit);
	// Instancing uniforms
	GLint bIsInstanced_location = glGetUniformLocation(shaderID, "bIsInstanced");
	GLint instanceModelMatrices_location = glGetUniformLocation(shaderID, "instanceModelMatrices");
	if (!instanceModelMatrices.empty())
	{
		glUniform1i(bIsInstanced_location, GL_TRUE);
		glUniformMatrix4fv(instanceModelMatrices_location, instanceModelMatrices.size(), GL_FALSE, glm::value_ptr(instanceModelMatrices[0]));
	}
	else
	{
		glUniform1i(bIsInstanced_location, GL_FALSE);
	}
	// Check if instanceModelMatrices isn't empty then use instancing if its not
	GLuint instanceVBO = 0;
	if (!instanceModelMatrices.empty()) {
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);

		GLsizei vec4Size = sizeof(glm::vec4);
		for (size_t i = 0; i < 4; ++i) {
			glEnableVertexAttribArray(3 + i);
			glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(i * vec4Size));
			glVertexAttribDivisor(3 + i, 1);
		}
	}

	// Find the model to draw
	sModelDrawInfo drawingInformation;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentMeshObject->meshName, drawingInformation))
	{
		glBindVertexArray(drawingInformation.VAO_ID);

		// glDrawElements for single objects, glDrawElementsInstanced for instancing
		if (instanceModelMatrices.empty()) {
			glDrawElements(GL_TRIANGLES, drawingInformation.numberOfIndices, GL_UNSIGNED_INT, (void*)0);
		}
		else {
			glDrawElementsInstanced(GL_TRIANGLES, drawingInformation.numberOfIndices, GL_UNSIGNED_INT, (void*)0, instanceModelMatrices.size());
		}

		glBindVertexArray(0);

		if (instanceVBO != 0) {
			glDeleteBuffers(1, &instanceVBO);
		}
	}
	else
	{
		std::cout << "Error: didn't find model to draw." << std::endl;
	}

	// Draw all the children
	for (std::vector< cMeshObject* >::iterator itCurrentMesh = pCurrentMeshObject->vecChildMeshes.begin();
		itCurrentMesh != pCurrentMeshObject->vecChildMeshes.end();
		itCurrentMesh++)
	{
		cMeshObject* pCurrentCHILDMeshObject = *itCurrentMesh;
		// All the drawing code has been moved to the DrawObject function
		DrawObject(pCurrentCHILDMeshObject,
			matModel,
			shaderID, pTextureManager,
			pVAOManager, mModel_location, mModelInverseTransform_location);
	}

	return;
}