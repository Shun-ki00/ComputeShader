#pragma once
#include <future>
#include "Framework/DebugCamera.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

	// �����f�[�^�̐�
	static const int MATRIX_PAIR_COUNT;

private:

	struct MatrixPair
	{
		DirectX::SimpleMath::Matrix matrixA;
		DirectX::SimpleMath::Matrix matrixB;
	};

public:

	// �R���X�g���N�^
	Scene();
	// �f�X�g���N�^
	~Scene() = default;

public:

	// ����������
	void Initialize();
	// �X�V����
	void Update();
	// �`�揈��
	void Render();
	// �I������
	void Finalize();

private:

	// �V�F�[�_�[�A�o�b�t�@�Ȃǂ̍쐬
	void CreateShaderBuffers();

	// �v�Z���s�� GPU
	void CalculateGPU();
	// �v�Z���s�� CPU
	void CalculateCPU();

private:

	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;
	// �R���s���[�g�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;

	// ���̓o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_inputBuffer;
	// �o�̓o�b�t�@ CPU
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_outputBufferCPU;
	// �o�̓o�b�t�@ GPU
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_outputBufferGPU;

	// ���̓V�F�[�_�[���\�[�X�r���[
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_inputShaderResourceView;
	// �o�̓V�F�[�_�[���\�[�X�r���[
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_outputUnorderedAccessView;

	Microsoft::WRL::ComPtr<ID3D11Query> m_timestampStart;
	Microsoft::WRL::ComPtr<ID3D11Query> m_timestampEnd;
	Microsoft::WRL::ComPtr<ID3D11Query> m_disjointQuery;

	// �v�Z����
	std::vector<MatrixPair> m_inputDatas;

	// GPU�v�Z���s�t���O
	bool m_isActiveGPU;
	// CPU�v�Z���s�t���O
	bool m_isActiveCPU;

	// �v�Z����
	float m_finalTime;
};