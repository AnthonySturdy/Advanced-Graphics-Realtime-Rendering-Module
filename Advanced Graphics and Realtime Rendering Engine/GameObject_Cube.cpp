#include "pch.h"
#include "GameObject_Cube.h"

HRESULT GameObject_Cube::InitMesh(ID3D11Device* device, ID3D11DeviceContext* context) {
	// load and setup textures
	HRESULT hr = DirectX::CreateDDSTextureFromFile(device, L"Resources\\Pipes\\pipes-basecolor.dds", nullptr, &m_textureResourceView);
	if (FAILED(hr))
		return hr;

	hr = DirectX::CreateDDSTextureFromFile(device, L"Resources\\Pipes\\pipes-normal.dds", nullptr, &m_normalResourceView);
	if (FAILED(hr))
		return hr;

	hr = DirectX::CreateDDSTextureFromFile(device, L"Resources\\Pipes\\pipes-height.dds", nullptr, &m_parallaxResourceView);
	if (FAILED(hr))
		return hr;

	return GameObject::InitMesh(device, context);
}

void GameObject_Cube::Render(ID3D11DeviceContext* context) {
	context->PSSetShaderResources(0, 1, m_textureResourceView.GetAddressOf());
	context->PSSetShaderResources(1, 1, m_normalResourceView.GetAddressOf());
	context->PSSetShaderResources(2, 1, m_parallaxResourceView.GetAddressOf());

	GameObject::Render(context);
}

void GameObject_Cube::RenderGUIControls(ID3D11Device* device, Camera* camera) {
	if (ImGui::CollapsingHeader("GameObject Controls")) {
		GameObject::RenderGUIControls(device, camera);

		// Texture selection
		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(ImGui::GetColumnIndex(), ImGui::GetWindowWidth() / 3);
		if (ImGui::ImageButton(m_textureResourceView.Get(), ImVec2(ImGui::GetWindowWidth() / 4, ImGui::GetWindowWidth() / 4))) {
			ImGuiFileDialog::Instance()->Close();
			ImGuiFileDialog::Instance()->OpenDialog("ChooseDiffuseTex", "Choose Diffuse Texture", ".dds", ".");
		}
		ImGui::NextColumn();
		ImGui::Checkbox("Diffuse Texture", (bool*)&m_material.Material.UseTexture);
		ImGui::Columns();

		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(ImGui::GetColumnIndex(), ImGui::GetWindowWidth() / 3);
		if (ImGui::ImageButton(m_normalResourceView.Get(), ImVec2(ImGui::GetWindowWidth() / 4, ImGui::GetWindowWidth() / 4))) {
			ImGuiFileDialog::Instance()->Close();
			ImGuiFileDialog::Instance()->OpenDialog("ChooseNormalTex", "Choose Normal Texture", ".dds", ".");
		}
		ImGui::NextColumn();
		ImGui::Checkbox("Normal Texture", (bool*)&m_material.Material.UseNormal);
		ImGui::Columns();

		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(ImGui::GetColumnIndex(), ImGui::GetWindowWidth() / 3);
		if (ImGui::ImageButton(m_parallaxResourceView.Get(), ImVec2(ImGui::GetWindowWidth() / 4, ImGui::GetWindowWidth() / 4))) {
			ImGuiFileDialog::Instance()->Close();
			ImGuiFileDialog::Instance()->OpenDialog("ChooseHeightTex", "Choose Height Texture", ".dds", ".");
		}
		ImGui::NextColumn();
		ImGui::Checkbox("Parallax Texture", (bool*)&m_material.Material.UseParallax);
		ImGui::DragFloat("Strength", &m_material.Material.ParallaxStrength, 0.001f, 0.0f, 1.0f);
		ImGui::Columns();

		if (ImGuiFileDialog::Instance()->Display("ChooseDiffuseTex")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::wstring wFilePath = std::wstring(filePath.begin(), filePath.end());
				HRESULT hr = DirectX::CreateDDSTextureFromFile(device, wFilePath.c_str(), nullptr, m_textureResourceView.ReleaseAndGetAddressOf());
				if (FAILED(hr))
					throw 0;
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseNormalTex")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::wstring wFilePath = std::wstring(filePath.begin(), filePath.end());
				HRESULT hr = DirectX::CreateDDSTextureFromFile(device, wFilePath.c_str(), nullptr, m_normalResourceView.ReleaseAndGetAddressOf());
				if (FAILED(hr))
					throw 0;
			}
			ImGuiFileDialog::Instance()->Close();
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseHeightTex")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				std::wstring wFilePath = std::wstring(filePath.begin(), filePath.end());
				HRESULT hr = DirectX::CreateDDSTextureFromFile(device, wFilePath.c_str(), nullptr, m_parallaxResourceView.ReleaseAndGetAddressOf());
				if (FAILED(hr))
					throw 0;
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
}
