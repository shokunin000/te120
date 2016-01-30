//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TE120 combine ball - launched by physconcussion
//
//=============================================================================//

#ifndef PROP_GRAVITY_BALL_H
#define PROP_GRAVITY_BALL_H
#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "player_pickup.h"	// for combine ball inheritance
#include "prop_combine_ball.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CWeaponPhysConcussion;

//-----------------------------------------------------------------------------
// Looks for enemies, bounces a max # of times before it breaks
//-----------------------------------------------------------------------------
class CPropGravityBall : public CPropCombineBall
{
	DECLARE_CLASS( CPropGravityBall, CPropCombineBall );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:

	CWeaponPhysConcussion * m_pWeaponPC;

	virtual void Precache();
	virtual void Spawn();
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual bool CreateVPhysics();

private:

	virtual void OnHitEntity( CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent );
	virtual void DoImpactEffect( const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent );
	virtual void DeflectTowardEnemy( float flSpeed, int index, gamevcollisionevent_t *pEvent );
	virtual bool IsHittableEntity( CBaseEntity *pHitEntity );

protected:
	// Pow!
	virtual void DoExplosion( );
};

// Creates a gravity ball
CBaseEntity *CreateGravityBall( const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner, CWeaponPhysConcussion *pWeapon );

// Query function to find out if a physics object is a combine ball (used for collision checks)
bool UTIL_IsGravityBall( CBaseEntity *pEntity );
bool UTIL_IsGravityBallDefinite( CBaseEntity *pEntity );
bool UTIL_IsAR2GravityBall( CBaseEntity *pEntity );

#endif // PROP_GRAVITY_BALL_H
