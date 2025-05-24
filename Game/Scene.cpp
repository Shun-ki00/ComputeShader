#include "pch.h"
#include "Game/Scene.h"
#include "Framework/Microsoft/ReadData.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"

const int Scene::MATRIX_PAIR_COUNT = 1000000;

/// <summary>
/// �R���X�g���N�^
/// </summary>
Scene::Scene()
	:
	m_commonResources{},
	m_device{},
	m_context{},
	m_computeShader{},
	m_inputBuffer{},
	m_outputBufferCPU{},
	m_outputBufferGPU{},
	m_inputShaderResourceView{},
	m_outputUnorderedAccessView{},
	m_timestampStart{},
	m_timestampEnd{},
	m_disjointQuery{},
	m_inputDatas{},
	m_isActiveGPU{},
	m_isActiveCPU{}
{
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{
	// �t���O�̏�����
	m_isActiveCPU  = false;
	m_isActiveGPU  = false;

	// ���l�̏�����
	m_finalTime = 0.0f;

	// �V�F�[�_�[�A�o�b�t�@�Ȃǂ̍쐬
	this->CreateShaderBuffers();

	// �������̓f�[�^���m��
	m_inputDatas = std::vector<MatrixPair>(MATRIX_PAIR_COUNT);
	for (int i = 0; i < m_inputDatas.size(); i++)
	{
		m_inputDatas[i].matrixA = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up);
		m_inputDatas[i].matrixB = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up);
	}
}

/// <summary>
/// �X�V����
/// </summary>
void Scene::Update()
{

}

/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render()
{
	// �E�B���h�E�T�C�Y���Œ�
	ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGui::Begin(u8"�v�Z�E�B���h�E", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove);

	ImGui::Text(u8"�s��ς̌v�Z (�v�f��: %d)", MATRIX_PAIR_COUNT);
	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	ImGui::Text(u8"����������I�����Čv�Z�����s:");

	// �����񂹂̂��߂̃��C�A�E�g�␳
	float contentWidth = ImGui::GetContentRegionAvail().x;
	float buttonWidth = 200.0f;
	float spacing = 20.0f;
	float totalButtonWidth = (buttonWidth * 2) + spacing;
	float offsetX = (contentWidth - totalButtonWidth) * 0.5f;
	ImGui::SetCursorPosX(offsetX);

	// CPU�v�Z�{�^��
	if (ImGui::Button(u8"CPU�Ōv�Z", ImVec2(buttonWidth, 0)))
	{
		this->CalculateCPU();
		m_isActiveCPU = true;
		m_isActiveGPU = false;
	}

	ImGui::SameLine(0.0f, spacing);

	// GPU�v�Z�{�^��
	if (ImGui::Button(u8"GPU�Ōv�Z", ImVec2(buttonWidth, 0)))
	{
		this->CalculateGPU();
		m_isActiveGPU = true;
		m_isActiveCPU = false;
	}

	ImGui::Dummy(ImVec2(0.0f, 20.0f));
	ImGui::Separator();

	// �������ԕ\��
	if (m_isActiveCPU || m_isActiveGPU)
	{
		const char* method = m_isActiveCPU ? "CPU" : "GPU";
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), u8"%s �̏�������: %.6f �b", method, m_finalTime);
	}

	ImGui::End();
}

void Scene::Finalize()
{

}


/// <summary>
/// CPU�Ōv�Z���s��
/// </summary>
void Scene::CalculateCPU()
{
	// ���ԑ���p�̃G�C���A�X
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration = std::chrono::duration<float>;

	DirectX::SimpleMath::Matrix result = DirectX::SimpleMath::Matrix::Identity;

	// --- �v�Z�J�n ---
	TimePoint startTime = Clock::now();

	// �v�Z���s��
	for (int i = 0; i < MATRIX_PAIR_COUNT; i++)
	{
		result = m_inputDatas[i].matrixA * m_inputDatas[i].matrixB;
	}

	// --- �v�Z�I�� ---
	TimePoint endTime = Clock::now();
	Duration elapsed = endTime - startTime;

	// �v�Z���Ԃ��擾
	m_finalTime = elapsed.count();
}


/// <summary>
/// GPU�Ōv�Z�������s��
/// </summary>
void Scene::CalculateGPU()
{
	const UINT elementCount = static_cast<UINT>(MATRIX_PAIR_COUNT);
	const UINT dispatchCount = (elementCount + 255) / 256;

	// ���̓o�b�t�@��GPU�ɓ]��
	m_context->UpdateSubresource(
		m_inputBuffer.Get(),
		0, nullptr,
		m_inputDatas.data(),
		0, 0
	);

	// �^�C�~���O�v���J�n
	m_context->Begin(m_disjointQuery.Get());
	m_context->End(m_timestampStart.Get());

	// �R���s���[�g�V�F�[�_�[�ݒ�
	m_context->CSSetShader(m_computeShader.Get(), nullptr, 0);

	ID3D11ShaderResourceView* inputSrv = m_inputShaderResourceView.Get();
	ID3D11UnorderedAccessView* outputUav = m_outputUnorderedAccessView.Get();

	// ����SRV���o�C���h
	m_context->CSSetShaderResources(0, 1, &inputSrv);

	// �o��UAV���o�C���h
	m_context->CSSetUnorderedAccessViews(0, 1, &outputUav, nullptr);

	// �R���s���[�g�V�F�[�_�[���s
	m_context->Dispatch(dispatchCount, 1, 1);

	// GPU�o�� �� CPU�A�N�Z�X�p�X�e�[�W���O�o�b�t�@�փR�s�[
	m_context->CopyResource(m_outputBufferCPU.Get(), m_outputBufferGPU.Get());

	// �^�C�~���O�v���I��
	m_context->End(m_timestampEnd.Get());
	m_context->End(m_disjointQuery.Get());

	// GPU����������ҋ@�i�^�C�~���O�f�[�^�擾�j
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData = {};
	while (m_context->GetData(m_disjointQuery.Get(), &disjointData, sizeof(disjointData), 0) != S_OK);

	if (!disjointData.Disjoint)
	{
		UINT64 startTime = 0;
		UINT64 endTime = 0;

		m_context->GetData(m_timestampStart.Get(), &startTime, sizeof(UINT64), 0);
		m_context->GetData(m_timestampEnd.Get(), &endTime, sizeof(UINT64), 0);

		double gpuTimeSeconds = static_cast<double>(endTime - startTime) / disjointData.Frequency;
		m_finalTime = static_cast<float>(gpuTimeSeconds);
	}

	// �R���s���[�g�V�F�[�_�[�ƃo�C���h����
	m_context->CSSetShader(nullptr, nullptr, 0);

	// ���\�[�X�r���[������
	ID3D11ShaderResourceView* srvDummy = nullptr;
	m_context->CSSetShaderResources(0, 1, &srvDummy);

	ID3D11UnorderedAccessView* uavDummy = nullptr;
	m_context->CSSetUnorderedAccessViews(0, 1, &uavDummy, 0);
}




void Scene::CreateShaderBuffers()
{
	
	// --- �R���s���[�g�V�F�[�_�[�����[�h ---

	std::vector<uint8_t> blob;
	// �V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/ComputeShader.cso");
	DX::ThrowIfFailed(
		m_device->CreateComputeShader(blob.data(), blob.size(), nullptr,
			m_computeShader.ReleaseAndGetAddressOf())
	);

	// --- ���͗p�o�b�t�@ ---

	// �o�b�t�@�L�q�\���̂��[��������
	D3D11_BUFFER_DESC inputDesc = {};
	inputDesc.Usage = D3D11_USAGE_DEFAULT;                                   // �o�b�t�@�̎g�p�p�r���w��
	inputDesc.ByteWidth = sizeof(MatrixPair) * MATRIX_PAIR_COUNT;            // �o�b�t�@�S�̂̃o�C�g�T�C�Y
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;                        // �V�F�[�_�[���\�[�X�ǂݎ���p�Ƃ��Ďg���邱�Ƃ�����
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;             // �\�����o�b�t�@�ł��邱�Ƃ��w�肷��t���O
	inputDesc.StructureByteStride = sizeof(MatrixPair);                      // �\����1������̃T�C�Y

	// �o�b�t�@�̍쐬
	HRESULT hr = m_device->CreateBuffer(&inputDesc, nullptr, &m_inputBuffer);

	// SRV�\���̂��[��������
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;               // �t�H�[�}�b�g�̎w��
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; // �r���[�̎�ނ��w��
	srvDesc.Buffer.ElementWidth = MATRIX_PAIR_COUNT;        // �v�f�����w��

	// SRV�̍쐬
	hr = m_device->CreateShaderResourceView(m_inputBuffer.Get(), &srvDesc, &m_inputShaderResourceView);


	// --- �o�͗p�o�b�t�@ ---

	// �o�b�t�@�L�q�\���̂��[��������
	D3D11_BUFFER_DESC outputDesc = {};
	outputDesc.Usage = D3D11_USAGE_DEFAULT;                                     // �o�b�t�@�̎g�p�p�r���w��
	outputDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix) * MATRIX_PAIR_COUNT; // �o�b�t�@�S�̂̃o�C�g�T�C�Y
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;                         // �V�F�[�_�[���\�[�X�ǂݎ���p�Ƃ��Ďg���邱�Ƃ�����
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;               // �\�����o�b�t�@�ł��邱�Ƃ��w�肷��t���O
	outputDesc.StructureByteStride = sizeof(DirectX::SimpleMath::Matrix);       // �\����1������̃T�C�Y

	// �o�b�t�@�̍쐬
	hr = m_device->CreateBuffer(&outputDesc, nullptr, &m_outputBufferGPU);


	// SRV�̍쐬
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;               // �t�H�[�}�b�g�̎w��
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // �r���[�̎�ނ��w��
	uavDesc.Buffer.NumElements = MATRIX_PAIR_COUNT;         // �v�f�����w��

	// SRV�̍쐬
	hr = m_device->CreateUnorderedAccessView(m_outputBufferGPU.Get(), &uavDesc, &m_outputUnorderedAccessView);


	// --- �X�e�[�W���O�o�b�t�@�iGPU �� CPU�ǂݏo���j ---

	D3D11_BUFFER_DESC stagingDesc = {};
	stagingDesc.Usage = D3D11_USAGE_STAGING;                                     // �f�[�^�]����p�̈ꎞ�o�b�t�@�Ƃ��Ďg��
	stagingDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix) * MATRIX_PAIR_COUNT; // �s�񌋉ʂ��ꎞ�I�ɕێ����邽�߂̃T�C�Y���w��
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;                          // CPU ���f�[�^��ǂݎ�邽�߂̃o�b�t�@
	stagingDesc.BindFlags = 0;                                                   // �V�F�[�_�[�Ȃǂɒ��ڎg��Ȃ��o�b�t�@
	stagingDesc.MiscFlags = 0;                                                   // ���ʂȋ@�\�������Ȃ��ʏ�̃o�b�t�@

	// �o�b�t�@�̍쐬
	hr = m_device->CreateBuffer(&stagingDesc, nullptr, &m_outputBufferCPU);


	D3D11_QUERY_DESC desc = {};

	desc.Query = D3D11_QUERY_TIMESTAMP;
	m_device->CreateQuery(&desc, &m_timestampStart);
	m_device->CreateQuery(&desc, &m_timestampEnd);

	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	m_device->CreateQuery(&desc, &m_disjointQuery);
}
