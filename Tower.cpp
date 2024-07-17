#include "Game/Tower.hpp"
#include "Game/TowerDefinition.hpp"
#include "Game/Bloon.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/BloonDefinition.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/OBB2.hpp"


//
//game flow functions
//
void Tower::Update(float deltaSeconds)
{
	if (m_isBeingHeld) return;

	m_curvedProjArc = CubicBezierCurve2D::CreateUsingHermite(m_position, (m_iBasis * 0.9f + m_iBasis.GetRotatedMinus90Degrees()) * SCREEN_CAMERA_SIZE_Y, m_position, 
		(-m_iBasis * 0.9f + m_iBasis.GetRotatedMinus90Degrees()) * SCREEN_CAMERA_SIZE_Y);

	FindTarget();

	if (m_target != nullptr)
	{
		if (m_definition->m_isTracking)
		{
			m_iBasis = (m_target->m_position - m_position).GetNormalized();
		}

		m_cooldownTimer += deltaSeconds;
		if (m_cooldownTimer > m_definition->m_attackCooldown)
		{
			ShootProjectile();
		}
	}

	m_target = nullptr;
}


void Tower::Render() const
{
	std::vector<Vertex_PCU> verts;

	float const& size = m_definition->m_size;
	OBB2 renderBounds = OBB2(m_position, m_iBasis.GetRotated90Degrees(), Vec2(size, size));

	AddVertsForOBB2D(verts, renderBounds);

	g_theRenderer->BindTexture(m_definition->m_texture);
	g_theRenderer->DrawVertexArray(verts);

	/*DebugAddScreenText("A", m_curvedProjArc.A, SIZE_MODIFIER, Vec2(0.5f, 0.5f), 0.0f);
	DebugAddScreenText("B", m_curvedProjArc.B, SIZE_MODIFIER, Vec2(0.5f, 0.5f), 0.0f);
	DebugAddScreenText("C", m_curvedProjArc.C, SIZE_MODIFIER, Vec2(0.5f, 0.5f), 0.0f);
	DebugAddScreenText("D", m_curvedProjArc.D, SIZE_MODIFIER, Vec2(0.5f, 0.5f), 0.0f);*/
}


void Tower::RenderRange(bool redRange) const
{
	std::vector<Vertex_PCU> verts;

	if (redRange)
	{
		AddVertsForDisc2D(verts, m_position, m_definition->m_range, Rgba8(255, 50, 50, 127));
	}
	else
	{
		AddVertsForDisc2D(verts, m_position, m_definition->m_range, Rgba8(255, 255, 255, 127));
	}
	
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(verts);
}


//
//gameplay functions
//
void Tower::FindTarget()
{
	for (int bloonIndex = 0; bloonIndex < m_map->m_bloons.size(); bloonIndex++)
	{
		Bloon* const& bloon = m_map->m_bloons[bloonIndex];
		
		if (bloon != nullptr && DoDiscsOverlap(m_position, m_definition->m_range, bloon->m_position, bloon->m_definition->m_size))
		{
			switch (m_targetingMode)
			{
				case TargetingMode::FIRST:
				{
					if (m_target == nullptr || bloon->m_trackDistance > m_target->m_trackDistance)
					{
						m_target = bloon;
					}
					break;
				}
				case TargetingMode::LAST:
				{
					if (m_target == nullptr || bloon->m_trackDistance < m_target->m_trackDistance)
					{
						m_target = bloon;
					}
					break;
				}
				case TargetingMode::NEAR:
				{
					if (m_target == nullptr || GetDistanceSquared2D(m_position, bloon->m_position) < GetDistanceSquared2D(m_position, m_target->m_position))
					{
						m_target = bloon;
					}
					break;
				}
				case TargetingMode::STRONG:
				{
					if (m_target == nullptr || bloon->m_definition->m_RBE > m_target->m_definition->m_RBE)
					{
						m_target = bloon;
					}
					break;
				}
				case TargetingMode::WEAK:
				{
					if (m_target == nullptr || bloon->m_definition->m_RBE < m_target->m_definition->m_RBE)
					{
						m_target = bloon;
					}
					break;
				}
			}
		}
	}

	//if no target found, cooldown starts over
	if (m_target == nullptr)
	{
		m_cooldownTimer = 0.0f;
	}
}


void Tower::ShootProjectile()
{
	m_cooldownTimer = 0.0f;
	
	ProjectileDefinition const* projDef = m_definition->m_projectileDef;
	float angleBetweenProjectiles = 360.0f / m_definition->m_numProjectiles;
	for (int projIndex = 0; projIndex < m_definition->m_numProjectiles; projIndex++)
	{
		float degrees = projIndex * angleBetweenProjectiles;
		Vec2 direction = m_iBasis.GetRotatedDegrees(degrees);
		m_map->SpawnProjectile(projDef, m_position, direction, m_definition->m_addedPierce, m_definition->m_addedLifespan, m_definition->m_addedSize, m_definition->m_addedFreezeTime, 
			m_curvedProjArc);
	}
}


std::string Tower::GetTargetingModeAsString() const
{
	switch (m_targetingMode)
	{
		case TargetingMode::FIRST:
		{
			return "First";
		}
		case TargetingMode::LAST:
		{
			return "Last";
		}
		case TargetingMode::NEAR:
		{
			return "Near";
		}
		case TargetingMode::STRONG:
		{
			return "Strong";
		}
		case TargetingMode::WEAK:
		{
			return "Weak";
		}	
	}

	return "ERROR";
}
