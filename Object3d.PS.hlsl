#include "Object3d.hlsli"

// float4 main() : SV_TARGET
//{
//     return float4(1.0f, 1.0f, 1.0f, 1.0f);
// }

struct Material {
	float32_t4 color;
	int32_t enableLighting;
	float32_t4x4 uvTransform;
    float32_t shininess;
};

struct DirectionalLight {
	float32_t4 color;
	float32_t3 direction;
	float intensity;
};

struct Camera
{
    float32_t3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0);

SamplerState gSampler : register(s0);

ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput {
	float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	float4 transformdUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float32_t4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);

	PixelShaderOutput output;

	if (gMaterial.enableLighting != 0) {
		
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
		
		float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
		float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
		
        float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(RdotE), gMaterial.shininess);
		
        float32_t3 diffuse =
		gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		
        float32_t3 specular =
		gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
		
        output.color.rgb = diffuse + specular;
		
        output.color.a = gMaterial.color.a * textureColor.a;
		
        //output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    } else {
		output.color = gMaterial.color * textureColor;
	}

	return output;
}