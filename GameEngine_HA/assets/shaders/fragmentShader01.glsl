//Fragment shader
#version 420

in vec4 fColour;		
in vec4 fNormal;
in vec4 fVertWorldLocation;	
in vec4 fUVx2;
in vec4 fTangent;
in vec4 fBinormal;

// This replaces gl_Fragcolour
out vec4 pixelOutputColour;		
out vec4 FBO_vertexWorldPos;	

float rotSpeed = 0.05;

uniform vec4 RGBA_Colour;		// "Diffuse" colour
uniform bool bUseRGBA_Colour;	

uniform sampler2D texture0;		
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;
uniform sampler2D texture7;

uniform vec4 texRatio_0_3;		// Ratios of textures to blend together (0 to 1)
uniform vec4 texRatio_4_7;

uniform vec4 specularColour;	

uniform vec4 debugColour;		
uniform bool bDoNotLight;		

uniform vec4 eyeLocation;		// Position of the camera

uniform vec2 inFocusPlanes;		// Depth of field 

uniform vec2 FBO_size_width_height;			// FBO texture size
uniform vec2 screen_width_height;			// Screen size

uniform sampler2D samplerFBO_colour_TEXTURE_01;			
uniform sampler2D samplerFBO_VertexWorldPosition;		

uniform samplerCube skyboxTexture;
uniform bool bIsSkyboxObject;		
uniform bool bIsSelected;			

struct sLight
{
	vec4 position;		
	vec4 diffuse;	
	vec4 specular;	
	vec4 atten;		
	vec4 direction;	
	vec4 param1;	
	vec4 param2;	
	// param1: x = lightType(0 point, 1 spot, 2 directional), y = inner angle, z = outer angle, w = unused 
	// param2: x = on(0)/off(off), yzw = unused
};

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;

const int NUMBEROFLIGHTS = 10;
uniform sLight theLights[NUMBEROFLIGHTS];  	

vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, vec3 vertexWorldPos, vec4 vertexSpecular );
							
float screen_width = screen_width_height.x;
float screen_height = screen_width_height.y;

void main()
{
	// Check if its a skybox
	if (bIsSkyboxObject)
	{
		vec3 cubeMapColour = texture(skyboxTexture, fNormal.xyz).rgb;
		pixelOutputColour.rgb = cubeMapColour.rgb;
		pixelOutputColour.a = 1.0f;
		FBO_vertexWorldPos.xyzw = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	
	vec3 materialColour = fColour.rgb;
	float alphaTransparency = RGBA_Colour.w;
	// If bUseRGBA is true then use a solid colour
	if (bUseRGBA_Colour)
	{
		materialColour = RGBA_Colour.rgb;
	}
	// Otherwise sample the colour from texture
	else
	{
		vec3 textColour0 = texture(texture0, fUVx2.st).rgb;
		vec3 textColour1 = texture(texture1, fUVx2.st).rgb;
		vec3 textColour2 = texture(texture2, fUVx2.st).rgb;
		vec3 textColour3 = texture(texture3, fUVx2.st).rgb;
		materialColour = (textColour0.rgb * texRatio_0_3.x) 
						+ (textColour1.rgb * texRatio_0_3.y) 
						+ (textColour2.rgb * texRatio_0_3.z) 
						+ (textColour3.rgb * texRatio_0_3.w);
	}

	// Check if its not supposed to be lit, and exit early if so
	if (bDoNotLight)
	{
		pixelOutputColour = vec4(materialColour.rgb, alphaTransparency);
		FBO_vertexWorldPos.xyz = fVertWorldLocation.xyz;
		return;
	}

	// Get the light contribution
	vec4 outColour = calculateLightContrib(materialColour.rgb, fNormal.xyz, fVertWorldLocation.xyz, specularColour);

	// Add ambient light to the colour
	float amountOfAmbientLight = 0.15f;
	pixelOutputColour.rgb = outColour.rgb + (materialColour.rgb * amountOfAmbientLight);
	pixelOutputColour.a = alphaTransparency;

	// Output the vertex location
	FBO_vertexWorldPos.xyz = fVertWorldLocation.xyz * 0.001f;
	FBO_vertexWorldPos.rgb = vec3(1.0f, 1.0f, 1.0f);
}

vec4 calculateLightContrib(vec3 vertexMaterialColour, vec3 vertexNormal, vec3 vertexWorldPos, vec4 vertexSpecular)
{
    vec3 norm = normalize(vertexNormal);
    vec4 finalObjectColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Loop through all the lights
    for (int index = 0; index < NUMBEROFLIGHTS; index++)
    {
        // Check if the light is off
        if (theLights[index].param2.x == 0.0f)
            continue;
        
        // Get the light type as an int
        int intLightType = int(theLights[index].param1.x);
        
        // Handle directional lights 
        if (intLightType == DIRECTIONAL_LIGHT_TYPE)
        {
            // Calculate the light contribution
            vec3 lightContrib = theLights[index].diffuse.rgb;
            float dotProduct = dot(-theLights[index].direction.xyz, normalize(norm.xyz));
            dotProduct = max(0.0f, dotProduct);
            lightContrib *= dotProduct;
            
            // Add the light contribution to the final object colour
            finalObjectColour.rgb += vertexMaterialColour.rgb * theLights[index].diffuse.rgb * lightContrib;
            
            continue; 
        }
        
        // Handle point lights
        vec3 vLightToVertex = theLights[index].position.xyz - vertexWorldPos.xyz;
        float distanceToLight = length(vLightToVertex); 
        vec3 lightVector = normalize(vLightToVertex);
        float dotProduct = dot(lightVector, vertexNormal.xyz);    
        dotProduct = max(0.0f, dotProduct); 
        
        // Calculate the diffuse of the light
        vec3 lightDiffuseContrib = dotProduct * theLights[index].diffuse.rgb;
        
        // Calculate the specular of the light
        vec3 lightSpecularContrib = vec3(0.0f);
        vec3 reflectVector = reflect(-lightVector, normalize(norm.xyz));
        vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);
        float objectSpecularPower = vertexSpecular.w;
        lightSpecularContrib = pow(max(0.0f, dot(eyeVector, reflectVector)), objectSpecularPower) * (vertexSpecular.rgb * theLights[index].specular.rgb);
        
        // Apply attenuation to both the diffuse and specular 
        float attenuation = 1.0f / (theLights[index].atten.x +
            theLights[index].atten.y * distanceToLight +
            theLights[index].atten.z * distanceToLight * distanceToLight);
        lightDiffuseContrib *= attenuation;
        lightSpecularContrib *= attenuation;
        
		// Handle spot lights
		if (intLightType == SPOT_LIGHT_TYPE)
		{
		    // Calculate the light vector for the vertex in world space
		    vec3 vertexToLight = vertexWorldPos.xyz - theLights[index].position.xyz;
		    vertexToLight = normalize(vertexToLight);
		
		    // Calculate the angle between the light direction and the vector from the light to the vertex
		    float currentLightRayAngle = dot(vertexToLight.xyz, theLights[index].direction.xyz);
		    currentLightRayAngle = max(0.0f, currentLightRayAngle);
		
		    // Determine the outer and inner cone angles of the spotlight
		    float outerConeAngleCos = cos(radians(theLights[index].param1.z));
		    float innerConeAngleCos = cos(radians(theLights[index].param1.y));
		
		    // Check if the vertex is outside the spotlight cone
		    if (currentLightRayAngle < outerConeAngleCos)
		    {
		        // The vertex is outside the cone, so it receives no light
		        lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
		        lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
		    }
		    else if (currentLightRayAngle < innerConeAngleCos)
		    {
		        // The vertex is inside the spotlight cone, so it receives partial light based on the penumbra ratio
		        float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / (innerConeAngleCos - outerConeAngleCos);
		        lightDiffuseContrib *= penumbraRatio;
		        lightSpecularContrib *= penumbraRatio;
		    }
		}
		
		// Add the lights contribution to the final object colour
		finalObjectColour.rgb += (vertexMaterialColour.rgb * lightDiffuseContrib.rgb) + (vertexSpecular.rgb * lightSpecularContrib.rgb);
	}
	finalObjectColour.a = 1.0f;
	return finalObjectColour;
}