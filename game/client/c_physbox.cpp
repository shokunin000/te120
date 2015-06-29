//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_physbox.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar fpb_TransitionDist("fpb_TransitionDist", "800"); //TE120

IMPLEMENT_CLIENTCLASS_DT(C_PhysBox, DT_PhysBox, CPhysBox)
	RecvPropFloat(RECVINFO(m_mass), 0), // Test..
	RecvPropFloat(RECVINFO(m_fDisappearDist)),//TE120
END_RECV_TABLE()


C_PhysBox::C_PhysBox()
{
	m_fDisappearDist = 0.0f;//TE120
}

//-----------------------------------------------------------------------------
// Should this object cast shadows?
//-----------------------------------------------------------------------------
ShadowType_t C_PhysBox::ShadowCastType()
{
	if (IsEffectActive(EF_NODRAW | EF_NOSHADOW))
		return SHADOWS_NONE;
	return SHADOWS_RENDER_TO_TEXTURE;
}

C_PhysBox::~C_PhysBox()
{
}

//TE120-------------------------------------------------------------
//-----------------------------------------------------------------------------
// Purpose: Calculate a fade.
//-----------------------------------------------------------------------------
unsigned char C_PhysBox::GetClientSideFade()
{
	if (m_fDisappearDist == 0.0)
		return 255;
	else
		return UTIL_ComputeEntityFade( this, m_fDisappearDist, m_fDisappearDist + fpb_TransitionDist.GetFloat(), 1.0f );
}
//TE120-------------------------

