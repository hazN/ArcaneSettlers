//Fragment shader
#version 420
in vec4 fColour;		
in vec4 fNormal;
in vec4 fVertWorldLocation;	
in vec4 fUVx2;
in vec4 fTangent;
in vec4 fBinormal;


// This replaces gl_FragColor
out vec4 pixelOutputColour;		
out vec4 FBO_vertexWorldPos;	


float rotSpeed = 0.05;

uniform vec4 RGBA_Colour;		// aka "diffuse" RGB + Alpha (w)
uniform bool bUseRGBA_Colour;

// This is for the flame alpha transparency object
uniform bool bIsFlameObject;
uniform float time;
uniform bool bUseDiscardTexture;

// If this is true, then we will use a different texture than the "regular" list of textures
uniform sampler2D samplerFBO_COLOR_TEXTURE_01;		// Color texture from the FBO
uniform sampler2D samplerFBO_VertexWorldPosition;		// Color texture from the FBO
uniform vec2 FBO_size_width_height;					// x = width, y = height
uniform vec2 screen_width_height;					// x = width, y = height

// This is for a basic depth of field 
// * x is the centre of the object that's 'in focus' by depth from the eye
// * y is how 'deep' that focus are is. 
uniform vec2 inFocusPlanes;
// This is a number between 0 and 1 (0 to 100 percent)
// 	0 meaning no blur, and 100 percent being 'the most' blur 
//	(depending on how strong our kernel is)
uniform float amountOfBlur;


//uniform vec4 diffuseColour;		// RGB + Alpha (w)
uniform vec4 specularColour;			// RGB object hightlight COLOUR
										// For most material, this is white (1,1,1)
										// For metals google "metal specular hightlight colour"
										// For plastic, it's the same colour as the diffuse
										// W value is the "specular power" or "Shininess" 
										// Starts at 1, and goes to 10,000s
										//	1 = not shiny 
										// 10 = "meh" shiny
										// 1,000 and up gets shinier
// Used to draw debug (or unlit) objects
uniform vec4 debugColour;
uniform bool bDoNotLight;		

uniform vec4 eyeLocation;

// Texture "samplers" 
uniform sampler2D texture0;		// "Brick texture"
uniform sampler2D texture1;		// "Lady Gaga"
uniform sampler2D texture2;		// "Lady Gaga"
uniform sampler2D texture3;		// "Lady Gaga"
uniform sampler2D texture4;		// "Brick texture"
uniform sampler2D texture5;		// "Lady Gaga"
uniform sampler2D texture6;		// "Lady Gaga"
uniform sampler2D texture7;		// "Lady Gaga"

uniform vec4 texRatio_0_3;		// x = texture0, y = texture1, etc. 0 to 1
uniform vec4 texRatio_4_7;		// 0 to 1

uniform samplerCube skyboxTexture;
// When true, applies the skybox texture
uniform bool bIsSkyboxObject;
uniform bool bIsSelected;


struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
};

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;

const int NUMBEROFLIGHTS = 10;
uniform sLight theLights[NUMBEROFLIGHTS];  	// 

vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular );
							

float screen_width = screen_width_height.x;
float screen_height = screen_width_height.y;
float d;
float lookup(vec2 p, float dx, float dy);
void main()
{

	if (bIsSkyboxObject)
	{
		vec3 cubeMapColour = texture( skyboxTexture, fNormal.xyz ).rgb;
		pixelOutputColour.rgb = cubeMapColour.rgb;
		pixelOutputColour.a = 1.0f;
		FBO_vertexWorldPos.xyzw = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	vec3 materialColour = fColour.rgb;
	//	finalColour.r = 1.0f;

	float alphaTransparency = RGBA_Colour.w;

	// For the exhaust of the drop ship
	if (bIsFlameObject)
	{
		// DON'T light. Apply the texture. Use colour as alpha
		vec3 flameColour = texture( texture0, fUVx2.st ).rgb;	
		
		pixelOutputColour.rgb = flameColour;
		
		// Set the alpha transparency based on the colour.
		float RGBcolourSum = pixelOutputColour.r + pixelOutputColour.g + pixelOutputColour.b;
		pixelOutputColour.a = max( ((RGBcolourSum - 0.1f) / 3.0f), 0.0f);
	
		// Output the vertex location, too
		FBO_vertexWorldPos.xyz = fVertWorldLocation.xyz;
	
		// Exit early so bypasses lighting
		return;
	}
	
	if ( bUseDiscardTexture )
	{	
		// Compare the colour in the texture07 black and white texture
		// If it's 'black enough' then don't draw the pixel
		// NOTE: I'm only sampling from the red 
		// (since it's black and white, all channels would be the same)
		float greyscalevalue = texture( texture7, fUVx2.st ).r;
		
		// Here, 0.5 is "black enough" 
		if ( greyscalevalue < 0.5f )
		{
			discard;
		}
	}
	
	if ( bUseRGBA_Colour )
	{
		materialColour = RGBA_Colour.rgb;
	}
	else
	{
		//gl_FragColor = vec4(finalColour, 1.0f);
	//	pixelOutputColour = vec4(finalColour, 1.0f);	
	//	pixelOutputColour = vec4(fNormal.xyz, 1.0f);

		// Sample from a texture 
		vec3 textColour0 = texture( texture0, fUVx2.st ).rgb;		
		vec3 textColour1 = texture( texture1, fUVx2.st ).rgb;	
		vec3 textColour2 = texture( texture2, fUVx2.st ).rgb;	
		vec3 textColour3 = texture( texture3, fUVx2.st ).rgb;	
		
		
		materialColour =   (textColour0.rgb * texRatio_0_3.x) 
						 + (textColour1.rgb * texRatio_0_3.y) 
						 + (textColour2.rgb * texRatio_0_3.z) 
						 + (textColour3.rgb * texRatio_0_3.w);
	}//if ( bUseRGBA_Colour )

	if ( bDoNotLight )
	{
		// Set the output colour and exit early
		// (Don't apply the lighting to this)
		pixelOutputColour = vec4(materialColour.rgb, alphaTransparency);
			
		// Output the vertex location, too
		FBO_vertexWorldPos.xyz = fVertWorldLocation.xyz;

		return;
	}

	vec4 outColour = calculateLightContrib( materialColour.rgb, fNormal.xyz, 
	                                        fVertWorldLocation.xyz, specularColour );
										
	// If my blend function is (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) 
	// then it's reading whatever the 4th value of the output is:
	pixelOutputColour = vec4(outColour.rgb, alphaTransparency);
	
	float amountOfAmbientLight = 0.15f;
	pixelOutputColour.rgb += (materialColour.rgb * amountOfAmbientLight);
	
	// Output the vertex location, too
	FBO_vertexWorldPos.xyz = fVertWorldLocation.xyz;
	FBO_vertexWorldPos *= 0.001f;
	FBO_vertexWorldPos.rgb = vec3(1.0f, 1.0f, 1.0f);
}


vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec3 norm = normalize(vertexNormal);
	
	vec4 finalObjectColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	
	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{	
		// ********************************************************
		// is light "on"
		if ( theLights[index].param2.x == 0.0f )
		{	// it's off
			continue;
		}
		
		// Cast to an int (note with c'tor)
		int intLightType = int(theLights[index].param1.x);
		
		// We will do the directional light here... 
		// (BEFORE the attenuation, since sunlight has no attenuation, really)
		if ( intLightType == DIRECTIONAL_LIGHT_TYPE )		// = 2
		{
			// This is supposed to simulate sunlight. 
			// SO: 
			// -- There's ONLY direction, no position
			// -- Almost always, there's only 1 of these in a scene
			// Cheapest light to calculate. 

			vec3 lightContrib = theLights[index].diffuse.rgb;
			
			// Get the dot product of the light and normalize
			float dotProduct = dot( -theLights[index].direction.xyz,  
									   normalize(norm.xyz) );	// -1 to 1

			dotProduct = max( 0.0f, dotProduct );		// 0 to 1
		
			lightContrib *= dotProduct;		
			
			finalObjectColour.rgb += (vertexMaterialColour.rgb * theLights[index].diffuse.rgb * lightContrib); 
									 //+ (materialSpecular.rgb * lightSpecularContrib.rgb);
			// NOTE: There isn't any attenuation, like with sunlight.
			// (This is part of the reason directional lights are fast to calculate)

			// return finalObjectColour;		
			
			// Go to next light (skip rest)
			continue;
		}
		
		// Assume it's a point light 
		// intLightType = 0
		
		// Contribution for this light
		vec3 vLightToVertex = theLights[index].position.xyz - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	
		vec3 lightVector = normalize(vLightToVertex);
		float dotProduct = dot(lightVector, vertexNormal.xyz);	 
		
		dotProduct = max( 0.0f, dotProduct );	
		
		vec3 lightDiffuseContrib = dotProduct * theLights[index].diffuse.rgb;
			

		// Specular 
		vec3 lightSpecularContrib = vec3(0.0f);
			
		vec3 reflectVector = reflect( -lightVector, normalize(norm.xyz) );

		// Get eye or view vector
		// The location of the vertex in the world to your eye
		vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);

		// To simplify, we are NOT using the light specular value, just the object’s.
		float objectSpecularPower = vertexSpecular.w;   
		
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * vertexSpecular.rgb;	//* theLights[lightIndex].Specular.rgb
// This only takes into account the colour of the light.
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * theLights[index].specular.rgb;
							   
// This one takes into account the object "colour" AND the light colour
		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
			                   * (vertexSpecular.rgb * theLights[index].specular.rgb);							   
		// Attenuation
		float attenuation = 1.0f / 
				( theLights[index].atten.x + 										
				  theLights[index].atten.y * distanceToLight +						
				  theLights[index].atten.z * distanceToLight*distanceToLight );  	
				  
		// total light contribution is Diffuse + Specular
		lightDiffuseContrib *= attenuation;
		lightSpecularContrib *= attenuation;
		
		
		// But is it a spot light
		if ( intLightType == SPOT_LIGHT_TYPE )		// = 1
		{	
		

			// Yes, it's a spotlight
			// Calcualate light vector (light to vertex, in world)
			vec3 vertexToLight = vertexWorldPos.xyz - theLights[index].position.xyz;

			vertexToLight = normalize(vertexToLight);

			float currentLightRayAngle
					= dot( vertexToLight.xyz, theLights[index].direction.xyz );
					
			currentLightRayAngle = max(0.0f, currentLightRayAngle);

			//vec4 param1;	
			// x = lightType, y = inner angle, z = outer angle, w = TBD

			// Is this inside the cone? 
			float outerConeAngleCos = cos(radians(theLights[index].param1.z));
			float innerConeAngleCos = cos(radians(theLights[index].param1.y));
							
			// Is it completely outside of the spot?
			if ( currentLightRayAngle < outerConeAngleCos )
			{
				// Nope. so it's in the dark
				lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
				lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )
			{
				// Angle is between the inner and outer cone
				// (this is called the penumbra of the spot light, by the way)
				// 
				// This blends the brightness from full brightness, near the inner cone
				//	to black, near the outter cone
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);
									  
				lightDiffuseContrib *= penumbraRatio;
				lightSpecularContrib *= penumbraRatio;
			}
						
		}// if ( intLightType == 1 )
		
		
					
		finalObjectColour.rgb += (vertexMaterialColour.rgb * lightDiffuseContrib.rgb)
								  + (vertexSpecular.rgb  * lightSpecularContrib.rgb );

	}//for(intindex=0...
	
	finalObjectColour.a = 1.0f;
	
	return finalObjectColour;
}
//https://www.shadertoy.com/view/Xscyzn
vec4 mainImage( vec4 fragColor, vec2 fragCoord )
{
    float effectRadius = .5;
    float effectAngle = 2. * 3.14;
    
    vec2 center = screen_width_height.xy / 2;
    center = center == vec2(0., 0.) ? vec2(.5, .5) : center;
    
    vec2 uv = fragCoord.xy / screen_width_height.xy - center;
    
    float len = length(uv * vec2(screen_width_height.x / screen_width_height.y, 1.));
    float angle = atan(uv.y, uv.x) + effectAngle * smoothstep(effectRadius, 0., len);
    float radius = length(uv);

    vec4 frag = texture(samplerFBO_COLOR_TEXTURE_01, vec2(radius * cos(angle), radius * sin(angle)) + center);
	return frag;
}


// Variable gaussian blur
// This will take a value from 0 to 20 
// * 0 = no blur
// * 21 = kernel with an equivalent of a 41 x 41 2D kernel (Yikes!)
vec3 calculateGaussianBlur( int kernelSize1D )
{
		// http://demofox.org/gauss.html
		// Note: We aren't using the full 2D kernel and it's symetrical so 
		//	we can use the row of numbers at the very bottom
		// 
		// Filter with Sigma = 3 and Support = 0.99
		// Gives a 21x21 Gaussian kernel:
		// 
		// 0	1	2	3	4	5	6	7	8	9	10
		// 0.1324,	0.1253,	0.1063,	0.0807,	0.0549,	0.0334,	0.0183,	0.0089,	0.0039,	0.0015,	0.0005
		
		const int MAX_KERNEL_1D_SIZE = 20;
		int numberOfGaussianElementsToUse = clamp( kernelSize1D, 0, MAX_KERNEL_1D_SIZE );
		
		float blurWeightArray[] = { 
			0.1324f, 		// 0
			0.1253f, 		// 1
			0.1063f, 		// 2
			0.0807f, 		// 3
			0.0549f,		// 4
			0.0334f, 		// 5
			0.0183f, 		// 6
			0.0089f, 		// 7
			0.0039f, 		// 8
			0.0015f,		// 9
			0.0005f };		// 10
			
		// How this works:
		// 1. We load the sampled values into an array of pixels
		// 2. Depending on the number of kernels, we apply the weights (in the switch-case)
		
		vec3 pixelSampleValues[MAX_KERNEL_1D_SIZE];

		// Special case: no blur, so just sample the pixel and return
		if ( numberOfGaussianElementsToUse == 0 )
		{
			// Don't blur
			vec3 pixelColour = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( gl_FragCoord.x / screen_width, 			
											  gl_FragCoord.y / screen_height ) ).rgb;

			return pixelColour;
		}

		// Apply some blur...
		
		// Sample the actual pixel
		pixelSampleValues[0] = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( gl_FragCoord.x / screen_width, 			
											  gl_FragCoord.y / screen_height ) ).rgb;
			
		// Now load the pixel array depending on how large the kernel is
		// Note we are starting at 1, not zero, because zero is the pixel location we are at
		for ( int index = 1; index < numberOfGaussianElementsToUse; index++ )
		{
			// Right 
			vec3 pixelColourRight = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( (gl_FragCoord.x + index) / screen_width, 			
											  gl_FragCoord.y         / screen_height ) ).rgb;
			
			// Left 
			vec3 pixelColourLeft = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( (gl_FragCoord.x - index) / screen_width, 			
											  gl_FragCoord.y         / screen_height ) ).rgb;
		
			// Up 
			vec3 pixelColourUp = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( gl_FragCoord.x         / screen_width, 			
											  (gl_FragCoord.y + index) / screen_height ) ).rgb;
		
			// Down 
			vec3 pixelColourDown = texture( samplerFBO_COLOR_TEXTURE_01, 
										vec2( gl_FragCoord.x         / screen_width, 			
											  (gl_FragCoord.y - index) / screen_height ) ).rgb;
							
			// Store the sum of these values in the array
			pixelSampleValues[index] = pixelColourRight + pixelColourLeft + pixelColourUp + pixelColourDown;
															  
		}
}
float lookup(vec2 p, float dx, float dy)
{
    vec2 uv = (p.xy + vec2(dx * d, dy * d)) / screen_width_height.xy;
    vec4 c = texture(samplerFBO_COLOR_TEXTURE_01, uv.xy);
	
	// return as luma
    return 0.2126*c.r + 0.7152*c.g + 0.0722*c.b;
}