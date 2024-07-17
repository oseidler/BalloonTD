#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct CubicBezierCurve2D;
class  Texture;


class MapDefinition
{
//public member functions
public:
	MapDefinition(XmlElement const& element);

	static void InitializeMapDefinitions();
	static MapDefinition const* GetMapDefinitionByIndex(unsigned int index);
	static MapDefinition const* GetMapDefinitionByName(std::string const& name);

//public member variables
public:
	std::string m_name = "Invalid";

	Texture* m_texture = nullptr;

	std::vector<CubicBezierCurve2D> m_trackSpline;

	static std::vector<MapDefinition> s_mapDefinitions;
};
