#include "Particle.hlsli"

struct TransformationMatrix {
	float32_t4x4 WVP;
	float32_t4x4 World;
};
struct ParticleForGPU
{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 color;
};

//ConstantBuffer<TransformationMatrix> gTranfsformationMatrix : register(b0);
StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct VertexShaderInput {
	float32_t4 position : POSITION0;
	float32_t2 texcoord : TEXCOORD0;
	float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID){
	VertexShaderOutput output;
	//output.position = mul(input.position, gTranfsformationMatrix.WVP);
    output.position = mul(input.position, gTransformationMatrices[instanceId].WVP);
    output.texcoord = input.texcoord;
    //output.normal = normalize(mul(input.normal, (float32_t3x3) gTransformationMatrices[instanceId].World));
    output.color = gParticle[instanceId].color;
    output.color = gMaterial.color * textureColor * input.color;
	//output.normal = normalize(mul(input.normal, (float32_t3x3)gTranfsformationMatrix.World));
	return output;
}