#define MaxLights 16

// Light source structure
struct Light
{
	// Strength
	float3 Strength;

	// Attenuation start (Point/Spot only)
	float FalloffStart;
	
	// Direction (Directional/Spot only)
	float3 Direction;

	// Attenuation end (Point/Spot only)
	float FalloffEnd;

	// World position (Point/Spot only)
	float3 Position;

	// Spot only (indirectionaly denotes angle of spot)
	float SpotPower;
};

// Object's material
struct Material
{
	// Diffuse albedo (factor for reflection)
	float4 DiffuseAlbedo;

	// Fresnel base
	float3 FresnelR0;
	
	// Shininess = 1 - Roughness
	float Shininess;
};

// Computes attenuation for point/spot lights
float ComputeAttenuation(float distanceSource, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - distanceSource) / (falloffEnd - falloffStart));
}

// Reflection base on fresnel approximation
float3 FresnelReflaction(float3 R0, float3 vNormal, float3 lightDirection)
{
    float cosNL = saturate(dot(vNormal, lightDirection));

    float f0 = 1.0f - cosNL;
	float3 reflectFactor = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectFactor;
}

// Computes light based on fresnel reflection and roughness factor
float3 BlinnPhong(float3 lightStrength, float3 lightDirection,
	float3 vNormal, float3 toCamera, Material material)
{
    const float m = material.Shininess * 256.0f;

    float3 microfacetNormal = normalize(toCamera + lightDirection);

	float roughnessFactor = 
		(m + 8.0f) * pow(max(dot(microfacetNormal, vNormal), 0.0f), m) / 8.0f;
	float3 fresnelFactor =
		FresnelReflaction(material.FresnelR0, microfacetNormal, lightDirection);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	// Normalize for ldr (low-density range of colors)
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (material.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light light, Material material, float3 vNormal,
	float3 toCamera)
{
    float3 lightDirection = -light.Direction;

	// clamp because of possible negative cos
	float cosNL= max(dot(lightDirection, vNormal), 0.0f);

	// because of lambert's cos law
	float3 lightStrength = light.Strength*cosNL;

	return BlinnPhong(lightStrength, lightDirection, vNormal, toCamera, material);
}

float3 ComputePointLight(Light light, Material material, float3 vPos, float3 vNormal, float3 toCamera, float gameTime)
{
    float3 lightDirection = light.Position - vPos;

    float distanceToLight = length(lightDirection);

	// if distance to light is more than attenuation end -> return
    if (distanceToLight > light.FalloffEnd)
		return 0.0f;

	// normalize
    lightDirection /= distanceToLight;

    float cosLN = max(dot(lightDirection, vNormal), 0.0f);
    float3 lightStrength = light.Strength * cosLN;

    float attenuationFactor = ComputeAttenuation(distanceToLight, light.FalloffStart, light.FalloffEnd);
    lightStrength *= attenuationFactor;

    return BlinnPhong(lightStrength, lightDirection, vNormal, toCamera, material);
}

float3 ComputeSpotLight(Light light, Material material, float3 vPos,
	float3 vNormal, float3 toCamera, float gameTime)
{
    float3 lightDirection = light.Position - vPos;

    float distanceToLight = length(lightDirection);

	// if distance to light is more than attenuation end -> return
    if (distanceToLight > light.FalloffEnd)
        return 0.0f;

	// normalize
    lightDirection /= distanceToLight;

    float cosLN = max(dot(lightDirection, vNormal), 0.0f);
    float3 lightStrength = light.Strength * cosLN;

    float attenuationFactor = ComputeAttenuation(distanceToLight, light.FalloffStart, light.FalloffEnd);
    lightStrength *= attenuationFactor;

    float spotFactor = pow(max(dot(-lightDirection, light.Direction), 0.0f), light.SpotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightDirection, vNormal, toCamera, material);
}

float4 ComputeLighting(Light lights[MaxLights], Material material,
	float3 vPos, float3 vNormal, float3 toCamera, float3 shadowFactor, float gameTime)
{
	float3 result = 0.0f;

	int i = 0;

#if (NUM_DIR_LIGHTS > 0)
	for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
	{
		result += shadowFactor[i] * ComputeDirectionalLight(
			lights[i], material, vNormal, toCamera);
	}
#endif

#if (NUM_POINT_LIGHTS > 0)
	for (i = NUM_DIR_LIGHTS;
		i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
	{
		result += ComputePointLight(lights[i], material,
			vPos, vNormal, toCamera, gameTime);
	}
#endif
#if (NUM_SPOT_LIGHTS > 0)
	for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
		i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
	{
		result += ComputeSpotLight(lights[i], material,
			vPos, vNormal, toCamera, gameTime);
	}
#endif

	return float4(result, 0.0f);
}