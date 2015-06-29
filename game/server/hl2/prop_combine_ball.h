//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PROP_COMBINE_BALL_H
#define PROP_COMBINE_BALL_H
#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "player_pickup.h"	// for combine ball inheritance

//TE120-----
#define PROP_COMBINE_BALL_MODEL	"models/effects/combineball.mdl"
#define PROP_COMBINE_BALL_SPRITE_TRAIL "sprites/combineball_trail_black_1.vmt" 

#define PROP_COMBINE_BALL_LIFETIME	4.0f	// Seconds

#define PROP_COMBINE_BALL_HOLD_DISSOLVE_TIME	8.0f

#define SF_COMBINE_BALL_BOUNCING_IN_SPAWNER		0x10000

#define	MAX_COMBINEBALL_RADIUS	12


//-----------------------------------------------------------------------------
//
// Spawns combine balls
//
//-----------------------------------------------------------------------------
#define SF_SPAWNER_START_DISABLED 0x1000
#define SF_SPAWNER_POWER_SUPPLY 0x2000

//-----------------------------------------------------------------------------
// Context think
//-----------------------------------------------------------------------------
static const char *s_pWhizThinkContext = "WhizThinkContext";
static const char *s_pHoldDissolveContext = "HoldDissolveContext";
static const char *s_pExplodeTimerContext = "ExplodeTimerContext";
static const char *s_pAnimThinkContext = "AnimThinkContext";
static const char *s_pCaptureContext = "CaptureContext";
static const char *s_pRemoveContext = "RemoveContext";
//TE120-----

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CFuncCombineBallSpawner;
class CSpriteTrail;

//-----------------------------------------------------------------------------
// Looks for enemies, bounces a max # of times before it breaks
//-----------------------------------------------------------------------------
class CPropCombineBall : public CBaseAnimating, public CDefaultPlayerPickupVPhysics
{
public:
	DECLARE_CLASS( CPropCombineBall, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void Precache();
	virtual void Spawn();
	virtual void UpdateOnRemove();
	void StopLoopingSounds();

	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	virtual bool OverridePropdata();
	virtual bool CreateVPhysics();

	CFuncCombineBallSpawner *GetSpawner();

	virtual void ExplodeThink( void );

	// Override of IPlayerPickupVPhysics;
	virtual bool ShouldPuntUseLaunchForces( PhysGunForce_t reason ) { return ( reason == PHYSGUN_FORCE_PUNTED ); }

	void SetRadius( float flRadius );
	void SetSpeed( float flSpeed ) { m_flSpeed = flSpeed; }
	float GetSpeed( void ) const { return m_flSpeed; }

	void CaptureBySpawner( );
	bool IsBeingCaptured() const { return m_bCaptureInProgress; }

	void ReplaceInSpawner( float flSpeed );

	// Input
	void InputExplode( inputdata_t &inputdata );
	void InputFadeAndRespawn( inputdata_t &inputdata );
	void InputKill( inputdata_t &inputdata );
	void InputSocketed( inputdata_t &inputdata );

	enum
	{
		STATE_NOT_THROWN = 0,
		STATE_HOLDING,
		STATE_THROWN,
		STATE_LAUNCHED, //by a combine_ball launcher
	};

	void SetState( int state );
	bool IsInField() const;

	void StartWhizSoundThink( void );

	void StartLifetime( float flDuration );
	void ClearLifetime( );
	void SetMass( float mass );

	void SetWeaponLaunched( bool state = true ) { m_bWeaponLaunched = state; m_bLaunched = state; }
	bool WasWeaponLaunched( void ) const { return m_bWeaponLaunched; }

	bool WasFiredByNPC() const { return (GetOwnerEntity() && GetOwnerEntity()->IsNPC()); }

	bool ShouldHitPlayer() const;

	virtual CBasePlayer *HasPhysicsAttacker( float dt );

	void	SetSpawner( CFuncCombineBallSpawner *pSpawner ) { m_hSpawner = pSpawner; }
	void	NotifySpawnerOfRemoval( void );


	float	LastCaptureTime() const;

	unsigned char GetState() const { return m_nState;	}

	int  NumBounces( void ) const { return m_nBounceCount; }

	void SetMaxBounces( int iBounces )
	{
		m_nMaxBounces = iBounces;
	}

	void SetEmitState( bool bEmit )
	{
		m_bEmit = bEmit;
	}
//TE120-----
	void SetFiredGrabbedOutput( bool bFiredGrabbedOutput )
	{
		m_bFiredGrabbedOutput = bFiredGrabbedOutput;
	}

	void SetForward( bool bForward )
	{
		m_bForward = bForward;
	}

	void SetCaptureInProgress( bool bCaptureInProgress )
	{
		m_bCaptureInProgress = bCaptureInProgress;
	}

	void SetEmit( bool bEmit )
	{
		m_bEmit = bEmit;
	}

	void SetHeld( bool bHeld )
	{
		m_bHeld = bHeld;
	}

	void SetLaunched( bool bLaunched )
	{
		m_bLaunched = bLaunched;
	}

	void SetStruckEntity( bool bStruckEntity )
	{
		m_bStruckEntity = bStruckEntity;
	}

	void SetNextDamageTime( float flNextDamageTime )
	{
		m_flNextDamageTime = flNextDamageTime;
	}

	void SetGlowTrail( CSpriteTrail *pGlowTrail )
	{
		m_pGlowTrail = pGlowTrail;
	}

	CSpriteTrail *GetGlowTrail() { return m_pGlowTrail; }
//TE120-----
	void SetOriginalOwner( CBaseEntity *pEntity ) { m_hOriginalOwner = pEntity; }
//TE120-----	
	void SetNState( char nState )
	{
		m_nState = nState;
	}
	
	void SetLastBounceTime( float flLastBounceTime )
	{
		m_flLastBounceTime = flLastBounceTime;
	}
//TE120-----
	CBaseEntity *GetOriginalOwner() { return m_hOriginalOwner; }

private:

	void SetPlayerLaunched( CBasePlayer *pOwner );

	float GetBallHoldDissolveTime();
	float GetBallHoldSoundRampTime();
	//TE120----- removed void DoExplosion( );
	void StartAnimating( void );
	void StopAnimating( void );

	void SetBallAsLaunched( void );
	//TE120----- removed void CollisionEventToTrace
	bool DissolveEntity( CBaseEntity *pEntity );
	virtual void OnHitEntity( CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent );//TE120-----
	virtual void DoImpactEffect( const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent );//TE120-----

	// Bounce inside the spawner: 
	void BounceInSpawner( float flSpeed, int index, gamevcollisionevent_t *pEvent );

	bool IsAttractiveTarget( CBaseEntity *pEntity );

	// Deflects the ball toward enemies in case of a collision 
	virtual void DeflectTowardEnemy( float flSpeed, int index, gamevcollisionevent_t *pEvent );//TE120-----

	// Is this something we can potentially dissolve? 
	virtual bool IsHittableEntity( CBaseEntity *pHitEntity );//TE120-----

	// Sucky. 
	void WhizSoundThink();
	void DieThink();
	void DissolveThink();
	void DissolveRampSoundThink();
	void AnimThink( void );

	void FadeOut( float flDuration );
//TE120----- removed
private:

	int		m_nBounceCount;
	int		m_nMaxBounces;
	bool	m_bBounceDie;

	float	m_flLastBounceTime;

	bool	m_bFiredGrabbedOutput;
	//TE120----- removed m_bStruckEntity;
	bool	m_bWeaponLaunched;		// Means this was fired from the AR2
	bool	m_bForward;				// Movement direction in ball spawner

	unsigned char m_nState;
	bool	m_bCaptureInProgress;

	float	m_flSpeed;

	CSpriteTrail *m_pGlowTrail;
	CSoundPatch *m_pHoldingSound;

	float	m_flNextDamageTime;
	float	m_flLastCaptureTime;

	CHandle < CFuncCombineBallSpawner > m_hSpawner;

	EHANDLE m_hOriginalOwner;

	CNetworkVar( bool, m_bEmit );
	CNetworkVar( bool, m_bHeld );
	CNetworkVar( bool, m_bLaunched );
//TE120-----	
protected:

	// Pow!
	virtual void DoExplosion( );
	void CollisionEventToTrace( int index, gamevcollisionevent_t *pEvent, trace_t &tr );

	bool OutOfBounces( void ) const
	{
		return ( m_nState == STATE_LAUNCHED && m_nMaxBounces != 0 && m_nBounceCount >= m_nMaxBounces );
	}

	bool	m_bStruckEntity;		// Has hit an entity already (control accuracy)
//TE120-----
	CNetworkVar( float, m_flRadius );
};

class CFuncCombineBallSpawner : public CBaseEntity
{
	DECLARE_CLASS( CFuncCombineBallSpawner, CBaseEntity );
	DECLARE_DATADESC();

public:
	CFuncCombineBallSpawner();

	virtual void Spawn();
	virtual void Precache();

	// Balls call this to figure out where to bounce to
	void GetTargetEndpoint( bool bForward, Vector *pVecEndpoint );

	// Balls call this when they've been removed from the spawner
	void RespawnBall( float flRespawnTime );
	void RespawnBallPostExplosion( void );

	// Fire ball grabbed output
	void BallGrabbed( CBaseEntity *pEntity );

	// Get speed of ball to place into the field
	float GetBallSpeed( ) const;

	// Register that a reflection occurred
	void RegisterReflection( CPropCombineBall *pBall, bool bForward );

	// Spawn a ball
	virtual void SpawnBall();

private:

	// Choose a random point inside the cylinder
	void ChoosePointInCylinder( Vector *pVecPoint );

	// Choose a random point inside the box
	void ChoosePointInBox( Vector *pVecPoint );

	// Used to determine when to respawn balls
	void BallThink();

	// Input
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	// Fire ball grabbed output
	void	GrabBallTouch( CBaseEntity *pOther );

public:
	bool m_bShooter;
	float m_flBallRadius;
	float m_flBallRespawnTime;
	float m_flMinSpeed;
	float m_flMaxSpeed;

private:
	CUtlVector< float > m_BallRespawnTime;
	int m_nBallCount;
	int m_nBallsRemainingInField;
	float m_flRadius;
	float m_flDisableTime;
	bool m_bEnabled;

	COutputEvent m_OnBallGrabbed;
	COutputEvent m_OnBallReinserted;
	COutputEvent m_OnBallHitTopSide;
	COutputEvent m_OnBallHitBottomSide;
	COutputEvent m_OnLastBallGrabbed;
	COutputEvent m_OnFirstBallReinserted;
};


class CPointCombineBallLauncher : public CFuncCombineBallSpawner
{
	DECLARE_CLASS( CPointCombineBallLauncher, CFuncCombineBallSpawner );

	DECLARE_DATADESC();

public:

	virtual void Spawn( void );
	virtual void SpawnBall( void );
	void InputLaunchBall ( inputdata_t &inputdata );

	CPointCombineBallLauncher();

private:

	int			m_iBounces;
	float		m_flConeDegrees;
	string_t	m_iszBullseyeName;
};

// Creates a combine ball
CBaseEntity *CreateCombineBall( const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner );

// Query function to find out if a physics object is a combine ball (used for collision checks)
bool UTIL_IsCombineBall( CBaseEntity *pEntity );
bool UTIL_IsCombineBallDefinite( CBaseEntity *pEntity );
bool UTIL_IsAR2CombineBall( CBaseEntity *pEntity );

#endif // PROP_COMBINE_BALL_H
