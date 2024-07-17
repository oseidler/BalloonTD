#include "Game/Projectile.hpp"
#include "Game/ProjectileDefinition.hpp"
#include "Game/Bloon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Audio/AudioSystem.hpp"


//
//constructor
//
Projectile::Projectile(ProjectileDefinition const* definition, Map* map, Vec2 const& position, Vec2 const& direction, int addedPierce, float addedLifespan, float addedSize, 
	float addedFreezeTime, CubicBezierCurve2D curvedProjArc)
	: m_definition(definition)
	, m_map(map)
	, m_position(position)
	, m_velocity(direction * definition->m_speed)
	, m_remainingLifespan(definition->m_lifespan)
	, m_remainingPierce(definition->m_pierce)
	, m_addedFreezeTime(addedFreezeTime)
	, m_curvedProjArc(curvedProjArc)
{
	m_bloonsToPassOver.reserve(definition->m_pierce + 1);
	m_remainingPierce += addedPierce;
	m_remainingLifespan += addedLifespan;
	m_size = definition->m_size + addedSize;

	if (m_definition->m_spawnSound != 0)
	{
		g_theAudio->StartSound(m_definition->m_spawnSound);
	}
}


//
//public game flow functions
//
void Projectile::Update(float deltaSeconds)
{
	//If curved arc, move in an arc back to the thrower
	if (m_definition->m_curvedArc)
	{
		float totalSplineDistance = m_curvedProjArc.GetApproximateLength(NUM_CURVE_SUBDIVISIONS);
		
		m_curvedArcDistance += m_definition->m_speed * deltaSeconds;

		if (m_curvedArcDistance > totalSplineDistance)
		{
			DieFromLifespan();
		}
		else
		{
			m_position = m_curvedProjArc.EvaluateAtApproximateDistance(m_curvedArcDistance, NUM_CURVE_SUBDIVISIONS);
			m_directionDegrees += m_definition->m_speed * deltaSeconds * 2.5f;
		}
	}
	else
	{
		//move based on velocity
		m_position += m_velocity * deltaSeconds;

		//reduce lifespan and die if lifespan runs out
		m_remainingLifespan -= deltaSeconds;
		if (m_remainingLifespan <= 0.0f && !m_definition->m_isRoadItem)
		{
			DieFromLifespan();
		}
	}
}


void Projectile::Render() const
{
	//#TODO: Replace current system with having all projectiles in one draw call using spritesheet
	std::vector<Vertex_PCU> verts;
	
	Vec2 direction = m_velocity;
	if (m_velocity.GetLength() == 0.0f)
	{
		direction = Vec2(0.0f, -1.0f);
	}
	OBB2 renderBounds = OBB2(m_position, direction.GetNormalized().GetRotated90Degrees().GetRotatedDegrees(m_directionDegrees), Vec2(m_size, m_size));

	AddVertsForOBB2D(verts, renderBounds);

	g_theRenderer->BindTexture(m_definition->m_texture);
	g_theRenderer->DrawVertexArray(verts);
}


//
//public gameplay functions
//
void Projectile::DeductPierce()
{
	m_remainingPierce--;
	if (m_remainingPierce <= 0)
	{
		DieFromPierce();
	}
}


void Projectile::DieFromLifespan()
{
	m_outOfLifespan = true;
}


void Projectile::DieFromPierce()
{
	m_outOfPierce = true;
}
