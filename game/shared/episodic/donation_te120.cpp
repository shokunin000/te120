//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opens TE120 donation page
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

static void OpenURLCallback( IConVar *pConVar, const char *oldValue, float flOldValue )
{
	ConVarRef var( pConVar );
	const char *pURLString = var.GetString();

	if (  Q_strlen( pURLString ) != 0 && (!Q_strncmp( pURLString, "http://", 7 ) || !Q_strncmp( pURLString, "https://", 8 )) )
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(pURLString);
	else
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("https://www.redditdonate.com/donate/34");
}
static ConVar s_cl_openurl("s_cl_openurl", "https://www.redditdonate.com/donate/34", 0, "Open URL in Steam", OpenURLCallback );