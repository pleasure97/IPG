// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "Tasks/Task.h"
#include "Guild/GuildResource.h"
#include "Exclusive/IPGExclusiveTask.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGuildExclusiveTaskTest,
    "IPGTaskSystem.GuildExclusiveTask",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

    bool FGuildExclusiveTaskTest::RunTest(const FString& Parameters)
{
    // ---- 1. Create resource ----------------------------------------------------------------
    TSharedPtr<FGuildMasterResource> GuildMaster = MakeShared<FGuildMasterResource>();
    TSharedPtr<FGuildMemberResource> GuildMember = MakeShared<FGuildMemberResource>();
    TSharedPtr<FGuildResource>       Guild = MakeShared<FGuildResource>();

    // ---- 2. Allocate logs to heap memory ----------------------------------------------------
    TSharedPtr<TArray<FString>>   ExecutionLog = MakeShared<TArray<FString>>();
    TSharedPtr<FCriticalSection>  LogLock = MakeShared<FCriticalSection>();

    // ---- 3. Define log lambda function ------------------------------------------------------
    auto Log = [ExecutionLog, LogLock](const FString& Name)
        {
            FScopeLock Lock(LogLock.Get());
            UE_LOG(LogTemp, Log, TEXT("[GuildTest] %s executed"), *Name);
            ExecutionLog->Add(Name);
        };

    // ---- 4. Launch task  ----------------------------------------------------------------
    TArray<UE::Tasks::FTask> LaunchedTasks; 

    LaunchedTasks.Add(IPGTaskSystem::LaunchExclusive(
        TEXT("T1_CreateGuild"),
        { GuildMaster.Get() },
        [=] { Log(TEXT("T1_CreateGuild")); }
    ));

    LaunchedTasks.Add(IPGTaskSystem::LaunchExclusive(
        TEXT("T2_InviteToGuild"),
        { GuildMaster.Get(), GuildMember.Get(), Guild.Get() },
        [=] { Log(TEXT("T2_InviteToGuild")); }
    ));

    LaunchedTasks.Add(IPGTaskSystem::LaunchExclusive(
        TEXT("T3_ChangeNickName"),
        { GuildMember.Get() },
        [=] { Log(TEXT("T3_ChangeNickName")); }
    ));

    LaunchedTasks.Add(IPGTaskSystem::LaunchExclusive(
        TEXT("T4_ChangeGuildNotice"),
        { GuildMaster.Get(), Guild.Get() },
        [=] { Log(TEXT("T4_ChangeGuildNotice")); }
    ));

    LaunchedTasks.Add(IPGTaskSystem::LaunchExclusive(
        TEXT("T5_LeaveGuild"),
        { GuildMember.Get(), Guild.Get() },
        [=] { Log(TEXT("T5_LeaveGuild")); }
    ));

    // ---- 5. Wait for all tasks ----------------------------------------------------------------
    UE::Tasks::Wait(LaunchedTasks); 

    // ---- 6. Validate results ----------------------------------------------------------------
    TestEqual(TEXT("All 5 tasks executed"), ExecutionLog->Num(), 5);

    if (ExecutionLog->Num() == 5)
    {
        int32 idxT1 = ExecutionLog->Find(TEXT("T1_CreateGuild"));
        int32 idxT2 = ExecutionLog->Find(TEXT("T2_InviteToGuild"));
        int32 idxT3 = ExecutionLog->Find(TEXT("T3_ChangeNickName"));
        int32 idxT4 = ExecutionLog->Find(TEXT("T4_ChangeGuildNotice"));
        int32 idxT5 = ExecutionLog->Find(TEXT("T5_LeaveGuild"));

        TestTrue(TEXT("T1 before T2 (Master)"), idxT1 < idxT2);
        TestTrue(TEXT("T2 before T4 (Master)"), idxT2 < idxT4);
        TestTrue(TEXT("T2 before T3 (Member)"), idxT2 < idxT3);
        TestTrue(TEXT("T3 before T5 (Member)"), idxT3 < idxT5);
        TestTrue(TEXT("T2 before T4 (Guild)"), idxT2 < idxT4);
        TestTrue(TEXT("T4 before T5 (Guild)"), idxT4 < idxT5);
    }

    return true;
}