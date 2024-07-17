#pragma once
#include "Game/DamageTypes.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"


class BloonDefinition;
class Map;
class Projectile;


class Bloon
{
//public member functions
public:
	Bloon(BloonDefinition const* definition, Map const* map, CubicBezierCurve2D const* startingCurve, float trackDistance = 0.0f);

	//game flow functions
	void Update(float deltaSeconds);
	void Render() const;

	//gameplay functions
	void TakeDamage(Projectile& damageSource);
	void Pop(Projectile& popper);
	void Leak();

//public member variables
public:
	BloonDefinition const* m_definition = nullptr;
	Map const* m_map = nullptr;

	Vec2  m_position = Vec2();

	CubicBezierCurve2D const* m_currentSplineCurve = nullptr;
	int m_splineCurveIndex = 0;
	float m_trackDistance = 0.0f;

	int  m_currentHealth = 1;

	//bool m_isFrozen = false;
	//bool m_isGlued = false;

	float m_freezeTimer = 0.0f;

	bool m_hasPopped = false;
	bool m_hasLeaked = false;

	Projectile* m_popper = nullptr;
};
