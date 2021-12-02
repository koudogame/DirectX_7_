// Include
#include <memory>
#include <chrono>
#include <Windows.h>
#include <d3d11.h> // DirectX11を使用する

// DirectX Tool Kit
#include <SimpleMath.h>     // DirectX数学補助ライブラリ
#include <PrimitiveBatch.h> // ポリゴン描画ライブラリ
#include <VertexTypes.h>    // 頂点フォーマット定義クラス
#include <Effects.h>        // エフェクトクラス

// ライブラリの関連付け
#pragma comment( lib, "d3d11.lib" )

// 名前空間
using namespace DirectX;
using namespace SimpleMath;
using namespace std::chrono;


// プロトタイプ宣言
LRESULT CALLBACK WindowProc(
    HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // ウィンドウクラスの作成
    WNDCLASSEX wnd;
    ZeroMemory(&wnd, sizeof(wnd));                              // 全てのメンバを0で初期化
    wnd.cbSize = sizeof(WNDCLASSEX);                       // この構造体型のサイズを渡す
    wnd.style = CS_HREDRAW | CS_VREDRAW;                    // クラススタイルの指定
    wnd.hInstance = hInstance;                                  // インスタンスハンドルを指定
    wnd.lpszClassName = "ClassName";                                // クラス名
    wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);           // カーソル形状
    wnd.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // 背景色
    wnd.lpfnWndProc = WindowProc;                                 // ウィンドウプロシージャの関数ポインタ

    // ウィンドウクラスの登録
    if (!RegisterClassEx(&wnd))
    {
        // エラー
        return 0;
    }

    // ウィンドウスタイルの決定
    // どのようなウィンドウにするか（最大化、最小化、閉じるボタンの有無、サイズ変更を許可するかなど）
    const DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    const DWORD ex_style = WS_EX_OVERLAPPEDWINDOW;

    // クライアント領域の設定
    RECT rect = { 0L, 0L, 1280L, 720L };
    AdjustWindowRectEx(&rect, style, false, ex_style);

    // ウィンドウの作成
    // 関数が成功するとウィンドウハンドルが戻り値として返ってくる
    HWND hWnd = CreateWindowEx(
        ex_style,               // 拡張ウィンドウスタイル
        "ClassName",            // クラス名
        "Game",                 // ウィンドウタイトル
        style,                  // ウィンドウスタイル
        CW_USEDEFAULT,          // 表示座標X
        CW_USEDEFAULT,          // 表示座標Y
        rect.right - rect.left, // ウィンドウサイズX
        rect.bottom - rect.top, // ウィンドウサイズY
        nullptr,                // 親ウィンドウのハンドル
        nullptr,                // メニューハンドル
        hInstance,              // インスタンスハンドル
        nullptr);              // ウィンドウ作成結果受取先ポインタ

    // ウィンドウハンドルが作成できたか
    if (!hWnd)
    {
        // エラー
        return 0;
    }

    // 機能レベルの設定
    D3D_FEATURE_LEVEL level_array[] =
    {
        D3D_FEATURE_LEVEL_11_0, // DirectX11
        D3D_FEATURE_LEVEL_10_1, // DirectX10.1
        D3D_FEATURE_LEVEL_10_0  // Directx10
    };

    // スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC sc;
    ZeroMemory(&sc, sizeof(sc));
    sc.Windowed = true; // ウィンドウモード
    sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // スワップエフェクトの設定
    sc.OutputWindow = hWnd; // ウィンドウハンドルの指定
    sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // バックバッファの使用方法
    sc.BufferCount = 1U; // バックバッファの数
    sc.BufferDesc.Width = 1280U; // バックバッファサイズ（横）
    sc.BufferDesc.Height = 720U; // バックバッファサイズ（縦）
    sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // フォーマット
    sc.BufferDesc.RefreshRate.Numerator = 60U; // リフレッシュレート分子
    sc.BufferDesc.RefreshRate.Denominator = 1U;// リフレッシュレート分母
    sc.SampleDesc.Count = 1; // マルチサンプリングレベル
    sc.SampleDesc.Quality = 0; // マルチサンプリング品質
    sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // モード切替有効

    // Direct3Dインターフェイス
    ID3D11Device* d3d_device = nullptr; // 描画先の作成等
    ID3D11DeviceContext* device_context = nullptr; // 描画制御等
    IDXGISwapChain* swap_chain = nullptr; // 実際の描画等
    D3D_FEATURE_LEVEL    level;                    // 機能レベル

    // 機能レベル数を計算
    int num_feature_level = sizeof(level_array) / sizeof(level_array[0]);

    // IDXGUSwapChain、D3DDevice、D3DDeviceContextをまとめて作成する
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,                    // DXGIアダプター
        D3D_DRIVER_TYPE_HARDWARE,   // ドライバータイプ(頂点を処理するのは誰か)
        // ↑もし失敗するようであればD3D_DRIVER_TYPE_WARPを指定
        nullptr,                    // ソフトウェアラスタライザーDLLハンドル
        D3D11_CREATE_DEVICE_DEBUG,  // フラグ(Debug機能を有効にする）
        //0, // ↑Release時はD3D11_CREATE_DEVICE_DEBUGを指定せずに0を渡す
        level_array,                // 機能レベル
        num_feature_level,          // レベル数
        D3D11_SDK_VERSION,          // 常にこの値
        &sc,                        // DXGI_SWAP_CHAIN_DESC構造体ポインタ
        &swap_chain,                // 受取先IDXGISwapChainポインタの変数
        &d3d_device,                // 受取先ID3D11Deviceポインタの変数
        &level,                     // 設定された機能レベル
        &device_context)))       // 受取先ID3D11DeviceContextポインタの変数
    {
        // エラー
        return 0;
    }

    // バックバッファの描画ターゲットを取得
    // SwapChainが持っている描画領域（キャンパスのようなもの）を取り出す
    ID3D11Texture2D* back_buffer = nullptr;
    if (FAILED(swap_chain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&back_buffer))))
    {
        // エラー
        return 0;
    }

    // 描画ターゲット・ビューの作成
    // 取り出したキャンパスが絵を描くところであると教える
    ID3D11RenderTargetView* render_target_view = nullptr;
    if (FAILED(d3d_device->CreateRenderTargetView(
        back_buffer,
        nullptr,
        &render_target_view)))
    {
        // エラー
        return 0;
    }

    // 描画ターゲットビューを出力マネージャーの描画ターゲットとして設定
    device_context->OMSetRenderTargets(
        1,                   // 描画ターゲット数
        &render_target_view, // ターゲットビュー配列
        nullptr);           // 深度ステンシルビュー

    // バックバッファの解放
    back_buffer->Release();

    // ビューポートの作成と設定
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = 1280.0F; // 描画領域の幅
    vp.Height = 720.0F; // 描画領域の高さ
    vp.MinDepth = 0.0F; // 最小深度値
    vp.MaxDepth = 1.0F; // 最大深度値
    device_context->RSSetViewports(1, &vp);

    // COMの初期化
    // これを行わないとDirectX Tool Kitが持っている関数のほとんどで失敗する
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        // エラー
        return 0;
    }

    // カメラ設定
    // DirectXTKは右手座標系なので手前が＋
    Vector3 eye(0.0F, 1.0F, 3.0F);
    Vector3 target(0.0F, 0.0F, 0.0F);
    Vector3 up(0.0F, 1.0F, 0.0F);

    // ビュー行列の作成
    Matrix view = Matrix::CreateLookAt(eye, target, up);

    // 画面比率を計算
    float ratio = 1280.0F / 720.0F;

    // 射影行列の作成
    Matrix projection = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(30.0F), // 画角
        ratio,                       // 画面比率
        1.0F,                        // NEARクリップ面
        100.0F);                    // FARクリップ面

    // ポリゴン描画管理クラスの宣言
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> primitive;
    primitive = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device_context);

    // エフェクトクラスの宣言
    BasicEffect effect(d3d_device);

    // 頂点カラーを有効にする(これを書かないと頂点カラーが出力されない)
    effect.SetVertexColorEnabled(true);

    // コンパイル済みシェーダへのポインタとサイズを取得
    const void* blob;
    size_t size;
    effect.GetVertexShaderBytecode(&blob, &size);

    // 頂点入力レイアウトの作成
    ID3D11InputLayout* layout;
    if (FAILED(d3d_device->CreateInputLayout(
        VertexPositionColor::InputElements,     // 入力データ配列
        VertexPositionColor::InputElementCount, // 入力データ数
        blob,
        size,
        &layout)))
    {
        // エラー
        return 0;
    }

    // 頂点データの作成
    VertexPositionColor v0(Vector3(-0.5F, 0.5F, 0.5F), Vector4(1.0F, 0.0F, 0.0F, 1.0F));
    VertexPositionColor v1(Vector3(0.5F, 0.5F, 0.5F), Vector4(0.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v2(Vector3(-0.5F, -0.5F, 0.5F), Vector4(0.0F, 0.0F, 1.0F, 1.0F));
    VertexPositionColor v3(Vector3(0.5F, -0.5F, 0.5F), Vector4(1.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v4(Vector3(-0.5F, 0.5F, -0.5F), Vector4(1.0F, 0.0F, 0.0F, 1.0F));
    VertexPositionColor v5(Vector3(0.5F, 0.5F, -0.5F), Vector4(0.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v6(Vector3(-0.5F, -0.5F, -0.5F), Vector4(1.0F, 0.0F, 1.0F, 1.0F));
    VertexPositionColor v7(Vector3(0.5F, -0.5F, -0.5F), Vector4(1.0F, 1.0F, 0.0F, 1.0F));

    // 配列に格納
    VertexPositionColor p[] = { v0, v1, v2, v3, v4, v5, v6, v7 };

    // インデックス値の作成
    unsigned short index[] =
    {
        0, 1, 2,
        2, 1, 3,
        4, 5, 0,
        0, 5, 1,
        1, 5, 3,
        5, 7, 3,
        5, 6, 7,
        5, 4, 6,
        4, 0, 2,
        4, 2, 6,
        7, 6, 3,
        6, 2, 3
    };

    // ウィンドウの表示
    ShowWindow(hWnd, SW_SHOWNORMAL);

    // メインループ
    MSG msg = { 0 };

    // ループ制御のための時刻計測
    auto prev = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT)
    {
        // メッセージ処理
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // メッセージが来ていたらウィンドウプロシージャを呼び出す
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // 現在の時刻を取得
            auto now = high_resolution_clock::now();

            // 現在の時刻から前回の時刻を引く
            auto elapsed = duration_cast<microseconds>(now - prev);

            // 約16ミリ秒以上経過していたら更新を行う
            if (elapsed.count() > 16666LL)
            {
                // 現在時刻を過去の時刻として上書き
                prev = now;

                // ワールド行列の作成
                Matrix world = Matrix::Identity;// 単位行列で初期化

                // 【拡大縮小】→【X回転】→【Y回転】→【Z回転】→【移動】の順番にかけ合わせること

                // 拡大縮小
                world += Matrix::CreateScale(1.5f, 0.5f, 1.0f);

                // Vector3構造体を引数で渡して設定することもできる
                //Vector3 scale(2.0f, 2.0f, 2.0f);
                //world += Matrix::CreateScale(scale);

                // Matrix scale = Matrix::CreateScale(1.5f, 1.5f, 1.5f);

                // 回転行列
                world *= Matrix::CreateRotationX(XMConvertToRadians(30.0f));
                world *= Matrix::CreateRotationY(XMConvertToRadians(45.0f));
                world *= Matrix::CreateRotationZ(XMConvertToRadians(60.0f));

                // 平行移動行列
                world *= Matrix::CreateTranslation(-0.5f, 0.2f, -0.5f);

                // 画面クリア
                float color[] = { 0.3F, 0.3F, 0.3F, 1.0F }; // RGBAを0.0～1.0の間で指定
                device_context->ClearRenderTargetView(render_target_view, color);

                // 行列の設定
                effect.SetWorld(world);
                //effect.SetView(view);
                effect.SetProjection(projection);

                // エフェクトの設定を適用
                effect.Apply(device_context);

                // 頂点入力レイアウトの設定
                device_context->IASetInputLayout(layout);

                // ポリゴン描画開始
                primitive->Begin();

                // 描画
                primitive->DrawIndexed(
                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, // 描画方法
                    index,                                 // インデックス配列先頭ポインタ
                    sizeof(index),                         // インデックス配列のサイズ
                    p,                                     // 頂点データ配列先頭ポインタ
                    8                                      // 描画する頂点数
                );
                // ポリゴン描画終了
                primitive->End();

                // 画面更新
                swap_chain->Present(1, 0);
            }
        }
    }

    // インターフェースの解放
    // 確保した順の逆に解放していく
    layout->Release();
    render_target_view->Release();
    swap_chain->Release();
    device_context->ClearState();
    device_context->Release();
    d3d_device->Release();

    return 0;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(
    HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // メッセージにより分岐
    switch (Msg)
    {
    case WM_KEYDOWN: // キーが押された
        switch (wParam)
        {
        case VK_ESCAPE: // ESCキーが押された
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        break;

    case WM_DESTROY: // アプリケーションが閉じられた
        PostQuitMessage(0); // WM_QUITメッセージを発生させる関数
        break;
    }

    // ウィンドウプロシージャ内で処理しなかったメッセージのデフォルトの動作を
    // 自動的に行なってくれる肝鈴羽
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}