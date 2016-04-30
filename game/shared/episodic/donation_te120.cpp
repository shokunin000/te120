//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opens TE120 donation page
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void OpenDonationPage_f( void )
{
  if ( steamapicontext && steamapicontext->SteamFriends() )
  {
    steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( "http://www.transmissions-element120.com/" );
  }
}

ConCommand te120_opendonationpage( "te120_opendonationpage", OpenDonationPage_f, "Opens the TE120 donation page.", 0 );
