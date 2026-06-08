// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class IIPGThreadQueue
{
public:
	using FTaskFunction = TFunction<void()>;

	static IIPGThreadQueue& Get();

	virtual ~IIPGThreadQueue() = default;
	virtual void AddTask(FTaskFunction&& Task) = 0;
};

class FIPGThreadScope
{
public:
	FIPGThreadScope();
	~FIPGThreadScope();

	bool ProcessThread(int64 WaitCycles) const;
};