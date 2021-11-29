#include "pch.h"
#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT4 position, DirectX::XMFLOAT4 lookAt, DirectX::XMFLOAT4 up, CAMERA_TYPE cameraType, float aspectRatio, float fov, float nearPlane, float farPlane) :
	m_Position(position), m_LookAt(lookAt), m_Up(up), m_CameraType(cameraType), m_AspectRatio(aspectRatio), m_FOV(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane), m_BackgroundColour(DirectX::XMFLOAT4(19/255.f, 18/255.f, 24/255.f, 1.0f))
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

void Camera::SetFOV(float val) {
	m_FOV = val;
}

void Camera::SetNearPlane(float val) {
	m_NearPlane = val;
}

void Camera::SetFarPlane(float val) {
	m_FarPlane = val;
}

void Camera::SetBackgroundColour(DirectX::XMFLOAT4 val) {
	m_BackgroundColour = val;
}

void Camera::SetAspectRatio(float val) {
	m_AspectRatio = val;
}

void Camera::RenderGUIControls() {
	if (ImGui::CollapsingHeader("Active Camera Controls")) {
		ImGui::DragFloat3("Eye", &m_Position.x, 0.015f);
		ImGui::DragFloat3("LookAt", &m_LookAt.x, 0.015f);
		ImGui::DragFloat3("Up", &m_Up.x, 0.005f);

		ImGui::SliderAngle("FOV", &m_FOV, 5.0f, 160.0f);

		const char* items[] = { "Orthographic", "Perspective" };
		static int selection = 1;
		ImGui::Combo("Camera Type", &selection, items, 2);
		m_CameraType = (CAMERA_TYPE)selection;

		ImGui::DragFloat("Near Plane", &m_NearPlane, 0.01f, 0.1f, 100.0f);
		ImGui::DragFloat("Far Plane", &m_FarPlane, 0.1f, 0.2f, 5000.0f);

		ImGui::ColorEdit3("Background Colour", &m_BackgroundColour.x);
	}
}

DirectX::XMMATRIX Camera::CalculateViewMatrix() {
	const DirectX::XMVECTOR eye = DirectX::XMLoadFloat4(&m_Position);
	const DirectX::XMVECTOR at = DirectX::XMLoadFloat4(&m_LookAt);
	const DirectX::XMVECTOR up = DirectX::XMLoadFloat4(&m_Up);

	return DirectX::XMMatrixLookAtLH(eye, at, up);
}

DirectX::XMMATRIX Camera::CalculateProjectionMatrix() {
	float orthoScale = 10.0f;

	switch (m_CameraType) {
	case CAMERA_TYPE::ORTHOGRAPHIC:
		return DirectX::XMMatrixOrthographicLH(m_FOV * m_AspectRatio * orthoScale, m_FOV * orthoScale, m_NearPlane, m_FarPlane);
		break;

	case CAMERA_TYPE::PERSPECTIVE:
		return DirectX::XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
		break;

	default:
		std::cout << "ERROR: Invalid Camera Type!" << std::endl;
		return DirectX::XMMATRIX();
		break;
	}
}
