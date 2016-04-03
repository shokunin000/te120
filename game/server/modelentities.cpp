//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "entityoutput.h"
#include "ndebugoverlay.h"
#include "modelentities.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar ent_debugkeys;
extern ConVar	showtriggers;



LINK_ENTITY_TO_CLASS( func_brush, CFuncBrush );

BEGIN_DATADESC( CFuncBrush )

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_KEYFIELD( m_iDisabled, FIELD_INTEGER, "StartDisabled" ),
	DEFINE_KEYFIELD( m_iSolidity, FIELD_INTEGER, "Solidity" ),
	DEFINE_KEYFIELD( m_bSolidBsp, FIELD_BOOLEAN, "solidbsp" ),
	DEFINE_KEYFIELD( m_iszExcludedClass, FIELD_STRING, "excludednpc" ),
	DEFINE_KEYFIELD( m_bInvertExclusion, FIELD_BOOLEAN, "invert_exclusion" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetExcluded", InputSetExcluded ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetInvert", InputSetInvert ),

END_DATADESC()


void CFuncBrush::Spawn( void )
{
	SetMoveType( MOVETYPE_PUSH );  // so it doesn't get pushed by anything

	SetSolid( SOLID_VPHYSICS );
	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

	if ( m_iSolidity == BRUSHSOLID_NEVER )
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	SetModel( STRING( GetModelName() ) );

	if ( m_iDisabled )
		TurnOff();

	// If it can't move/go away, it's really part of the world
	if ( !GetEntityName() || !m_iParent )
		AddFlag( FL_WORLDBRUSH );

	CreateVPhysics();

	// Slam the object back to solid - if we really want it to be solid.
	if ( m_bSolidBsp )
	{
		SetSolid( SOLID_BSP );
	}
}

//-----------------------------------------------------------------------------

bool CFuncBrush::CreateVPhysics( void )
{
	// NOTE: Don't init this static.  It's pretty common for these to be constrained
	// and dynamically parented.  Initing shadow avoids having to destroy the physics
	// object later and lose the constraints.
	IPhysicsObject *pPhys = VPhysicsInitShadow(false, false);
	if ( pPhys )
	{
		int contents = modelinfo->GetModelContents( GetModelIndex() );
		if ( ! (contents & (MASK_SOLID|MASK_PLAYERSOLID|MASK_NPCSOLID)) )
		{
			// leave the physics shadow there in case it has crap constrained to it
			// but disable collisions with it
			pPhys->EnableCollisions( false );
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CFuncBrush::DrawDebugTextOverlays( void )
{
	int nOffset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		Q_snprintf( tempstr,sizeof(tempstr), "angles: %g %g %g", (double)GetLocalAngles()[PITCH], (double)GetLocalAngles()[YAW], (double)GetLocalAngles()[ROLL] );
		EntityText( nOffset, tempstr, 0 );
		nOffset++;
	}

	return nOffset;
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling the hidden/shown state of the brush.
//-----------------------------------------------------------------------------
void CFuncBrush::InputToggle( inputdata_t &inputdata )
{
	if ( IsOn() )
	{
		TurnOff();
		return;
	}

	TurnOn();
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for hiding the brush.
//-----------------------------------------------------------------------------
void CFuncBrush::InputTurnOff( inputdata_t &inputdata )
{
	TurnOff();
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for showing the brush.
//-----------------------------------------------------------------------------
void CFuncBrush::InputTurnOn( inputdata_t &inputdata )
{
	TurnOn();
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CFuncBrush::InputSetExcluded( inputdata_t &inputdata )
{
	m_iszExcludedClass = inputdata.value.StringID();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CFuncBrush::InputSetInvert( inputdata_t &inputdata )
{
	m_bInvertExclusion = inputdata.value.Bool();
}


//-----------------------------------------------------------------------------
// Purpose: Hides the brush.
//-----------------------------------------------------------------------------
void CFuncBrush::TurnOff( void )
{
	if ( !IsOn() )
		return;

	if ( m_iSolidity != BRUSHSOLID_ALWAYS )
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	AddEffects( EF_NODRAW );
	m_iDisabled = TRUE;
}


//-----------------------------------------------------------------------------
// Purpose: Shows the brush.
//-----------------------------------------------------------------------------
void CFuncBrush::TurnOn( void )
{
	if ( IsOn() )
		return;

	if ( m_iSolidity != BRUSHSOLID_NEVER )
	{
		RemoveSolidFlags( FSOLID_NOT_SOLID );
	}

	RemoveEffects( EF_NODRAW );
}


bool CFuncBrush::IsOn( void )
{
	return !IsEffectActive( EF_NODRAW );
}


//-----------------------------------------------------------------------------
// Purpose: Invisible field that activates when touched
//			All inputs are passed up to the main entity, unless filtered out
//-----------------------------------------------------------------------------
// DVS TODO: obsolete, remove
class CTriggerBrush : public CBaseEntity
{
	//
	// Filters controlling what this trigger responds to.
	//
	enum TriggerFilters_e
	{
		TRIGGER_IGNOREPLAYERS	= 0x01,
		TRIGGER_IGNORENPCS		= 0x02,
		TRIGGER_IGNOREPUSHABLES	= 0x04,
		TRIGGER_IGNORETOUCH		= 0x08,
		TRIGGER_IGNOREUSE		= 0x10,
		TRIGGER_IGNOREALL		= 0x20,
	};

public:
	DECLARE_CLASS( CTriggerBrush, CBaseEntity );

	// engine inputs
	void Spawn( void );
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// input filtering (use/touch/blocked)
	bool PassesInputFilter( CBaseEntity *pOther, int filter );

	// input functions
	void InputEnable( inputdata_t &inputdata )
	{
		RemoveFlag( FL_DONTTOUCH );
	}

	void InputDisable( inputdata_t &inputdata )
	{
		// this ensures that all the remaining EndTouch() calls still get passed through
		AddFlag( FL_DONTTOUCH );
	}

	// outputs
	COutputEvent m_OnStartTouch;
	COutputEvent m_OnEndTouch;
	COutputEvent m_OnUse;

	// data
	int m_iInputFilter;
	int m_iDontMessageParent;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( trigger_brush, CTriggerBrush );

BEGIN_DATADESC( CTriggerBrush )

	DEFINE_KEYFIELD( m_iInputFilter, FIELD_INTEGER, "InputFilter" ),
	DEFINE_KEYFIELD( m_iDontMessageParent, FIELD_INTEGER, "DontMessageParent" ),

	DEFINE_OUTPUT( m_OnStartTouch, "OnStartTouch" ),
	DEFINE_OUTPUT( m_OnEndTouch, "OnEndTouch" ),
	DEFINE_OUTPUT( m_OnUse, "OnUse" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

END_DATADESC()


void CTriggerBrush::Spawn( void )
{
	SetSolid( SOLID_BSP );
	AddSolidFlags( FSOLID_TRIGGER );
	SetMoveType( MOVETYPE_NONE );

	SetModel( STRING( GetModelName() ) );    // set size and link into world

	if ( !showtriggers.GetInt() )
	{
		AddEffects( EF_NODRAW );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - the entity that is now touching us.
//-----------------------------------------------------------------------------
void CTriggerBrush::StartTouch( CBaseEntity *pOther )
{
	if ( PassesInputFilter(pOther, m_iInputFilter) && !(m_iInputFilter & TRIGGER_IGNORETOUCH) )
	{
		m_OnStartTouch.FireOutput( pOther, this );
		if ( !m_iDontMessageParent )
			BaseClass::StartTouch( pOther );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - the entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerBrush::EndTouch( CBaseEntity *pOther )
{
	if ( PassesInputFilter(pOther, m_iInputFilter) && !(m_iInputFilter & TRIGGER_IGNORETOUCH) )
	{
		m_OnEndTouch.FireOutput( pOther, this );

		if ( !m_iDontMessageParent )
			BaseClass::EndTouch( pOther );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when we are triggered by another entity or used by the player.
// Input  : pActivator -
//			pCaller -
//			useType -
//			value -
//-----------------------------------------------------------------------------
void CTriggerBrush::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( PassesInputFilter(pActivator, m_iInputFilter) && !(m_iInputFilter & TRIGGER_IGNOREUSE) )
	{
		m_OnUse.FireOutput( pActivator, this );
		if ( !m_iDontMessageParent )
		{
			BaseClass::Use( pActivator, pCaller, useType, value );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Checks an entity input, against a filter and the current entity
// Input  : *pOther - the entity that is part of the input (touch/untouch/block/use)
//			filter - a field of standard filters (TriggerFilters_e)
// Output : Returns true if the input passes, false if it should be ignored
//-----------------------------------------------------------------------------
bool CTriggerBrush::PassesInputFilter( CBaseEntity *pOther, int filter )
{
	if ( !filter )
		return true;

	// check for players
	if ( (filter & TRIGGER_IGNOREPLAYERS) && pOther->IsPlayer() )
		return false;

	// NPCs
	if ( (filter & TRIGGER_IGNORENPCS) && pOther->edict() && (pOther->GetFlags() & FL_NPC) )
		return false;

	// pushables
	if ( (filter & TRIGGER_IGNOREPUSHABLES) && FStrEq(STRING(pOther->m_iClassname), "func_pushable") )
		return false;

	return true;
}

//TE120--
#define DT_BRUSH_GLOW 0.01f

LINK_ENTITY_TO_CLASS( func_brushglow, CFuncBrushGlow );

BEGIN_DATADESC( CFuncBrushGlow )

	// Function pointers
	DEFINE_THINKFUNC( IllumThink ),
	DEFINE_THINKFUNC( UpdateThink ),

	// Fields
	DEFINE_FIELD( m_bAbsorbing, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fIsIlluminated, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flAbsorbRate, FIELD_FLOAT ),
	DEFINE_FIELD( m_flEmitRate, FIELD_FLOAT ),
	DEFINE_KEYFIELD( m_bStartOff, FIELD_BOOLEAN, "StartOff"),
	DEFINE_KEYFIELD( m_flAbsorbTime, FIELD_FLOAT, "AbsorbTime"),
	DEFINE_KEYFIELD( m_flEmitTime, FIELD_FLOAT, "EmitTime"),

	// Outputs
	DEFINE_OUTPUT( m_OnIlluminated, "OnIlluminated" ),
	DEFINE_OUTPUT( m_OnNotIlluminated, "OnNotIlluminated" ),
	DEFINE_OUTPUT( m_Energy, "Energy" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "EnergyAbsorb", InputAbsorbEnergy ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnergyEmit", InputEmitEnergy ),

END_DATADESC()

//------------------------------------------------------------------------------
// Purpose : Init
//------------------------------------------------------------------------------
void CFuncBrushGlow::Spawn()
{
	// Find our desired transition time, divide by delta to find change rate.
	m_flAbsorbRate = DT_BRUSH_GLOW / m_flAbsorbTime;
	m_flEmitRate = DT_BRUSH_GLOW / m_flEmitTime;

	m_fIsIlluminated = false;

	if ( m_bStartOff )
	{
		m_bAbsorbing = false;
		m_Energy.Set(0.0f, UTIL_GetLocalPlayer(), this);
	}
	else
	{
		m_bAbsorbing = true;
		m_Energy.Set(1.0f, UTIL_GetLocalPlayer(), this);
	}

	RegisterThinkContext( "IllumContext" );
	SetContextThink( &CFuncBrushGlow::IllumThink, gpGlobals->curtime + 0.01f, "IllumContext" );

	BaseClass::Spawn();
}


//------------------------------------------------------------------------------
// Purpose : Begin building energy towards 1.0
// Input   : m_Energy
//------------------------------------------------------------------------------
void CFuncBrushGlow::InputAbsorbEnergy( inputdata_t &inputdata )
{
	m_bAbsorbing = true;

	// Use think to start building up energy
	SetThink( &CFuncBrushGlow::UpdateThink );
	SetNextThink( gpGlobals->curtime + DT_BRUSH_GLOW );
}

//------------------------------------------------------------------------------
// Purpose : Begin building energy towards 0.0
// Input   : m_Energy
//------------------------------------------------------------------------------
void CFuncBrushGlow::InputEmitEnergy( inputdata_t &inputdata )
{
	m_bAbsorbing = false;

	// Use think to start building up energy
	SetThink( &CFuncBrushGlow::UpdateThink );
	SetNextThink( gpGlobals->curtime + DT_BRUSH_GLOW );
}

//-----------------------------------------------------------------------------
// Purpose: Think function for absorbing and emitting energy
//-----------------------------------------------------------------------------
void CFuncBrushGlow::UpdateThink( void )
{
	VPROF_BUDGET( "CVisibleShadowList::GlowCalc", VPROF_BUDGETGROUP_GLOWCALC );

	float fCurrentEnergy = m_Energy.Get();

	if ( m_bAbsorbing && fCurrentEnergy < 1.0f )
	{
		DevMsg( "Absorbing Energy %f at %f rate.\n", fCurrentEnergy, m_flAbsorbRate );

		fCurrentEnergy = clamp(fCurrentEnergy + m_flAbsorbRate, 0.0f, 1.0f);

		// Change at absorb rate every think until we reach 1.0
		m_Energy.Set(fCurrentEnergy, UTIL_GetLocalPlayer(), this);

		// If less than 1.0 then continue thinking
		if ( fCurrentEnergy < 1.0f )
			SetNextThink( gpGlobals->curtime + DT_BRUSH_GLOW );
	}
	else if ( fCurrentEnergy > 0.0f )
	{
		DevMsg( "Emitting Energy %f at %f rate.\n", fCurrentEnergy, m_flEmitRate );

		fCurrentEnergy = clamp(fCurrentEnergy - m_flEmitRate, 0.0f, 1.0f);

		// Change at emit rate every think until we reach 0.0
		m_Energy.Set(fCurrentEnergy, UTIL_GetLocalPlayer(), this);

		// If greater than 0.0 then continue thinking
		if ( fCurrentEnergy > 0.0f )
			SetNextThink( gpGlobals->curtime + DT_BRUSH_GLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function for outputing illumination events
//-----------------------------------------------------------------------------
void CFuncBrushGlow::IllumThink( void )
{
	VPROF_BUDGET( "CVisibleShadowList::IllumThink", VPROF_BUDGETGROUP_GLOWCALC );

	if ( ( UTIL_FindClientInPVS( edict() ) != NULL ) || ( UTIL_ClientPVSIsExpanded() && UTIL_FindClientInVisibilityPVS( edict() ) ) )
	{
		CheckIlluminated();

		// Think constantly
		SetNextThink(gpGlobals->curtime + 0.01f, "IllumContext" );
	}
	else
	{
		// Think every 1 second
		SetNextThink(gpGlobals->curtime + 1.0f, "IllumContext" );
	}
}

void CFuncBrushGlow::CheckIlluminated()
{
	VPROF_BUDGET( "CVisibleShadowList::CheckIlluminated", VPROF_BUDGETGROUP_GLOWCALC );

	// If we're being illuminated by the flashlight send output
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
 	if ( pPlayer )
	{
		float fDot;

		pPlayer->IsIlluminatedByFlashlight( this, &fDot );

		DevMsg( "fDot: %f\n", fDot );

		// Is this glow entity within a 15 degree cone and visible?
		if ( fDot > 0.96f && pPlayer->FVisible(this) )
		{
			if ( !m_fIsIlluminated )
			{
				m_fIsIlluminated = true;

				// Send output that I am illuminated
				m_OnIlluminated.FireOutput( this, this );
				DevMsg( "I am illuminated!\n" ); //Debug
			}
		}
		else if ( m_fIsIlluminated )
		{
			m_fIsIlluminated = false;

			// Send out that I am no longer illuminated
			m_OnNotIlluminated.FireOutput(this, this);

			DevMsg( "I am no longer illuminated!\n" ); //Debug
		}
	}
}
//TE120--
