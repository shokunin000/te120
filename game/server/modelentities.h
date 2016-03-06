//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MODELENTITIES_H
#define MODELENTITIES_H
#ifdef _WIN32
#pragma once
#endif

//!! replace this with generic start enabled/disabled
#define SF_WALL_START_OFF		0x0001
#define SF_IGNORE_PLAYERUSE		0x0002

//-----------------------------------------------------------------------------
// Purpose: basic solid geometry
// enabled state:	brush is visible
// disabled staute:	brush not visible
//-----------------------------------------------------------------------------
class CFuncBrush : public CBaseEntity
{
public:
	DECLARE_CLASS( CFuncBrush, CBaseEntity );

	virtual void Spawn( void );
	bool CreateVPhysics( void );

	virtual int	ObjectCaps( void ) { return HasSpawnFlags(SF_IGNORE_PLAYERUSE) ? BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION : (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE) & ~FCAP_ACROSS_TRANSITION; }//TE120

	virtual int DrawDebugTextOverlays( void );

	void TurnOff( void );
	void TurnOn( void );

	// Input handlers
	void InputTurnOff( inputdata_t &inputdata );
	void InputTurnOn( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputSetExcluded( inputdata_t &inputdata );
	void InputSetInvert( inputdata_t &inputdata );

	enum BrushSolidities_e {
		BRUSHSOLID_TOGGLE = 0,
		BRUSHSOLID_NEVER  = 1,
		BRUSHSOLID_ALWAYS = 2,
	};

	BrushSolidities_e m_iSolidity;
	int m_iDisabled;
	bool m_bSolidBsp;
	string_t m_iszExcludedClass;
	bool m_bInvertExclusion;

	DECLARE_DATADESC();

	virtual bool IsOn( void );
};

//-----------------------------------------------------------------------------
// Purpose: Used for glowing qr codes
//-----------------------------------------------------------------------------
class CFuncBrushGlow : public CFuncBrush
{
	DECLARE_CLASS( CFuncBrushGlow, CFuncBrush );

public:

	void	Spawn();

	void	UpdateThink();
	void	IllumThink();

	//---------------------------------
	//	Inputs
	//---------------------------------
	void	InputAbsorbEnergy( inputdata_t &inputdata );
	void	InputEmitEnergy( inputdata_t &inputdata );

	DECLARE_DATADESC();

	bool			m_bAbsorbing;
	bool			m_bStartOff;
	bool			m_fIsIlluminated;	// is this brush illuminated by the player flashlight?
	float			m_flAbsorbTime;
	float			m_flEmitTime;

	//---------------------------------
	//	Outputs
	//---------------------------------
	COutputFloat	m_Energy;
	COutputEvent	m_OnIlluminated;
	COutputEvent	m_OnNotIlluminated;

private:

	void	CheckIlluminated();

	float			m_flAbsorbRate;
	float			m_flEmitRate;
};

#endif // MODELENTITIES_H
