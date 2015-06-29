//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef LIGHTS_H
#define LIGHTS_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CLight : public CServerOnlyEntity//TE120
{
public:
	DECLARE_CLASS( CLight, CServerOnlyEntity );//TE120

	bool	KeyValue( const char *szKeyName, const char *szValue );
	void	Spawn( void );
	virtual void Activate( void );//TE120
	void	FadeThink( void );
	void	ToggleBugThink( void );//TE120
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	void	TurnOn( void );
	void	TurnOff( void );
	void	Toggle( void );

	// Input handlers
	void	InputSetPattern( inputdata_t &inputdata );
	void	InputFadeToPattern( inputdata_t &inputdata );
	void	InputSetLightValue( inputdata_t &inputdata );//TE120

	void	InputToggle( inputdata_t &inputdata );
	void	InputTurnOn( inputdata_t &inputdata );
	void	InputTurnOff( inputdata_t &inputdata );

	DECLARE_DATADESC();

private:
	int		m_iStyle;
	int		m_iDefaultStyle;
	string_t m_iszPattern;
	char	m_iCurrentFade;
	char	m_iTargetFade;
	bool	m_bRequestedToggleOff;//TE120
	bool	m_bRequestedToggleOn;//TE120
};

#endif // LIGHTS_H
