#version 150

in vec4 vPosition;
in vec3 vNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

uniform vec3 uLightPosition;
uniform vec3 uLightColor;
uniform float uLightSpotLightFactor;

uniform mat3 uAmbientMaterial;
uniform mat3 uDiffuseMaterial;
uniform mat3 uSpecularMaterial;
uniform float uSpecularityExponent;

out vec4 oFragColor; //write the final color into this variable

void main() {
    vec3 p;
    if(vPosition.w != 0.0)
        p = vec3(vPosition.x/vPosition.w, vPosition.y/vPosition.w, vPosition.z/vPosition.w);
    else
        return;

    vec3 n = normalize(vNormal);

    // Find ambient component
    vec3 ambient = uLightColor*uAmbientMaterial;

    // Find diffuse component
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    float cosPhi = dot(n, uLightPosition-p)/length(uLightPosition-p);
    if(cosPhi > 0.0)
        diffuse = uLightColor*uDiffuseMaterial*cosPhi;

    // Find specular component with Phong-Blinn
    vec3 specular = vec3(0.0, 0.0, 0.0);
    vec3 v = vec3(0.0, 0.0, 0.0);
    vec3 w_l = normalize(uLightPosition-p);
    vec3 w_v = normalize(v-p);
    vec3 h = normalize(w_l+w_v);
    cosPhi = dot(h, n);
    if(cosPhi > 0.0)
        specular = uLightColor*uSpecularMaterial*pow(cosPhi, uSpecularityExponent);

    // Find spotlight factor
    vec3 orig = vec3(uModelViewMatrix*vec4(0.0, 0.0, 0.0, 1.0));
    vec3 d = normalize(orig-uLightPosition);
    cosPhi = dot(d, p-uLightPosition)/length(p-uLightPosition);
    float spot = 0.0;
    if(cosPhi > 0.0)
        spot = pow(cosPhi, uLightSpotLightFactor);

    vec3 vColor = ambient + spot*(diffuse + specular);
    oFragColor = vec4(vColor, 1.0);
}
