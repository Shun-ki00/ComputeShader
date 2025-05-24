#pragma once
#include <future>
#include "Framework/DebugCamera.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

	// 初期データの数
	static const int MATRIX_PAIR_COUNT;

private:

	struct MatrixPair
	{
		DirectX::SimpleMath::Matrix matrixA;
		DirectX::SimpleMath::Matrix matrixB;
	};

public:

	// コンストラクタ
	Scene();
	// デストラクタ
	~Scene() = default;

public:

	// 初期化処理
	void Initialize();
	// 更新処理
	void Update();
	// 描画処理
	void Render();
	// 終了処理
	void Finalize();

private:

	// シェーダー、バッファなどの作成
	void CreateShaderBuffers();

	// 計算を行う GPU
	void CalculateGPU();
	// 計算を行う CPU
	void CalculateCPU();

private:

	// 共有リソース
	CommonResources* m_commonResources;

	// デバイス
	ID3D11Device1* m_device;
	// コンテキスト
	ID3D11DeviceContext1* m_context;
	// コンピュートシェーダー
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;

	// 入力バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_inputBuffer;
	// 出力バッファ CPU
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_outputBufferCPU;
	// 出力バッファ GPU
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_outputBufferGPU;

	// 入力シェーダーリソースビュー
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_inputShaderResourceView;
	// 出力シェーダーリソースビュー
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_outputUnorderedAccessView;

	Microsoft::WRL::ComPtr<ID3D11Query> m_timestampStart;
	Microsoft::WRL::ComPtr<ID3D11Query> m_timestampEnd;
	Microsoft::WRL::ComPtr<ID3D11Query> m_disjointQuery;

	// 計算処理
	std::vector<MatrixPair> m_inputDatas;

	// GPU計算実行フラグ
	bool m_isActiveGPU;
	// CPU計算実行フラグ
	bool m_isActiveCPU;

	// 計算時間
	float m_finalTime;
};