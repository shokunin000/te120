//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: spawn and think functions for editor-placed lights
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "lights.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( light, CLight );

BEGIN_DATADESC( CLight )

	DEFINE_FIELD( m_iCurrentFade, FIELD_CHARACTER),
	DEFINE_FIELD( m_iTargetFade, FIELD_CHARACTER),

	DEFINE_KEYFIELD( m_iStyle, FIELD_INTEGER, "style" ),
	DEFINE_KEYFIELD( m_iDefaultStyle, FIELD_INTEGER, "defaultstyle" ),
	DEFINE_KEYFIELD( m_iszPattern, FIELD_STRING, "pattern" ),

	// Fuctions
	DEFINE_FUNCTION( FadeThink ),
	DEFINE_FUNCTION( ToggleBugThink ),//TE120

	// Inputs
	DEFINE_INPUTFUNC( FIELD_STRING, "SetPattern", InputSetPattern ),
	DEFINE_INPUTFUNC( FIELD_FLOAT,	"SetLightValue", InputSetLightValue ),//TE120
	DEFINE_INPUTFUNC( FIELD_STRING, "FadeToPattern", InputFadeToPattern ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"TurnOff", InputTurnOff ),

END_DATADESC()



//
// Cache user-entity-field values until spawn is called.
//
bool CLight::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "pitch"))
	{
		QAngle angles = GetAbsAngles();
		angles.x = atof(szValue);
		SetAbsAngles( angles );
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}

// Light entity
// If targeted, it will toggle between on or off.
void CLight::Spawn( void )
{
	if (!GetEntityName())
	{       // inert light
		UTIL_Remove( this );
		return;
	}
	
	if (m_iStyle >= 32)
	{
		if ( m_iszPattern == NULL_STRING && m_iDefaultStyle > 0 )
		{
			m_iszPattern = MAKE_STRING(GetDefaultLightstyleString(m_iDefaultStyle));
		}

		if (FBitSet(m_spawnflags, SF_LIGHT_START_OFF))
			engine->LightStyle(m_iStyle, "a");
		else if (m_iszPattern != NULL_STRING)
			engine->LightStyle(m_iStyle, (char *)STRING( m_iszPattern ));
		else
			engine->LightStyle(m_iStyle, "m");
	}
}
//TE120----
void CLight::Activate( void )
{
	if (!GetEntityName())
	{
		BaseClass::Activate();
	}
	else
	{
		m_bRequestedToggleOff = false;

		// Find switch lights that are on & toggle them off & on to fix perf bug. This is hacky & bad
		// but I'm not sure what else to do without access to the engine to debug.
		if ( !FBitSet( m_spawnflags, SF_LIGHT_START_OFF ) )
		{
			m_bRequestedToggleOff = true;
			SetThink(&CLight::ToggleBugThink);
			SetNextThink( gpGlobals->curtime + 0.4 );
		}
	}

	BaseClass::Activate();
}

void CLight::ToggleBugThink( void )
{
	// Light is on and we requested it off, turn it off
	if ( !FBitSet( m_spawnflags, SF_LIGHT_START_OFF ) && m_bRequestedToggleOff )
	{
		m_bRequestedToggleOff = false;
		TurnOff();
		m_bRequestedToggleOn = true;
		SetThink(&CLight::ToggleBugThink);
		SetNextThink( gpGlobals->curtime + 0.1 );
		return;
	}
	else if (m_bRequestedToggleOff) // Light was requested off but the player already did it 
	{
		m_bRequestedToggleOff = false;
		return;
	}

	// We turned the light off and want to turn it back on here to fix perf bug
	if ( FBitSet( m_spawnflags, SF_LIGHT_START_OFF ) && m_bRequestedToggleOn )
	{
		m_bRequestedToggleOn = false;
		TurnOn();
	}
	else if (m_bRequestedToggleOn)
	{
		// Here the player managed to turn the light back on before we did, but it was probably intended to be turned off.
		m_bRequestedToggleOn = false;
		TurnOff();
	}

}//TE120----

void CLight::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (m_iStyle >= 32)
	{
		if ( !ShouldToggle( useType, !FBitSet(m_spawnflags, SF_LIGHT_START_OFF) ) )
			return;

		Toggle();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Turn the light on
//-----------------------------------------------------------------------------
void CLight::TurnOn( void )
{
	if ( m_iszPattern != NULL_STRING )
	{
		engine->LightStyle( m_iStyle, (char *) STRING( m_iszPattern ) );
	}
	else
	{
		engine->LightStyle( m_iStyle, "m" );
	}

	CLEARBITS( m_spawnflags, SF_LIGHT_START_OFF );
}

//-----------------------------------------------------------------------------
// Purpose: Turn the light off
//-----------------------------------------------------------------------------
void CLight::TurnOff( void )
{
	engine->LightStyle( m_iStyle, "a" );
	SETBITS( m_spawnflags, SF_LIGHT_START_OFF );
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the light on/off
//-----------------------------------------------------------------------------
void CLight::Toggle( void )
{
	//Toggle it
	if ( FBitSet( m_spawnflags, SF_LIGHT_START_OFF ) )
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle the "turnon" input handler
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CLight::InputTurnOn( inputdata_t &inputdata )
{
	TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Handle the "turnoff" input handler
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CLight::InputTurnOff( inputdata_t &inputdata )
{
	TurnOff();
}

//-----------------------------------------------------------------------------
// Purpose: Handle the "toggle" input handler
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CLight::InputToggle( inputdata_t &inputdata )
{
	Toggle();
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for setting a light pattern
//-----------------------------------------------------------------------------
void CLight::InputSetPattern( inputdata_t &inputdata )
{
	m_iszPattern = inputdata.value.StringID();
	engine->LightStyle(m_iStyle, (char *)STRING( m_iszPattern ));

	// Light is on if pattern is set
	CLEARBITS(m_spawnflags, SF_LIGHT_START_OFF);
}
//TE120----
//-----------------------------------------------------------------------------
// Purpose: Input handler for fading a light
//-----------------------------------------------------------------------------
void CLight::InputSetLightValue( inputdata_t &inputdata )
{
	float fTemp = clamp( inputdata.value.Float(), 0, 1 );
	fTemp *= 26;

	switch ( Ceil2Int(fTemp) )
	{
	case 26:
		engine->LightStyle( m_iStyle, "z" );
		CLEARBITS( m_spawnflags, SF_LIGHT_START_OFF );
		break;
	case 25:
		engine->LightStyle( m_iStyle, "y" );
		break;
	case 24:
		engine->LightStyle( m_iStyle, "x" );
		break;
	case 23:
		engine->LightStyle( m_iStyle, "w" );
		break;
	case 22:
		engine->LightStyle( m_iStyle, "v" );
		break;
	case 21:
		engine->LightStyle( m_iStyle, "u" );
		break;
	case 20:
		engine->LightStyle( m_iStyle, "t" );
		break;
	case 19:
		engine->LightStyle( m_iStyle, "s" );
		break;
	case 18:
		engine->LightStyle( m_iStyle, "r" );
		break;
	case 17:
		engine->LightStyle( m_iStyle, "q" );
		break;
	case 16:
		engine->LightStyle( m_iStyle, "p" );
		break;
	case 15:
		engine->LightStyle( m_iStyle, "o" );
		break;
	case 14:
		engine->LightStyle( m_iStyle, "n" );
		break;
	case 13:
		engine->LightStyle( m_iStyle, "m" );
		break;
	case 12:
		engine->LightStyle( m_iStyle, "l" );
		break;
	case 11:
		engine->LightStyle( m_iStyle, "k" );
		break;
	case 10:
		engine->LightStyle( m_iStyle, "j" );
		break;
	case 9:
		engine->LightStyle( m_iStyle, "i" );
		break;
	case 8:
		engine->LightStyle( m_iStyle, "h" );
		break;
	case 7:
		engine->LightStyle( m_iStyle, "g" );
		break;
	case 6:
		engine->LightStyle( m_iStyle, "f" );
		break;
	case 5:
		engine->LightStyle( m_iStyle, "e" );
		break;
	case 4:
		engine->LightStyle( m_iStyle, "d" );
		break;
	case 3:
		engine->LightStyle( m_iStyle, "c" );
		break;
	case 2:
		engine->LightStyle( m_iStyle, "b" );
		break;
	case 1:
	default:
		engine->LightStyle( m_iStyle, "a" );
		SETBITS( m_spawnflags, SF_LIGHT_START_OFF );
		break;
	}
}
//TE120----

//-----------------------------------------------------------------------------
// Purpose: Input handler for fading from first value in old pattern to 
//			first value in new pattern
//-----------------------------------------------------------------------------
void CLight::InputFadeToPattern( inputdata_t &inputdata )
{
	m_iCurrentFade	= (STRING(m_iszPattern))[0];
	m_iTargetFade	= inputdata.value.String()[0];
	m_iszPattern	= inputdata.value.StringID();
	SetThink(&CLight::FadeThink);
	SetNextThink( gpGlobals->curtime );

	// Light is on if pattern is set
	CLEARBITS(m_spawnflags, SF_LIGHT_START_OFF);
}


//------------------------------------------------------------------------------
// Purpose : Fade light to new starting pattern value then stop thinking
//------------------------------------------------------------------------------
void CLight::FadeThink(void)
{
	if (m_iCurrentFade < m_iTargetFade)
	{
		m_iCurrentFade++;
	}
	else if (m_iCurrentFade > m_iTargetFade)
	{
		m_iCurrentFade--;
	}

	// If we're done fading instantiate our light pattern and stop thinking
	if (m_iCurrentFade == m_iTargetFade)
	{
		engine->LightStyle(m_iStyle, (char *)STRING( m_iszPattern ));
		SetNextThink( TICK_NEVER_THINK );
	}
	// Otherwise instantiate our current fade value and keep thinking
	else
	{
		char sCurString[2];
		sCurString[0] = m_iCurrentFade;
		sCurString[1] = 0;
		engine->LightStyle(m_iStyle, sCurString);

		// UNDONE: Consider making this settable war to control fade speed
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//
// shut up spawn functions for new spotlights
//
LINK_ENTITY_TO_CLASS( light_spot, CLight );
LINK_ENTITY_TO_CLASS( light_glspot, CLight );


class CEnvLight : public CLight
{
public:
	DECLARE_CLASS( CEnvLight, CLight );

	bool	KeyValue( const char *szKeyName, const char *szValue ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_environment, CEnvLight );

bool CEnvLight::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "_light"))
	{
		// nothing
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}


void CEnvLight::Spawn( void )
{
	BaseClass::Spawn( );
}
