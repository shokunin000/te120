//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TE120 combine ball - launched by physconcussion
//
//
//=============================================================================//

#include "cbase.h"
#include "prop_combine_ball.h"
#include "prop_gravity_ball.h"
#include "props.h"
#include "explode.h"
#include "saverestore_utlvector.h"
#include "hl2_shareddefs.h"
#include "materialsystem/imaterial.h"
#include "beam_flags.h"
#include "physics_prop_ragdoll.h"
#include "soundent.h"
#include "soundenvelope.h"
#include "te_effect_dispatch.h"
#include "ai_basenpc.h"
#include "npc_bullseye.h"
#include "filters.h"
#include "SpriteTrail.h"
#include "decals.h"
#include "hl2_player.h"
#include "eventqueue.h"
#include "physics_collisionevent.h"
#include "gamestats.h"
#include "weapon_physconcussion.h"
#include "util.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PROP_GRAVITY_BALL_MODEL	"models/effects/combineball_b.mdl"
#define PROP_GRAVITY_BALL_SPRITE_TRAIL "sprites/combineball_trail_black_1.vmt"

ConVar gravityball_tracelength( "gravityball_tracelength", "96" );
ConVar gravityball_magnitude( "gravityball_magnitude", "9000" );

// For our ring explosion
int s_nExplosionGBTexture = -1;

//-----------------------------------------------------------------------------
// Context think
//-----------------------------------------------------------------------------
static const char *s_pRemoveContext = "RemoveContext";

//-----------------------------------------------------------------------------
// Purpose:
// Input  : radius -
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CreateGravityBall( const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner, CWeaponPhysConcussion *pWeapon )
{
	CPropGravityBall *pBall = static_cast<CPropGravityBall*>( CreateEntityByName( "prop_gravity_ball" ) );
	pBall->m_pWeaponPC = pWeapon;
	pBall->SetRadius( radius );

	pBall->SetAbsOrigin( origin );
	pBall->SetOwnerEntity( pOwner );
	pBall->SetOriginalOwner( pOwner );

	pBall->SetAbsVelocity( velocity );
	pBall->Spawn();

	pBall->SetState( CPropCombineBall::STATE_THROWN );
	pBall->SetSpeed( velocity.Length() );

	pBall->EmitSound( "NPC_GravityBall.Launch" );

	PhysSetGameFlags( pBall->VPhysicsGetObject(), FVPHYSICS_WAS_THROWN );

	pBall->StartWhizSoundThink();

	pBall->SetMass( mass );
	pBall->StartLifetime( lifetime );
	pBall->SetWeaponLaunched( true );
	pBall->SetModel( PROP_GRAVITY_BALL_MODEL );

	return pBall;
}

//-----------------------------------------------------------------------------
// Purpose: Determines whether a physics object is a combine ball or not
// Input  : *pObj - Object to test
// Output : Returns true on success, false on failure.
// Notes  : This function cannot identify a combine ball that is held by
//			the physcannon because any object held by the physcannon is
//			COLLISIONGROUP_DEBRIS.
//-----------------------------------------------------------------------------
bool UTIL_IsGravityBall( CBaseEntity *pEntity )
{
	// Must be the correct collision group
	if ( pEntity->GetCollisionGroup() != HL2COLLISION_GROUP_COMBINE_BALL )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Determines whether a physics object is an AR2 combine ball or not
// Input  : *pEntity -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool UTIL_IsAR2GravityBall( CBaseEntity *pEntity )
{
	// Must be the correct collision group
	if ( pEntity->GetCollisionGroup() != HL2COLLISION_GROUP_COMBINE_BALL )
		return false;

	CPropGravityBall *pBall = dynamic_cast<CPropGravityBall *>(pEntity);

	if ( pBall && pBall->WasWeaponLaunched() )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Uses a deeper casting check to determine if pEntity is a combine
//			ball. This function exists because the normal (much faster) check
//			in UTIL_IsCombineBall() can never identify a combine ball held by
//			the physcannon because the physcannon changes the held entity's
//			collision group.
// Input  : *pEntity - Entity to check
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool UTIL_IsGravityBallDefinite( CBaseEntity *pEntity )
{
	CPropGravityBall *pBall = dynamic_cast<CPropGravityBall *>(pEntity);

	return pBall != NULL;
}

//-----------------------------------------------------------------------------
// Implementation of CPropCombineBall
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( prop_gravity_ball, CPropGravityBall );

//-----------------------------------------------------------------------------
// Save/load:
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CPropGravityBall )

	DEFINE_FIELD( m_flLastBounceTime, FIELD_TIME ),
	DEFINE_FIELD( m_flRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_nState, FIELD_CHARACTER ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_CLASSPTR ),
	DEFINE_SOUNDPATCH( m_pHoldingSound ),
	DEFINE_FIELD( m_bFiredGrabbedOutput, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEmit, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bHeld, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bLaunched, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bStruckEntity, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWeaponLaunched, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bForward, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flSpeed, FIELD_FLOAT ),

	DEFINE_FIELD( m_flNextDamageTime, FIELD_TIME ),
	DEFINE_FIELD( m_flLastCaptureTime, FIELD_TIME ),
	DEFINE_FIELD( m_bCaptureInProgress, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nBounceCount,	FIELD_INTEGER ),
	DEFINE_FIELD( m_nMaxBounces,	FIELD_INTEGER ),
	DEFINE_FIELD( m_bBounceDie,	FIELD_BOOLEAN ),


	DEFINE_FIELD( m_hSpawner, FIELD_EHANDLE ),

	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_THINKFUNC( WhizSoundThink ),
	DEFINE_THINKFUNC( DieThink ),
	DEFINE_THINKFUNC( DissolveThink ),
	DEFINE_THINKFUNC( DissolveRampSoundThink ),
	DEFINE_THINKFUNC( AnimThink ),
	DEFINE_THINKFUNC( CaptureBySpawner ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Explode", InputExplode ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FadeAndRespawn", InputFadeAndRespawn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Socketed", InputSocketed ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropGravityBall, DT_PropGravityBall )
	SendPropBool( SENDINFO( m_bEmit ) ),
	SendPropFloat( SENDINFO( m_flRadius ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bHeld ) ),
	SendPropBool( SENDINFO( m_bLaunched ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CPropGravityBall::Precache( void )
{
	// NOTENOTE: We don't call into the base class because it chains multiple
	//					 precaches we don't need to incur

	PrecacheModel( PROP_GRAVITY_BALL_MODEL );
	PrecacheModel( PROP_GRAVITY_BALL_SPRITE_TRAIL);

	s_nExplosionGBTexture = PrecacheModel( "sprites/lgtning.vmt" );

	PrecacheScriptSound( "NPC_GravityBall.Launch" );
	PrecacheScriptSound( "NPC_GravityBall.Explosion" );
	PrecacheScriptSound( "NPC_GravityBall.WhizFlyby" );

	if ( hl2_episodic.GetBool() )
	{
		PrecacheScriptSound( "NPC_CombineBall_Episodic.Impact" );
	}
	else
	{
		PrecacheScriptSound( "NPC_CombineBall.Impact" );
	}
}

//-----------------------------------------------------------------------------
// Spawn:
//-----------------------------------------------------------------------------
void CPropGravityBall::Spawn( void )
{
	CBaseAnimating::Spawn();

	SetModel( PROP_GRAVITY_BALL_MODEL );

	if( ShouldHitPlayer() )
	{
		// This allows the combine ball to hit the player.
		SetCollisionGroup( HL2COLLISION_GROUP_COMBINE_BALL_NPC );
	}
	else
	{
		SetCollisionGroup( HL2COLLISION_GROUP_COMBINE_BALL );
	}

	CreateVPhysics();

	Vector vecAbsVelocity = GetAbsVelocity();
	VPhysicsGetObject()->SetVelocity( &vecAbsVelocity, NULL );

	SetNState( STATE_NOT_THROWN );
	SetLastBounceTime( -1.0f );
	SetFiredGrabbedOutput( false );
	SetForward( true );
	SetCaptureInProgress( false );

	// No shadow!
	AddEffects( EF_NOSHADOW );

	// Start up the eye trail
	CSpriteTrail *pGlowTrail = CSpriteTrail::SpriteTrailCreate( PROP_GRAVITY_BALL_SPRITE_TRAIL, GetAbsOrigin(), false );
	SetGlowTrail( pGlowTrail );

	if ( pGlowTrail != NULL )
	{
		pGlowTrail->FollowEntity( this );
		pGlowTrail->SetTransparency( kRenderTransAdd, 0, 0, 0, 255, kRenderFxNone );
		pGlowTrail->SetStartWidth( m_flRadius );
		pGlowTrail->SetEndWidth( 0 );
		pGlowTrail->SetLifeTime( 0.1f );
		pGlowTrail->TurnOff();
	}

	SetEmit( true );
	SetHeld( false );
	SetLaunched( false );
	SetStruckEntity( false );
	SetWeaponLaunched( false );

	SetNextDamageTime( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Create vphysics
//-----------------------------------------------------------------------------
bool CPropGravityBall::CreateVPhysics()
{
	SetSolid( SOLID_BBOX );

	float flSize = m_flRadius;

	SetCollisionBounds( Vector(-flSize, -flSize, -flSize), Vector(flSize, flSize, flSize) );
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);
	int nMaterialIndex = physprops->GetSurfaceIndex("metal_bouncy");
	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject( flSize, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if ( !pPhysicsObject )
		return false;

	VPhysicsSetObject( pPhysicsObject );
	SetMoveType( MOVETYPE_VPHYSICS );
	pPhysicsObject->Wake();

	pPhysicsObject->SetMass( 750.0f );
	pPhysicsObject->EnableGravity( false );
	pPhysicsObject->EnableDrag( false );

	float flDamping = 0.0f;
	float flAngDamping = 0.5f;
	pPhysicsObject->SetDamping( &flDamping, &flAngDamping );
	pPhysicsObject->SetInertia( Vector( 1e30, 1e30, 1e30 ) );

	PhysSetGameFlags( pPhysicsObject, FVPHYSICS_NO_NPC_IMPACT_DMG );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropGravityBall::DoImpactEffect( const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent )
{
	// Do that crazy impact effect!
	trace_t tr;
	CollisionEventToTrace( !index, pEvent, tr );

	CBaseEntity *pTraceEntity = pEvent->pEntities[index];
	UTIL_TraceLine( tr.startpos - preVelocity * 2.0f, tr.startpos + preVelocity * 2.0f, MASK_SOLID, pTraceEntity, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0f )
	{
		// See if we hit the sky
		if ( tr.surface.flags & SURF_SKY )
		{
			DoExplosion();
			return;
		}

		// Send the effect over
		CEffectData	data;

		data.m_flRadius = 16;
		data.m_vNormal	= tr.plane.normal;
		data.m_vOrigin	= tr.endpos + tr.plane.normal * 1.0f;

		DispatchEffect( "gball_bounce", data );

		// We need to affect ragdolls on the client
		CEffectData	dataRag;
		dataRag.m_vOrigin = GetAbsOrigin();
		dataRag.m_flRadius = gravityball_tracelength.GetFloat();
		dataRag.m_flMagnitude = 1.0f;
		DispatchEffect( "RagdollConcussion", dataRag );
	}

	if ( hl2_episodic.GetBool() )
	{
		EmitSound( "NPC_CombineBall_Episodic.Impact" );
	}
	else
	{
		EmitSound( "NPC_CombineBall.Impact" );
	}
}

//-----------------------------------------------------------------------------
// Lighten the mass so it's zippy toget to the gun
//-----------------------------------------------------------------------------
void CPropGravityBall::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	/* Do nothing, don't want to allow phys gun pickup. */
	CDefaultPlayerPickupVPhysics::OnPhysGunPickup( pPhysGunUser, reason );
}

//------------------------------------------------------------------------------
// Pow!
//------------------------------------------------------------------------------
void CPropGravityBall::DoExplosion( )
{
	// don't do this twice
	if ( GetMoveType() == MOVETYPE_NONE )
		return;

	if ( PhysIsInCallback() )
	{
		g_PostSimulationQueue.QueueCall( this, &CPropGravityBall::DoExplosion );
		return;
	}

	//Shockring
	CBroadcastRecipientFilter filter2;

	EmitSound( "NPC_GravityBall.Explosion" );

	UTIL_ScreenShake( GetAbsOrigin(), 20.0f, 150.0, 1.0, 1250.0f, SHAKE_START );

	CEffectData data;

	data.m_vOrigin = GetAbsOrigin();

	te->BeamRingPoint( filter2, 0, GetAbsOrigin(),	//origin
		m_flRadius,	//start radius
		gravityball_tracelength.GetFloat() + 128.0,		//end radius
		s_nExplosionGBTexture, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.2f,		//life
		64,			//width
		0,			//spread
		0,			//amplitude
		255,	//r
		255,	//g
		225,	//b
		32,		//a
		0,		//speed
		FBEAM_FADEOUT
		);

	//Shockring
	te->BeamRingPoint( filter2, 0, GetAbsOrigin(),	//origin
		m_flRadius,	//start radius
		gravityball_tracelength.GetFloat() + 128.0,		//end radius
		s_nExplosionGBTexture, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.5f,		//life
		64,			//width
		0,			//spread
		0,			//amplitude
		255,	//r
		255,	//g
		225,	//b
		64,		//a
		0,		//speed
		FBEAM_FADEOUT
		);

	if( hl2_episodic.GetBool() )
	{
		CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_EXPLOSION, WorldSpaceCenter(), 180.0f, 0.25, this );
	}

	// Turn us off and wait because we need our trails to finish up properly
	SetAbsVelocity( vec3_origin );
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	SetEmitState( false );

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>( GetOwnerEntity() );

	if( !m_bStruckEntity && hl2_episodic.GetBool() && GetOwnerEntity() != NULL )
	{
		// Notify the player proxy that this combine ball missed so that it can fire an output.
		if ( pPlayer )
		{
			pPlayer->MissedAR2AltFire();
		}
	}

	SetContextThink( &CPropCombineBall::SUB_Remove, gpGlobals->curtime + 0.5f, s_pRemoveContext );
	StopLoopingSounds();

	// Gravity Push these mofos

	CBaseEntity *list[512];		// An array to store all the nearby gravity affected entities
	Vector start, end, forward;		// Vectors for traces if valid entity
	CBaseEntity *pEntity;

	int count = UTIL_EntitiesInSphere(list, 512, GetAbsOrigin(), gravityball_tracelength.GetFloat(), 0);

	// Loop through each entity and apply a force
	for ( int i = 0; i < count; i++ )
	{
		// Make sure the entity is valid or not itself or not the firing weapon
		pEntity = list[i];

		// Make sure the entity is valid or not itself or not the firing weapon
		if ( !pEntity || pEntity == this || pEntity == this->m_pWeaponPC )
			continue;

		// Make sure its a gravity touchable entity
		if ( (pEntity->IsEFlagSet( EFL_NO_PHYSCANNON_INTERACTION ) || pEntity->GetMoveType() != MOVETYPE_VPHYSICS) && ( pEntity->m_takedamage == DAMAGE_NO ) )
		{
			//DevMsg("Found invalid gravity entity %s\n", pEntity->GetClassname() );
			continue;
		}
		else
		{
			//DevMsg("Found valid gravity entity %s\n", pEntity->GetClassname() );

			// If anything changes here remember to duplciate in c_baseanimating.cpp
			start = GetAbsOrigin();
			end = pEntity->GetAbsOrigin();

			forward.x = end.x - start.x;
			if ( forward.x != 0 )
				forward.x /= gravityball_tracelength.GetFloat();

			forward.y = end.y - start.y;
			if ( forward.y != 0 )
				forward.y /= gravityball_tracelength.GetFloat();

			forward.z = end.z - start.z;
			// Skew the z direction upward
			forward.z += 44.0f;
			if ( forward.z != 0 )
				forward.z /= gravityball_tracelength.GetFloat();

			trace_t tr;
			UTIL_TraceLine( start,end, (MASK_SHOT|CONTENTS_GRATE), pEntity, COLLISION_GROUP_NONE, &tr );
			// debugoverlay->AddLineOverlay( start, end, 0,255,0, true, 18.0 );

			// Punt Non VPhysics Objects
			if ( pEntity->GetMoveType() != MOVETYPE_VPHYSICS )
			{
				// Amplify the height of the push
				forward.z *= 1.4f;

				if ( pEntity->IsNPC() && !pEntity->IsEFlagSet( EFL_NO_MEGAPHYSCANNON_RAGDOLL ) && pEntity->MyNPCPointer()->CanBecomeRagdoll() )
				{
					CTakeDamageInfo info( pPlayer, pPlayer, 1.0f, DMG_GENERIC );
					CBaseEntity *pRagdoll = CreateServerRagdoll( pEntity->MyNPCPointer(), 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true );
					PhysSetEntityGameFlags( pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS );
					pRagdoll->SetCollisionBounds( pEntity->CollisionProp()->OBBMins(), pEntity->CollisionProp()->OBBMaxs() );

					// Necessary to cause it to do the appropriate death cleanup
					CTakeDamageInfo ragdollInfo( pPlayer, pPlayer, 10000.0, DMG_PHYSGUN | DMG_REMOVENORAGDOLL );
					pEntity->TakeDamage( ragdollInfo );

					if ( m_pWeaponPC )
						m_pWeaponPC->PuntRagdoll( pRagdoll, forward, tr );
				}
				else if ( m_pWeaponPC )
				{
					m_pWeaponPC->PuntNonVPhysics( pEntity, forward, tr );
				}
			}
			else
			{
				if ( m_pWeaponPC->EntityAllowsPunts( pEntity ) == false )
				{
					continue;
				}

				if ( dynamic_cast<CRagdollProp*>(pEntity) )
				{
					// Amplify the height of the push
					forward.z *= 1.4f;
					if ( m_pWeaponPC )
						m_pWeaponPC->PuntRagdoll( pEntity, forward, tr );
				}
				else if ( m_pWeaponPC )
				{
						m_pWeaponPC->PuntVPhysics( pEntity, forward, tr );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPropGravityBall::IsHittableEntity( CBaseEntity *pHitEntity )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropGravityBall::OnHitEntity( CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent )
{
	DoExplosion();
}

//-----------------------------------------------------------------------------
// Deflects the ball toward enemies in case of a collision
//-----------------------------------------------------------------------------
void CPropGravityBall::DeflectTowardEnemy( float flSpeed, int index, gamevcollisionevent_t *pEvent )
{
	/* Do nothing, we just want this to explode. */
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropGravityBall::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	CPropCombineBall::VPhysicsCollision( index, pEvent );

	DoExplosion();
}
