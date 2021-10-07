#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include <directxmath.h>
#include <iostream>

class Camera {
public:
	enum CAMERA_TYPE {
		ORTHOGRAPHIC = 0,
		PERSPECTIVE
	};

private:
	CAMERA_TYPE m_CameraType;

	DirectX::XMFLOAT4 m_Position;
	DirectX::XMFLOAT4 m_LookAt;
	DirectX::XMFLOAT4 m_Up;
	float m_FOV;
	float m_AspectRatio;
	float m_NearPlane;
	float m_FarPlane;

	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;

public:
	Camera(DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 lookAt, DirectX::XMFLOAT4 up, CAMERA_TYPE cameraType, float aspectRatio, float fov, float nearPlane, float farPlane);
	~Camera();

	DirectX::XMFLOAT4 GetCameraPosition()			{ return m_Position; }
	DirectX::XMFLOAT4 GetCameraLookAt()				{ return m_LookAt; }
	DirectX::XMFLOAT4 GetCameraUp()					{ return m_Up; }

	DirectX::XMMATRIX CalculateViewMatrix();
	DirectX::XMMATRIX CalculateProjectionMatrix();

	void SetCameraPosition(DirectX::XMFLOAT4 val);
	void SetCameraLookAt(DirectX::XMFLOAT4 val);
	void SetCameraUp(DirectX::XMFLOAT4 val);
};