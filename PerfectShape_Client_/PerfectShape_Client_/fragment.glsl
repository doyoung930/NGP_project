#version 330 core
in vec3 FragPos;
in vec3 Normal; 
out vec4 FragColor;

uniform vec3 lightPos; 
uniform vec3 lightColor; 
uniform vec3 objectColor; 
uniform vec3 viewPos;
uniform vec3 ambientLightColor;

uniform float fragKind;

in vec2 TexCoord;
uniform sampler2D texture1; 

void main ()
{
	vec3 ambientLight = ambientLightColor;
	vec3 ambient = ambientLight;

	vec3 normalVector = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diffuseLight = max(dot(normalVector, lightDir), 0.0);
	vec3 diffuse = diffuseLight * lightColor;
	
	int shininess = 64; //--- 광택 계수
	vec3 viewDir = normalize(viewPos - FragPos); 
	vec3 reflectDir = reflect(-lightDir, normalVector);
	float specularLight = max(dot(viewDir, reflectDir), 0.0);
	specularLight = pow(specularLight, shininess); 
	vec3 specular = specularLight * lightColor; 

	vec3 result = (ambient + diffuse + specular) * objectColor * fragKind + objectColor * (1 - fragKind); 
	FragColor = vec4 (result, 1.0); 
	FragColor = texture(texture1, TexCoord) * FragColor;
}