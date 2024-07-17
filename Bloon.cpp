#include "Game/Bloon.hpp"
#include "Game/BloonDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Projectile.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//
//constructor
//
Bloon::Bloon(BloonDefinition const* definition, Map const* map, CubicBezierCurve2D const* startingCurve, float trackDistance)
	: m_definition(definition)
	, m_map(map)
	, m_currentSplineCurve(startingCurve)
	, m_trackDistance(trackDistance)
{
	if (map == nullptr || startingCurve == nullptr)
	{
		ERROR_AND_DIE("Bloon must be constructed with valid map and curve!");
	}

	if (m_trackDistance < 0.0f) m_trackDistance = 0.0f;

	//#TO-DO: After map splines are finalized, calculate this once ahead of time and store in Map
	float totalSplineDistance = 0.0f;
	for (int curveIndex = 0; curveIndex < m_map->m_trackSpline.size(); curveIndex++)
	{
		totalSplineDistance += m_map->m_trackSpline[curveIndex].GetApproximateLength(NUM_CURVE_SUBDIVISIONS);
	}

	//calculate new position along track
	float approximateDistanceOnSpline = m_trackDistance;
	float approximateDistanceOnCurve = 0.0f;
	int curveNum = 0;
	for (int curveIndex = 0; curveIndex < m_map->m_trackSpline.size(); curveIndex++)
	{
		float curveAproxLength = m_map->m_trackSpline[curveIndex].GetApproximateLength(NUM_CURVE_SUBDIVISIONS);
		if (approximateDistanceOnSpline > curveAproxLength)
		{
			approximateDistanceOnSpline -= curveAproxLength;
		}
		else
		{
			approximateDistanceOnCurve = approximateDistanceOnSpline;
			curveNum = curveIndex;
			break;
		}
	}
	m_position = m_map->m_trackSpline[curveNum].EvaluateAtApproximateDistance(approximateDistanceOnCurve, NUM_CURVE_SUBDIVISIONS);
}

//
//public game flow functions
//
void Bloon::Update(float deltaSeconds)
{
	//if frozen, don't move and deduct freeze time
	if (m_freezeTimer > 0.0f)
	{
		m_freezeTimer -= deltaSeconds;
		return;
	}

	//#TO-DO: After map splines are finalized, calculate this once ahead of time and store in Map
	float totalSplineDistance = 0.0f;
	for (int curveIndex = 0; curveIndex < m_map->m_trackSpline.size(); curveIndex++)
	{
		totalSplineDistance += m_map->m_trackSpline[curveIndex].GetApproximateLength(NUM_CURVE_SUBDIVISIONS);
	}

	//move along track based on speed
	m_trackDistance += m_definition->m_speed * deltaSeconds;

	//if bloon is past end of track, leak
	if (m_trackDistance >= totalSplineDistance)
	{
		Leak();
		return;
	}

	//calculate new position along track
	float approximateDistanceOnSpline = m_trackDistance;
	float approximateDistanceOnCurve = 0.0f;
	int curveNum = 0;
	for (int curveIndex = 0; curveIndex < m_map->m_trackSpline.size(); curveIndex++)
	{
		float curveAproxLength = m_map->m_trackSpline[curveIndex].GetApproximateLength(NUM_CURVE_SUBDIVISIONS);
		if (approximateDistanceOnSpline > curveAproxLength)
		{
			approximateDistanceOnSpline -= curveAproxLength;
		}
		else
		{
			approximateDistanceOnCurve = approximateDistanceOnSpline;
			curveNum = curveIndex;
			break;
		}
	}
	m_position = m_map->m_trackSpline[curveNum].EvaluateAtApproximateDistance(approximateDistanceOnCurve, NUM_CURVE_SUBDIVISIONS);
}


void Bloon::Render() const
{
	//#TODO: Replace current system with having all bloons in one draw call using spritesheet
	std::vector<Vertex_PCU> verts;

	float const& size = m_definition->m_size;
	AABB2 renderBounds = AABB2(m_position.x - size, m_position.y - size, m_position.x + size, m_position.y + size);

	AddVertsForAABB2(verts, renderBounds, m_definition->m_color);

	if (m_freezeTimer > 0.0f)
	{
		AddVertsForAABB2(verts, renderBounds, Rgba8(255, 255, 255, 150));
	}

	g_theRenderer->DrawVertexArray(verts);
}


//
//public gameplay functions
//
void Bloon::TakeDamage(Projectile& damageSource)
{
	DamageType damageType = damageSource.m_definition->m_damageType;
	int damageAmount = damageSource.m_definition->m_damage;

	bool immune = false;
	//if frozen, check if damage is Sharp or Freeze
	if (m_freezeTimer > 0.0f && (damageType == DamageType::Sharp || damageType == DamageType::Freeze))
	{
		immune = true;
	}
	//now check other immunities if necessary
	if (!immune)
	{
		for (int immunityIndex = 0; immunityIndex < m_definition->m_immunities.size(); immunityIndex++)
		{
			if (m_definition->m_immunities[immunityIndex] == damageType)
			{
				immune = true;
				break;
			}
		}
	}

	if (!immune)
	{
		m_currentHealth -= damageAmount;
		if (m_currentHealth <= 0)
		{
			Pop(damageSource);
		}
		else
		{
			if (damageSource.m_definition->m_freezeTimer + damageSource.m_addedFreezeTime > 0.0f)
			{
				m_freezeTimer = damageSource.m_definition->m_freezeTimer + damageSource.m_addedFreezeTime;
			}

			damageSource.m_bloonsToPassOver.emplace_back(this);
		}
	}
	else
	{
		/*while (damageSource.m_remainingPierce > 0)
		{
			damageSource.DeductPierce();
		}*/

		//play immunity sound
		if (m_freezeTimer > 0.0f && damageType != DamageType::Freeze)
		{
			g_theAudio->StartSound(g_theGame->m_frozenHitSound, false, 0.9f);
		}
		else
		{
			if (m_definition->m_noDamageSound != 0) g_theAudio->StartSound(m_definition->m_noDamageSound, false, 0.95f);
		}
	}
}


void Bloon::Pop(Projectile& popper)
{
	m_hasPopped = true;
	m_popper = &popper;

	g_theAudio->StartSound(m_definition->m_popSound, false, 0.64f);
}


void Bloon::Leak()
{
	m_hasLeaked = true;
}
