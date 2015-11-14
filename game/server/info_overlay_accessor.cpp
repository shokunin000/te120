//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------- //
// An entity used to access overlays (and change their texture)
// -------------------------------------------------------------------------------- //

class CInfoOverlayAccessor : public CServerOnlyEntity//TE120
{
public:

	DECLARE_CLASS( CInfoOverlayAccessor, CServerOnlyEntity );//TE120

	DECLARE_DATADESC();

private:

	CNetworkVar( int, m_iOverlayID );
};

LINK_ENTITY_TO_CLASS( info_overlay_accessor, CInfoOverlayAccessor );

BEGIN_DATADESC( CInfoOverlayAccessor )
	DEFINE_KEYFIELD( m_iOverlayID,	FIELD_INTEGER, "OverlayID" ),
END_DATADESC()
