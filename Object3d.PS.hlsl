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

struct CameraForGPU
{
    Vector3 worldPosition;
};
struct Camera
{
    float32_t3 worldPosition;
};

//PixeShaderでCameraへの方向を算出
float32_t3 toEye = normalize(gCamera.worldPositon - input.worldPosition);
//入射光の反射ベクトルを求める
float32_t3 reflectLight = reflect(gDirectionalLight.direction, nomalize(input.normal));
//後は内積をとって、saturateして、shininess階乗すると鏡面反射の強度が求まる
float RdotE = dot(reflectLight, toEye);
float specularPow = pow(saturate(RdotE), gMaterial.shininess);//反射強度

//拡散反射
float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
//鏡面反射
float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
//拡散反射+鏡面反射
output.color.rgb = diffuse + specular;
//アルファはいつも通り
output.color.a = gMaterial.color.a * textureColor.a;


ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0);

SamplerState gSampler : register(s0);

struct PixelShaderOutput {
	float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	float4 transformdUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float32_t4 textureColor = gTexture.Sample(gSampler, transformdUV.xy);

	PixelShaderOutput output;

	if (gMaterial.enableLighting != 0) {
		float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
		float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
		//output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		output.color.a = gMaterial.color.a * textureColor.a;
	} else {
		output.color = gMaterial.color * textureColor;
	}


	if (textureColor.a <= 0.5) {
		discard;
	}

	if (textureColor.a == 0.0) {
		discard;
	}

	if (output.color.a == 0.0) {
		discard;
	}

	return output;
}