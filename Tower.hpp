#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"
#include <string>


class TowerDefinition;
class Bloon;
class Map;


enum class TargetingMode
{
	FIRST,
	LAST,
	NEAR,
	STRONG,
	WEAK,

	NUM_TARGETING_MODES
};


class Tower
{
//public member functions
public:
	Tower(TowerDefinition const* definition, Map* map, Vec2 const& position)
		: m_definition(definition)
		, m_map(map)
		, m_position(position)
	{}

	//game flow functions
	void Update(float deltaSeconds);
	void Render() const;
	void RenderRange(bool redRange = false) const;

	//gameplay functions
	void FindTarget();
	void ShootProjectile();
	std::string GetTargetingModeAsString() const;

//public member variables
public:
	TowerDefinition const* m_definition = nullptr;
	Map* m_map = nullptr;

	Vec2  m_position = Vec2();
	Vec2  m_iBasis = Vec2(0.0f, -1.0f);

	float m_cooldownTimer = 0.0f;

	Bloon const* m_target = nullptr;
	TargetingMode m_targetingMode = TargetingMode::FIRST;

	bool m_isBeingHeld = false;

	CubicBezierCurve2D m_curvedProjArc = CubicBezierCurve2D();
};
