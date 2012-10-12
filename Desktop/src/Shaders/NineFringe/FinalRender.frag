#version 330

uniform sampler2D phaseMap;
uniform sampler2D normals;

uniform mat4 normalMatrix;

uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;

in vec3 lightDirection;
in vec2 fragTexCoord;

out vec4 fragColor;

void main()
{
	float phase = texture2D(phaseMap, fragTexCoord).r;
	if(phase == 0)	// If the phase is 0 we should just get rid of this fragment
	{
		discard;
	}

	vec3 normal = normalize(normalMatrix * texture2D(normals, fragTexCoord)).xyz;
	vec3 L = normalize(lightDirection);		// Light direction
	vec3 R = normalize(-reflect(L, normal));	// Reflection direction

	//	Ambient
	vec4 Iamb = ambientColor;

	//	Diffuse
	vec4 Idiff = diffuseColor * max(dot(normal,L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);

	//	Specular
	vec3 reflection = normalize(-reflect(normalize(lightDirection), normal));
	float Ispec = pow(max(dot(normal, R), 0.0), 128.0);

	fragColor = Iamb + Idiff + Ispec;
}