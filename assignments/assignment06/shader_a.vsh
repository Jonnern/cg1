#version 150

in vec4 aPosition;
in vec3 aNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

uniform vec3 uLightPosition;
uniform vec3 uLightColor;

uniform mat3 uAmbientMaterial;
uniform mat3 uDiffuseMaterial;
uniform mat3 uSpecularMaterial;
uniform float uSpecularityExponent;

out vec3 vColor;

void main() {
	// Transform to eye-space
    vec4 newPosition = uModelViewMatrix*aPosition;
    vec3 p;
    if(newPosition.w != 0.0)
        p = vec3(newPosition.x/newPosition.w, newPosition.y/newPosition.w, newPosition.z/newPosition.w);
    else
        return;

    // Transform normals using the correct matrix.
    // It's important to normalize in the end
    vec3 newNormal = normalize(vec3(inverse(transpose(uModelViewMatrix))*vec4(aNormal, 0.0f)));

    // Find ambient component
    vec3 ambient = uLightColor*uAmbientMaterial;

    // Find diffuse component
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    float cosPhi = dot(newNormal, uLightPosition-p)/length(uLightPosition-p);
    if(cosPhi > 0.0)
        diffuse = uLightColor*uDiffuseMaterial*cosPhi;

    // Find specular component with Phong-Blinn
    vec3 specular = vec3(0.0, 0.0, 0.0);
    vec3 v = vec3(0.0, 0.0, 0.0);
    vec3 w_l = normalize(uLightPosition-p);
    vec3 w_v = normalize(v-p);
    vec3 h = normalize(w_l+w_v);
    cosPhi = dot(h, newNormal);
    if(cosPhi > 0.0)
        specular = uLightColor*uSpecularMaterial*pow(cosPhi, uSpecularityExponent);

    vColor = ambient + diffuse + specular;
    gl_Position = uProjectionMatrix*newPosition;
}

