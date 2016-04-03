//========= Copyright, CSProMod Team, All rights reserved. =========//
//
// Purpose: provide world light related functions to the client
//
// As the engine provides no access to brush/model data (brushdata_t, model_t),
// we hence have no access to dworldlight_t. Therefore, we manually extract the
// world light data from the BSP itself, before entities are initialised on map
// load.
//
// To find the brightest light at a point, all world lights are iterated.
// Lights whose radii do not encompass our sample point are quickly rejected,
// as are lights which are not in our PVS, or visible from the sample point.
// If the sky light is visible from the sample point, then it shall supersede
// all other world lights.
//
// Written: November 2011
// Author: Saul Rennison
//
//===========================================================================//

#include "cbase.h"
#include "worldlight.h"
#include "bspfile.h"
#include "filesystem.h"
#include "client_factorylist.h" // FactoryList_Retrieve
#include "eiface.h" // IVEngineServer
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static IVEngineServer *g_pEngineServer = NULL;

//-----------------------------------------------------------------------------
// Singleton exposure
//-----------------------------------------------------------------------------
static CWorldLights s_WorldLights;
CWorldLights *g_pWorldLights = &s_WorldLights;

//-----------------------------------------------------------------------------
// Purpose: calculate intensity ratio for a worldlight by distance
// Author: Valve Software
//-----------------------------------------------------------------------------
static float Engine_WorldLightDistanceFalloff( const dworldlight_t *wl, const Vector& delta )
{
	float falloff;

	switch (wl->type)
	{
	case emit_surface:
		// Cull out stuff that's too far
		if(wl->radius != 0)
		{
			if(DotProduct( delta, delta ) > (wl->radius * wl->radius))
				return 0.0f;
		}

		return InvRSquared(delta);
		break;

	case emit_skylight:
		return 1.f;
		break;

	case emit_quakelight:
		// X - r;
		falloff = wl->linear_attn - FastSqrt( DotProduct( delta, delta ) );
		if ( falloff < 0 )
			return 0.f;

		return falloff;
		break;

	case emit_skyambient:
		return 1.f;
		break;

	case emit_point:
	case emit_spotlight:	// directional & positional
		{
			float dist2, dist;

			dist2 = DotProduct(delta, delta);
			dist = FastSqrt(dist2);

			// Cull out stuff that's too far
			if ( wl->radius != 0 && dist > wl->radius )
				return 0.f;

			return 1.f / (wl->constant_attn + wl->linear_attn * dist + wl->quadratic_attn * dist2);
		}

		break;
	}

	return 1.f;
}

//-----------------------------------------------------------------------------
// Purpose: initialise game system and members
//-----------------------------------------------------------------------------
CWorldLights::CWorldLights() : CAutoGameSystem("World lights")
{
	m_nWorldLights = 0;
	m_pWorldLights = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: clear worldlights, free memory
//-----------------------------------------------------------------------------
void CWorldLights::Clear()
{
	m_nWorldLights = 0;

	if(m_pWorldLights)
	{
		delete [] m_pWorldLights;
		m_pWorldLights = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: get the IVEngineServer, we need this for the PVS functions
//-----------------------------------------------------------------------------
bool CWorldLights::Init()
{
	factorylist_t factories;
	FactoryList_Retrieve(factories);

	if ( ( g_pEngineServer = (IVEngineServer*)factories.appSystemFactory(INTERFACEVERSION_VENGINESERVER, NULL) ) == NULL )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: get all world lights from the BSP
//-----------------------------------------------------------------------------
void CWorldLights::LevelInitPreEntity()
{
	// Get the map path
	const char *pszMapName = modelinfo->GetModelName(modelinfo->GetModel(1));

	// Open map
	FileHandle_t hFile = g_pFullFileSystem->Open(pszMapName, "rb");
	if ( !hFile )
	{
		Warning("CWorldLights: unable to open map\n");
		return;
	}

	// Read the BSP header. We don't need to do any version checks, etc. as we
	// can safely assume that the engine did this for us
	dheader_t hdr;
	g_pFullFileSystem->Read(&hdr, sizeof(hdr), hFile);

	// Grab the light lump and seek to it
	lump_t &lightLump = hdr.lumps[LUMP_WORLDLIGHTS];

	// If we can't divide the lump data into a whole number of worldlights,
	// then the BSP format changed and we're unaware
	if ( lightLump.filelen % sizeof(dworldlight_t) )
	{
		Warning("CWorldLights: unknown world light lump\n");

		// Close file
		g_pFullFileSystem->Close(hFile);
		return;
	}

	g_pFullFileSystem->Seek(hFile, lightLump.fileofs, FILESYSTEM_SEEK_HEAD);

	// Allocate memory for the worldlights
	m_nWorldLights = lightLump.filelen / sizeof(dworldlight_t);
	m_pWorldLights = new dworldlight_t[m_nWorldLights];

	// Read worldlights then close
	g_pFullFileSystem->Read(m_pWorldLights, lightLump.filelen, hFile);
	g_pFullFileSystem->Close(hFile);

	DevMsg("CWorldLights: load successful (%d lights at 0x%p)\n", m_nWorldLights, m_pWorldLights);
}

//-----------------------------------------------------------------------------
// Purpose: find the brightest light source at a point
//-----------------------------------------------------------------------------
bool CWorldLights::GetBrightestLightSource(const Vector &vecPosition, Vector &vecLightPos, Vector &vecLightBrightness)
{
	VPROF_BUDGET( "CWorldLights::GetBrightestLightSource", VPROF_BUDGETGROUP_SHADOW_RENDERING );

	if ( !m_nWorldLights || !m_pWorldLights )
		return false;

	// Default light position and brightness to zero
	vecLightBrightness.Init();
	vecLightPos.Init();

	// Find the size of the PVS for our current position
	int nCluster = g_pEngineServer->GetClusterForOrigin(vecPosition);
	int nPVSSize = g_pEngineServer->GetPVSForCluster(nCluster, 0, NULL);

	// Get the PVS at our position
	byte *pvs = new byte[nPVSSize];
	g_pEngineServer->GetPVSForCluster(nCluster, nPVSSize, pvs);

	// Iterate through all the worldlights
	for ( int i = 0; i < m_nWorldLights; ++i )
	{
		dworldlight_t *light = &m_pWorldLights[i];

		// Skip skyambient
		if ( light->type == emit_skyambient )
		{
			//DevMsg("CWorldLights: skyambient %d\n", i);
			continue;
		}

		// Handle sun
		if ( light->type == emit_skylight )
		{
			// Calculate sun position
			Vector vecAbsStart = vecPosition + Vector(0, 0, 30);
			Vector vecAbsEnd = vecAbsStart - (light->normal * MAX_TRACE_LENGTH);

			trace_t tr;
			UTIL_TraceLine(vecPosition, vecAbsEnd, MASK_OPAQUE, NULL, COLLISION_GROUP_NONE, &tr);

			// If we didn't hit anything then we have a problem
			if(!tr.DidHit())
			{
				//DevMsg("CWorldLights: skylight %d couldn't touch sky\n", i);
				continue;
			}

			// If we did hit something, and it wasn't the skybox, then skip
			// this worldlight
			if ( !(tr.surface.flags & SURF_SKY) && !(tr.surface.flags & SURF_SKY2D) )
			{
				//DevMsg("CWorldLights: skylight %d no sight to sun\n", i);
				continue;
			}

			// Act like we didn't find any valid worldlights, so the shadow
			// manager uses the default shadow direction instead (should be the
			// sun direction)

			delete[] pvs;

			return false;
		}

		// Calculate square distance to this worldlight
		Vector vecDelta = light->origin - vecPosition;
		float flDistSqr = vecDelta.LengthSqr();
		float flRadiusSqr = light->radius * light->radius;

		// Skip lights that are out of our radius
		if ( flRadiusSqr > 0 && flDistSqr >= flRadiusSqr )
		{
			//DevMsg("CWorldLights: %d out-of-radius (dist: %d, radius: %d)\n", i, sqrt(flDistSqr), light->radius);
			continue;
		}

		// Is it out of our PVS?
		if ( !g_pEngineServer->CheckOriginInPVS(light->origin, pvs, nPVSSize) )
		{
			//DevMsg("CWorldLights: %d out of PVS\n", i);
			continue;
		}

		// Calculate intensity at our position
		float flRatio = Engine_WorldLightDistanceFalloff(light, vecDelta);
		Vector vecIntensity = light->intensity * flRatio;

		// Is this light more intense than the one we already found?
		if ( vecIntensity.LengthSqr() <= vecLightBrightness.LengthSqr() )
		{
			//DevMsg("CWorldLights: %d too dim\n", i);
			continue;
		}

		// Can we see the light?
		trace_t tr;
		Vector vecAbsStart = vecPosition + Vector(0, 0, 30);
		UTIL_TraceLine(vecAbsStart, light->origin, MASK_OPAQUE, NULL, COLLISION_GROUP_NONE, &tr);

		if ( tr.DidHit() )
		{
			//DevMsg("CWorldLights: %d trace failed\n", i);
			continue;
		}

		vecLightPos = light->origin;
		vecLightBrightness = vecIntensity;

		//DevMsg("CWorldLights: %d set (%.2f)\n", i, vecIntensity.Length());
	}

	delete[] pvs;
	
	//DevMsg("CWorldLights: result %d\n", !vecLightBrightness.IsZero());
	return !vecLightBrightness.IsZero();
}
