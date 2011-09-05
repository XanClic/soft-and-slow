#version 110

// Eeyup, this is no standard shader.
typedef struct sas_light sas_light_t;

struct sas_light
{
    bool enabled;

    vec4 position, ambient, diffuse, specular;
};

typedef struct sas_material sas_material_t;

struct sas_material
{
    vec4 ambient, diffuse, specular, emission;
    float shininess;
};


extern "C" sas_light_t sas_lights[8];
extern "C" sas_material_t sas_current_material;


void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;

    vec4 col = vec4(0.f, 0.f, 0.f, 0.f);

    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);

    for (int i = 0; i < 8; i++)
    {
        if (!sas_lights[i].enabled)
            continue;

        vec3 pos = normalize(sas_lights[i].position);

        col += sas_lights[i].ambient * sas_current_material.ambient;
        // TODO: Point lights
        if (!sas_lights[i].position[3])
        {
            float dotp = dot(normal, pos);
            if (dotp > 0.f)
                col += dotp * sas_lights[i].diffuse * sas_current_material.diffuse;
        }
        // TODO: Specular
    }

    col += sas_current_material.emission;

    gl_FrontColor = col;
}
