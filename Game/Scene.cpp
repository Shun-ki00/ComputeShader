#include "pch.h"
#include "Game/Scene.h"
#include "Framework/Microsoft/ReadData.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"

const int Scene::MATRIX_PAIR_COUNT = 1000000;

/// <summary>
/// コンストラクタ
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
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{
	// フラグの初期化
	m_isActiveCPU  = false;
	m_isActiveGPU  = false;

	// 数値の初期化
	m_finalTime = 0.0f;

	// シェーダー、バッファなどの作成
	this->CreateShaderBuffers();

	// 初期入力データを確保
	m_inputDatas = std::vector<MatrixPair>(MATRIX_PAIR_COUNT);
	for (int i = 0; i < m_inputDatas.size(); i++)
	{
		m_inputDatas[i].matrixA = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up);
		m_inputDatas[i].matrixB = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up);
	}
}

/// <summary>
/// 更新処理
/// </summary>
void Scene::Update()
{

}

/// <summary>
/// 描画処理
/// </summary>
void Scene::Render()
{
	// ウィンドウサイズを固定
	ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	ImGui::Begin(u8"計算ウィンドウ", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove);

	ImGui::Text(u8"行列積の計算 (要素数: %d)", MATRIX_PAIR_COUNT);
	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	ImGui::Text(u8"処理方式を選択して計算を実行:");

	// 中央寄せのためのレイアウト補正
	float contentWidth = ImGui::GetContentRegionAvail().x;
	float buttonWidth = 200.0f;
	float spacing = 20.0f;
	float totalButtonWidth = (buttonWidth * 2) + spacing;
	float offsetX = (contentWidth - totalButtonWidth) * 0.5f;
	ImGui::SetCursorPosX(offsetX);

	// CPU計算ボタン
	if (ImGui::Button(u8"CPUで計算", ImVec2(buttonWidth, 0)))
	{
		this->CalculateCPU();
		m_isActiveCPU = true;
		m_isActiveGPU = false;
	}

	ImGui::SameLine(0.0f, spacing);

	// GPU計算ボタン
	if (ImGui::Button(u8"GPUで計算", ImVec2(buttonWidth, 0)))
	{
		this->CalculateGPU();
		m_isActiveGPU = true;
		m_isActiveCPU = false;
	}

	ImGui::Dummy(ImVec2(0.0f, 20.0f));
	ImGui::Separator();

	// 処理時間表示
	if (m_isActiveCPU || m_isActiveGPU)
	{
		const char* method = m_isActiveCPU ? "CPU" : "GPU";
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), u8"%s の処理時間: %.6f 秒", method, m_finalTime);
	}

	ImGui::End();
}

void Scene::Finalize()
{

}


/// <summary>
/// CPUで計算を行う
/// </summary>
void Scene::CalculateCPU()
{
	// 時間測定用のエイリアス
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Duration = std::chrono::duration<float>;

	DirectX::SimpleMath::Matrix result = DirectX::SimpleMath::Matrix::Identity;

	// --- 計算開始 ---
	TimePoint startTime = Clock::now();

	// 計算を行う
	for (int i = 0; i < MATRIX_PAIR_COUNT; i++)
	{
		result = m_inputDatas[i].matrixA * m_inputDatas[i].matrixB;
	}

	// --- 計算終了 ---
	TimePoint endTime = Clock::now();
	Duration elapsed = endTime - startTime;

	// 計算時間を取得
	m_finalTime = elapsed.count();
}


/// <summary>
/// GPUで計算処理を行う
/// </summary>
void Scene::CalculateGPU()
{
	const UINT elementCount = static_cast<UINT>(MATRIX_PAIR_COUNT);
	const UINT dispatchCount = (elementCount + 255) / 256;

	// 入力バッファをGPUに転送
	m_context->UpdateSubresource(
		m_inputBuffer.Get(),
		0, nullptr,
		m_inputDatas.data(),
		0, 0
	);

	// タイミング計測開始
	m_context->Begin(m_disjointQuery.Get());
	m_context->End(m_timestampStart.Get());

	// コンピュートシェーダー設定
	m_context->CSSetShader(m_computeShader.Get(), nullptr, 0);

	ID3D11ShaderResourceView* inputSrv = m_inputShaderResourceView.Get();
	ID3D11UnorderedAccessView* outputUav = m_outputUnorderedAccessView.Get();

	// 入力SRVをバインド
	m_context->CSSetShaderResources(0, 1, &inputSrv);

	// 出力UAVをバインド
	m_context->CSSetUnorderedAccessViews(0, 1, &outputUav, nullptr);

	// コンピュートシェーダー実行
	m_context->Dispatch(dispatchCount, 1, 1);

	// GPU出力 → CPUアクセス用ステージングバッファへコピー
	m_context->CopyResource(m_outputBufferCPU.Get(), m_outputBufferGPU.Get());

	// タイミング計測終了
	m_context->End(m_timestampEnd.Get());
	m_context->End(m_disjointQuery.Get());

	// GPU処理完了を待機（タイミングデータ取得）
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

	// コンピュートシェーダーとバインド解除
	m_context->CSSetShader(nullptr, nullptr, 0);

	// リソースビューを解除
	ID3D11ShaderResourceView* srvDummy = nullptr;
	m_context->CSSetShaderResources(0, 1, &srvDummy);

	ID3D11UnorderedAccessView* uavDummy = nullptr;
	m_context->CSSetUnorderedAccessViews(0, 1, &uavDummy, 0);
}




void Scene::CreateShaderBuffers()
{
	
	// --- コンピュートシェーダーをロード ---

	std::vector<uint8_t> blob;
	// シェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/ComputeShader.cso");
	DX::ThrowIfFailed(
		m_device->CreateComputeShader(blob.data(), blob.size(), nullptr,
			m_computeShader.ReleaseAndGetAddressOf())
	);

	// --- 入力用バッファ ---

	// バッファ記述構造体をゼロ初期化
	D3D11_BUFFER_DESC inputDesc = {};
	inputDesc.Usage = D3D11_USAGE_DEFAULT;                                   // バッファの使用用途を指定
	inputDesc.ByteWidth = sizeof(MatrixPair) * MATRIX_PAIR_COUNT;            // バッファ全体のバイトサイズ
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;                        // シェーダーリソース読み取り専用として使われることを示す
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;             // 構造化バッファであることを指定するフラグ
	inputDesc.StructureByteStride = sizeof(MatrixPair);                      // 構造体1つあたりのサイズ

	// バッファの作成
	HRESULT hr = m_device->CreateBuffer(&inputDesc, nullptr, &m_inputBuffer);

	// SRV構造体をゼロ初期化
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;               // フォーマットの指定
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; // ビューの種類を指定
	srvDesc.Buffer.ElementWidth = MATRIX_PAIR_COUNT;        // 要素数を指定

	// SRVの作成
	hr = m_device->CreateShaderResourceView(m_inputBuffer.Get(), &srvDesc, &m_inputShaderResourceView);


	// --- 出力用バッファ ---

	// バッファ記述構造体をゼロ初期化
	D3D11_BUFFER_DESC outputDesc = {};
	outputDesc.Usage = D3D11_USAGE_DEFAULT;                                     // バッファの使用用途を指定
	outputDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix) * MATRIX_PAIR_COUNT; // バッファ全体のバイトサイズ
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;                         // シェーダーリソース読み取り専用として使われることを示す
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;               // 構造化バッファであることを指定するフラグ
	outputDesc.StructureByteStride = sizeof(DirectX::SimpleMath::Matrix);       // 構造体1つあたりのサイズ

	// バッファの作成
	hr = m_device->CreateBuffer(&outputDesc, nullptr, &m_outputBufferGPU);


	// SRVの作成
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;               // フォーマットの指定
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER; // ビューの種類を指定
	uavDesc.Buffer.NumElements = MATRIX_PAIR_COUNT;         // 要素数を指定

	// SRVの作成
	hr = m_device->CreateUnorderedAccessView(m_outputBufferGPU.Get(), &uavDesc, &m_outputUnorderedAccessView);


	// --- ステージングバッファ（GPU → CPU読み出し） ---

	D3D11_BUFFER_DESC stagingDesc = {};
	stagingDesc.Usage = D3D11_USAGE_STAGING;                                     // データ転送専用の一時バッファとして使う
	stagingDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix) * MATRIX_PAIR_COUNT; // 行列結果を一時的に保持するためのサイズを指定
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;                          // CPU がデータを読み取るためのバッファ
	stagingDesc.BindFlags = 0;                                                   // シェーダーなどに直接使わないバッファ
	stagingDesc.MiscFlags = 0;                                                   // 特別な機能を持たない通常のバッファ

	// バッファの作成
	hr = m_device->CreateBuffer(&stagingDesc, nullptr, &m_outputBufferCPU);


	D3D11_QUERY_DESC desc = {};

	desc.Query = D3D11_QUERY_TIMESTAMP;
	m_device->CreateQuery(&desc, &m_timestampStart);
	m_device->CreateQuery(&desc, &m_timestampEnd);

	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	m_device->CreateQuery(&desc, &m_disjointQuery);
}
