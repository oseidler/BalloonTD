#include "Game/Map.hpp"
#include "Game/Bloon.hpp"
#include "Game/Tower.hpp"
#include "Game/BloonDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/Projectile.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/TowerDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"


//
//destructor
//
Map::~Map()
{
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		delete m_bloons[bloonIndex];
	}
	for (int towerIndex = 0; towerIndex < m_towers.size(); towerIndex++)
	{
		delete m_towers[towerIndex];
	}
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		delete m_projectiles[projIndex];
	}
}


//
//public game flow functions
//
void Map::Update(float deltaSeconds)
{
	//update all map-owned entities
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon*& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr)
		{
			bloon->Update(deltaSeconds);
		}
	}
	if (g_theGame->m_resetTimer == 0.0f)	//towers only update if game isn't over
	{
		for (int towerIndex = 0; towerIndex < m_towers.size(); towerIndex++)
		{
			Tower*& tower = m_towers[towerIndex];

			if (tower != nullptr)
			{
				tower->Update(deltaSeconds);
			}
		}
	}
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		Projectile*& projectile = m_projectiles[projIndex];

		if (projectile != nullptr)
		{
			projectile->Update(deltaSeconds);
		}
	}

	//check each projectile against each bloon to check for collisions
	CollideProjectilesAgainstBloons();

	//spawn children and give money for all popped bloons
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon*& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr && bloon->m_hasPopped)
		{
			SpawnBloonChildren(bloon);
		}
	}
	//do the loop a second time to actually kill bloons now
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon*& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr && bloon->m_hasPopped)
		{
			g_theGame->m_numMoney++;
			delete bloon;
			bloon = nullptr;
		}
	}
	
	//handle all leaked bloons
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon*& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr && bloon->m_hasLeaked)
		{
			g_theGame->DeductLives(bloon->m_definition->m_RBE);

			delete bloon;
			bloon = nullptr;
		}
	}

	//handle all dead projectiles
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		Projectile*& projectile = m_projectiles[projIndex];

		if (projectile != nullptr && projectile->m_outOfPierce)
		{
			ProjectileDefinition const* def = projectile->m_definition;
			for (int projISpawnIndex = 0; projISpawnIndex < def->m_projectilesToSpawn.size(); projISpawnIndex++)
			{
				ProjectileDefinition const* spawnDef = ProjectileDefinition::GetProjectileDefinitionByName(def->m_projectilesToSpawn[projISpawnIndex]);
				SpawnProjectile(spawnDef, projectile->m_position, projectile->m_velocity.GetNormalized());
			}
		}
	}
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		Projectile*& projectile = m_projectiles[projIndex];

		if (projectile != nullptr && (projectile->m_outOfLifespan || projectile->m_outOfPierce))
		{
			delete projectile;
			projectile = nullptr;
		}
	}
}


void Map::Render() const
{
	//render all map-owned entities

	BloonDefinition const* redDef = BloonDefinition::GetBloonDefinitionByName("Red");
	BloonDefinition const* leadDef = BloonDefinition::GetBloonDefinitionByName("Lead");
	BloonDefinition const* rainbowDef = BloonDefinition::GetBloonDefinitionByName("Rainbow");

	g_theRenderer->BindTexture(redDef->m_texture);
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon* const& bloon = m_bloons[bloonIndex];
		
		if (bloon != nullptr && bloon->m_definition != leadDef && bloon->m_definition != rainbowDef)
		{
			bloon->Render();
		}
	}

	g_theRenderer->BindTexture(leadDef->m_texture);
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon* const& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr && bloon->m_definition == leadDef)
		{
			bloon->Render();
		}
	}

	g_theRenderer->BindTexture(rainbowDef->m_texture);
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		Bloon* const& bloon = m_bloons[bloonIndex];

		if (bloon != nullptr && bloon->m_definition == rainbowDef)
		{
			bloon->Render();
		}
	}

	for (int towerIndex = 0; towerIndex < m_towers.size(); towerIndex++)
	{
		Tower* const& tower = m_towers[towerIndex];

		if (tower != nullptr && (g_theGame->m_showAllTowerRanges || tower == g_theGame->m_selectedTower))
		{
			tower->RenderRange();
		}
	}

	for (int towerIndex = 0; towerIndex < m_towers.size(); towerIndex++)
	{
		Tower* const& tower = m_towers[towerIndex];

		if (tower != nullptr)
		{
			tower->Render();
		}
	}
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		Projectile* const& projectile = m_projectiles[projIndex];

		if (projectile != nullptr)
		{
			projectile->Render();
		}
	}
}


void Map::SpawnBloonAtStart(BloonDefinition const* bloonDef)
{
	//find empty space within vector first
	for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
	{
		if (m_bloons[bloonIndex] == nullptr)
		{
			m_bloons[bloonIndex] = new Bloon(bloonDef, this, &m_trackSpline[0]);
			return;
		}
	}

	//otherwise, add a new one to the vector
	m_bloons.emplace_back(new Bloon(bloonDef, this, &m_trackSpline[0]));
}


void Map::SpawnBloonChildren(Bloon const* bloon)
{
	BloonDefinition const* def = bloon->m_definition;

	float spawnOffset = 0.0f;
	for (int childIndex = 0; childIndex < def->m_children.size(); childIndex++)
	{
		BloonDefinition const* childDef = def->m_children[childIndex];
		if (childDef != nullptr)
		{
			bool addedToExistingSlot = false;

			//find empty space within vector first
			for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
			{
				if (m_bloons[bloonIndex] == nullptr)
				{
					m_bloons[bloonIndex] = new Bloon(childDef, this, bloon->m_currentSplineCurve, bloon->m_trackDistance - spawnOffset);
					addedToExistingSlot = true;
					bloon->m_popper->m_bloonsToPassOver.emplace_back(m_bloons[bloonIndex]);
					break;
				}
			}

			//otherwise, add a new one to the vector
			if (!addedToExistingSlot)
			{
				m_bloons.emplace_back(new Bloon(childDef, this, bloon->m_currentSplineCurve, bloon->m_trackDistance - spawnOffset));
				bloon->m_popper->m_bloonsToPassOver.emplace_back(m_bloons[m_bloons.size() - 1]);
			}
			
			spawnOffset -= CHILD_SPACING;
		}
	}
}


void Map::SpawnProjectile(ProjectileDefinition const* projectileDef, Vec2 const& position, Vec2 const& direction, int addedPierce, float addedLifespan, float addedSize, float addedFreezeTime,
	CubicBezierCurve2D curvedProjArc)
{
	//find empty space within vector first
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		if (m_projectiles[projIndex] == nullptr)
		{
			m_projectiles[projIndex] = new Projectile(projectileDef, this, position, direction, addedPierce, addedLifespan, addedSize, addedFreezeTime, curvedProjArc);
			return;
		}
	}

	//otherwise, add a new one to the vector
	m_projectiles.emplace_back(new Projectile(projectileDef, this, position, direction, addedPierce, addedLifespan, addedSize, addedFreezeTime, curvedProjArc));
}


void Map::CollideProjectilesAgainstBloons()
{
	for (int projIndex = 0; projIndex < m_projectiles.size(); projIndex++)
	{
		Projectile*& projectile = m_projectiles[projIndex];

		if (projectile != nullptr && !projectile->m_outOfLifespan && !projectile->m_outOfPierce)
		{
			for (int bloonIndex = 0; bloonIndex < m_bloons.size(); bloonIndex++)
			{
				Bloon*& bloon = m_bloons[bloonIndex];

				if (bloon != nullptr && !bloon->m_hasLeaked && !bloon->m_hasPopped)
				{
					CollideProjectileAgainstBloon(*projectile, *bloon);
					//early out of loop if pierce runs out
					if (projectile->m_remainingPierce <= 0)
					{
						break;
					}
				}
			}
		}
	}
}


bool Map::CollideProjectileAgainstBloon(Projectile& projectile, Bloon& bloon)
{
	for (int bloonIndex = 0; bloonIndex < projectile.m_bloonsToPassOver.size(); bloonIndex++)
	{
		Bloon*& bloonToCheck = projectile.m_bloonsToPassOver[bloonIndex];

		if (&bloon == bloonToCheck)
		{
			return false;
		}
	}

	if (DoDiscsOverlap(projectile.m_position, projectile.m_size, bloon.m_position, bloon.m_definition->m_size))
	{
		projectile.DeductPierce();
		bloon.TakeDamage(projectile);
		return true;
	}

	return false;
}


void Map::SellTower(int towerIndex)
{
	Tower*& tower = m_towers[towerIndex];
	int cost = tower->m_definition->m_cost;

	delete tower;
	tower = nullptr;

	g_theGame->m_numMoney += cost * 8 / 10;
}


Vec2 Map::GetNearestPointOnTrack(Vec2 const& referencePoint) const
{
	Vec2 nearestPoint = Vec2(FLT_MAX, FLT_MAX);

	for (int curveIndex = 0; curveIndex < m_trackSpline.size(); curveIndex++)
	{
		CubicBezierCurve2D const& curve = m_trackSpline[curveIndex];

		Vec2 curveSegmentStart = curve.EvaluateAtParametric(0.0f);
		for (int subdivIndex = 0; subdivIndex < NUM_CURVE_SUBDIVISIONS; subdivIndex++)
		{
			float curveSegmentEndT = static_cast<float>(subdivIndex + 1) * (1.0f / NUM_CURVE_SUBDIVISIONS);
			Vec2 curveSegmentEnd = curve.EvaluateAtParametric(curveSegmentEndT);

			Vec2 nearestPointOnSegment = GetNearestPointOnLineSegment(referencePoint, curveSegmentStart, curveSegmentEnd);

			if (GetDistanceSquared2D(nearestPointOnSegment, referencePoint) < GetDistanceSquared2D(nearestPoint, referencePoint))
			{
				nearestPoint = nearestPointOnSegment;
			}

			curveSegmentStart = curveSegmentEnd;
		}
	}

	return nearestPoint;
}
