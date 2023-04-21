// Vertex shader
#version 420

in vec4 vColour;			// Vertex Colour
in vec4 vPosition;			// Vertex Position
in vec4 vNormal;			// Vertex Normal
in vec4 vUVx2;				// 2 x Texture coords
in vec4 vTangent;			// For bump mapping
in vec4 vBiNormal;			// For bump mapping
in vec4 vBoneID;			// For skinned mesh (FBX)
in vec4 vBoneWeight;		// For skinned mesh (FBX)

// Output to the fragment shader
out vec4 fColour;			
out vec4 fNormal;
out vec4 fVertWorldLocation;	
out vec4 fUVx2;
out vec4 fTangent;
out vec4 fBinormal;

uniform mat4 mModel;
uniform mat4 mModelInverseTranspose;		// mModel with Only Rotation
uniform mat4 mView;
uniform mat4 mProjection;
uniform bool bHasBones;
uniform mat4 BoneMatrices[66];
void main()
{
	vec3 vertPosition = vPosition.xyz;
	
	// Calculate the MVP matrix (X/Y are in normalized screen space, Z is the depth)
	mat4 mMVP = mProjection * mView * mModel;
	
	// Apply bone transformations if the model has bones
	if (bHasBones)
	{
		mat4 boneTransform = BoneMatrices[int(vBoneID[0])] * vBoneWeight.x;
		boneTransform += BoneMatrices[int(vBoneID[1])] * vBoneWeight.y;
		boneTransform += BoneMatrices[int(vBoneID[2])] * vBoneWeight.z;
		boneTransform += BoneMatrices[int(vBoneID[3])] * vBoneWeight.w;
		vec4 position = boneTransform * vec4(vPosition.xyz, 1.0);
		gl_Position = mMVP * position;
	} 
	// Otherwise transform the model using the MVP
	else 
	{
		gl_Position = mMVP * vec4(vertPosition, 1.0f);
	}
	
	// Calculate the world space location of the vertex for lighting
	fVertWorldLocation.xyz = (mModel * vec4(vertPosition, 1.0f)).xyz;
	fVertWorldLocation.w = 1.0f;
	
	// Send the normals to the fragment shader and rotate them if necessary
	fNormal.xyz = normalize(mModelInverseTranspose * vec4(vNormal.xyz, 1.0f)).xyz;
	fNormal.w = 1.0f;
	
	// Copy the rest of the vertex values to the fragment shader
	fColour = vColour;
	fUVx2 = vUVx2;
	fTangent = vTangent;
	fBinormal = vBiNormal;
}
