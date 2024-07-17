#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"


std::vector<MapDefinition> MapDefinition::s_mapDefinitions;


//
//constructor
//
MapDefinition::MapDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "texture", "invalid path");
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	XmlElement const* splineElement = element.FirstChildElement();
	std::string elementName;
	if (splineElement != nullptr)
	{
		elementName = splineElement->Name();
	}

	if (splineElement != nullptr && elementName == "Spline")
	{
		XmlElement const* curveElement = splineElement->FirstChildElement();
		while (curveElement != nullptr)
		{
			std::string splineElementName = curveElement->Name();
			if (splineElementName == "Curve")
			{
				Vec2 a = ParseXmlAttribute(*curveElement, "a", Vec2());
				Vec2 b = ParseXmlAttribute(*curveElement, "b", Vec2());
				Vec2 c = ParseXmlAttribute(*curveElement, "c", Vec2());
				Vec2 d = ParseXmlAttribute(*curveElement, "d", Vec2());

				m_trackSpline.emplace_back(CubicBezierCurve2D(a, b, c, d));
			}

			curveElement = curveElement->NextSiblingElement();
		}
	}
}


//
//static functions
//
void MapDefinition::InitializeMapDefinitions()
{
	s_mapDefinitions.reserve(12);

	XmlDocument mapDefsXml;
	char const* filePath = "Data/Definitions/MapDefinitions.xml";
	XmlError result = mapDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open map definitions xml file!");

	XmlElement* rootElement = mapDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read map definitions root element!");

	XmlElement* mapDefElement = rootElement->FirstChildElement();
	while (mapDefElement != nullptr)
	{
		std::string elementName = mapDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "MapDefinition", "Child element names in map definitions xml file must be <MapDefinition>!");
		MapDefinition newMapDef = MapDefinition(*mapDefElement);
		s_mapDefinitions.emplace_back(newMapDef);
		mapDefElement = mapDefElement->NextSiblingElement();
	}
}


MapDefinition const* MapDefinition::GetMapDefinitionByIndex(unsigned int index)
{
	if (index >= s_mapDefinitions.size()) return nullptr;

	return &s_mapDefinitions[index];
}


MapDefinition const* MapDefinition::GetMapDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_mapDefinitions.size(); defIndex++)
	{
		if (s_mapDefinitions[defIndex].m_name == name)
		{
			return &s_mapDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
