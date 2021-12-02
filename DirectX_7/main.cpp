// Include
#include <memory>
#include <chrono>
#include <Windows.h>
#include <d3d11.h> // DirectX11���g�p����

// DirectX Tool Kit
#include <SimpleMath.h>     // DirectX���w�⏕���C�u����
#include <PrimitiveBatch.h> // �|���S���`�惉�C�u����
#include <VertexTypes.h>    // ���_�t�H�[�}�b�g��`�N���X
#include <Effects.h>        // �G�t�F�N�g�N���X

// ���C�u�����̊֘A�t��
#pragma comment( lib, "d3d11.lib" )

// ���O���
using namespace DirectX;
using namespace SimpleMath;
using namespace std::chrono;


// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(
    HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    // �E�B���h�E�N���X�̍쐬
    WNDCLASSEX wnd;
    ZeroMemory(&wnd, sizeof(wnd));                              // �S�Ẵ����o��0�ŏ�����
    wnd.cbSize = sizeof(WNDCLASSEX);                       // ���̍\���̌^�̃T�C�Y��n��
    wnd.style = CS_HREDRAW | CS_VREDRAW;                    // �N���X�X�^�C���̎w��
    wnd.hInstance = hInstance;                                  // �C���X�^���X�n���h�����w��
    wnd.lpszClassName = "ClassName";                                // �N���X��
    wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);           // �J�[�\���`��
    wnd.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1); // �w�i�F
    wnd.lpfnWndProc = WindowProc;                                 // �E�B���h�E�v���V�[�W���̊֐��|�C���^

    // �E�B���h�E�N���X�̓o�^
    if (!RegisterClassEx(&wnd))
    {
        // �G���[
        return 0;
    }

    // �E�B���h�E�X�^�C���̌���
    // �ǂ̂悤�ȃE�B���h�E�ɂ��邩�i�ő剻�A�ŏ����A����{�^���̗L���A�T�C�Y�ύX�������邩�Ȃǁj
    const DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    const DWORD ex_style = WS_EX_OVERLAPPEDWINDOW;

    // �N���C�A���g�̈�̐ݒ�
    RECT rect = { 0L, 0L, 1280L, 720L };
    AdjustWindowRectEx(&rect, style, false, ex_style);

    // �E�B���h�E�̍쐬
    // �֐�����������ƃE�B���h�E�n���h�����߂�l�Ƃ��ĕԂ��Ă���
    HWND hWnd = CreateWindowEx(
        ex_style,               // �g���E�B���h�E�X�^�C��
        "ClassName",            // �N���X��
        "Game",                 // �E�B���h�E�^�C�g��
        style,                  // �E�B���h�E�X�^�C��
        CW_USEDEFAULT,          // �\�����WX
        CW_USEDEFAULT,          // �\�����WY
        rect.right - rect.left, // �E�B���h�E�T�C�YX
        rect.bottom - rect.top, // �E�B���h�E�T�C�YY
        nullptr,                // �e�E�B���h�E�̃n���h��
        nullptr,                // ���j���[�n���h��
        hInstance,              // �C���X�^���X�n���h��
        nullptr);              // �E�B���h�E�쐬���ʎ���|�C���^

    // �E�B���h�E�n���h�����쐬�ł�����
    if (!hWnd)
    {
        // �G���[
        return 0;
    }

    // �@�\���x���̐ݒ�
    D3D_FEATURE_LEVEL level_array[] =
    {
        D3D_FEATURE_LEVEL_11_0, // DirectX11
        D3D_FEATURE_LEVEL_10_1, // DirectX10.1
        D3D_FEATURE_LEVEL_10_0  // Directx10
    };

    // �X���b�v�`�F�[���̐ݒ�
    DXGI_SWAP_CHAIN_DESC sc;
    ZeroMemory(&sc, sizeof(sc));
    sc.Windowed = true; // �E�B���h�E���[�h
    sc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // �X���b�v�G�t�F�N�g�̐ݒ�
    sc.OutputWindow = hWnd; // �E�B���h�E�n���h���̎w��
    sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �o�b�N�o�b�t�@�̎g�p���@
    sc.BufferCount = 1U; // �o�b�N�o�b�t�@�̐�
    sc.BufferDesc.Width = 1280U; // �o�b�N�o�b�t�@�T�C�Y�i���j
    sc.BufferDesc.Height = 720U; // �o�b�N�o�b�t�@�T�C�Y�i�c�j
    sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �t�H�[�}�b�g
    sc.BufferDesc.RefreshRate.Numerator = 60U; // ���t���b�V�����[�g���q
    sc.BufferDesc.RefreshRate.Denominator = 1U;// ���t���b�V�����[�g����
    sc.SampleDesc.Count = 1; // �}���`�T���v�����O���x��
    sc.SampleDesc.Quality = 0; // �}���`�T���v�����O�i��
    sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ���[�h�ؑ֗L��

    // Direct3D�C���^�[�t�F�C�X
    ID3D11Device* d3d_device = nullptr; // �`���̍쐬��
    ID3D11DeviceContext* device_context = nullptr; // �`�搧�䓙
    IDXGISwapChain* swap_chain = nullptr; // ���ۂ̕`�擙
    D3D_FEATURE_LEVEL    level;                    // �@�\���x��

    // �@�\���x�������v�Z
    int num_feature_level = sizeof(level_array) / sizeof(level_array[0]);

    // IDXGUSwapChain�AD3DDevice�AD3DDeviceContext���܂Ƃ߂č쐬����
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr,                    // DXGI�A�_�v�^�[
        D3D_DRIVER_TYPE_HARDWARE,   // �h���C�o�[�^�C�v(���_����������̂͒N��)
        // ���������s����悤�ł����D3D_DRIVER_TYPE_WARP���w��
        nullptr,                    // �\�t�g�E�F�A���X�^���C�U�[DLL�n���h��
        D3D11_CREATE_DEVICE_DEBUG,  // �t���O(Debug�@�\��L���ɂ���j
        //0, // ��Release����D3D11_CREATE_DEVICE_DEBUG���w�肹����0��n��
        level_array,                // �@�\���x��
        num_feature_level,          // ���x����
        D3D11_SDK_VERSION,          // ��ɂ��̒l
        &sc,                        // DXGI_SWAP_CHAIN_DESC�\���̃|�C���^
        &swap_chain,                // ����IDXGISwapChain�|�C���^�̕ϐ�
        &d3d_device,                // ����ID3D11Device�|�C���^�̕ϐ�
        &level,                     // �ݒ肳�ꂽ�@�\���x��
        &device_context)))       // ����ID3D11DeviceContext�|�C���^�̕ϐ�
    {
        // �G���[
        return 0;
    }

    // �o�b�N�o�b�t�@�̕`��^�[�Q�b�g���擾
    // SwapChain�������Ă���`��̈�i�L�����p�X�̂悤�Ȃ��́j�����o��
    ID3D11Texture2D* back_buffer = nullptr;
    if (FAILED(swap_chain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&back_buffer))))
    {
        // �G���[
        return 0;
    }

    // �`��^�[�Q�b�g�E�r���[�̍쐬
    // ���o�����L�����p�X���G��`���Ƃ���ł���Ƌ�����
    ID3D11RenderTargetView* render_target_view = nullptr;
    if (FAILED(d3d_device->CreateRenderTargetView(
        back_buffer,
        nullptr,
        &render_target_view)))
    {
        // �G���[
        return 0;
    }

    // �`��^�[�Q�b�g�r���[���o�̓}�l�[�W���[�̕`��^�[�Q�b�g�Ƃ��Đݒ�
    device_context->OMSetRenderTargets(
        1,                   // �`��^�[�Q�b�g��
        &render_target_view, // �^�[�Q�b�g�r���[�z��
        nullptr);           // �[�x�X�e���V���r���[

    // �o�b�N�o�b�t�@�̉��
    back_buffer->Release();

    // �r���[�|�[�g�̍쐬�Ɛݒ�
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = 1280.0F; // �`��̈�̕�
    vp.Height = 720.0F; // �`��̈�̍���
    vp.MinDepth = 0.0F; // �ŏ��[�x�l
    vp.MaxDepth = 1.0F; // �ő�[�x�l
    device_context->RSSetViewports(1, &vp);

    // COM�̏�����
    // ������s��Ȃ���DirectX Tool Kit�������Ă���֐��̂قƂ�ǂŎ��s����
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        // �G���[
        return 0;
    }

    // �J�����ݒ�
    // DirectXTK�͉E����W�n�Ȃ̂Ŏ�O���{
    Vector3 eye(0.0F, 1.0F, 3.0F);
    Vector3 target(0.0F, 0.0F, 0.0F);
    Vector3 up(0.0F, 1.0F, 0.0F);

    // �r���[�s��̍쐬
    Matrix view = Matrix::CreateLookAt(eye, target, up);

    // ��ʔ䗦���v�Z
    float ratio = 1280.0F / 720.0F;

    // �ˉe�s��̍쐬
    Matrix projection = Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(30.0F), // ��p
        ratio,                       // ��ʔ䗦
        1.0F,                        // NEAR�N���b�v��
        100.0F);                    // FAR�N���b�v��

    // �|���S���`��Ǘ��N���X�̐錾
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> primitive;
    primitive = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device_context);

    // �G�t�F�N�g�N���X�̐錾
    BasicEffect effect(d3d_device);

    // ���_�J���[��L���ɂ���(����������Ȃ��ƒ��_�J���[���o�͂���Ȃ�)
    effect.SetVertexColorEnabled(true);

    // �R���p�C���ς݃V�F�[�_�ւ̃|�C���^�ƃT�C�Y���擾
    const void* blob;
    size_t size;
    effect.GetVertexShaderBytecode(&blob, &size);

    // ���_���̓��C�A�E�g�̍쐬
    ID3D11InputLayout* layout;
    if (FAILED(d3d_device->CreateInputLayout(
        VertexPositionColor::InputElements,     // ���̓f�[�^�z��
        VertexPositionColor::InputElementCount, // ���̓f�[�^��
        blob,
        size,
        &layout)))
    {
        // �G���[
        return 0;
    }

    // ���_�f�[�^�̍쐬
    VertexPositionColor v0(Vector3(-0.5F, 0.5F, 0.5F), Vector4(1.0F, 0.0F, 0.0F, 1.0F));
    VertexPositionColor v1(Vector3(0.5F, 0.5F, 0.5F), Vector4(0.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v2(Vector3(-0.5F, -0.5F, 0.5F), Vector4(0.0F, 0.0F, 1.0F, 1.0F));
    VertexPositionColor v3(Vector3(0.5F, -0.5F, 0.5F), Vector4(1.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v4(Vector3(-0.5F, 0.5F, -0.5F), Vector4(1.0F, 0.0F, 0.0F, 1.0F));
    VertexPositionColor v5(Vector3(0.5F, 0.5F, -0.5F), Vector4(0.0F, 1.0F, 0.0F, 1.0F));
    VertexPositionColor v6(Vector3(-0.5F, -0.5F, -0.5F), Vector4(1.0F, 0.0F, 1.0F, 1.0F));
    VertexPositionColor v7(Vector3(0.5F, -0.5F, -0.5F), Vector4(1.0F, 1.0F, 0.0F, 1.0F));

    // �z��Ɋi�[
    VertexPositionColor p[] = { v0, v1, v2, v3, v4, v5, v6, v7 };

    // �C���f�b�N�X�l�̍쐬
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

    // �E�B���h�E�̕\��
    ShowWindow(hWnd, SW_SHOWNORMAL);

    // ���C�����[�v
    MSG msg = { 0 };

    // ���[�v����̂��߂̎����v��
    auto prev = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT)
    {
        // ���b�Z�[�W����
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // ���b�Z�[�W�����Ă�����E�B���h�E�v���V�[�W�����Ăяo��
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // ���݂̎������擾
            auto now = high_resolution_clock::now();

            // ���݂̎�������O��̎���������
            auto elapsed = duration_cast<microseconds>(now - prev);

            // ��16�~���b�ȏ�o�߂��Ă�����X�V���s��
            if (elapsed.count() > 16666LL)
            {
                // ���ݎ������ߋ��̎����Ƃ��ď㏑��
                prev = now;

                // ���[���h�s��̍쐬
                Matrix world = Matrix::Identity;// �P�ʍs��ŏ�����

                // �y�g��k���z���yX��]�z���yY��]�z���yZ��]�z���y�ړ��z�̏��Ԃɂ������킹�邱��

                // �g��k��
                world += Matrix::CreateScale(1.5f, 0.5f, 1.0f);

                // Vector3�\���̂������œn���Đݒ肷�邱�Ƃ��ł���
                //Vector3 scale(2.0f, 2.0f, 2.0f);
                //world += Matrix::CreateScale(scale);

                // Matrix scale = Matrix::CreateScale(1.5f, 1.5f, 1.5f);

                // ��]�s��
                world *= Matrix::CreateRotationX(XMConvertToRadians(30.0f));
                world *= Matrix::CreateRotationY(XMConvertToRadians(45.0f));
                world *= Matrix::CreateRotationZ(XMConvertToRadians(60.0f));

                // ���s�ړ��s��
                world *= Matrix::CreateTranslation(-0.5f, 0.2f, -0.5f);

                // ��ʃN���A
                float color[] = { 0.3F, 0.3F, 0.3F, 1.0F }; // RGBA��0.0�`1.0�̊ԂŎw��
                device_context->ClearRenderTargetView(render_target_view, color);

                // �s��̐ݒ�
                effect.SetWorld(world);
                //effect.SetView(view);
                effect.SetProjection(projection);

                // �G�t�F�N�g�̐ݒ��K�p
                effect.Apply(device_context);

                // ���_���̓��C�A�E�g�̐ݒ�
                device_context->IASetInputLayout(layout);

                // �|���S���`��J�n
                primitive->Begin();

                // �`��
                primitive->DrawIndexed(
                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, // �`����@
                    index,                                 // �C���f�b�N�X�z��擪�|�C���^
                    sizeof(index),                         // �C���f�b�N�X�z��̃T�C�Y
                    p,                                     // ���_�f�[�^�z��擪�|�C���^
                    8                                      // �`�悷�钸�_��
                );
                // �|���S���`��I��
                primitive->End();

                // ��ʍX�V
                swap_chain->Present(1, 0);
            }
        }
    }

    // �C���^�[�t�F�[�X�̉��
    // �m�ۂ������̋t�ɉ�����Ă���
    layout->Release();
    render_target_view->Release();
    swap_chain->Release();
    device_context->ClearState();
    device_context->Release();
    d3d_device->Release();

    return 0;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(
    HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // ���b�Z�[�W�ɂ�蕪��
    switch (Msg)
    {
    case WM_KEYDOWN: // �L�[�������ꂽ
        switch (wParam)
        {
        case VK_ESCAPE: // ESC�L�[�������ꂽ
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        break;

    case WM_DESTROY: // �A�v���P�[�V����������ꂽ
        PostQuitMessage(0); // WM_QUIT���b�Z�[�W�𔭐�������֐�
        break;
    }

    // �E�B���h�E�v���V�[�W�����ŏ������Ȃ��������b�Z�[�W�̃f�t�H���g�̓����
    // �����I�ɍs�Ȃ��Ă����̗�H
    return DefWindowProc(hWnd, Msg, wParam, lParam);
}