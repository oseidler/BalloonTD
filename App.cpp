#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


App* g_theApp = nullptr;

Renderer*	 g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
Window*		 g_theWindow = nullptr;

Game* g_theGame = nullptr;


//public game flow functions
void App::Startup()
{
	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);
	
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "DFS2: Bloons Tower Defense";
	windowConfig.m_clientAspect = 1.7777778f;
	windowConfig.m_isFullscreen = true;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_theRenderer;
	devConsoleConfig.m_camera = &m_devConsoleCamera;
	g_theDevConsole = new DevConsole(devConsoleConfig);
	
	AudioSystemConfig audioSystemConfig;
	g_theAudio = new AudioSystem(audioSystemConfig);
	
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup(debugRenderConfig);

	g_theGame = new Game();
	g_theGame->Startup();

	SubscribeEventCallbackFunction("quit", Event_Quit);

	m_devConsoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));

	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Controls: ");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Left Click: Buy Tower, Place Bought Tower, Select Placed Tower, Select Buttons");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Right Click: Cancel Buying Tower");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " ~: Open Dev Console");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Escape: Exit to Attract, Exit Game");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MAJOR, "Debug Controls: ");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F1: Toggle Spline Editor");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Left Click: Drag Spline Point (When spline editor is open)");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Right Click: Drag Entire Curve (When spline editor is open)");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Left Shift + Left Click: Hold to Snap Spline Points");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " ,: Remove Latest Spline");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " .: Add Horizontal Spline");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " ;: Add Vertical Spline");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " 1-6: Debug Spawn Bloons");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Left Shift + 1-6: Hold to Spawn Bloons");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F2: Draw All Tower Ranges");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " F8: Restart Game");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " T: Slow Speed");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " Y: Fast Speed");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " T+Y: Super Fast Speed");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " P: Pause Time");
	g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, " O: Progress 1 Frame");
}


void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}


void App::Shutdown()
{
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	DebugRenderSystemShutdown();

	g_theAudio->Shutdown();
	delete g_theAudio;
	g_theAudio = nullptr;

	g_theRenderer->Shutdown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theWindow->Shutdown();
	delete g_theWindow;
	g_theWindow = nullptr;

	g_theInput->Shutdown();
	delete g_theInput;
	g_theInput = nullptr;

	g_theDevConsole->Shutdown();
	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	g_theEventSystem->Shutdown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


void App::RunFrame()
{
	//tick the system clock
	Clock::TickSystemClock();

	//run through the four parts of the frame
	BeginFrame();
	Update();
	Render();
	EndFrame();
}


//
//public app utilities
//
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return m_isQuitting;
}


//
//static app utilities
//
bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);

	if (g_theApp != nullptr)
	{
		g_theApp->HandleQuitRequested();
	}

	return true;
}


//
//private game flow functions
//
void App::BeginFrame()
{
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}


void App::Update()
{
	//quit or leave attract mode if q is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		/*if (g_theGame->m_isAttractMode)
		{*/
			HandleQuitRequested();
			/*}
			else
			{
				RestartGame();
			}*/
	}

	//recreate game if f8 is pressed
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		RestartGame();
	}

	//update the game
	g_theGame->Update();

	//go back to the start if the game finishes
	if (g_theGame->m_isFinished)
	{
		RestartGame();
	}
}


void App::Render() const
{	
	g_theGame->Render();

	//render dev console separately from and after rest of game
	g_theRenderer->BeginCamera(m_devConsoleCamera);
	g_theDevConsole->Render(AABB2(0.0f, 0.0f, SCREEN_CAMERA_SIZE_X * 0.9f, SCREEN_CAMERA_SIZE_Y * 0.9f));
	g_theRenderer->EndCamera(m_devConsoleCamera);
}


void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theDevConsole->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theAudio->EndFrame();

	DebugRenderEndFrame();
}


//
//private app utilities
//
void App::RestartGame()
{
	//delete old game
	g_theGame->Shutdown();
	delete g_theGame;
	g_theGame = nullptr;

	DebugRenderClear();

	//initialize new game
	g_theGame = new Game();
	g_theGame->Startup();
}
