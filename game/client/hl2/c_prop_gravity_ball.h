//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef CPROPGRAVITYBALL_H_
#define CPROPGRAVITYBALL_H_

#ifdef _WIN32
#pragma once
#endif

#include "c_prop_combine_ball.h"

class C_PropGravityBall : public C_PropCombineBall
{
	DECLARE_CLASS( C_PropGravityBall, C_PropCombineBall );
	DECLARE_CLIENTCLASS();
public:

	C_PropGravityBall( void );

	virtual RenderGroup_t GetRenderGroup( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

protected:

	void	DrawMotionBlur( void );
	void	DrawFlicker( void );
	virtual bool	InitMaterials( void );

	Vector	m_vecLastOrigin;
	bool	m_bEmit;
	float	m_flRadius;
	bool	m_bHeld;
	bool	m_bLaunched;

	IMaterial	*m_pFlickerMaterial;
	IMaterial	*m_pBodyMaterial;
	IMaterial	*m_pBlurMaterial;
};


#endif
