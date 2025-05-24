//
// Game.h
//

#include "DeviceResources.h"

#pragma once


class CommonResources;
class InputManager;
class Scene;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:
    // コンストラクタ
    Game() noexcept(false);
    // デストラクタ
    ~Game();
    Game(Game&&) = default;
    Game& operator= (Game&&) = default;
    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // 初期化処理
    void Initialize(HWND window, int width, int height);

    // ゲームループ
    void Tick();

    // IDeviceNotifyインターフェース
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // メッセージハンドラー
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // 規定ウィンドウサイズプロパティを取得する
    void GetDefaultSize(int& width, int& height) const noexcept;

private:
    // 更新処理
    void Update(DX::StepTimer const& timer);
    // 描画処理
    void Render();

    void Clear();
    // デバイス依存したリソースを生成する
    void CreateDeviceDependentResources();
    // ウィンドウサイズに依存したリソースを生成する
    void CreateWindowSizeDependentResources();


private:
    // ウィンドウハンドル
    HWND            m_hWnd;
    // タイマー
    DX::StepTimer   m_timer;

    // デバイス
    ID3D11Device1* m_device;
    // コンテキスト
    ID3D11DeviceContext1* m_context;
    //  コモンステート
    std::unique_ptr<DirectX::CommonStates>  m_commonStates;
    // デバイスリソース
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // 共有リソース
    CommonResources* m_commonResources;

    // シーン
    std::unique_ptr<Scene> m_scene;

};
