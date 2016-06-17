//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Provides specific convars for TE120
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

void OpenURL_f()
{
	if ( steamapicontext && steamapicontext->SteamFriends() )
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("https://www.youcaring.com/TE120andCarebox");
	else
		DevMsg("Missing Steam API context, failed to execute: te120_opendonationpage\n");
}
ConCommand te120_opendonationpage( "te120_opendonationpage", OpenURL_f, "Open TE120 donation page in Steam Overlay.", 0 );

void OpenAchievements_f()
{
	if ( steamapicontext && steamapicontext->SteamFriends() )
		steamapicontext->SteamFriends()->ActivateGameOverlay("Achievements");
	else
		DevMsg("Missing Steam API context, failed to execute: te120_openachievements\n");
}
ConCommand te120_openachievements( "te120_openachievements", OpenAchievements_f, "Open TE120 Achievements in Steam Overlay.", 0 );
