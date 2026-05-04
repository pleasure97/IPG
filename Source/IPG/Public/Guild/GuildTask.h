// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IPGExclusiveTask.h"

class FGuildMasterResource : public FIPGExclusiveResource
{
public:
	FString MasterName;
	int64 MasterUID = 0; 
};

class FGuildResource : public FIPGExclusiveResource
{
public:
	FString GuildName;
	int64 GuildUID = 0;
};

class FGuildMemberResource : public FIPGExclusiveResource
{
public:
	FString GuildName;
	int64 GuildUID = 0;
};

void CreateGuild(FGuildMasterResource* Master)
{
	// TODO
}

void InviteToGuild(FGuildMasterResource* Master, FGuildMemberResource* Member, FGuildResource* Guild)
{
	// TODO
}

void ChangeNickName(FGuildMemberResource* Member)
{
	// TODO
}

void ChangeGuildNotice(FGuildMasterResource* Master, FGuildResource* Guild)
{
	// TODO
}

void LeaveGuild(FGuildMemberResource* Member, FGuildResource* Guild)
{
	// TODO
}

void GuildExclusiveTask(
	FGuildMasterResource* GuildMaster,
	FGuildResource* Guild,
	FGuildMemberResource* GuildMember)
{
	// 式式 1. Create Guild 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Occupied Resource : GuildMaster
	FIPGExclusiveTask::Create([=] { CreateGuild(GuildMaster); })
		->BuildTaskGraph({ GuildMaster });

	// 式式 2. Invite to Guild 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式-
	// Occupied Resource : GuildMaster, GuildMember, Guild
	FIPGExclusiveTask::Create([=] { InviteToGuild(GuildMaster, GuildMember, Guild); })
		->BuildTaskGraph({ GuildMaster, GuildMember, Guild });

	// 式式 3. Change Nickname 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式-
	// Occupied Resource : GuildMember
	FIPGExclusiveTask::Create([=] { ChangeNickName(GuildMember); })
		->BuildTaskGraph({ GuildMember });

	// 式式 4. Change Guild Notice 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Occupied Resource : GuildMaster, Guild
	FIPGExclusiveTask::Create([=] { ChangeGuildNotice(GuildMaster, Guild); })
		->BuildTaskGraph({ GuildMaster, Guild });

	// 式式 5. Leave Guild 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
	// Occupied Resource : GuildMember, Guild
	FIPGExclusiveTask::Create([=] { LeaveGuild(GuildMember, Guild); })
		->BuildTaskGraph({ GuildMember, Guild });
}