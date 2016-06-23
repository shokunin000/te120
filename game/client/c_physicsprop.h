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
#ifdef GLOWS_ENABLE
#include "glow_outline_effect.h"
#endif // GLOWS_ENABLE

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
#ifdef GLOWS_ENABLE
	bool m_bEnableGlow;

private:
	bool m_bClientGlow;
	int m_GlowObjectHandle;
#endif // GLOWS_ENABLE
//TE120--
};

#endif // C_PHYSICSPROP_H
