#pragma once
#include "pch.h"

struct SimpleVertex {
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT3 BiTangent;
};

struct ConstantBuffer {
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
	DirectX::XMFLOAT4 vOutputColor;
};

struct _Material {
	_Material()
		: Emissive(0.0f, 0.0f, 0.0f, 1.0f)
		, Ambient(0.2f, 0.2f, 0.2f, 1.0f)
		, Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
		, Specular(1.0f, 1.0f, 1.0f, 1.0f)
		, SpecularPower(128.0f)
		, UseTexture(false)
		, UseNormal(false)
		, UseParallax(false)
		, ParallaxStrength(0.1f)
		, Padding{ 0.0f, 0.0f, 0.0f }
	{
	}

	DirectX::XMFLOAT4   Emissive;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Ambient;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Diffuse;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Specular;
	//----------------------------------- (16 byte boundary)
	float               SpecularPower;
	int                 UseTexture;
	int					UseNormal;
	int					UseParallax;
	//----------------------------------- (16 byte boundary)
	float				ParallaxStrength;
	float				Padding[3];
}; // Total:                                80 bytes (5 * 16)

struct MaterialPropertiesConstantBuffer {
	_Material   Material;
};

enum LightType {
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};


struct Light {
	Light()
		: Position(0.0f, 0.0f, 0.0f, 1.0f)
		, Direction(0.0f, 0.0f, 1.0f, 0.0f)
		, Color(1.0f, 1.0f, 1.0f, 1.0f)
		, SpotAngle(DirectX::XM_PIDIV2)
		, ConstantAttenuation(1.0f)
		, LinearAttenuation(0.0f)
		, QuadraticAttenuation(0.0f)
		, LightType(DirectionalLight)
		, Enabled(0)
		, Padding{0, 0}
	{
	}

	DirectX::XMFLOAT4    Position;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4    Direction;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4    Color;
	//----------------------------------- (16 byte boundary)
	float       SpotAngle;
	float       ConstantAttenuation;
	float       LinearAttenuation;
	float       QuadraticAttenuation;
	//----------------------------------- (16 byte boundary)
	int         LightType;
	int         Enabled;
	// Add some padding to make this struct size a multiple of 16 bytes.
	int         Padding[2];
	//----------------------------------- (16 byte boundary)
};  // Total:                              80 bytes ( 5 * 16 )

#define MAX_LIGHTS 2

struct LightPropertiesConstantBuffer {
	LightPropertiesConstantBuffer()
		: EyePosition(0, 0, 0, 1)
		, GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f) {
	}

	DirectX::XMFLOAT4   EyePosition;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   GlobalAmbient;
	//----------------------------------- (16 byte boundary)
	Light               Lights[MAX_LIGHTS]; // 80 * 8 bytes
};  // Total:                                  672 bytes (42 * 16)

struct BloomConstantBuffer {
	float size;
	float quality;
	float directions;

	float _padding;
};

struct ImageFilterConstantBuffer {
	int filterType;
	float filterIntensity;
	float time;
	float _padding;
	// 16 bytes
	float resolution[2];
	float __padding[2];
};

struct DepthOfFieldConstantBuffer {
	float size;
	float quality;
	float directions;
	float farPlaneDepth;

	int resolution[2];
	float depth;
	float _padding;
};

struct ShadowMappingConstantBuffer{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT4 position;
};