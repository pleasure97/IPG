// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FGuildMasterResource;
class FGuildMemberResource;
class FGuildResource;

/* Guild Util Function */
void CreateGuild(FGuildMasterResource* Master);
void InviteToGuild(FGuildMasterResource* Master, FGuildMemberResource* Member, FGuildResource* Guild);
void ChangeNickName(FGuildMemberResource* Member);
void ChangeGuildNotice(FGuildMasterResource* Master, FGuildResource* Guild);
void LeaveGuild(FGuildMemberResource* Member, FGuildResource* Guild);
/* Guild Util Function End */

/* Guild Task Graph */
void GuildExclusiveTask(FGuildMasterResource* GuildMaster, FGuildResource* Guild, FGuildMemberResource* GuildMember);