#pragma once
#include "Game/DamageTypes.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"


class Texture;


class BloonDefinition
{
//public member functions
public:
	BloonDefinition(XmlElement const& element);

	static void InitializeBloonDefinitions();
	static BloonDefinition const* GetBloonDefinitionByIndex(unsigned int index);
	static BloonDefinition const* GetBloonDefinitionByName(std::string const& name);

//public member variables
public:
	std::string m_name = "Invalid";

	Texture*	m_texture = nullptr;
	Rgba8		m_color = Rgba8();
	SoundID		m_popSound = 0;
	SoundID		m_damageSound = 0;
	SoundID		m_noDamageSound = 0;

	float m_speed = 0.0f;
	int   m_health = 0;
	int	  m_RBE = 0;
	float m_size = 0.0f;

	std::vector<DamageType> m_immunities;
	std::vector<BloonDefinition const*> m_children;

	static std::vector<BloonDefinition> s_bloonDefinitions;
};
