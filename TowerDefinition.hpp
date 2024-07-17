#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"


constexpr float RANGE_MODIFIER = 1.5f;


class Texture;
class ProjectileDefinition;


class TowerDefinition
{
//public member functions
public:
	TowerDefinition(XmlElement const& element);

	static void InitializeTowerDefinitions();
	static TowerDefinition const* GetTowerDefinitionByIndex(unsigned int index);
	static TowerDefinition const* GetTowerDefinitionByName(std::string const& name);

//public member variables
public:
	std::string m_name = "Invalid";

	Texture* m_texture = nullptr;

	ProjectileDefinition const* m_projectileDef = nullptr;
	int							m_numProjectiles = 0;

	int	  m_cost = 0;
	float m_range = 0.0f;
	float m_attackCooldown = 0.0f;
	bool  m_isTracking = false;
	float m_size = 0.0f;

	int   m_addedPierce = 0;
	float m_addedLifespan = 0.0f;
	float m_addedFreezeTime = 0.0f;
	float m_addedSize = 0.0f;

	std::string m_description = "No description";

	std::string m_upgrade1 = "";
	int			m_upgrade1Cost = 0;
	std::string m_upgrade1Name = "No name";
	std::string m_upgrade1Desc = "No description";
	Texture*	m_upgrade1Tex = nullptr;
	std::string m_upgrade2 = "";
	int			m_upgrade2Cost = 0;
	std::string m_upgrade2Name = "No name";
	std::string m_upgrade2Desc = "No description";
	Texture*	m_upgrade2Tex = nullptr;

	static std::vector<TowerDefinition> s_towerDefinitions;
};
