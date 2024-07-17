#include "Game/RoundDefinition.hpp"
#include "Game/BloonDefinition.hpp"


std::vector<RoundDefinition> RoundDefinition::s_roundDefinitions;


//
//constructor
//
RoundDefinition::RoundDefinition(XmlElement const& element)
{
	XmlElement const* waveElement = element.FirstChildElement();
	std::string elementName;
	while (waveElement != nullptr)
	{
		elementName = waveElement->Name();

		if (elementName == "Wave")
		{
			Wave wave;
			std::string bloonDefName = ParseXmlAttribute(*waveElement, "bloonDef", "invalid");
			if (bloonDefName == "invalid")
			{
				ERROR_AND_DIE("Invalid bloon def in round definitions!");
			}
			wave.m_bloonDef = BloonDefinition::GetBloonDefinitionByName(bloonDefName);
			wave.m_numBloons = ParseXmlAttribute(*waveElement, "numBloons", 0);
			wave.m_timeBetweenSpawns = ParseXmlAttribute(*waveElement, "timeBetweenSpawns", 0.0f);
			wave.m_timeToStart = ParseXmlAttribute(*waveElement, "timeToStart", 0.0f);
			m_waves.emplace_back(wave);
		}

		waveElement = waveElement->NextSiblingElement();
	}
}


//
//static functions
//
void RoundDefinition::InitializeRoundDefinitions()
{
	s_roundDefinitions.reserve(50);

	XmlDocument roundDefsXml;
	char const* filePath = "Data/Definitions/RoundDefinitions.xml";
	XmlError result = roundDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open round definitions xml file!");

	XmlElement* rootElement = roundDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read round definitions root element!");

	XmlElement* roundDefElement = rootElement->FirstChildElement();
	while (roundDefElement != nullptr)
	{
		std::string elementName = roundDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "RoundDefinition", "Child element names in round definitions xml file must be <RoundDefinition>!");
		RoundDefinition newRoundDef = RoundDefinition(*roundDefElement);
		s_roundDefinitions.emplace_back(newRoundDef);
		roundDefElement = roundDefElement->NextSiblingElement();
	}
}


RoundDefinition const* RoundDefinition::GetRoundDefinitionByIndex(unsigned int index)
{
	if (index >= s_roundDefinitions.size()) return nullptr;

	return &s_roundDefinitions[index];
}
