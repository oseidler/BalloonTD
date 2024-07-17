#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/Button.hpp"


class Map;
class Tower;
class TowerDefinition;
class RoundDefinition;
class ProjectileDefinition;
class BitmapFont;


class Game 
{
//public member functions
public:
	//game flow functions
	void Startup();
	void Update();
	void Render() const;
	void Shutdown();

	//gameplay functions
	void OpenMap(unsigned int mapIndex);
	void DeductLives(int livesLost);
	void GameOver();
	void EndRound();
	void WinGame();
	void BuyTower(TowerDefinition const* def);
	//void BuyProjectile(ProjectileDefinition const* def);
	bool PlaceHeldTower();

	//commands
	static bool Event_WriteMap(EventArgs& args);
	static bool Event_BuyTower(EventArgs& args);
	static bool Event_SellTower(EventArgs& args);
	static bool Event_BuyUpgrade1(EventArgs& args);
	static bool Event_BuyUpgrade2(EventArgs& args);
	static bool Event_StartRound(EventArgs& args);
	static bool Event_ToggleTargetingMode(EventArgs& args);
	static bool Event_SelectMap(EventArgs& args);

//public member variables
public:
	bool m_isFinished = false;
	//bool m_isAttractMode = true;
	bool m_isMapSelection = false;

	float m_timeBeforeReset = 4.0f;
	float m_resetTimer = 0.0f;

	Clock m_gameClock = Clock();
	BitmapFont* m_menuFont = nullptr;

	Map* m_currentMap = nullptr;

	Tower* m_selectedTower = nullptr;
	Tower* m_heldTower = nullptr;
	//Projectile* m_heldProjectile = nullptr;
	bool m_canPlaceTower = false;

	int m_numLives = 100;
	int m_numMoney = 650;

	RoundDefinition const* m_roundDef = nullptr;
	int m_roundNumber = 1;
	bool m_isRoundActive = false;
	std::vector<float> m_waveTimers;
	std::vector<int> m_waveCounts;

	SoundID m_gameMusic;
	SoundPlaybackID m_gameMusicPlayback;
	SoundID m_frozenHitSound;

	bool m_showAllTowerRanges = false;

//private member functions
private:
	//game flow sub-functions
	//void UpdateAttract();
	//void RenderAttract() const;
	void UpdateUISidebar(Vec2 orthoMousePos);
	void RenderUISidebar() const;
	void RenderTowerInfo(TowerDefinition const* def) const;
	void UpdateSplineEditor(Vec2 orthoMousePos);
	void RenderSplineEditor() const;
	void AddVertsForBezierCurve(std::vector<Vertex_PCU>& verts, CubicBezierCurve2D const& curve) const;

	//setup functions
	void AddButtonsForShop();
	//void EnterAttractMode();
	void EnterGameplay();

	//data management functions
	void LoadDefinitions();
	void WriteCurrentMapToDisk(std::string const& mapName);

//private member variables
private:
	Camera m_screenCamera;

	bool  m_isSplineEditing = false;
	Vec2* m_highlightedControlPoint = nullptr;
	Vec2* m_selectedControlPoint = nullptr;
	CubicBezierCurve2D* m_highlightedCurve = nullptr;
	CubicBezierCurve2D* m_selectedCurve = nullptr;

	AABB2 m_UIBaseBounds = AABB2(SCREEN_CAMERA_CENTER_X + SCREEN_CAMERA_SIZE_X * 0.25f, 0.0f, SCREEN_CAMERA_SIZE_X, SCREEN_CAMERA_SIZE_Y);
	std::vector<Button> m_shopButtons;
	AABB2 m_infoBounds = AABB2(m_UIBaseBounds.m_mins + Vec2(0.0f, SCREEN_CAMERA_SIZE_Y * 0.1f), m_UIBaseBounds.m_maxs - Vec2(0.0f, SCREEN_CAMERA_SIZE_Y * 0.3f));
	Button m_sellButton;
	AABB2 m_sellButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_X * 0.05f, SCREEN_CAMERA_SIZE_Y * 0.05f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_X * 0.05f, SCREEN_CAMERA_SIZE_X * 0.08f);
	Button m_upgrade1Button;
	AABB2 m_upgrade1ButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_X * 0.025f, SCREEN_CAMERA_SIZE_Y * 0.40f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_X * 0.025f, SCREEN_CAMERA_SIZE_Y * 0.50f);
	Button m_upgrade2Button;
	AABB2 m_upgrade2ButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_X * 0.025f, SCREEN_CAMERA_SIZE_Y * 0.25f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_X * 0.025f, SCREEN_CAMERA_SIZE_Y * 0.35f);
	Button m_targetModeButton;
	AABB2 m_targetModeButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_X * 0.022f, SCREEN_CAMERA_SIZE_Y * 0.175f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_X * 0.022f, SCREEN_CAMERA_SIZE_Y * 0.22f);
	Button m_startButton;
	AABB2 m_startButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_X * 0.035f, SCREEN_CAMERA_SIZE_Y * 0.02f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_X * 0.035f, SCREEN_CAMERA_SIZE_Y * 0.09f);
	Button m_map1Button;
	AABB2 m_map1ButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.65f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.85f);
	Button m_map2Button;
	AABB2 m_map2ButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.4f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.6f);
	Button m_map3Button;
	AABB2 m_map3ButtonBounds = AABB2(m_UIBaseBounds.m_mins.x + SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.15f, SCREEN_CAMERA_SIZE_X - SCREEN_CAMERA_SIZE_Y * 0.14f, SCREEN_CAMERA_SIZE_Y * 0.35f);
};
