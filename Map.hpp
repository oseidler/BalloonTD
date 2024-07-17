#pragma once
#include "Game/MapDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"


class Bloon;
class BloonDefinition;
class Tower;
class Projectile;
class ProjectileDefinition;


class Map
{
//public member functions
public:
	Map(MapDefinition const* definition)
		: m_definition(definition), m_trackSpline(definition->m_trackSpline) 
	{}
	~Map();

	//game flow functions
	void Update(float deltaSeconds);
	void Render() const;

	//gameplay functions
	void SpawnBloonAtStart(BloonDefinition const* bloonDef);
	void SpawnBloonChildren(Bloon const* bloon);
	void SpawnProjectile(ProjectileDefinition const* projectileDef, Vec2 const& position, Vec2 const& direction, int addedPierce = 0, float addedLifespan = 0.0f, float addedSize = 0.0f,
		float addedFreezeTime = 0.0f, CubicBezierCurve2D curvedProjArc = CubicBezierCurve2D());
	void CollideProjectilesAgainstBloons();
	bool CollideProjectileAgainstBloon(Projectile& projectile, Bloon& bloon);
	void SellTower(int towerIndex);
	Vec2 GetNearestPointOnTrack(Vec2 const& referencePoint) const;

//public member variables
public:
	MapDefinition const* m_definition = nullptr;

	std::vector<CubicBezierCurve2D> m_trackSpline;

	std::vector<Bloon*>		 m_bloons;
	std::vector<Tower*>		 m_towers;
	std::vector<Projectile*> m_projectiles;
};
