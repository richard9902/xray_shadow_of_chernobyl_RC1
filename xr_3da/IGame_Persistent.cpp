#include "stdafx.h"
#pragma hdrstop

#include "IGame_Persistent.h"
#include "environment.h"
#ifndef _EDITOR
#	include "x_ray.h"
#	include "IGame_Level.h"
#	include "XR_IOConsole.h"
#	include "resourcemanager.h"
#	include "Render.h"
#	include "ps_instance.h"
#	include "CustomHUD.h"
#endif

ENGINE_API	IGame_Persistent*		g_pGamePersistent	= NULL;

IGame_Persistent::IGame_Persistent	()
{
	Device.seqAppStart.Add			(this);
	Device.seqAppEnd.Add			(this);
	Device.seqFrame.Add				(this,REG_PRIORITY_HIGH+1);
	Device.seqAppActivate.Add		(this);
	Device.seqAppDeactivate.Add		(this);

	m_pMainMenu						= NULL;

	pEnvironment					= xr_new<CEnvironment>();
}

IGame_Persistent::~IGame_Persistent	()
{
	Device.seqFrame.Remove			(this);
	Device.seqAppStart.Remove		(this);
	Device.seqAppEnd.Remove			(this);
	Device.seqAppActivate.Remove	(this);
	Device.seqAppDeactivate.Remove	(this);
	xr_delete						(pEnvironment);
}

void IGame_Persistent::OnAppActivate		()
{
}

void IGame_Persistent::OnAppDeactivate		()
{
}

void IGame_Persistent::OnAppStart	()
{
//	if (strstr(Core.Params,"-dedicated"))	bDedicatedServer	= TRUE;
//	else									bDedicatedServer	= FALSE;
#ifndef DEDICATED_SERVER
	bDedicatedServer	= FALSE;
#else
	bDedicatedServer	= TRUE;
#endif
	Environment().load				();
}

void IGame_Persistent::OnAppEnd		()
{
	Environment().unload				();
	OnGameEnd						();

	DEL_INSTANCE					(g_hud);
}

void IGame_Persistent::PreStart		(LPCSTR op)
{
	string256						prev_type;
	params							new_game_params;
	strcpy							(prev_type,m_game_params.m_game_type);
	new_game_params.parse_cmd_line	(op);
	// change game type
	if (0!=xr_strcmp(prev_type,new_game_params.m_game_type)){
		OnGameEnd					();
	}
}

void IGame_Persistent::Start		(LPCSTR op)
{
	string256						prev_type;
	strcpy							(prev_type,m_game_params.m_game_type);
	m_game_params.parse_cmd_line	(op);
//.	if(g_hud)
//.		g_hud->OnConnected				();
	// change game type
	if ((0!=xr_strcmp(prev_type,m_game_params.m_game_type))) {
//		OnGameEnd					();
		if (*m_game_params.m_game_type)
			OnGameStart					();
		if(g_hud)
			DEL_INSTANCE			(g_hud);
	}
	VERIFY							(ps_destroy.empty());
}

void IGame_Persistent::Disconnect	()
{
#ifndef _EDITOR
	// clear "need to play" particles
	ps_needtoplay.clear				();

	while (ps_destroy.size())
	{
		CPS_Instance*	psi		= ps_destroy.back	();		
		VERIFY					(psi);
		VERIFY					(!psi->Locked());
		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}

	// delete active particles
	while (!ps_active.empty())
		(*ps_active.begin())->PSI_internal_delete();
	VERIFY(ps_needtoplay.empty()&&ps_destroy.empty()&&ps_active.empty());
	if(g_hud)
		g_hud->OnDisconnected			();
#endif
}

void IGame_Persistent::OnGameStart	()
{
#ifndef _EDITOR
	LoadTitle								("st_prefetching_objects");
	if (strstr(Core.Params,"-noprefetch"))	return;

	// prefetch game objects & models
	float	p_time		=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec();
	u32	mem_0			=			Memory.mem_usage()	;

	Log				("Loading objects...");
	ObjectPool.prefetch					();
	Log				("Loading models...");
	Render->models_Prefetch				();
	Device.Resources->DeferredUpload	();

	p_time				=			1000.f*Device.GetTimerGlobal()->GetElapsed_sec() - p_time;
	u32		p_mem		=			Memory.mem_usage() - mem_0	;

	Msg					("* [prefetch] time:    %d ms",	iFloor(p_time));
	Msg					("* [prefetch] memory:  %dKb",	p_mem/1024);
#endif
}

void IGame_Persistent::OnGameEnd	()
{
#ifndef _EDITOR
	ObjectPool.clear					();
	Render->models_Clear				(TRUE);
#endif
}

void IGame_Persistent::OnFrame		()
{
	if(!Device.Paused() || Device.dwPrecacheFrame)
		Environment().OnFrame				();
#ifndef _EDITOR

	Device.Statistic->Particles_starting= ps_needtoplay.size	();
	Device.Statistic->Particles_active	= ps_active.size		();
	Device.Statistic->Particles_destroy	= ps_destroy.size		();

	// Play req particle systems
	while (ps_needtoplay.size())
	{
		CPS_Instance*	psi		= ps_needtoplay.back	();
		ps_needtoplay.pop_back	();
		psi->Play				();
	}
	// Destroy inactive particle systems
	while (ps_destroy.size())
	{
//		u32 cnt					= ps_destroy.size();
		CPS_Instance*	psi		= ps_destroy.back();
		VERIFY					(psi);
		if (psi->Locked())
		{
			Log("--locked");
			break;
		}
		ps_destroy.pop_back		();
		psi->PSI_internal_delete();
	}
#endif
}
