#version 410

uniform mat4 matrix;
uniform mat4 perspective;
uniform mat3 normalMatrix;
uniform bool noColor;
uniform vec3 lightPosition;
uniform mat4 worldToLightspace;

in vec4 vertex;
in vec4 normal;
in vec4 color;

out vec4 eyeVector;
out vec4 lightVector;
out vec4 lightSpace;
out vec4 vertColor;
out vec4 vertNormal;

void main( void )
{
    if (noColor) vertColor = vec4(0.2, 0.6, 0.7, 1.0 );
    else vertColor = color;
    vertNormal.xyz = normalize(normalMatrix * normal.xyz);
    vertNormal.w = 0.0;

    gl_Position = perspective * matrix * vertex;
	//TODO : compute L & V
    // From point to light
	lightVector = -(matrix*(vertex - vec4(lightPosition, 1.0f)));
	vec4 eyePosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	eyeVector = normalize(eyePosition - matrix*vertex);// in camera coordinate

    lightSpace = normalize(worldToLightspace * vertex);
}
