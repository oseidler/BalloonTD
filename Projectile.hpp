#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"


class ProjectileDefinition;
class Bloon;
class Map;


class Projectile
{
//public member functions
public:
	Projectile(ProjectileDefinition const* definition, Map* map, Vec2 const& position, Vec2 const& direction, int addedPierce = 0, float addedLifespan = 0.0f, float addedSize = 0.0f,
		float addedFreezeTime = 0.0f, CubicBezierCurve2D curvedProjArc = CubicBezierCurve2D());

	//game flow functions
	void Update(float deltaSeconds);
	void Render() const;

	//gameplay functions
	void DeductPierce();
	void DieFromLifespan();
	void DieFromPierce();

//public member variables
public:
	ProjectileDefinition const* m_definition = nullptr;
	Map* m_map = nullptr;

	Vec2 m_position = Vec2();
	Vec2 m_velocity = Vec2();

	float m_remainingLifespan = 0.0f;
	int   m_remainingPierce = 0;
	float m_addedFreezeTime = 0.0f;
	float m_size = 0.0f;

	bool m_outOfLifespan = false;
	bool m_outOfPierce = false;

	std::vector<Bloon*> m_bloonsToPassOver;

	CubicBezierCurve2D m_curvedProjArc = CubicBezierCurve2D();
	float m_curvedArcDistance = 0.0f;
	float m_directionDegrees = 0.0f;
};
