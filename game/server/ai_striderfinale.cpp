//========= Copyright, Dorian Gorski, All rights reserved. ============//
//
// Purpose: Manage Striders in finale combat, one stays near player the other patrols
// at nearby defined patrol location.
//
//=================================================================================//

#include "cbase.h"
#include "ai_striderfinale.h"
//#include "ndebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-------------------------------------

void CAI_StriderFinale::Spawn()
{
	m_vLastPos = Vector(FLT_MAX, FLT_MAX, FLT_MAX);

	if ( m_bActive )
	{
		FillRoles();

		SetThink(&CAI_StriderFinale::MovementThink);
		SetNextThink(gpGlobals->curtime + AISF_THINK_INTERVAL);
	}
}

//-------------------------------------
// Should be called when a strider dies or spawns
void CAI_StriderFinale::InputActivate( inputdata_t &inputdata )
{
	m_bActive = true;

	FillRoles();

	SetThink(&CAI_StriderFinale::MovementThink);
	SetNextThink(gpGlobals->curtime + AISF_THINK_INTERVAL);
}

//-------------------------------------
// Called if active & every AISF_THINK_INTERVAL seconds
void CAI_StriderFinale::MovementThink()
{
	// Update Roles if any striders have died
	FillRoles();

	// If the player moved more than AISF_UPDATE_DIST units from the last position update strider 1 position
	if ( m_npcStrider1 )
	{
		CBasePlayer *pPlayer = AI_GetSinglePlayer();
		if ( pPlayer && ( ( pPlayer->GetAbsOrigin() - m_vLastPos).Length() > AISF_UPDATE_DIST ) )
		{
			StriderFollowPlayer();
		}
	}

	SetNextThink(gpGlobals->curtime + AISF_THINK_INTERVAL);
}

//-------------------------------------
// Called whenever slot 1 is filled or whenever the player has moved AISF_UPDATE_DIST units.
// If a guard path is set Strider1 will go there instead.
void CAI_StriderFinale::StriderFollowPlayer()
{
	if ( m_npcStrider1 )
	{
		if ( m_pGuardPath )
		{
			StriderGuard();
		}
		else
		{
			inputdata_t id_temp;
			castable_string_t sz_temp = castable_string_t("!player");

			id_temp.value.SetString(sz_temp);
			m_npcStrider1->InputSetTargetPath(id_temp);

			// Update last player position
			CBasePlayer *pPlayer = AI_GetSinglePlayer();
			if ( pPlayer )
				m_vLastPos = pPlayer->GetAbsOrigin();
		}
	}
}

//-------------------------------------
// This function is only called whenever slot 2 is filled or a new patrol path is set
void CAI_StriderFinale::StriderPatrol()
{
	if ( m_npcStrider2 && m_pPatrolPath )
	{
		inputdata_t id_temp;
		id_temp.value.SetString( m_pPatrolPath->GetEntityName() );
		m_npcStrider2->InputSetTargetPath(id_temp);
	}
}

//-------------------------------------
// This function is only called whenever slot 2 is filled or a new patrol path is set
void CAI_StriderFinale::StriderGuard()
{
	if ( m_npcStrider1 && m_pGuardPath )
	{
		inputdata_t id_temp;
		id_temp.value.SetString( m_pGuardPath->GetEntityName() );
		m_npcStrider1->InputSetTargetPath(id_temp);
	}
}

//-------------------------------------

void CAI_StriderFinale::FillRoles()
{
	bool bUpdated = false;

	// Find strider for slot 1 or player follower
	if ( !m_npcStrider1 || !m_npcStrider1->IsAlive() )
	{
		bUpdated = true;
		m_npcStrider1 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider1 ) );

		// Could not find strider 1, use strider 2 instead
		if ( !m_npcStrider1 || !m_npcStrider1->IsAlive() )
		{
			m_npcStrider1 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider2 ) );

			// Could not find strider 2, use strider 3 instead
			if ( !m_npcStrider1 || !m_npcStrider1->IsAlive() )
			{
				m_npcStrider1 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider3 ) );
			}
		}

		// If a strider takes over slot 1 set its target path to the player
		StriderFollowPlayer();
	}

	// Find strider for slot 2 or patroller
	if ( !m_npcStrider2 || !m_npcStrider2->IsAlive() || m_npcStrider2 == m_npcStrider1 )
	{
		bUpdated = true;
		m_npcStrider2 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider1 ) );

		// If no strider found or both slot 1 & 2 are assigned to the same strider than try strider 2
		if ( !m_npcStrider2 || !m_npcStrider2->IsAlive() || m_npcStrider2 == m_npcStrider1 )
		{
			m_npcStrider2 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider2 ) );

			// If no strider found or both slot 1 & 2 are assigned to the same strider than try strider 3
			if ( !m_npcStrider2 || !m_npcStrider2->IsAlive() || m_npcStrider2 == m_npcStrider1 )
			{
				m_npcStrider2 = dynamic_cast<CNPC_Strider*>( gEntList.FindEntityByName( NULL, m_iszStrider3 ) );

				// If both slot 1 & 2 are still assigned to the same strider than clear slot 2
				if ( m_npcStrider2 == m_npcStrider1 )
				{
					m_npcStrider2 = NULL;
				}
			}
		}

		// If strider 2 slot is filled than make it patrol
		StriderPatrol();
	}

	// If strider 2 is closer to the player than strider 1 than swap roles
	// also only swap if any roles were recently updated
	// this prevents issue where a patrol strider could get closer and cause swap pattern
	if ( m_npcStrider1 && m_npcStrider2 && bUpdated )
	{
		CBasePlayer *pPlayer = AI_GetSinglePlayer();

		if ( m_npcStrider1->StriderEnemyDistance(pPlayer) > m_npcStrider2->StriderEnemyDistance(pPlayer) )
		{
			CNPC_Strider *pTemp = m_npcStrider1;

			m_npcStrider1 = m_npcStrider2;
			m_npcStrider2 = pTemp;

			StriderFollowPlayer();
			StriderPatrol();
		}
	}
}

//-------------------------------------
