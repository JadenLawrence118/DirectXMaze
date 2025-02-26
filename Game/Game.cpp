//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <time.h>
#include <iostream>

#include <iostream>

//Scarle Headers
#include "GameData.h"
#include "GameState.h"
#include "DrawData.h"
#include "DrawData2D.h"
#include "ObjectList.h"

#include "CMOGO.h"
#include <DirectXCollision.h>
#include "Collision.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(1280),
    m_outputHeight(720),
    m_featureLevel(D3D_FEATURE_LEVEL_11_0)
{

}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND _window, int _width, int _height)
{
    m_window = _window;
    m_outputWidth = std::max(_width, 1);
    m_outputHeight = std::max(_height, 1);

    CreateDevice();

    CreateResources();

    // setting FPS to 60
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    

    //seed the random number generator
    srand((UINT)time(NULL));

    //set up keyboard and mouse system
    //documentation here: https://github.com/microsoft/DirectXTK/wiki/Mouse-and-keyboard-input
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(_window);
    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    //Hide the mouse pointer
    ShowCursor(false);

    //create GameData struct and populate its pointers
    m_GD = new GameData;
    m_GD->m_GS = GS_PLAY_MAIN_CAM;

    //set up systems for 2D rendering
    m_DD2D = new DrawData2D();
    m_DD2D->m_Sprites.reset(new SpriteBatch(m_d3dContext.Get()));
    m_DD2D->m_Font.reset(new SpriteFont(m_d3dDevice.Get(), L"..\\Assets\\italic.spritefont"));
    m_states = new CommonStates(m_d3dDevice.Get());

    //set up DirectXTK Effects system
    m_fxFactory = new EffectFactory(m_d3dDevice.Get());
    //Tell the fxFactory to look to the correct build directory to pull stuff in from
    ((EffectFactory*)m_fxFactory)->SetDirectory(L"..\\Assets");
    //init render system for VBGOs
    VBGO::Init(m_d3dDevice.Get());

    //set audio system
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif
    m_audioEngine = std::make_unique<AudioEngine>(eflags);

    //create a set of dummy things to show off the engine

    //create a base light
    m_light = new Light(Vector3(0.0f, 100.0f, 160.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.4f, 0.1f, 0.1f, 1.0f));
    m_GameObjects.push_back(m_light);

    //find how big my window is to correctly calculate my aspect ratio
    AR = (float)_width / (float)_height;
    
    // camera
    m_TPScam = new TPSCamera(0.25f * XM_PI, AR, 3.5f, 10000.0f, pPlayer, Vector3::UnitY, Vector3(0.0f, 0.0f, 0.01f));
    m_GameObjects.push_back(m_TPScam);

    //create DrawData struct and populate its pointers
    m_DD = new DrawData;
    m_DD->m_pd3dImmediateContext = nullptr;
    m_DD->m_states = m_states;
    m_DD->m_cam = m_cam;
    m_DD->m_light = m_light;

    orbEffect = new TestSound(m_audioEngine.get(), "sonic_ring");
    m_Sounds.push_back(orbEffect);


    // UI

    // Menu text
    titleText = new TextGO2D("Magic Maze");
    titleText->SetPos(Vector2(m_outputWidth / 2 - 250, m_outputHeight / 10));
    titleText->SetColour(Color((float*)&Colors::Yellow));
    titleText->SetScale(2);
    m_GameObjects2D_Menu.push_back(titleText);

    playText = new TextGO2D("> Play <");
    playText->SetPos(Vector2(m_outputWidth / 2 - 100, m_outputHeight / 2));
    playText->SetColour(Color((float*)&Colors::Red));
    playText->SetScale(1);
    m_GameObjects2D_Menu.push_back(playText);

    instrText = new TextGO2D("Instructions");
    instrText->SetPos(Vector2(m_outputWidth / 2 - 125, m_outputHeight / 4 * 3));
    instrText->SetColour(Color((float*)&Colors::Yellow));
    instrText->SetScale(1);
    m_GameObjects2D_Menu.push_back(instrText);

    // Instructions text
    backText = new TextGO2D("Tab to go back");
    backText->SetPos(Vector2(50, m_outputHeight - 100));
    backText->SetColour(Color((float*)&Colors::Yellow));
    backText->SetScale(1);
    m_GameObjects2D_Instr.push_back(backText);

}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& _timer)
{
    float elapsedTime = float(_timer.GetElapsedSeconds());
    m_GD->m_dt = elapsedTime;

    //this will update the audio engine but give us chance to do somehting else if that isn't working
    if (!m_audioEngine->Update())
    {
        if (m_audioEngine->IsCriticalError())
        {
            // We lost the audio device!
        }
    }
    else
    {
        //update sounds playing
        for (list<Sound*>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); it++)
        {
            (*it)->Tick(m_GD);
        }
    }

    ReadInput();

    //update all objects
    switch (current)
    {
    case MENU:
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Menu.begin(); it != m_GameObjects2D_Menu.end(); it++)
        {
            (*it)->Tick(m_GD);
        }

        titleText->SetText("Magic Maze");
        titleText->SetPos(Vector2(m_outputWidth / 2 - 250, m_outputHeight / 10));

        if (menuSelection == 0)
        {
            playText->SetText("> Play <");
            playText->SetPos(Vector2(m_outputWidth / 2 - 100, m_outputHeight / 2));
            playText->SetColour(Color((float*)&Colors::Red));

            instrText->SetText("Instructions");
            instrText->SetPos(Vector2(m_outputWidth / 2 - 125, m_outputHeight / 4 * 3));
            instrText->SetColour(Color((float*)&Colors::Yellow));
        }
        else if (menuSelection == 1)
        {
            playText->SetText("Play");
            playText->SetPos(Vector2(m_outputWidth / 2 - 75, m_outputHeight / 2));
            playText->SetColour(Color((float*)&Colors::Yellow));

            instrText->SetText("> Instructions <");
            instrText->SetPos(Vector2(m_outputWidth / 2 - 150, m_outputHeight / 4 * 3));
            instrText->SetColour(Color((float*)&Colors::Red));
        }
        break;

    case INSTRUCTIONS:
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Instr.begin(); it != m_GameObjects2D_Instr.end(); it++)
        {
            (*it)->Tick(m_GD);
        }

        titleText->SetText("Instructions");
        titleText->SetPos(Vector2(m_outputWidth / 2 - 250, m_outputHeight / 10));

        playText->SetText("Find the magic orbs to open the doors!\nGet through the portal to win!\nStepping on spikes will kill you!");
        playText->SetPos(Vector2(150, m_outputHeight / 3));
        playText->SetColour(Color((float*)&Colors::Yellow));

        instrText->SetText("Move with WASD\nSprint with LShift");
        instrText->SetPos(Vector2(150, m_outputHeight / 5 * 3));
        instrText->SetColour(Color((float*)&Colors::Yellow));

        break;

    case MAIN:

        for (list<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        {
            (*it)->Tick(m_GD);
        }
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Main.begin(); it != m_GameObjects2D_Main.end(); it++)
        {
            (*it)->Tick(m_GD);
        }

        // Moving Walls
        for (int i = 0; i < noOfCollectables; i++)
        {
            if (walls[i]->moving)
            {
                walls[i]->Move(0.5, 50);
            }
        }
        

        CheckCollision();
        CheckTrigger();
        Gravity();
        break;

    case WIN:

        titleText->SetText("You Win!");
        titleText->SetPos(Vector2(m_outputWidth / 2 - 250, m_outputHeight / 10));

        playText->SetText("You found the portal and managed to escape!");
        playText->SetPos(Vector2(150, m_outputHeight / 3));
        playText->SetColour(Color((float*)&Colors::Yellow));

        instrText->SetText("Great job!");
        instrText->SetPos(Vector2(150, m_outputHeight / 5 * 3));
        instrText->SetColour(Color((float*)&Colors::Yellow));

        break;

    case LOSE:

        titleText->SetText("You Lose");
        titleText->SetPos(Vector2(m_outputWidth / 2 - 250, m_outputHeight / 10));

        playText->SetText("Spikes are very deadly!");
        playText->SetPos(Vector2(150, m_outputHeight / 3));
        playText->SetColour(Color((float*)&Colors::Yellow));

        instrText->SetText("Better luck next time");
        instrText->SetPos(Vector2(150, m_outputHeight / 5 * 3));
        instrText->SetColour(Color((float*)&Colors::Yellow));

        break;
    }
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();
    
    //set immediate context of the graphics device
    m_DD->m_pd3dImmediateContext = m_d3dContext.Get();

    //set which camera to be used
    m_DD->m_cam = m_TPScam;
    

    //update the constant buffer for the rendering of VBGOs
    VBGO::UpdateConstantBuffer(m_DD);

    m_DD2D->m_Sprites->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

    switch (current)
    {
    case MENU:
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Menu.begin(); it != m_GameObjects2D_Menu.end(); it++)
        {
            (*it)->Draw(m_DD2D);
        }
        break;

    case INSTRUCTIONS:
    case WIN:
    case LOSE:
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Menu.begin(); it != m_GameObjects2D_Menu.end(); it++)
        {
            (*it)->Draw(m_DD2D);
        }
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Instr.begin(); it != m_GameObjects2D_Instr.end(); it++)
        {
            (*it)->Draw(m_DD2D);
        }
        break;

    case MAIN:
        //Draw 3D Game Objects
        for (list<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
        {
            if ((*it)->IsActive())
            {
                (*it)->Draw(m_DD);
            }
        }
        for (list<GameObject2D*>::iterator it = m_GameObjects2D_Main.begin(); it != m_GameObjects2D_Main.end(); it++)
        {
            (*it)->Draw(m_DD2D);
        }
        break;
    }

    m_DD2D->m_Sprites->End();
    //drawing text screws up the Depth Stencil State, this puts it back again!
    m_d3dContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
    mouseLocked = true;
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
    mouseLocked = false;
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
    mouseLocked = false;
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
    mouseLocked = false;
}

void Game::OnWindowSizeChanged(int _width, int _height)
{
    m_outputWidth = std::max(_width, 1);
    m_outputHeight = std::max(_height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& _width, int& _height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    _width = 1280;
    _height = 720;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    //creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    //something missing on the machines in 2Q28
    //this should work!
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}

void Game::GenerateMaze()
{
    // Maze
    Terrain* mazeWall1 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-20.0f, -60.0f, 20.0f), 0.0f, 1.57f, 0.0f, Vector3(350.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall1);
    m_ColliderObjects.push_back(mazeWall1);

    Terrain* mazeWall2 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(20.0f, -60.0f, 20.0f), 0.0f, 1.57f, 0.0f, Vector3(350.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall2);
    m_ColliderObjects.push_back(mazeWall2);

    Terrain* mazeWall3 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(5.0f, -60.0f, -50.0f), 0.0f, 0.0f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall3);
    m_ColliderObjects.push_back(mazeWall3);

    Terrain* mazeWall4 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(50.0f, -60.0f, 101.9f), 0.0f, 0.0f, 0.0f, Vector3(90.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall4);
    m_ColliderObjects.push_back(mazeWall4);

    Terrain* mazeWall5 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(65.4f, -60.0f, 55.0f), 0.0f, 1.57f, 0.0f, Vector3(200.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall5);
    m_ColliderObjects.push_back(mazeWall5);

    Terrain* mazeWall6 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(105.0f, -60.0f, 4.0f), 0.0f, 0.0f, 0.0f, Vector3(160.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall6);
    m_ColliderObjects.push_back(mazeWall6);

    Terrain* mazeWall7 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(105.4f, -60.0f, 100.0f), 0.0f, 1.57f, 0.0f, Vector3(200.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall7);
    m_ColliderObjects.push_back(mazeWall7);

    Terrain* mazeWall8 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(145.4f, -60.0f, 77.0f), 0.0f, 1.57f, 0.0f, Vector3(285.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall8);
    m_ColliderObjects.push_back(mazeWall8);

    Terrain* mazeWall9 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-85.0f, -60.0f, 101.9f), 0.0f, 0.0f, 0.0f, Vector3(280.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall9);
    m_ColliderObjects.push_back(mazeWall9);

    Terrain* mazeWall10 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-85.0f, -60.0f, 141.9f), 0.0f, 0.0f, 0.0f, Vector3(280.0f, 70.0f, 250.0f));
    m_GameObjects.push_back(mazeWall10);
    m_ColliderObjects.push_back(mazeWall10);

    Terrain* mazeWall11 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-158.0f, -60.0f, 20.0f), 0.0f, 1.57f, 0.0f, Vector3(350.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall11);
    m_ColliderObjects.push_back(mazeWall11);

    Terrain* mazeWall12 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-198.0f, -60.0f, 180.0f), 0.0f, 1.57f, 0.0f, Vector3(1000.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall12);
    m_ColliderObjects.push_back(mazeWall12);

    Terrain* mazeWall13 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-175.0f, -60.0f, -50.0f), 0.0f, 0.0f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall13);
    m_ColliderObjects.push_back(mazeWall13);

    Terrain* mazeWall14 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(92.0f, -60.0f, 143.5f), 0.0f, 0.0f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall14);
    m_ColliderObjects.push_back(mazeWall14);

    Terrain* mazeWall15 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(71.5f, -60.0f, 164.0f), 0.0f, 1.57f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall15);
    m_ColliderObjects.push_back(mazeWall15);

    Terrain* mazeWall16 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(50.0f, -60.0f, 177.0f), 0.0f, 0.0f, 0.0f, Vector3(90.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall16);
    m_ColliderObjects.push_back(mazeWall16);

    Terrain* mazeWall17 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(20.0f, -60.0f, 244.85f), 0.0f, 1.57f, 0.0f, Vector3(265.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall17);
    m_ColliderObjects.push_back(mazeWall17);

    Terrain* mazeWall18 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-68.0f, -60.0f, 305.0f), 0.0f, 0.0f, 0.0f, Vector3(350.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall18);
    m_ColliderObjects.push_back(mazeWall18);

    Terrain* mazeWall19 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-160.0f, -60.0f, 345.0f), 0.0f, 0.0f, 0.0f, Vector3(350.0f, 70.0f, 350.0f));
    m_GameObjects.push_back(mazeWall19);
    m_ColliderObjects.push_back(mazeWall19);

    Terrain* mazeWall20 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(22.0f, -60.0f, 345.0f), 0.0f, 0.0f, 0.0f, Vector3(223.0f, 70.0f, 110.0f));
    m_GameObjects.push_back(mazeWall20);
    m_ColliderObjects.push_back(mazeWall20);

    Terrain* mazeWall21 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(55.0f, -60.0f, 437.0f), 0.0f, 0.0f, 0.0f, Vector3(350.0f, 70.0f, 150.0f));
    m_GameObjects.push_back(mazeWall21);
    m_ColliderObjects.push_back(mazeWall21);

    Terrain* mazeWall22 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(-55.0f, -60.0f, 500.0f), 0.0f, 0.0f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall22);
    m_ColliderObjects.push_back(mazeWall22);

    Terrain* mazeWall23 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(71.4f, -60.0f, 325.0f), 0.0f, 1.57f, 0.0f, Vector3(80.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall23);
    m_ColliderObjects.push_back(mazeWall23);

    Terrain* mazeWall24 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(111.4f, -60.0f, 372.0f), 0.0f, 1.57f, 0.0f, Vector3(260.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall24);
    m_ColliderObjects.push_back(mazeWall24);

    Terrain* mazeWall25 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(220.0f, -60.0f, 305.5f), 0.0f, 0.0f, 0.0f, Vector3(400.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall25);
    m_ColliderObjects.push_back(mazeWall25);

    Terrain* mazeWall26 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(262.0f, -60.0f, 142.25f), 0.0f, 0.0f, 0.0f, Vector3(450.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall26);
    m_ColliderObjects.push_back(mazeWall26);

    Terrain* mazeWall27 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(213.0f, -60.0f, 182.25f), 0.0f, 0.0f, 0.0f, Vector3(260.0f, 70.0f, 170.0f));
    m_GameObjects.push_back(mazeWall27);
    m_ColliderObjects.push_back(mazeWall27);

    Terrain* mazeWall28 = new Terrain("blockPurple", m_d3dDevice.Get(), m_fxFactory, Vector3(320.0f, -60.0f, 230.0f), 0.0f, 1.57f, 0.0f, Vector3(330.0f, 70.0f, 15.0f));
    m_GameObjects.push_back(mazeWall28);
    m_ColliderObjects.push_back(mazeWall28);
}

void Game::ReadInput()
{
    m_GD->m_KBS = m_keyboard->GetState();
    m_GD->m_KBS_tracker.Update(m_GD->m_KBS);
    //quit game on hitting escape
    if (m_GD->m_KBS.Escape)
    {
        ExitGame();
    }


    // unlock mouse if left control key is pressed
    if (controlUp)
    {
        if (m_GD->m_KBS.IsKeyDown(m_keyboard.get()->LeftControl))
        {
            mouseLocked = !mouseLocked;
            std::cout << (mouseLocked);
            controlUp = false;
        }
    }

    if (m_GD->m_KBS.IsKeyUp(m_keyboard.get()->LeftControl))
    {
        controlUp = true;
    }

    m_GD->m_MS = m_mouse->GetState();

    //lock the cursor to the centre of the window
    if (mouseLocked)
    {
        RECT window;
        GetWindowRect(m_window, &window);
        SetCursorPos((window.left + window.right) >> 1, (window.bottom + window.top) >> 1);
    }
    switch (current)
    {
    case MENU:
        if (m_GD->m_KBS.Down)
        {
            if (menuSelection < 1)
            {
                menuSelection++;
            }
        }
        if (m_GD->m_KBS.Up)
        {
            if (menuSelection > 0)
            {
                menuSelection--;
            }
        }
        if (m_GD->m_KBS.Enter)
        {
            if (menuSelection == 0)
            {
                Reset();
                current = MAIN;
            }
            if (menuSelection == 1)
            {
                current = INSTRUCTIONS;
            }
        }
        break;
    case INSTRUCTIONS:
    case LOSE:
    case WIN:
        if (m_GD->m_KBS.Tab)
        {
            current = MENU;
        }
    }
}

void Game::CheckCollision()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_ColliderObjects.size(); j++)
    {
        if (m_PhysicsObjects[i]->IsActive() && m_ColliderObjects[j]->IsActive())
        {
            if (m_PhysicsObjects[i]->Intersects(*m_ColliderObjects[j])) //std::cout << "Collision Detected!" << std::endl;
            {
                XMFLOAT3 eject_vect = Collision::ejectionCMOGO(*m_PhysicsObjects[i], *m_ColliderObjects[j]);
                auto pos = m_PhysicsObjects[i]->GetPos();
                m_PhysicsObjects[i]->SetPos(pos - eject_vect);
            }
        }
    }
}

void Game::CheckTrigger()
{
    for (int i = 0; i < m_PhysicsObjects.size(); i++) for (int j = 0; j < m_TriggerObjects.size(); j++)
    {
        if (m_PhysicsObjects[i]->IsActive() && m_TriggerObjects[j]->IsActive())
        {
            if (m_PhysicsObjects[i]->Intersects(*m_TriggerObjects[j]))
            {
                std::cout << "Collision Detected!" << std::endl;
                for (int k = 0; k < noOfCollectables; k++)
                {
                    if (m_PhysicsObjects[i] == pPlayer && m_TriggerObjects[j] == keys[k])
                    {
                        orbEffect->Play();
                        keys[k]->SetActive(false);
                        walls[k]->moving = true;
                        collected++;
                        if (collected == 1)
                        {
                            collectBlue->SetText("Blue: Collected");
                        }
                        else if (collected == 2)
                        {
                            collectRed->SetText("Red: Collected");
                        }
                        else if (collected == 3)
                        {
                            collectYellow->SetText("Yellow: Collected");
                        }
                    }
                }
                for (int k = 0; k < noOfSpikes; k++)
                {
                    if (m_PhysicsObjects[i] == pPlayer && m_TriggerObjects[j] == spikes[k])
                    {
                        current = LOSE;
                    }
                }
                if (m_PhysicsObjects[i] == pPlayer && m_TriggerObjects[j] == portal)
                {
                    current = WIN;
                }
            }
        }
    }
}

void Game::Gravity()
{
    for (int i = 0; i < m_GravityObjects.size(); i++)
    {
        auto pos = m_GravityObjects[i]->GetPos();
        m_GravityObjects[i]->SetPos(Vector3(pos.x, pos.y - gravityScale, pos.z));
    }
}

void Game::Reset()
{
    collected = 0;

    m_ColliderObjects.clear();
    m_TriggerObjects.clear();
    m_PhysicsObjects.clear();
    m_GravityObjects.clear();

    m_GameObjects.clear();
    m_GameObjects2D_Main.clear();

    // Floor
    Terrain* ground = new Terrain("ground", m_d3dDevice.Get(), m_fxFactory, Vector3(0.0f, -73.0f, 0.0f), 0.0f, 0.0f, 0.0f, Vector3(20000, 100, 20000));
    m_GameObjects.push_back(ground);
    m_ColliderObjects.push_back(ground);

    GenerateMaze();

    // Collectables
    for (int i = 0; i < noOfCollectables; i++)
    {
        keys[i] = new Collectable(keys[i]->GetModel(i), m_d3dDevice.Get(), m_fxFactory, keys[i]->InitialPosition(i), keys[i]->InitialScale(i));
        m_GameObjects.push_back(keys[i]);
        m_TriggerObjects.push_back(keys[i]);
    }

    // Moving Walls
    for (int i = 0; i < noOfCollectables; i++)
    {
        walls[i] = new Wall(walls[i]->GetModel(i), m_d3dDevice.Get(), m_fxFactory, walls[i]->InitialPosition(i), walls[i]->InitialScale(i), walls[i]->InitialRotation(i));
        m_GameObjects.push_back(walls[i]);
        m_ColliderObjects.push_back(walls[i]);
    }

    // Spikes
    for (int i = 0; i < noOfSpikes; i++)
    {
        spikes[i] = new DeathPlane("spikes", m_d3dDevice.Get(), m_fxFactory, spikes[i]->InitialPosition(i), Vector3(35, 15, 35), spikes[i]->InitialRotation(i));
        m_GameObjects.push_back(spikes[i]);
        m_TriggerObjects.push_back(spikes[i]);
    }

    // Win portal
    portal = new EndPortal("portal", m_d3dDevice.Get(), m_fxFactory, Vector3(-52, -25, 475), Vector3(125.0f, 10.0f, 125.0f));
    m_GameObjects.push_back(portal);
    m_TriggerObjects.push_back(portal);

    // Player
    pPlayer = new Player("sphere", m_d3dDevice.Get(), m_fxFactory);
    pPlayer->SetScale(40, 100, 40);
    m_GameObjects.push_back(pPlayer);
    m_PhysicsObjects.push_back(pPlayer);
    m_GravityObjects.push_back(pPlayer);

    // camera
    m_TPScam = new TPSCamera(0.25f * XM_PI, AR, 3.5f, 10000.0f, pPlayer, Vector3::UnitY, Vector3(0.0f, 0.0f, 0.01f));
    m_GameObjects.push_back(m_TPScam);


    // UI

    // Collectable counter text
    collectBlue = new TextGO2D("Blue: Not Collected");
    collectBlue->SetPos(Vector2(10, 10));
    collectBlue->SetColour(Color((float*)&Colors::Yellow));
    collectBlue->SetScale(0.75);
    m_GameObjects2D_Main.push_back(collectBlue);

    collectRed = new TextGO2D("Red: Not Collected");
    collectRed->SetPos(Vector2(10, 50));
    collectRed->SetColour(Color((float*)&Colors::Yellow));
    collectRed->SetScale(0.75);
    m_GameObjects2D_Main.push_back(collectRed);

    collectYellow = new TextGO2D("Yellow: Not Collected");
    collectYellow->SetPos(Vector2(10, 90));
    collectYellow->SetColour(Color((float*)&Colors::Yellow));
    collectYellow->SetScale(0.75);
    m_GameObjects2D_Main.push_back(collectYellow);

    current = MENU;
}
