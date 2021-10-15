#pragma once
class Camera {
public:
	enum class CAMERA_TYPE {
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
	DirectX::XMFLOAT3 m_BackgroundColour;

public:
	Camera(DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 lookAt, DirectX::XMFLOAT4 up, CAMERA_TYPE cameraType, float aspectRatio, float fov, float nearPlane, float farPlane);
	~Camera();

	DirectX::XMFLOAT4 GetCameraPosition() { return m_Position; }
	DirectX::XMFLOAT4 GetCameraLookAt() { return m_LookAt; }
	DirectX::XMFLOAT4 GetCameraUp() { return m_Up; }
	float GetFOV() { return m_FOV; }
	float GetNearPlane() { return m_NearPlane; }
	float GetFarPlane() { return m_FarPlane; }
	DirectX::XMFLOAT3 GetBackgroundColour() { return m_BackgroundColour; }

	DirectX::XMMATRIX CalculateViewMatrix();
	DirectX::XMMATRIX CalculateProjectionMatrix();

	void SetCameraPosition(DirectX::XMFLOAT4 val);
	void SetCameraLookAt(DirectX::XMFLOAT4 val);
	void SetCameraUp(DirectX::XMFLOAT4 val);
	void SetFOV(float val);
	void SetNearPlane(float val);
	void SetFarPlane(float val);
	void SetBackgroundColour(DirectX::XMFLOAT3 val);

	void RenderGUIControls();
};