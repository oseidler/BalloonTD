#include "Game/BloonDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"


std::vector<BloonDefinition> BloonDefinition::s_bloonDefinitions;


//
//constructor
//
BloonDefinition::BloonDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "texture", "invalid path");
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());
	m_color = ParseXmlAttribute(element, "color", m_color);

	std::string popSoundFilePath = ParseXmlAttribute(element, "popSound", "invalid path");
	if (popSoundFilePath != "invalid path")
	{
		m_popSound = g_theAudio->CreateOrGetSound(popSoundFilePath);
	}
	std::string damageSoundFilePath = ParseXmlAttribute(element, "damageSound", "invalid path");
	if (damageSoundFilePath != "invalid path")
	{
		m_damageSound = g_theAudio->CreateOrGetSound(damageSoundFilePath);
	}
	std::string noDamageSoundFilePath = ParseXmlAttribute(element, "noDamageSound", "invalid path");
	if (noDamageSoundFilePath != "invalid path")
	{
		m_noDamageSound = g_theAudio->CreateOrGetSound(noDamageSoundFilePath);
	}

	m_speed = ParseXmlAttribute(element, "speed", m_speed) * SPEED_MODIFIER;
	m_health = ParseXmlAttribute(element, "health", m_health);
	m_RBE = ParseXmlAttribute(element, "RBE", m_RBE);
	m_size = ParseXmlAttribute(element, "size", m_size) * SIZE_MODIFIER;

	XmlElement const* subElement = element.FirstChildElement();
	std::string elementName;
	if (subElement != nullptr)
	{
		elementName = subElement->Name();
	}

	if (subElement != nullptr && elementName == "Immunities")
	{
		XmlElement const* immunityElement = subElement->FirstChildElement();
		while (immunityElement != nullptr)
		{
			std::string immunityElementName = immunityElement->Name();
			if (immunityElementName == "Immunity")
			{
				std::string immunityStr = ParseXmlAttribute(*immunityElement, "type", "none");
				if (immunityStr == "Sharp")
				{
					m_immunities.emplace_back(DamageType::Sharp);
				}
				else if (immunityStr == "Shatter")
				{
					m_immunities.emplace_back(DamageType::Shatter);
				}
				else if (immunityStr == "Thermal")
				{
					m_immunities.emplace_back(DamageType::Thermal);
				}
				else if (immunityStr == "Explosion")
				{
					m_immunities.emplace_back(DamageType::Explosion);
				}
				else if (immunityStr == "Freeze")
				{
					m_immunities.emplace_back(DamageType::Freeze);
				}
			}

			immunityElement = immunityElement->NextSiblingElement();
		}

		subElement = subElement->NextSiblingElement();

		if (subElement != nullptr)
		{
			elementName = subElement->Name();
		}
	}

	if (subElement != nullptr && elementName == "Children")
	{
		XmlElement const* childElement = subElement->FirstChildElement();
		while (childElement != nullptr)
		{
			std::string childElementName = childElement->Name();
			if (childElementName == "Child")
			{
				std::string childDefStr = ParseXmlAttribute(*childElement, "definition", "none");
				BloonDefinition const* childDef = GetBloonDefinitionByName(childDefStr);
				m_children.emplace_back(childDef);
			}

			childElement = childElement->NextSiblingElement();
		}
	}
}


//
//static functions
//
void BloonDefinition::InitializeBloonDefinitions()
{
	s_bloonDefinitions.reserve(10);
	
	XmlDocument bloonDefsXml;
	char const* filePath = "Data/Definitions/BloonDefinitions.xml";
	XmlError result = bloonDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open bloon definitions xml file!");

	XmlElement* rootElement = bloonDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read bloon definitions root element!");

	XmlElement* bloonDefElement = rootElement->FirstChildElement();
	while (bloonDefElement != nullptr)
	{
		std::string elementName = bloonDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "BloonDefinition", "Child element names in bloon definitions xml file must be <BloonDefinition>!");
		BloonDefinition newBloonDef = BloonDefinition(*bloonDefElement);
		s_bloonDefinitions.emplace_back(newBloonDef);
		bloonDefElement = bloonDefElement->NextSiblingElement();
	}
}


BloonDefinition const* BloonDefinition::GetBloonDefinitionByIndex(unsigned int index)
{
	if (index >= s_bloonDefinitions.size()) return nullptr;

	return &s_bloonDefinitions[index];
}


BloonDefinition const* BloonDefinition::GetBloonDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_bloonDefinitions.size(); defIndex++)
	{
		if (s_bloonDefinitions[defIndex].m_name == name)
		{
			return &s_bloonDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
