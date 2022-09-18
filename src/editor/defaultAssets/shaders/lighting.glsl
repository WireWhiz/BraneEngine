#ifndef LIGHTING_FRAG
#define LIGHTING_FRAG

struct PointLight{
    vec3 position;
    vec4 color;
};

layout(binding = 2) buffer PointLightBuffer{
    uint numLights;
    PointLight[] lights;
} plb;

vec4 shadeDiffuse(vec4 color, vec3 position, vec3 normal)
{
    vec3 lightHue = vec3(0,0,0);
    for(uint i = 0; i < plb.numLights; i++)
    {
        float sqrdist = distance(plb.lights[i].position, position);
        sqrdist *= sqrdist;
        float b = max(0, dot(normalize(plb.lights[i].position - position), normal) * max(0, plb.lights[i].color.a * (1 / sqrdist)));
        lightHue += plb.lights[i].color.xyz * b;
    }
    color *= vec4(lightHue, 1);

    vec3 cc = color.xyz / (color.xyz + vec3(1.0));
    color = vec4(pow(cc, vec3(1.0/2.2)), color.a);
    return color;
}

#endif