#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/BloonDefinition.hpp"
#include "Game/TowerDefinition.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/RoundDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Bloon.hpp"
#include "Game/Projectile.hpp"
#include "Game/Tower.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/BufferUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


//game flow functions
void Game::Startup()
{
	LoadDefinitions();
	
	m_gameMusic = g_theAudio->CreateOrGetSound("Data/Audio/Music.mp3");
	m_frozenHitSound = g_theAudio->CreateOrGetSound("Data/Audio/Frozen.mp3");

	m_menuFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	
	//set camera bounds
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y));
	g_theRenderer->SetModelConstants();

	AddButtonsForShop();
	m_sellButton = Button(g_theRenderer, g_theInput, m_sellButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "[sell text]", AABB2(0.05f, 0.15f, 0.95f, 0.85f),
		Rgba8(200, 0, 50), Rgba8(), Rgba8(255, 25, 25), Rgba8());
	EventArgs blankArgs;
	m_sellButton.AddEvent("SellTower", blankArgs);
	m_upgrade1Button = Button(g_theRenderer, g_theInput, m_upgrade1ButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "[upgrade 1 text]", AABB2(0.05f, 0.15f, 0.95f, 0.85f),
		Rgba8(0, 0, 0), Rgba8(), Rgba8(0, 0 ,0), Rgba8());
	m_upgrade1Button.AddEvent("BuyUpgrade1", blankArgs);
	m_upgrade2Button = Button(g_theRenderer, g_theInput, m_upgrade2ButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "[upgrade 2 text]", AABB2(0.05f, 0.15f, 0.95f, 0.85f),
		Rgba8(0, 0, 0), Rgba8(), Rgba8(0, 0, 0), Rgba8());
	m_upgrade2Button.AddEvent("BuyUpgrade2", blankArgs);
	m_targetModeButton = Button(g_theRenderer, g_theInput, m_targetModeButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "Targeting Mode: none", AABB2(0.05f, 0.15f, 0.95f, 0.85f),
		Rgba8(0, 0, 255), Rgba8(), Rgba8(50, 127, 255), Rgba8());
	m_targetModeButton.AddEvent("ToggleTargetingMode", blankArgs);
	m_startButton = Button(g_theRenderer, g_theInput, m_startButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "Start Round", AABB2(0.05f, 0.15f, 0.95f, 0.85f),
		Rgba8(25, 225, 25), Rgba8(), Rgba8(50, 255, 50), Rgba8());
	m_startButton.AddEvent("StartRound", blankArgs);
	m_map1Button = Button(g_theRenderer, g_theInput, m_map1ButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "1", AABB2(0.05f, 0.15f, 0.95f, 0.85f), Rgba8(150, 150, 150),
		Rgba8(), Rgba8(190, 190, 190), Rgba8(0, 0, 0));
	m_map1Button.AddEvent("SelectMap", blankArgs);
	m_map2Button = Button(g_theRenderer, g_theInput, m_map2ButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "2", AABB2(0.05f, 0.15f, 0.95f, 0.85f), Rgba8(150, 150, 150),
		Rgba8(), Rgba8(190, 190, 190), Rgba8(0, 0, 0));
	m_map2Button.AddEvent("SelectMap", blankArgs);
	m_map3Button = Button(g_theRenderer, g_theInput, m_map3ButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), nullptr, "3", AABB2(0.05f, 0.15f, 0.95f, 0.85f), Rgba8(150, 150, 150),
		Rgba8(), Rgba8(190, 190, 190), Rgba8(0, 0, 0));
	m_map3Button.AddEvent("SelectMap", blankArgs);

	//subscribe to events
	SubscribeEventCallbackFunction("WriteMap", Event_WriteMap);
	SubscribeEventCallbackFunction("BuyTower", Event_BuyTower);
	SubscribeEventCallbackFunction("SellTower", Event_SellTower);
	SubscribeEventCallbackFunction("BuyUpgrade1", Event_BuyUpgrade1);
	SubscribeEventCallbackFunction("BuyUpgrade2", Event_BuyUpgrade2);
	SubscribeEventCallbackFunction("StartRound", Event_StartRound);
	SubscribeEventCallbackFunction("ToggleTargetingMode", Event_ToggleTargetingMode);
	SubscribeEventCallbackFunction("SelectMap", Event_SelectMap);

	//EnterAttractMode();
	EnterGameplay();
}


void Game::Update()
{
	//if in attract mode, just update that and don't bother with anything else
	/*if (m_isAttractMode)
	{
		UpdateAttract();
		return;
	}*/

	Vec2 normalizedMousePos = g_theWindow->GetCursorNormalizedPos();
	AABB2 bounds = AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight());
	Vec2 orthoMousePos = bounds.GetPointAtUV(normalizedMousePos);
	
	//check if p is pressed to toggle pause
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock.TogglePause();
	}
	//check if t is currently being held to turn slow-mo on
	if (g_theInput->IsKeyDown('T') && !g_theInput->IsKeyDown('Y'))
	{
		m_gameClock.SetTimeScale(0.1f);
	}
	else if (g_theInput->IsKeyDown('Y') && !g_theInput->IsKeyDown('T'))
	{
		m_gameClock.SetTimeScale(3.0f);
	}
	else if (g_theInput->IsKeyDown('T') && g_theInput->IsKeyDown('Y'))
	{
		m_gameClock.SetTimeScale(10.0f);
	}
	else
	{
		m_gameClock.SetTimeScale(1.0f);
	}
	//check if o is pressed and do single step if so
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock.StepSingleFrame();
	}

	//if game over, increment reset timer, then reset game once reset timer is done
	if (m_resetTimer > 0.0f)
	{
		m_resetTimer -= m_gameClock.GetDeltaSeconds();
		/*std::string resetStr = Stringf("Reset timer: %.2f", m_resetTimer);
		DebugAddMessage(resetStr, 0.0f);*/
		if (m_resetTimer <= 0.0f)
		{
			m_isFinished = true;
		}
	}
	else
	{
		//select tower with left click
		if (g_theInput->WasKeyJustPressed(KEYCODE_LMB))
		{
			if (!IsPointInsideAABB2D(orthoMousePos, m_UIBaseBounds))
			{
				if (m_heldTower == nullptr)
				{
					bool towerSelected = false;
					for (int towerIndex = 0; towerIndex < m_currentMap->m_towers.size(); towerIndex++)
					{
						Tower*& tower = m_currentMap->m_towers[towerIndex];

						if (tower != nullptr && IsPointInsideDisc2D(orthoMousePos, tower->m_position, tower->m_definition->m_size))
						{
							m_selectedTower = tower;
							towerSelected = true;
							break;
						}
					}

					if (!towerSelected)
					{
						m_selectedTower = nullptr;
					}
				}
				else
				{
					PlaceHeldTower();
				}
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_RMB))
		{
			delete m_heldTower;
			m_heldTower = nullptr;
		}

		//debug controls
		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			m_isSplineEditing = !m_isSplineEditing;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
		{
			m_showAllTowerRanges = !m_showAllTowerRanges;
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			g_theRenderer->SetSamplerMode(SamplerMode::BILINEAR_CLAMP);
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
		{
			g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		}

		if (g_theInput->WasKeyJustPressed('K'))
		{
			for (int bloonIndex = 0; bloonIndex < m_currentMap->m_bloons.size(); bloonIndex++)
			{
				if (m_currentMap->m_bloons[bloonIndex] != nullptr)
				{
					m_numMoney += m_currentMap->m_bloons[bloonIndex]->m_definition->m_RBE;
					delete m_currentMap->m_bloons[bloonIndex];
					m_currentMap->m_bloons[bloonIndex] = nullptr;
				}
			}

			EndRound();
		}

		if (g_theInput->WasKeyJustPressed('M'))
		{
			m_numMoney += 10000;
		}
		//bloon spawning debug controls
		if (g_theInput->WasKeyJustPressed('1') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('1')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Red"));
		}
		if (g_theInput->WasKeyJustPressed('2') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('2')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Blue"));
		}
		if (g_theInput->WasKeyJustPressed('3') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('3')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Green"));
		}
		if (g_theInput->WasKeyJustPressed('4') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('4')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Yellow"));
		}
		if (g_theInput->WasKeyJustPressed('5') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('5')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Black"));
		}
		if (g_theInput->WasKeyJustPressed('6') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('6')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("White"));
		}
		if (g_theInput->WasKeyJustPressed('7') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('7')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Rainbow"));
		}
		if (g_theInput->WasKeyJustPressed('8') || (g_theInput->IsKeyDown(KEYCODE_SHIFT) && g_theInput->IsKeyDown('8')))
		{
			m_currentMap->SpawnBloonAtStart(BloonDefinition::GetBloonDefinitionByName("Lead"));
		}

		//update spline editor and map
		if (m_isSplineEditing)
		{
			UpdateSplineEditor(orthoMousePos);
		}
	}
	
	if (m_currentMap != nullptr)
	{
		if (m_isRoundActive)
		{
			bool allWavesFinishedSpawning = true;
			
			for (int waveIndex = 0; waveIndex < m_roundDef->m_waves.size(); waveIndex++)
			{
				Wave const& wave = m_roundDef->m_waves[waveIndex];
				int& waveCount = m_waveCounts[waveIndex];
				float& waveTimer = m_waveTimers[waveIndex];
				
				if (waveCount > 0)
				{
					allWavesFinishedSpawning = false;

					waveTimer -= m_gameClock.GetDeltaSeconds();
					if (waveTimer <= 0.0f)
					{
						m_currentMap->SpawnBloonAtStart(wave.m_bloonDef);
						waveTimer = wave.m_timeBetweenSpawns;
						waveCount--;
					}
				}
			}

			if (allWavesFinishedSpawning)
			{
				bool allBloonsDead = true;

				for (int bloonIndex = 0; bloonIndex < m_currentMap->m_bloons.size(); bloonIndex++)
				{
					if (m_currentMap->m_bloons[bloonIndex] != nullptr)
					{
						allBloonsDead = false;
						break;
					}
				}

				if (allBloonsDead)
				{
					EndRound();
				}
			}
		}

		m_currentMap->Update(m_gameClock.GetDeltaSeconds());

		//update held tower
		if (m_heldTower != nullptr)
		{
			m_heldTower->m_position = orthoMousePos;
			m_canPlaceTower = true;

			//check if currently over any other towers
			for (int towerIndex = 0; towerIndex < m_currentMap->m_towers.size(); towerIndex++)
			{
				Tower* const& tower = m_currentMap->m_towers[towerIndex];

				if (tower == nullptr) continue;

				if (DoDiscsOverlap(m_heldTower->m_position, m_heldTower->m_definition->m_size, tower->m_position, tower->m_definition->m_size))
				{
					m_canPlaceTower = false;
				}
			}

			//#ToDo: Check if tower is over the track
			Vec2 nearestPointOnTrack = m_currentMap->GetNearestPointOnTrack(m_heldTower->m_position);
			if (DoDiscsOverlap(nearestPointOnTrack, TRACK_WIDTH, m_heldTower->m_position, m_heldTower->m_definition->m_size))
			{
				m_canPlaceTower = false;
			}
		}
	}
	
	if(m_resetTimer == 0.0f) UpdateUISidebar(orthoMousePos);
}


void Game::Render() const
{
	//if in attract mode, just render that and not anything else
	/*if (m_isAttractMode)
	{
		RenderAttract();
		return;
	}*/
	
	g_theRenderer->ClearScreen(Rgba8(0, 50, 100));
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->BindShader(nullptr);

	//render map texture
	std::vector<Vertex_PCU> backgroundVerts;
	AABB2 backgroundBounds = AABB2(0.0f, 0.0f, SCREEN_CAMERA_CENTER_X + SCREEN_CAMERA_SIZE_X * 0.25f, SCREEN_CAMERA_SIZE_Y);
	AddVertsForAABB2(backgroundVerts, backgroundBounds);

	g_theRenderer->BindTexture(m_currentMap->m_definition->m_texture);
	g_theRenderer->DrawVertexArray(backgroundVerts);

	//render spline editor and map
	if (m_isSplineEditing)
	{
		RenderSplineEditor();
	}
	if (m_currentMap != nullptr)
	{
		m_currentMap->Render();
	}

	//render tower being held
	if (m_heldTower != nullptr)
	{
		m_heldTower->Render();
		m_heldTower->RenderRange(!m_canPlaceTower);
	}

	RenderUISidebar();

	//debug rendering
	Clock& sysClock = Clock::GetSystemClock();
	std::string timeInfo = Stringf("Time: %.2f  FPS: %.1f  Time Scale: %.2f", m_gameClock.GetTotalSeconds(), 1.0f / sysClock.GetDeltaSeconds(), m_gameClock.GetTimeScale());
	DebugAddScreenText(timeInfo, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), 16.0f, Vec2(1.0f, 1.0f), 0.0f, Rgba8(), Rgba8());

	std::string gameInfo = Stringf("Round: %i   Lives: %i   Money: %i", m_roundNumber, m_numLives, m_numMoney);
	DebugAddMessage(gameInfo, 0.0f);

	g_theRenderer->EndCamera(m_screenCamera);

	DebugRenderScreen(m_screenCamera);
}


void Game::Shutdown()
{
	g_theAudio->StopSound(m_gameMusicPlayback);
	delete m_heldTower;
	delete m_currentMap;
}


//
//public gameplay functions
//
void Game::OpenMap(unsigned int mapIndex)
{
	MapDefinition const* def = MapDefinition::GetMapDefinitionByIndex(mapIndex);

	m_currentMap = new Map(def);
}


void Game::DeductLives(int livesLost)
{
	m_numLives -= livesLost;
	if (m_numLives <= 0)
	{
		m_numLives = 0;
		GameOver();
	}
}


void Game::GameOver()
{
	if (m_resetTimer > 0.0f) return;

	DebugAddScreenText("Game Over", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), SCREEN_CAMERA_SIZE_Y * 0.2f, Vec2(0.5f, 0.5f), m_timeBeforeReset, Rgba8(255, 25, 25), Rgba8(255, 25, 25));
	m_resetTimer = m_timeBeforeReset;

	delete m_heldTower;
	m_heldTower = nullptr;
}


void Game::EndRound()
{
	m_numMoney += m_roundNumber + 100;
	m_isRoundActive = false;
	m_roundNumber++;

	m_waveTimers.clear();
	m_waveCounts.clear();

	for (int projIndex = 0; projIndex < m_currentMap->m_projectiles.size(); projIndex++)
	{
		Projectile*& projectile = m_currentMap->m_projectiles[projIndex];

		if (projectile != nullptr && projectile->m_definition->m_isRoadItem)
		{
			projectile->m_outOfLifespan = true;
		}
	}

	//DebugAddMessage("End Round!", 5.0f);

	if (m_roundNumber > RoundDefinition::s_roundDefinitions.size())
	{
		m_roundNumber = static_cast<int>(RoundDefinition::s_roundDefinitions.size());
		WinGame();
	}
}


void Game::WinGame()
{
	if (m_resetTimer > 0.0f) return;

	DebugAddScreenText("YOU WIN!", Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), SCREEN_CAMERA_SIZE_Y * 0.2f, Vec2(0.5f, 0.5f), m_timeBeforeReset, Rgba8(25, 255, 25), Rgba8(25, 255, 25));
	m_resetTimer = m_timeBeforeReset;

	delete m_heldTower;
	m_heldTower = nullptr;
}


void Game::BuyTower(TowerDefinition const* def)
{
	if (def == nullptr)
	{
		DebugAddMessage("Tower def was nullptr!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return;
	}
	else if(def->m_cost > m_numMoney)
	{
		DebugAddMessage("Not enough money!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return;
	}
	//if we're already holding a tower, get rid of it
	if (m_heldTower != nullptr)
	{
		delete m_heldTower;
	}

	m_heldTower = new Tower(def, m_currentMap, Vec2(-1000.0f, -1000.0f));
	m_heldTower->m_isBeingHeld = true;
}


bool Game::PlaceHeldTower()
{
	if (!m_canPlaceTower) return false;

	//check cost, deduct if player has enough or disallow placing if player somehow doesn't
	if (m_numMoney < m_heldTower->m_definition->m_cost)
	{
		DebugAddMessage("Not enough money!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return false;
	}
	m_numMoney -= m_heldTower->m_definition->m_cost;

	//add tower to map
	m_currentMap->m_towers.emplace_back(m_heldTower);
	m_selectedTower = m_heldTower;

	//make tower no longer held
	m_heldTower->m_isBeingHeld = false;
	m_heldTower = nullptr;

	return true;
}


//
//commands
//
bool Game::Event_WriteMap(EventArgs& args)
{
	if (g_theGame == nullptr || g_theGame->m_currentMap == nullptr) return false;

	std::string mapName = args.GetValue("Name", "UnnamedMap");
	g_theGame->WriteCurrentMapToDisk(mapName);
	return true;
}


bool Game::Event_BuyTower(EventArgs& args)
{
	if (g_theGame == nullptr || g_theGame->m_currentMap == nullptr) return false;

	std::string towerDefName = args.GetValue("Name", "invalid");
	TowerDefinition const* towerDef = TowerDefinition::GetTowerDefinitionByName(towerDefName);
	g_theGame->BuyTower(towerDef);

	return true;
}


bool Game::Event_SellTower(EventArgs& args)
{
	UNUSED(args);

	if (g_theGame->m_selectedTower == nullptr)
	{
		return false;
	}

	//find the selected tower in the vector and delete it
	for (int towerIndex = 0; towerIndex < g_theGame->m_currentMap->m_towers.size(); towerIndex++)
	{
		Tower*& tower = g_theGame->m_currentMap->m_towers[towerIndex];
		
		if (tower == g_theGame->m_selectedTower)
		{
			g_theGame->m_selectedTower = nullptr;
			g_theGame->m_currentMap->SellTower(towerIndex);
			return true;
		}
	}

	//should never get here
	ERROR_RECOVERABLE("Attempted to sell tower that is not part of map");
	return false;
}


bool Game::Event_BuyUpgrade1(EventArgs& args)
{
	UNUSED(args);

	if (g_theGame->m_selectedTower == nullptr)
	{
		return false;
	}

	Tower*& tower = g_theGame->m_selectedTower;
	TowerDefinition const* towerDef = tower->m_definition;
	if (g_theGame->m_numMoney < towerDef->m_upgrade1Cost)
	{
		DebugAddMessage("Can't afford upgrade!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return false;
	}

	//replace the tower's definition with the definition of its first upgrade
	TowerDefinition const* upgrade1Def = TowerDefinition::GetTowerDefinitionByName(towerDef->m_upgrade1);
	if (upgrade1Def == nullptr)
	{
		DebugAddMessage("Upgrade was null!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return false;
	}

	g_theGame->m_numMoney -= towerDef->m_upgrade1Cost;
	tower->m_definition = upgrade1Def;
	return true;
}


bool Game::Event_BuyUpgrade2(EventArgs& args)
{
	UNUSED(args);

	if (g_theGame->m_selectedTower == nullptr)
	{
		return false;
	}

	Tower*& tower = g_theGame->m_selectedTower;
	TowerDefinition const* towerDef = tower->m_definition;
	if (g_theGame->m_numMoney < towerDef->m_upgrade2Cost)
	{
		DebugAddMessage("Can't afford upgrade!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return false;
	}

	//replace the tower's definition with the definition of its first upgrade
	TowerDefinition const* upgrade2Def = TowerDefinition::GetTowerDefinitionByName(towerDef->m_upgrade2);
	if (upgrade2Def == nullptr)
	{
		DebugAddMessage("Upgrade was null!", 7.0f, Rgba8(190, 20, 30), Rgba8(255, 255, 255, 0));
		return false;
	}

	g_theGame->m_numMoney -= towerDef->m_upgrade2Cost;
	tower->m_definition = upgrade2Def;
	return true;
}


bool Game::Event_StartRound(EventArgs& args)
{
	UNUSED(args);

	//DebugAddMessage("Start Round!", 5.0f);

	g_theGame->m_isRoundActive = true;
	g_theGame->m_roundDef = RoundDefinition::GetRoundDefinitionByIndex(g_theGame->m_roundNumber - 1);
	if (g_theGame->m_roundDef == nullptr)
	{
		DebugAddMessage("No more rounds!", 5.0f);
	}
	else
	{
		for (int waveIndex = 0; waveIndex < g_theGame->m_roundDef->m_waves.size(); waveIndex++)
		{
			Wave const& wave = g_theGame->m_roundDef->m_waves[waveIndex];

			g_theGame->m_waveTimers.emplace_back(wave.m_timeToStart);
			g_theGame->m_waveCounts.emplace_back(wave.m_numBloons);
		}
	}
	
	return true;
}


bool Game::Event_ToggleTargetingMode(EventArgs& args)
{
	UNUSED(args);

	switch (g_theGame->m_selectedTower->m_targetingMode)
	{
		case TargetingMode::FIRST:
		{
			g_theGame->m_selectedTower->m_targetingMode = TargetingMode::LAST;
			return true;
		}
		case TargetingMode::LAST:
		{
			g_theGame->m_selectedTower->m_targetingMode = TargetingMode::NEAR;
			return true;
		}
		case TargetingMode::NEAR:
		{
			g_theGame->m_selectedTower->m_targetingMode = TargetingMode::STRONG;
			return true;
		}
		case TargetingMode::STRONG:
		{
			g_theGame->m_selectedTower->m_targetingMode = TargetingMode::WEAK;
			return true;
		}
		case TargetingMode::WEAK:
		{
			g_theGame->m_selectedTower->m_targetingMode = TargetingMode::FIRST;
			return true;
		}
	}

	return false;
}


bool Game::Event_SelectMap(EventArgs& args)
{
	UNUSED(args);

	g_theGame->m_isMapSelection = false;

	return true;
}


//
//game flow sub-functions
//
//void Game::UpdateAttract()
//{
//	XboxController const& controller = g_theInput->GetController(0);
//
//	if (g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_A))
//	{
//		EnterGameplay();
//	}
//}


//void Game::RenderAttract() const
//{
//	g_theRenderer->ClearScreen(Rgba8(0, 0, 0));	//clear screen to black
//	
//	g_theRenderer->BeginCamera(m_screenCamera);	//render attract screen with the screen camera
//	
//	float currentTime = m_gameClock.GetTotalSeconds();
//	float ringRadius = 250.f + cosf(currentTime);
//	float ringWidth = 10.f * sinf(currentTime);
//	Rgba8 ringColor = Rgba8(255, 135, 50);
//	g_theRenderer->BindTexture(nullptr);
//	g_theRenderer->SetModelConstants();
//	DebugDrawRing(Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), ringRadius, ringWidth, ringColor);
//
//	g_theRenderer->EndCamera(m_screenCamera);
//}


void Game::UpdateUISidebar(Vec2 orthoMousePos)
{
	if (m_isMapSelection)
	{
		m_map1Button.Update();
		m_map2Button.Update();
		m_map3Button.Update();

		if (m_map1Button.IsMouseInsideBounds())
		{
			OpenMap(0);
		}
		else if (m_map2Button.IsMouseInsideBounds())
		{
			OpenMap(1);
		}
		else if (m_map3Button.IsMouseInsideBounds())
		{
			OpenMap(2);
		}

		return;
	}

	bool drawingShopInfo = false;
	for (int buttonIndex = 0; buttonIndex < m_shopButtons.size(); buttonIndex++)
	{
		Button& button = m_shopButtons[buttonIndex];

		button.Update();

		if (button.IsMouseInsideBounds())
		{
			drawingShopInfo = true;
		}
	}

	if (m_selectedTower != nullptr && !drawingShopInfo)
	{
		TowerDefinition const* towerDef = m_selectedTower->m_definition;

		if (TowerDefinition::GetTowerDefinitionByName(towerDef->m_upgrade1) == nullptr)
		{
			m_upgrade1Button.m_text = "Upgraded";
			m_upgrade1Button.m_color = Rgba8(100, 255, 100);
			m_upgrade1Button.m_selectedColor = Rgba8(100, 255, 100);
		}
		else
		{
			m_upgrade1Button.m_text = Stringf("%s: $%i", towerDef->m_upgrade1Name.c_str(), towerDef->m_upgrade1Cost);
			if (g_theGame->m_numMoney < towerDef->m_upgrade1Cost)
			{
				m_upgrade1Button.m_color = Rgba8(200, 50, 50);
				m_upgrade1Button.m_selectedColor = Rgba8(200, 50, 50);
			}
			else
			{
				m_upgrade1Button.m_color = Rgba8(50, 200, 50);
				m_upgrade1Button.m_selectedColor = Rgba8(100, 255, 100);
			}
			m_upgrade1Button.Update();
		}
		
		if (TowerDefinition::GetTowerDefinitionByName(towerDef->m_upgrade2) == nullptr)
		{
			m_upgrade2Button.m_text = "Upgraded";
			m_upgrade2Button.m_color = Rgba8(100, 255, 100);
			m_upgrade2Button.m_selectedColor = Rgba8(100, 255, 100);
		}
		else
		{
			m_upgrade2Button.m_text = Stringf("%s: $%i", towerDef->m_upgrade2Name.c_str(), towerDef->m_upgrade2Cost);
			if (g_theGame->m_numMoney < towerDef->m_upgrade2Cost)
			{
				m_upgrade2Button.m_color = Rgba8(200, 50, 50);
				m_upgrade2Button.m_selectedColor = Rgba8(200, 50, 50);
			}
			else
			{
				m_upgrade2Button.m_color = Rgba8(50, 200, 50);
				m_upgrade2Button.m_selectedColor = Rgba8(100, 255, 100);
			}
			m_upgrade2Button.Update();
		}
		
		if (towerDef->m_isTracking)
		{
			m_targetModeButton.m_text = Stringf("Targeting Mode: %s", m_selectedTower->GetTargetingModeAsString().c_str());
			m_targetModeButton.Update();
		}

		int sellPrice = towerDef->m_cost * 8 / 10;
		std::string sellButtonText = Stringf("Sell for: %i", sellPrice);
		m_sellButton.m_text = sellButtonText;
		m_sellButton.Update();
	}
	else if(m_selectedTower == nullptr && m_heldTower == nullptr && !m_isRoundActive)
	{
		m_startButton.Update();
	}
}


void Game::RenderUISidebar() const
{
	//draw base rectangle
	std::vector<Vertex_PCU> baseVerts;
	AddVertsForAABB2(baseVerts, m_UIBaseBounds, Rgba8(195, 210, 225));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(baseVerts);

	if (m_isMapSelection)
	{
		DebugAddScreenText("Select Map:", Vec2(SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_Y * 0.02f, SCREEN_CAMERA_SIZE_Y * 0.9f), 32.0f, Vec2(1.0f, 0.5f), 0.0f);

		m_map1Button.Render();
		m_map2Button.Render();
		m_map3Button.Render();
		
		return;
	}

	//draw tower buying icons
	bool drawingShopInfo = false;
	for (int buttonIndex = 0; buttonIndex < m_shopButtons.size(); buttonIndex++)
	{
		Button const& button = m_shopButtons[buttonIndex];

		button.Render();

		//render tower info if icon is hovered
		if (button.IsMouseInsideBounds() && m_resetTimer == 0.0f)
		{
			TowerDefinition const* def = TowerDefinition::GetTowerDefinitionByName(button.m_args.GetValue("Name", "invalid"));
			RenderTowerInfo(def);
			drawingShopInfo = true;
		}
	}

	//render upgrade menu if tower is selected
	if (m_selectedTower != nullptr && !drawingShopInfo)
	{
		m_upgrade1Button.Render();
		m_upgrade2Button.Render();
		if (m_selectedTower->m_definition->m_isTracking)
		{
			m_targetModeButton.Render();
		}
		m_sellButton.Render();
	}
	else if (m_selectedTower == nullptr && m_heldTower == nullptr && !m_isRoundActive)
	{
		m_startButton.Render();
	}
}


void Game::RenderTowerInfo(TowerDefinition const* def) const
{
	//render underlying quad
	std::vector<Vertex_PCU> boxVerts;
	AddVertsForAABB2(boxVerts, m_infoBounds, Rgba8(235, 245, 255));
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(boxVerts);

	//render cost
	std::vector<Vertex_PCU> textVerts;
	std::string costString = Stringf("Cost: %i", def->m_cost);
	m_menuFont->AddVertsForTextInBox2D(textVerts, m_infoBounds, 16.0f, costString, Rgba8(0, 0, 0), 1.0f, Vec2(0.5f, 1.0f));

	//render description
	AABB2 descBounds = AABB2(m_infoBounds.m_mins, m_infoBounds.m_maxs - Vec2(0.0f, 22.0f));
	m_menuFont->AddVertsForTextInBox2D(textVerts, descBounds, 14.0f, def->m_description, Rgba8(0, 0, 0), 1.0f, Vec2(0.5f, 1.0f));

	g_theRenderer->BindTexture(&m_menuFont->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts);
}


void Game::UpdateSplineEditor(Vec2 orthoMousePos)
{
	constexpr float SNAP_DISTANCE_SQUARED = 49.0f;

	if (m_currentMap == nullptr)
	{
		return;
	}

	bool snapPoints = false;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		snapPoints = true;
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB))
	{
		m_selectedControlPoint = nullptr;
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_RMB))
	{
		m_selectedCurve = nullptr;
		m_selectedControlPoint = nullptr;
	}

	m_highlightedControlPoint = nullptr;
	m_highlightedCurve = nullptr;
	if (m_selectedControlPoint == nullptr || m_selectedCurve == nullptr)
	{
		for (int curveIndex = 0; curveIndex < m_currentMap->m_trackSpline.size(); curveIndex++)
		{
			CubicBezierCurve2D& curve = m_currentMap->m_trackSpline[curveIndex];
			if (IsPointInsideDisc2D(orthoMousePos, curve.A, 12.5f))
			{
				m_highlightedCurve = &curve;
				m_highlightedControlPoint = &curve.A;
				break;
			}
			if (IsPointInsideDisc2D(orthoMousePos, curve.B, 12.5f))
			{
				m_highlightedCurve = &curve;
				m_highlightedControlPoint = &curve.B;
				break;
			}
			if (IsPointInsideDisc2D(orthoMousePos, curve.C, 12.5f))
			{
				m_highlightedCurve = &curve;
				m_highlightedControlPoint = &curve.C;
				break;
			}
			if (IsPointInsideDisc2D(orthoMousePos, curve.D, 12.5f))
			{
				m_highlightedCurve = &curve;
				m_highlightedControlPoint = &curve.D;
				break;
			}
		}
	}

	if (m_highlightedControlPoint != nullptr && g_theInput->WasKeyJustPressed(KEYCODE_LMB))
	{
		m_selectedControlPoint = m_highlightedControlPoint;
	}
	if (m_highlightedCurve != nullptr && g_theInput->WasKeyJustPressed(KEYCODE_RMB))
	{
		m_selectedCurve = m_highlightedCurve;
		m_selectedControlPoint = m_highlightedControlPoint;
	}

	
	if (m_selectedCurve != nullptr && m_selectedControlPoint != nullptr)
	{
		Vec2 disp = orthoMousePos - *m_selectedControlPoint;
		m_selectedCurve->A += disp;
		m_selectedCurve->B += disp;
		m_selectedCurve->C += disp;
		m_selectedCurve->D += disp;
	}
	else if (m_selectedControlPoint != nullptr)
	{
		*m_selectedControlPoint = orthoMousePos;

		if (snapPoints)
		{
			for (int curveIndex = 0; curveIndex < m_currentMap->m_trackSpline.size(); curveIndex++)
			{
				CubicBezierCurve2D const& curve = m_currentMap->m_trackSpline[curveIndex];

				if (curve.A != *m_selectedControlPoint && GetDistanceSquared2D(orthoMousePos, curve.A) < SNAP_DISTANCE_SQUARED)
				{
					*m_selectedControlPoint = curve.A;
					break;
				}
				if (curve.B != *m_selectedControlPoint && GetDistanceSquared2D(orthoMousePos, curve.B) < SNAP_DISTANCE_SQUARED)
				{
					*m_selectedControlPoint = curve.B;
					break;
				}
				if (curve.C != *m_selectedControlPoint && GetDistanceSquared2D(orthoMousePos, curve.C) < SNAP_DISTANCE_SQUARED)
				{
					*m_selectedControlPoint = curve.C;
					break;
				}
				if (curve.D != *m_selectedControlPoint && GetDistanceSquared2D(orthoMousePos, curve.D) < SNAP_DISTANCE_SQUARED)
				{
					*m_selectedControlPoint = curve.D;
					break;
				}
			}
		}
	}

	//remove or add spline to track
	if (g_theInput->WasKeyJustPressed(KEYCODE_COMMA))
	{
		m_currentMap->m_trackSpline.pop_back();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_PERIOD))
	{
		m_currentMap->m_trackSpline.emplace_back(CubicBezierCurve2D(Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), Vec2(SCREEN_CAMERA_CENTER_X + 33.3f, SCREEN_CAMERA_CENTER_Y),
			Vec2(SCREEN_CAMERA_CENTER_X + 66.7f, SCREEN_CAMERA_CENTER_Y), Vec2(SCREEN_CAMERA_CENTER_X + 100.0f, SCREEN_CAMERA_CENTER_Y)));
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_SEMICOLON))
	{
		m_currentMap->m_trackSpline.emplace_back(CubicBezierCurve2D(Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y), Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y + 33.3f),
			Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y + 66.7f), Vec2(SCREEN_CAMERA_CENTER_X, SCREEN_CAMERA_CENTER_Y + 100.0f)));
	}
}


void Game::RenderSplineEditor() const
{
	if (m_currentMap == nullptr)
	{
		return;
	}

	std::vector<Vertex_PCU> splineVerts;

	for (int curveIndex = 0; curveIndex < m_currentMap->m_trackSpline.size(); curveIndex++)
	{
		AddVertsForBezierCurve(splineVerts, m_currentMap->m_trackSpline[curveIndex]);
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(splineVerts);
}


void Game::AddVertsForBezierCurve(std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& curve) const
{
	//constexpr float SPLINE_THICKNESS = 5.0f;
	Rgba8 const splineColor = Rgba8(225, 0, 225);
	Rgba8 const discColor = Rgba8(225, 0, 50);
	Rgba8 const highlightedDiscColor = Rgba8(100, 175, 50);
	Rgba8 const selectedDiscColor = Rgba8(50, 255, 50);

	//render lines from control points to curve
	Vec2 oneThirdPoint = curve.EvaluateAtParametric(0.333f);
	AddVertsForLineSegment2D(verts, curve.B, oneThirdPoint, 2.0f, Rgba8(120, 50, 120));
	Vec2 twoThirdsPoint = curve.EvaluateAtParametric(0.666f);
	AddVertsForLineSegment2D(verts, curve.C, twoThirdsPoint, 2.0f, Rgba8(120, 50, 120));

	//render curve
	Vec2 curveSegmentStart = curve.EvaluateAtParametric(0.0f);
	for (int subdivIndex = 0; subdivIndex < NUM_CURVE_SUBDIVISIONS; subdivIndex++)
	{
		float curveSegmentEndT = static_cast<float>(subdivIndex + 1) * (1.0f / NUM_CURVE_SUBDIVISIONS);
		Vec2 curveSegmentEnd = curve.EvaluateAtParametric(curveSegmentEndT);

		if (subdivIndex == NUM_CURVE_SUBDIVISIONS - 1)
		{
			AddVertsForArrow2D(verts, curveSegmentStart, curveSegmentEnd, 15.0f, TRACK_WIDTH, splineColor);
		}
		else
		{
			AddVertsForLineSegment2D(verts, curveSegmentStart, curveSegmentEnd, TRACK_WIDTH, splineColor);
		}
		
		curveSegmentStart = curveSegmentEnd;
	}
	
	//render control points
	AddVertsForDisc2D(verts, curve.A, TRACK_WIDTH * 2.5f, discColor);
	AddVertsForDisc2D(verts, curve.B, TRACK_WIDTH * 2.5f, discColor);
	AddVertsForDisc2D(verts, curve.C, TRACK_WIDTH * 2.5f, discColor);
	AddVertsForDisc2D(verts, curve.D, TRACK_WIDTH * 2.5f, discColor);

	//render selected/highlighted control point
	if (m_highlightedControlPoint != nullptr)
	{
		if (*m_highlightedControlPoint == curve.A)
		{
			AddVertsForDisc2D(verts, curve.A, TRACK_WIDTH * 2.5f, highlightedDiscColor);
		}
		else if (*m_highlightedControlPoint == curve.B)
		{
			AddVertsForDisc2D(verts, curve.B, TRACK_WIDTH * 2.5f, highlightedDiscColor);
		}
		else if (*m_highlightedControlPoint == curve.C)
		{
			AddVertsForDisc2D(verts, curve.C, TRACK_WIDTH * 2.5f, highlightedDiscColor);
		}
		else if (*m_highlightedControlPoint == curve.D)
		{
			AddVertsForDisc2D(verts, curve.D, TRACK_WIDTH * 2.5f, highlightedDiscColor);
		}
	}
	if (m_selectedControlPoint != nullptr)
	{
		if (*m_selectedControlPoint == curve.A)
		{
			AddVertsForDisc2D(verts, curve.A, TRACK_WIDTH * 2.5f, selectedDiscColor);
		}
		else if (*m_selectedControlPoint == curve.B)
		{
			AddVertsForDisc2D(verts, curve.B, TRACK_WIDTH * 2.5f, selectedDiscColor);
		}
		else if (*m_selectedControlPoint == curve.C)
		{
			AddVertsForDisc2D(verts, curve.C, TRACK_WIDTH * 2.5f, selectedDiscColor);
		}
		else if (*m_selectedControlPoint == curve.D)
		{
			AddVertsForDisc2D(verts, curve.D, TRACK_WIDTH * 2.5f, selectedDiscColor);
		}
	}
}


//
//setup functions
//
void Game::AddButtonsForShop()
{
	//dart monkey button
	Vec2 dartMonkeyButtonMins = Vec2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_Y * 0.01f, m_UIBaseBounds.m_maxs.y - SCREEN_CAMERA_SIZE_Y * 0.25f);
	AABB2 dartMonkeyButtonBounds = AABB2(dartMonkeyButtonMins, dartMonkeyButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* dartMonkeyDef = TowerDefinition::GetTowerDefinitionByName("Dart Monkey");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, dartMonkeyButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), dartMonkeyDef->m_texture));
	EventArgs args;
	args.SetValue("Name", "Dart Monkey");
	m_shopButtons[0].AddEvent("BuyTower", args);

	//tack shooter button
	Vec2 tackShooterButtonMins = dartMonkeyButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.075f, 0.0f);
	AABB2 tackShooterButtonBounds = AABB2(tackShooterButtonMins, tackShooterButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* tackShooterDef = TowerDefinition::GetTowerDefinitionByName("Tack Shooter");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, tackShooterButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), tackShooterDef->m_texture));
	args.SetValue("Name", "Tack Shooter");
	m_shopButtons[1].AddEvent("BuyTower", args);

	//boomerang monkey button
	Vec2 boomerangMonkeyButtonMins = tackShooterButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.075f, 0.0f);
	AABB2 boomerangMonkeyButtonBounds = AABB2(boomerangMonkeyButtonMins, boomerangMonkeyButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* boomerangMonkeyDef = TowerDefinition::GetTowerDefinitionByName("Boomerang Monkey");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, boomerangMonkeyButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), boomerangMonkeyDef->m_texture));
	args.SetValue("Name", "Boomerang Monkey");
	m_shopButtons[2].AddEvent("BuyTower", args);

	//cannon button
	Vec2 cannonButtonMins = boomerangMonkeyButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.075f, 0.0f);
	AABB2 cannonButtonBounds = AABB2(cannonButtonMins, cannonButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* cannonDef = TowerDefinition::GetTowerDefinitionByName("Cannon");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, cannonButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), cannonDef->m_texture));
	args.SetValue("Name", "Cannon");
	m_shopButtons[3].AddEvent("BuyTower", args);

	//ice ball button
	Vec2 iceBallButtonMins = cannonButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.075f, 0.0f);
	AABB2 iceBallButtonBounds = AABB2(iceBallButtonMins, iceBallButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* iceBallDef = TowerDefinition::GetTowerDefinitionByName("Ice Ball");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, iceBallButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), iceBallDef->m_texture));
	args.SetValue("Name", "Ice Ball");
	m_shopButtons[4].AddEvent("BuyTower", args);

	//super monkey button
	Vec2 superMonkeyButtonMins = iceBallButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.075f, 0.0f);
	AABB2 superMonkeyButtonBounds = AABB2(superMonkeyButtonMins, superMonkeyButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	TowerDefinition const* superMonkeyDef = TowerDefinition::GetTowerDefinitionByName("Super Monkey");
	m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, superMonkeyButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), superMonkeyDef->m_texture));
	args.SetValue("Name", "Super Monkey");
	m_shopButtons[5].AddEvent("BuyTower", args);

	//road spikes button
	//Vec2 roadSpikesButtonMins = dartMonkeyButtonMins - Vec2(0.0f, SCREEN_CAMERA_SIZE_Y * 0.075f);
	//AABB2 roadSpikesButtonBounds = AABB2(roadSpikesButtonMins, roadSpikesButtonMins + Vec2(SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f));
	//ProjectileDefinition const* roadSpikesDef = ProjectileDefinition::GetProjectileDefinitionByName("Road Spikes");
	////m_shopButtons.emplace_back(Button(g_theRenderer, g_theInput, roadSpikesButtonBounds, Vec2(SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y), roadSpikesDef->m_texture));
	//args.SetValue("Name", "Road Spikes");
	
}


//void Game::EnterAttractMode()
//{
//	m_isAttractMode = true;
//}


void Game::EnterGameplay()
{
	//m_isAttractMode = false;
	m_isMapSelection = true;

	m_gameMusicPlayback = g_theAudio->StartSound(m_gameMusic, true, 0.6f);

	OpenMap(0);
}


//
//asset management functions
//
void Game::LoadDefinitions()
{
	if (BloonDefinition::s_bloonDefinitions.size() == 0)
	{
		BloonDefinition::InitializeBloonDefinitions();
	}
	if (ProjectileDefinition::s_projectileDefinitions.size() == 0)
	{
		ProjectileDefinition::InitializeProjectileDefinitions();
	}
	if (TowerDefinition::s_towerDefinitions.size() == 0)
	{
		TowerDefinition::InitializeTowerDefinitions();
	}
	if (MapDefinition::s_mapDefinitions.size() == 0)
	{
		MapDefinition::InitializeMapDefinitions();
	}
	if (RoundDefinition::s_roundDefinitions.size() == 0)
	{
		RoundDefinition::InitializeRoundDefinitions();
	}
}


void Game::WriteCurrentMapToDisk(std::string const& mapName)
{
	//use buffer writer to write map data out
	std::vector<uint8_t> splineBuffer;
	BufferWriter bw = BufferWriter(splineBuffer);
	std::vector<CubicBezierCurve2D>& spline = m_currentMap->m_trackSpline;

	for (int curveIndex = 0; curveIndex < spline.size(); curveIndex++)
	{
		CubicBezierCurve2D& curve = spline[curveIndex];
		/*std::string splineDesc = Stringf("A: %6.1f, %6.1f  -  B: %6.1f, %6.1f  -  C: %6.1f, %6.1f  -  D: %6.1f. %6.1f\n", curve.A.x, curve.A.y, curve.B.x, curve.B.y, curve.C.x, curve.C.y,
			curve.D.x, curve.D.y);*/
		std::string splineDesc = Stringf("<Curve a=\"%.1f, %.1f\" b=\"%.1f, %.1f\" c=\"%.1f, %.1f\" d=\"%.1f, %.1f\"/>\n", curve.A.x, curve.A.y, curve.B.x, curve.B.y, curve.C.x, curve.C.y, 
			curve.D.x, curve.D.y);
		for (int charIndex = 0; charIndex < splineDesc.length(); charIndex++)
		{
			bw.AppendChar(splineDesc[charIndex]);
		}
	}

	std::string filePath = Stringf("Data/Exported/%s.txt", mapName.c_str());
	FileWriteFromBuffer(splineBuffer, filePath);
}
