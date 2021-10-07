#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 lookAt, DirectX::XMFLOAT4 up, CAMERA_TYPE cameraType, float aspectRatio, float fov, float nearPlane, float farPlane) :
	m_Position(position), m_LookAt(lookAt), m_Up(up), m_CameraType(cameraType), m_AspectRatio(aspectRatio), m_FOV(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane) 
{ }

Camera::~Camera() { }

void Camera::SetCameraPosition(DirectX::XMFLOAT4 val) {
	m_Position = val;
}

void Camera::SetCameraLookAt(DirectX::XMFLOAT4 val) {
	m_LookAt = val;
}

void Camera::SetCameraUp(DirectX::XMFLOAT4 val) {
	m_Up = val;
}

DirectX::XMMATRIX Camera::CalculateViewMatrix() {
	DirectX::XMVECTOR eye = DirectX::XMLoadFloat4(&m_Position);
	DirectX::XMVECTOR at = DirectX::XMLoadFloat4(&m_LookAt);
	DirectX::XMVECTOR up = DirectX::XMLoadFloat4(&m_Up);

	return DirectX::XMMatrixLookAtLH(eye, at, up);
}

DirectX::XMMATRIX Camera::CalculateProjectionMatrix() {
	switch (m_CameraType) {
	case CAMERA_TYPE::ORTHOGRAPHIC:
		return DirectX::XMMatrixOrthographicLH(m_FOV * m_AspectRatio, m_FOV, m_NearPlane, m_FarPlane);
		break;

	case CAMERA_TYPE::PERSPECTIVE:
		return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
		break;

	default:
		std::cout << "ERROR: Invalid Camera Type: " << m_CameraType << std::endl;
		return DirectX::XMMATRIX();
		break;
	}
}
