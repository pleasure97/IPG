// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Exclusive/IPGExclusiveTask.h"

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