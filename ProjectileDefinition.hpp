#pragma once
#include "Game/DamageTypes.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"


class Texture;


class ProjectileDefinition
{
//public member functions
public:
	ProjectileDefinition(XmlElement const& element);

	static void InitializeProjectileDefinitions();
	static ProjectileDefinition const* GetProjectileDefinitionByIndex(unsigned int index);
	static ProjectileDefinition const* GetProjectileDefinitionByName(std::string const& name);

//public member variables
public:
	std::string m_name = "Invalid";

	Texture* m_texture = nullptr;
	SoundID  m_spawnSound = 0;

	int   m_pierce = 0;
	float m_lifespan = 0.0f;
	float m_speed = 0.0f;
	float m_size = 0.0f;
	int   m_damage = 0;

	bool m_curvedArc = false;
	bool m_isRoadItem = false;

	float m_freezeTimer = 0.0f;
	float m_glueTimer = 0.0f;

	DamageType m_damageType = DamageType::None;

	std::vector<std::string> m_projectilesToSpawn;

	static std::vector<ProjectileDefinition> s_projectileDefinitions;
};
