//========= Copyright, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef HUD_LOCATOR_H
#define HUD_LOCATOR_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "hl2_vehicle_radar.h"

class CLocatorContact
{
public:
	int		m_iType;
	EHANDLE	m_pEnt;

	CLocatorContact();
};

//-----------------------------------------------------------------------------
// Purpose: Shows positions of objects relative to the player.
//-----------------------------------------------------------------------------
class CHudLocator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudLocator, vgui::Panel );

public:
	CHudLocator( const char *pElementName );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	void VidInit( void );
	bool ShouldDraw();

	void AddLocatorContact( EHANDLE hEnt, int iType );
	int FindLocatorContact( EHANDLE hEnt );
	void MaintainLocatorContacts();
	void ClearAllLocatorContacts()	{ m_iNumlocatorContacts = 0; }

protected:
	void FillRect( int x, int y, int w, int h );
	float LocatorXPositionForYawDiff( float yawDiff );
	void DrawGraduations( float flYawPlayerFacing );
	virtual void Paint();

private:
	void Reset( void );

	int m_textureID_IconGeneric;
	int m_textureID_IconAmmo;
	int m_textureID_IconHealth;
	int m_textureID_IconBigEnemy;
	int m_textureID_IconBigTick;
	int m_textureID_IconSmallTick;
	int m_textureID_IconBigTickN;
	int m_textureID_IconRadiation;

	Vector			m_vecLocation;
	CLocatorContact	m_locatorContacts[LOCATOR_MAX_CONTACTS];
	int				m_iNumlocatorContacts;
	// float			flNextPaintTime;
};

extern CHudLocator *GetHudLocator();

#endif // HUD_LOCATOR_H
