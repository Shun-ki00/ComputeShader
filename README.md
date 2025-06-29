# ComputeShader


## 概要

このプロジェクトは、**DirectX 11 の Compute Shader（計算シェーダー）** を活用し、CPUとGPUの計算性能を比較するサンプルプログラムです。

10万個の行列要素の演算を **CPUまたはGPUで実行**し、処理時間をリアルタイムで表示します。

## 特徴

- **Compute Shader** による並列計算処理の実演
- **CPUとGPUの演算速度の比較表示**
- `Dear ImGui` による分かりやすいユーザーインターフェース
- 計算時間をミリ秒単位で可視化

## スクリーンショット

![スクリーンショット 2025-06-02 122105](https://github.com/user-attachments/assets/f810659e-68bc-44d1-ac78-8e89aac9e5b0)

- 上部で要素数と処理方式（CPU / GPU）を選択
- 計算後、所要時間をミリ秒単位で表示（例：GPU の処理時間: 0.038594 秒）

## 処理の流れ（概要）

1. 行列の初期化（ランダムまたは定数）
2. ImGuiのボタンで「CPU計算」または「GPU計算」を選択
3. 処理時間を計測（`std::chrono` および GPUタイマー）
4. 結果を ImGui でリアルタイム表示

## デモ実行ファイル
最新版の実行ファイルはこちらからダウンロードできます： [Download ComputeShader v1.0.0](https://github.com/Shun-ki00/ComputeShader/releases/latest)


## 動作環境
・OS：Windows 10 / 11  
・GPU : Direct3D 11対応グラフィックカード  
・開発環境：Visual Studio 2022  
・ビルド対象：x64  

## 使用ライブラリ
・Direct3D 11  
・DirectXTK  
・Windows SDK  
・Dear ImGui  
