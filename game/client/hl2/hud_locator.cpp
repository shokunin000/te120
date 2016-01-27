//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hud locator element, helps direct the player to objects in the world
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "c_basehlplayer.h"
//TE120--
#include "hl2_vehicle_radar.h"
#include "hud_locator.h"
//TE120--

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//TE120--
#define LOCATOR_MATERIAL_GENERIC		"vgui/icons/icon_lambda"
#define LOCATOR_MATERIAL_AMMO			"vgui/icons/icon_ammo"
#define LOCATOR_MATERIAL_HEALTH			"vgui/icons/icon_health"
#define LOCATOR_MATERIAL_LARGE_ENEMY	"vgui/icons/icon_strider"
//TE120--
#define LOCATOR_MATERIAL_BIG_TICK		"vgui/icons/tick_long"
#define LOCATOR_MATERIAL_SMALL_TICK		"vgui/icons/tick_short"
//TE120--
#define LOCATOR_MATERIAL_BIG_TICK_N		"vgui/icons/tick_long_n"
#define LOCATOR_MATERIAL_RADIATION		"vgui/icons/icon_radiation"
//TE120--

ConVar hud_locator_alpha( "hud_locator_alpha", "230" );
ConVar hud_locator_fov("hud_locator_fov", "350" );

using namespace vgui;

#ifdef HL2_EPISODIC
//TE120--
static CHudLocator *s_Locator = NULL;

CHudLocator *GetHudLocator()
{
	return s_Locator;
}
//TE120--

DECLARE_HUDELEMENT( CHudLocator );
#endif
//TE120--
CLocatorContact::CLocatorContact()
{
	m_iType = 0;
	m_pEnt = NULL;
}
//TE120--

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudLocator::CHudLocator( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudLocator" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );

//TE120--
	m_textureID_IconGeneric = -1;
	m_textureID_IconAmmo = -1;
	m_textureID_IconHealth = -1;
	m_textureID_IconBigEnemy = -1;
//TE120--
	m_textureID_IconSmallTick = -1;
	m_textureID_IconBigTick = -1;
//TE120--
	m_textureID_IconBigTickN = -1;
	m_textureID_IconRadiation = -1;

	m_iNumlocatorContacts = 0;

	s_Locator = this;
//TE120--
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pScheme -
//-----------------------------------------------------------------------------
void CHudLocator::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHudLocator::VidInit( void )
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHudLocator::ShouldDraw( void )
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;
//TE120--
	// Need the HEV suit ( HL2 )
	if ( !pPlayer->IsSuitEquipped() )
		return false;

	return true;
//TE120--
}

//-----------------------------------------------------------------------------
// Purpose: Start with our background off
//-----------------------------------------------------------------------------
void CHudLocator::Reset( void )
{
	m_vecLocation = Vector( 0, 0, 0 );
	m_iNumlocatorContacts = 0;//TE120
}

//-----------------------------------------------------------------------------
// Purpose: Make it a bit more convenient to do a filled rect.
//-----------------------------------------------------------------------------
void CHudLocator::FillRect( int x, int y, int w, int h )
{
	int panel_x, panel_y, panel_w, panel_h;
	GetBounds( panel_x, panel_y, panel_w, panel_h );
	vgui::surface()->DrawFilledRect( x, y, x+w, y+h );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CHudLocator::LocatorXPositionForYawDiff( float yawDiff )
{
	float fov = hud_locator_fov.GetFloat() / 2;
	float remappedAngle = RemapVal( yawDiff, -fov, fov, -90, 90 );
	float cosine = sin(DEG2RAD(remappedAngle));
	int element_wide = GetWide();

	float position = (element_wide>>1) + ((element_wide>>1) * cosine);

	return position;
}

//-----------------------------------------------------------------------------
// Draw the tickmarks on the locator
//-----------------------------------------------------------------------------
#define NUM_GRADUATIONS	16.0f
void CHudLocator::DrawGraduations( float flYawPlayerFacing )
{
	int icon_wide, icon_tall;
	int xPos, yPos;
	float fov = hud_locator_fov.GetFloat() / 2;

	if( m_textureID_IconBigTick == -1 )
	{
		m_textureID_IconBigTick = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconBigTick, LOCATOR_MATERIAL_BIG_TICK, true, false );

		m_textureID_IconSmallTick = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconSmallTick, LOCATOR_MATERIAL_SMALL_TICK, true, false );

		//TE120--
		m_textureID_IconBigTickN = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconBigTickN, LOCATOR_MATERIAL_BIG_TICK_N, true, false );
		//TE120--
	}

	int element_tall = GetTall();		// Height of the VGUI element

	surface()->DrawSetColor( 255, 255, 255, 255 );

	// Tick Icons

	float angleStep = 360.0f / NUM_GRADUATIONS;
	bool tallLine = true;

	for( float angle = -180 ; angle <= 180 ; angle += angleStep )
	{
		yPos = (element_tall>>1);

		if( tallLine )
		{
			//TE120--
			if (angle == -180 || angle == 180)
			{
				vgui::surface()->DrawSetTexture( m_textureID_IconBigTickN );
				vgui::surface()->DrawGetTextureSize( m_textureID_IconBigTickN, icon_wide, icon_tall );
			}
			else
			{
				vgui::surface()->DrawSetTexture( m_textureID_IconBigTick );
				vgui::surface()->DrawGetTextureSize( m_textureID_IconBigTick, icon_wide, icon_tall );
			}
			//TE120--

			tallLine = false;
		}
		else
		{
			vgui::surface()->DrawSetTexture( m_textureID_IconSmallTick );
			vgui::surface()->DrawGetTextureSize( m_textureID_IconSmallTick, icon_wide, icon_tall );
			tallLine = true;
		}

		float flDiff = UTIL_AngleDiff( flYawPlayerFacing, angle );

		if( fabs(flDiff) > fov )
			continue;

		float xPosition = LocatorXPositionForYawDiff( flDiff );

		xPos = (int)xPosition;
		xPos -= (icon_wide>>1);

		vgui::surface()->DrawTexturedRect(xPos, yPos, xPos+icon_wide, yPos+icon_tall);
	}
}

//TE120--
ConVar locator_range_start( "locator_range_start", "150" ); // 60 feet
ConVar locator_range_end( "locator_range_end", "1200" ); // 90 feet
//TE120--

//-----------------------------------------------------------------------------
// Purpose: draws the locator icons on the VGUI element.
//-----------------------------------------------------------------------------
void CHudLocator::Paint()
{
#ifdef HL2_EPISODIC
//TE120--
	if( m_textureID_IconGeneric == -1 )
	{
		m_textureID_IconGeneric = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconGeneric, LOCATOR_MATERIAL_GENERIC, true, false );

		m_textureID_IconAmmo = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconAmmo, LOCATOR_MATERIAL_AMMO, true, false );

		m_textureID_IconHealth = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconHealth, LOCATOR_MATERIAL_HEALTH, true, false );

		m_textureID_IconBigEnemy = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconBigEnemy, LOCATOR_MATERIAL_LARGE_ENEMY, true, false );

		m_textureID_IconRadiation = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_IconRadiation, LOCATOR_MATERIAL_RADIATION, true, false );
	}
//TE120--

	int alpha = hud_locator_alpha.GetInt();

	SetAlpha( alpha );

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	int element_tall = GetTall();		// Height of the VGUI element

	float fov = (hud_locator_fov.GetFloat()) / 2.0f;

	// Compute the relative position of objects we're tracking
	// We'll need the player's yaw for comparison.
	float flYawPlayerForward = pPlayer->EyeAngles().y;

//TE120--
	// Draw compass
	DrawGraduations( flYawPlayerForward );
	surface()->DrawSetColor( 255, 255, 255, 255 );

	// Draw icons, if any
	for ( int i = 0; i < pPlayer->m_HL2Local.m_iNumLocatorContacts; i++ )
	{
		// Copy this value out of the member variable in case we decide to expand this
		// feature later and want to iterate some kind of list.
		EHANDLE ent = pPlayer->m_HL2Local.m_locatorEnt[i];

		if ( ent )
		{
			Vector vecLocation = ent->GetAbsOrigin();

			Vector vecToLocation = vecLocation - pPlayer->GetAbsOrigin();
			QAngle locationAngles;

			VectorAngles( vecToLocation, locationAngles );
			float yawDiff = UTIL_AngleDiff( flYawPlayerForward, locationAngles.y );
			bool bObjectInFOV = (yawDiff > -fov && yawDiff < fov);

			// Draw the icons!
			int icon_wide, icon_tall;
			int xPos, yPos;

			if ( bObjectInFOV )
			{
				Vector vecPos = ent->GetAbsOrigin();
				float x_diff = vecPos.x - pPlayer->GetAbsOrigin().x;
				float y_diff = vecPos.y - pPlayer->GetAbsOrigin().y;
				float flDist = sqrt( ((x_diff)*(x_diff) + (y_diff)*(y_diff)) );

				if ( flDist <= locator_range_end.GetFloat() )
				{
					// The object's location maps to a valid position along the tape, so draw an icon.
					float tapePosition = LocatorXPositionForYawDiff(yawDiff);
					// Msg("tapePosition: %f\n", tapePosition);
					pPlayer->m_HL2Local.m_flTapePos[i] = tapePosition;

					// derive a scale for the locator icon
					yawDiff = fabs(yawDiff);
					float scale = 0.55f;
					float xscale = RemapValClamped( yawDiff, (fov/4), fov, 1.0f, 0.25f );

					switch (pPlayer->m_HL2Local.m_iLocatorContactType[i])
					{
						case LOCATOR_CONTACT_GENERIC:
							vgui::surface()->DrawSetTexture( m_textureID_IconGeneric );
							vgui::surface()->DrawGetTextureSize( m_textureID_IconGeneric, icon_wide, icon_tall );
							break;
						case LOCATOR_CONTACT_AMMO:
							vgui::surface()->DrawSetTexture( m_textureID_IconAmmo );
							vgui::surface()->DrawGetTextureSize( m_textureID_IconAmmo, icon_wide, icon_tall );
							break;
						case LOCATOR_CONTACT_HEALTH:
							vgui::surface()->DrawSetTexture( m_textureID_IconHealth );
							vgui::surface()->DrawGetTextureSize( m_textureID_IconHealth, icon_wide, icon_tall );
							break;
						case LOCATOR_CONTACT_LARGE_ENEMY:
							vgui::surface()->DrawSetTexture( m_textureID_IconBigEnemy );
							vgui::surface()->DrawGetTextureSize( m_textureID_IconBigEnemy, icon_wide, icon_tall );
							break;
						case LOCATOR_CONTACT_RADIATION:
							vgui::surface()->DrawSetTexture( m_textureID_IconRadiation );
							vgui::surface()->DrawGetTextureSize( m_textureID_IconRadiation, icon_wide, icon_tall );
							break;
						default:
							break;
					}

					float flIconWide = ((float)element_tall * 1.25f);
					float flIconTall = ((float)element_tall * 1.25f);

					// Scale the icon based on distance
					float flDistScale = 1.0f;
					if( flDist > locator_range_start.GetFloat() )
					{
						flDistScale = RemapValClamped( flDist, locator_range_start.GetFloat(), locator_range_end.GetFloat(), 1.0f, 0.5f );
					}

					// Put back into ints
					icon_wide = (int)flIconWide;
					icon_tall = (int)flIconTall;

					// Positon Scale
					icon_wide *= xscale;

					// Distance Scale Icons
					icon_wide *= flDistScale;
					icon_tall *= flDistScale;

					// Global Scale Icons
					icon_wide *= scale;
					icon_tall *= scale;
					//Msg("yawDiff:%f  xPos:%d  scale:%f\n", yawDiff, xPos, scale );

					// Center the icon around its position.
					xPos = (int)tapePosition;
					xPos -= (icon_wide >> 1);
					yPos = (element_tall >> 1) - (icon_tall >> 1);

					// If this overlaps the last drawn items reduce opacity
					float fMostOverlapDist = 14.0f;
					if ( pPlayer->m_HL2Local.m_iLocatorContactType[i] != LOCATOR_CONTACT_LARGE_ENEMY )
					{
						for ( int j = i - 1; j >= 0; j-- )
						{
							EHANDLE lastEnt = pPlayer->m_HL2Local.m_locatorEnt[j];
							if ( lastEnt.IsValid() )
							{
								if ( pPlayer->m_HL2Local.m_flTapePos[j] > 0 )
								{
									float fDiff = abs(pPlayer->m_HL2Local.m_flTapePos[j] - tapePosition);
									if ( fMostOverlapDist > fDiff )
										fMostOverlapDist = fDiff;
								}
							}
						}
					}

					// Msg("fMostOverlapDist: %f\n", fMostOverlapDist );
					if ( fMostOverlapDist < 14.0f )
					{
						int numOpacity = (int)(32.0f + fMostOverlapDist * 9.6f);
						// Msg("numOpacity: %d\n", numOpacity );

						surface()->DrawSetColor( 255, 255, 255, numOpacity );
					}
					else
					{
						surface()->DrawSetColor( 255, 255, 255, 255 );
					}

					//Msg("Drawing at %f %f\n", x, y );
					vgui::surface()->DrawTexturedRect(xPos, yPos-7, xPos + icon_wide, yPos + icon_tall - 7);
				}
			}
			else
			{
				pPlayer->m_HL2Local.m_flTapePos[i] = -1.0f;
			}
		}
	}

	MaintainLocatorContacts();
//TE120--
#endif // HL2_EPISODIC
}
//TE120--
//---------------------------------------------------------
// Purpose: Register a radar contact in the list of contacts
//---------------------------------------------------------
void CHudLocator::AddLocatorContact( EHANDLE hEnt, int iType )
{
	if( m_iNumlocatorContacts == LOCATOR_MAX_CONTACTS )
	{
		return;
	}

	if ( !hEnt )
	{
		return;
	}

	int iExistingContact = FindLocatorContact( hEnt );
	if ( iExistingContact == -1 )
	{
		m_locatorContacts[m_iNumlocatorContacts].m_pEnt = hEnt;
		m_locatorContacts[m_iNumlocatorContacts].m_iType = iType;
		m_iNumlocatorContacts++;
	}
}

//---------------------------------------------------------
// Purpose: Search the contact list for a specific contact
//---------------------------------------------------------
int CHudLocator::FindLocatorContact( EHANDLE hEnt )
{
	for ( int i = 0; i < m_iNumlocatorContacts; i++ )
	{
		if ( m_locatorContacts[ i ].m_pEnt == hEnt )
			return i;
	}

	return -1;
}

//---------------------------------------------------------
// Purpose: Go through all radar targets and see if any
//			have expired. If yes, remove them from the
//			list.
//---------------------------------------------------------
void CHudLocator::MaintainLocatorContacts()
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	for( int i = 0; i < m_iNumlocatorContacts; i++ )
	{
		// If I don't exist... remove me
		if ( m_locatorContacts[ i ].m_pEnt )
		{
			// If I'm too far, remove me
			Vector vecPos = m_locatorContacts[ i ].m_pEnt->GetAbsOrigin();

			float x_diff = vecPos.x - pPlayer->GetAbsOrigin().x;
			float y_diff = vecPos.y - pPlayer->GetAbsOrigin().y;
			float flDist = sqrt( ((x_diff)*(x_diff) + (y_diff)*(y_diff)) );

			if( flDist > locator_range_end.GetFloat() )
			{
				// Time for this guy to go. Easiest thing is just to copy the last element
				// into this element's spot and then decrement the count of entities.
				m_locatorContacts[ i ] = m_locatorContacts[ m_iNumlocatorContacts - 1 ];
				m_iNumlocatorContacts--;
				break;
			}
		}
		else
		{
			// Time for this guy to go. Easiest thing is just to copy the last element
			// into this element's spot and then decrement the count of entities.
			m_locatorContacts[ i ] = m_locatorContacts[ m_iNumlocatorContacts - 1 ];
			m_iNumlocatorContacts--;
			break;
		}
	}
}
//TE120--
