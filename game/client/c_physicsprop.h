//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef C_PHYSICSPROP_H
#define C_PHYSICSPROP_H
#ifdef _WIN32
#pragma once
#endif

#include "c_breakableprop.h"
#include "ge_screeneffects.h"//TE120

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class C_PhysicsProp : public C_BreakableProp
{
	typedef C_BreakableProp BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_PhysicsProp();
	~C_PhysicsProp();

	virtual bool OnInternalDrawModel( ClientModelRenderInfo_t *pInfo );

protected:
	// Networked vars.
	bool m_bAwake;
	bool m_bAwakeLastTime;
//TE120--
	bool m_bEnableGlow;

private:
	CEntGlowEffect *m_pEntGlowEffect;
	bool m_bClientGlow;
//TE120--
};

#endif // C_PHYSICSPROP_H
