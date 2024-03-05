#version 330 core

// input data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord; 
layout(location = 3) in vec3 aTangent; 
layout(location = 4) in vec3 aColor;

// uniform input data
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat3 uNormalMatrix;
uniform int uColorSet;

// output data
out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vColor;
out vec3 vTangent;

void main()
{
	// set vertex position  
	gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0f); 

	// set vertex shader output
	// will be interpolated for each fragment 
	vPosition = (uModelMatrix * vec4(aPosition, 1.0f)).xyz; 
	vNormal = uNormalMatrix * aNormal;
	vTangent = uNormalMatrix * aTangent;
	vTexCoord = aTexCoord;
	vColor = aColor;
}
