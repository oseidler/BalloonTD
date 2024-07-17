#include "Game/ProjectileDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"


std::vector<ProjectileDefinition> ProjectileDefinition::s_projectileDefinitions;


//
//constructor
//
ProjectileDefinition::ProjectileDefinition(XmlElement const& element)
{
	m_name = ParseXmlAttribute(element, "name", m_name);

	std::string textureFilePath = ParseXmlAttribute(element, "texture", "invalid path");
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(textureFilePath.c_str());

	std::string spawnSoundFilePath = ParseXmlAttribute(element, "spawnSound", "invalid path");
	if (spawnSoundFilePath != "invalid path")
	{
		m_spawnSound = g_theAudio->CreateOrGetSound(spawnSoundFilePath);
	}

	m_pierce = ParseXmlAttribute(element, "pierce", m_pierce);
	m_lifespan = ParseXmlAttribute(element, "lifespan", m_lifespan);
	m_speed = ParseXmlAttribute(element, "speed", m_speed) * SPEED_MODIFIER;
	m_size = ParseXmlAttribute(element, "size", m_size) * SIZE_MODIFIER;
	m_damage = ParseXmlAttribute(element, "damage", m_damage);

	m_curvedArc = ParseXmlAttribute(element, "curvedArc", m_curvedArc);
	m_isRoadItem = ParseXmlAttribute(element, "isRoadItem", m_isRoadItem);

	m_freezeTimer = ParseXmlAttribute(element, "freezeTimer", m_freezeTimer);
	m_glueTimer = ParseXmlAttribute(element, "glueTimer", m_glueTimer);

	std::string immunityStr = ParseXmlAttribute(element, "damageType", "invalid");
	if (immunityStr == "Sharp")
	{
		m_damageType = DamageType::Sharp;
	}
	else if (immunityStr == "Shatter")
	{
		m_damageType = DamageType::Shatter;
	}
	else if (immunityStr == "Thermal")
	{
		m_damageType = DamageType::Thermal;
	}
	else if (immunityStr == "Explosion")
	{
		m_damageType = DamageType::Explosion;
	}
	else if (immunityStr == "Freeze")
	{
		m_damageType = DamageType::Freeze;
	}

	XmlElement const* subElement = element.FirstChildElement();
	std::string elementName;
	if (subElement != nullptr)
	{
		elementName = subElement->Name();
	}

	if (subElement != nullptr && elementName == "SpawnProjectiles")
	{
		XmlElement const* spawnElement = subElement->FirstChildElement();
		while (spawnElement != nullptr)
		{
			std::string spawnElementName = spawnElement->Name();
			if (spawnElementName == "Projectile")
			{
				std::string spawnProjectileName = ParseXmlAttribute(*spawnElement, "name", "invalid name");
				if (spawnProjectileName != "invalid name")
				{
					m_projectilesToSpawn.emplace_back(spawnProjectileName);
				}
			}

			spawnElement = spawnElement->NextSiblingElement();
		}
	}
}


//
//static functions
//
void ProjectileDefinition::InitializeProjectileDefinitions()
{
	s_projectileDefinitions.reserve(16);

	XmlDocument projectileDefsXml;
	char const* filePath = "Data/Definitions/ProjectileDefinitions.xml";
	XmlError result = projectileDefsXml.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "Failed to open projectile definitions xml file!");

	XmlElement* rootElement = projectileDefsXml.RootElement();
	GUARANTEE_OR_DIE(rootElement != nullptr, "Failed to read projectile definitions root element!");

	XmlElement* projectileDefElement = rootElement->FirstChildElement();
	while (projectileDefElement != nullptr)
	{
		std::string elementName = projectileDefElement->Name();
		GUARANTEE_OR_DIE(elementName == "ProjectileDefinition", "Child element names in projectile definitions xml file must be <ProjectileDefinition>!");
		ProjectileDefinition newProjectileDef = ProjectileDefinition(*projectileDefElement);
		s_projectileDefinitions.emplace_back(newProjectileDef);
		projectileDefElement = projectileDefElement->NextSiblingElement();
	}
}


ProjectileDefinition const* ProjectileDefinition::GetProjectileDefinitionByIndex(unsigned int index)
{
	if (index >= s_projectileDefinitions.size()) return nullptr;

	return &s_projectileDefinitions[index];
}


ProjectileDefinition const* ProjectileDefinition::GetProjectileDefinitionByName(std::string const& name)
{
	for (int defIndex = 0; defIndex < s_projectileDefinitions.size(); defIndex++)
	{
		if (s_projectileDefinitions[defIndex].m_name == name)
		{
			return &s_projectileDefinitions[defIndex];
		}
	}

	//return null if it wasn't found
	return nullptr;
}
