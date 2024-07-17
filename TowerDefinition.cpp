#include "Game/TowerDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"


std::vector<TowerDefinition> TowerDefinition::s_towerDefinitions;


//
//constructor
//
TowerDefinition::TowerDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "texture", "invalid path");
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	std::string projDefName = ParseXmlAttribute(element, "projectile", "invalid projectile");
	m_projectileDef = ProjectileDefinition::GetProjectileDefinitionByName(projDefName);
	m_numProjectiles = ParseXmlAttribute(element, "numProjectiles", m_numProjectiles);

	m_cost = ParseXmlAttribute(element, "cost", m_cost);
	m_range = ParseXmlAttribute(element, "range", m_range) * RANGE_MODIFIER;
	m_attackCooldown = ParseXmlAttribute(element, "attackCooldown", m_attackCooldown);
	m_isTracking = ParseXmlAttribute(element, "isTracking", m_isTracking);
	m_size = ParseXmlAttribute(element, "size", m_size) * SIZE_MODIFIER;
	
	m_addedPierce = ParseXmlAttribute(element, "addedPierce", m_addedPierce);
	m_addedLifespan = ParseXmlAttribute(element, "addedLifespan", m_addedLifespan);
	m_addedFreezeTime = ParseXmlAttribute(element, "addedFreezeTime", m_addedFreezeTime);
	m_addedSize = ParseXmlAttribute(element, "addedSize", m_addedSize);

	m_description = ParseXmlAttribute(element, "description", m_description);

	XmlElement const* upgradesElement = element.FirstChildElement();
	std::string elementName;
	if (upgradesElement != nullptr)
	{
		elementName = upgradesElement->Name();
	}

	if (upgradesElement != nullptr && elementName == "Upgrades")
	{
		m_upgrade1 = ParseXmlAttribute(*upgradesElement, "upgrade1", m_upgrade1);
		m_upgrade1Cost = ParseXmlAttribute(*upgradesElement, "upgrade1Cost", m_upgrade1Cost);
		m_upgrade1Name = ParseXmlAttribute(*upgradesElement, "upgrade1Name", m_upgrade1Name);
		m_upgrade1Desc = ParseXmlAttribute(*upgradesElement, "upgrade1Desc", m_upgrade1Desc);
		std::string upgrade1TextureFilePath = ParseXmlAttribute(*upgradesElement, "upgrade1Texture", "invalid path");
		if (upgrade1TextureFilePath != "invalid path")
		{
			m_upgrade1Tex = g_theRenderer->CreateOrGetTextureFromFile(upgrade1TextureFilePath.c_str());
		}
		m_upgrade2 = ParseXmlAttribute(*upgradesElement, "upgrade2", m_upgrade2);
		m_upgrade2Cost = ParseXmlAttribute(*upgradesElement, "upgrade2Cost", m_upgrade2Cost);
		m_upgrade2Name = ParseXmlAttribute(*upgradesElement, "upgrade2Name", m_upgrade2Name);
		m_upgrade2Desc = ParseXmlAttribute(*upgradesElement, "upgrade2Desc", m_upgrade2Desc);
		std::string upgrade2TextureFilePath = ParseXmlAttribute(*upgradesElement, "upgrade2Texture", "invalid path");
		if (upgrade2TextureFilePath != "invalid path")
		{
			m_upgrade2Tex = g_theRenderer->CreateOrGetTextureFromFile(upgrade2TextureFilePath.c_str());
		}
	}
	
	ReplacePartOfString(m_description, "\\n", "\n");	//this has to be done because tinyxml reads in \n incorrectly
}


//
//static functions
//
void TowerDefinition::InitializeTowerDefinitions()
{
	s_towerDefinitions.reserve(8);

	XmlDocument towerDefsXml;
	char const* filePath = "Data/Definitions/TowerDefinitions.xml";
	XmlError result = towerDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open tower definitions xml file!");

	XmlElement* rootElement = towerDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read tower definitions root element!");

	XmlElement* towerDefElement = rootElement->FirstChildElement();
	while (towerDefElement != nullptr)
	{
		std::string elementName = towerDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "TowerDefinition", "Child element names in tower definitions xml file must be <TowerDefinition>!");
		TowerDefinition newTowerDef = TowerDefinition(*towerDefElement);
		s_towerDefinitions.emplace_back(newTowerDef);
		towerDefElement = towerDefElement->NextSiblingElement();
	}
}


TowerDefinition const* TowerDefinition::GetTowerDefinitionByIndex(unsigned int index)
{
	if (index >= s_towerDefinitions.size()) return nullptr;

	return &s_towerDefinitions[index];
}


TowerDefinition const* TowerDefinition::GetTowerDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_towerDefinitions.size(); defIndex++)
	{
		if (s_towerDefinitions[defIndex].m_name == name)
		{
			return &s_towerDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}

