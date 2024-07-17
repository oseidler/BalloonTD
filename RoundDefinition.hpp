#pragma once
#include "Engine/Core/EngineCommon.hpp"


class BloonDefinition;


struct Wave
{
	BloonDefinition const* m_bloonDef = nullptr;
	int m_numBloons = 0;
	float m_timeBetweenSpawns = 0.0f;
	float m_timeToStart = 0.0f;


};


class RoundDefinition
{
//public member functions
public:
	RoundDefinition(XmlElement const& element);

	static void InitializeRoundDefinitions();
	static RoundDefinition const* GetRoundDefinitionByIndex(unsigned int index);

//public member variables
public:
	std::vector<Wave> m_waves;

	static std::vector<RoundDefinition> s_roundDefinitions;
};
