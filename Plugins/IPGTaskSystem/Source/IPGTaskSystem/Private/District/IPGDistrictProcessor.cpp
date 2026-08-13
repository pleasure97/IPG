// Fill out your copyright notice in the Description page of Project Settings.


#include "District/IPGDistrictProcessor.h"

FIPGDistrictProcessor::FIPGDistrictProcessor(int32 InNumRows, int32 InNumCols, TSharedPtr<IIPGSectionProcessor> InSectionProcessor)
    : NumRows(InNumRows), NumCols(InNumCols), SectionProcessor(InSectionProcessor)
{
    check(NumRows > 0 && NumCols > 0); 
    check(SectionProcessor.IsValid());

    SectionEvents.SetNum(NumRows * NumCols);
}

FIPGDistrictProcessor::~FIPGDistrictProcessor()
{

}

void FIPGDistrictProcessor::Initialize()
{
    ExecutionOrders.Empty(); 
    ExecutionOrders.Reserve(NumRows * NumCols); 

    // in-degree array - the number of previous sections which each section wait for 
    TArray<int32> InDegree;
    InDegree.SetNumZeroed(NumRows * NumCols);

    for (int32 Row = 0; Row < NumRows; ++Row)
    {
        for (int32 Col = 0; Col < NumCols; ++Col)
        {
            TArray<FIntPoint> Prerequisites = GetPrerequisites(Row, Col); 
            InDegree[Index(Row, Col)] = Prerequisites.Num();
        }
    }

    // BFS - Start from the section with in-degree 0 
    TQueue<FIntPoint> Queue;
    for (int32 Row = 0; Row < NumRows; ++Row)
    {
        for (int32 Col = 0; Col < NumCols; ++Col)
        {
            if (InDegree[Index(Row, Col)] == 0)
            {
                Queue.Enqueue(FIntPoint(Row, Col));
            }
        }
    }

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current); 
        ExecutionOrders.Add(Current);

        int32 CurrentRow = Current.X;
        int32 CurrentCol = Current.Y;

        // Check if subsequent sections become executable when the section completes
        // (Row + 1, Col), (Row + 1, Col + 1), (Col + 1, Row) 
        TArray<FIntPoint> Successors;
        if (CurrentRow + 1 < NumRows)
        {
            Successors.Add(FIntPoint(CurrentRow + 1, CurrentCol)); 
            if (CurrentCol + 1 < NumCols)
            {
                Successors.Add(FIntPoint(CurrentRow + 1, CurrentCol + 1)); 
            }
        }
        if (CurrentCol + 1 < NumCols)
        {
            Successors.Add(FIntPoint(CurrentRow, CurrentCol + 1)); 
        }

        for (const FIntPoint& Successor : Successors)
        {
            TArray<FIntPoint> SuccessorPrerequisites = GetPrerequisites(Successor.X, Successor.Y);
            if (SuccessorPrerequisites.Contains(FIntPoint(CurrentRow, CurrentCol)))
            {
                int32& Degree = InDegree[Index(Successor.X, Successor.Y)];
                --Degree;
                if (Degree == 0)
                {
                    Queue.Enqueue(Successor);
                }
            }
        }
    }
}

void FIPGDistrictProcessor::Execute(float DeltaTime)
{
    // Initialize graph event at every Execute()
    for (int32 i = 0; i < SectionEvents.Num(); ++i)
    {
        SectionEvents[i] = FGraphEvent::CreateGraphEvent();
    }

    // Create tasks in topologically sorted order and connect prerequisite events
    for (const FIntPoint& ExecutionOrderCoord : ExecutionOrders)
    {
        int32 ExecutionOrderRow = ExecutionOrderCoord.X;
        int32 ExecutionOrderCol = ExecutionOrderCoord.Y; 

        // Collect graph event of prerequisite section
        FGraphEventArray PreRequisites;
        for (const FIntPoint& PreRequisiteCoord : GetPrerequisites(ExecutionOrderRow, ExecutionOrderCol))
        {
            FGraphEventRef& PreEvent = SectionEvents[Index(PreRequisiteCoord.X, PreRequisiteCoord.Y)];
            if (PreEvent.IsValid())
            {
                PreRequisites.Add(PreEvent);
            }
        }

        // Create and schedule tasks
        SectionEvents[Index(ExecutionOrderRow, ExecutionOrderCol)] = TGraphTask<FSectionTask>::CreateTask(
            PreRequisites.Num() > 0 ? &PreRequisites : nullptr,
            ENamedThreads::GameThread
        ).ConstructAndDispatchWhenReady(this, ExecutionOrderRow, ExecutionOrderCol, DeltaTime);
    }

    // Wait for completion of all tasks
    FGraphEventArray AllEvents;
    AllEvents.Reserve(SectionEvents.Num()); 
    for (const FGraphEventRef& SectionEvent : SectionEvents)
    {
        if (SectionEvent.IsValid())
        {
            AllEvents.Add(SectionEvent);
        }
    }

    FTaskGraphInterface::Get().WaitUntilTasksComplete(AllEvents, ENamedThreads::GameThread);
}

int32 FIPGDistrictProcessor::GetNumRows()
{
    return NumRows; 
}

int32 FIPGDistrictProcessor::GetNumCols()
{
    return NumCols;
}

TArray<FIntPoint> FIPGDistrictProcessor::GetPrerequisites(int32 Row, int32 Col) const
{
    TArray<FIntPoint> PreRequisites;

    // Edge section has no previous task
    if (IsEdgeSection(Row, Col))
    {
        return PreRequisites;
    }

    // Upper (Row - 1, Col)
    if (Row - 1 >= 0)
    {
        PreRequisites.Add(FIntPoint(Row - 1, Col));
    }

    // Upper left (Row - 1, Col - 1)
    if (Row - 1 >= 0 && Col - 1 >= 0)
    {
        PreRequisites.Add(FIntPoint(Row - 1, Col - 1)); 
    }

    // Left (Row, Col - 1)
    if (Col - 1 >= 0)
    {
        PreRequisites.Add(FIntPoint(Row, Col - 1));
    }

    return PreRequisites;
}

bool FIPGDistrictProcessor::IsEdgeSection(int32 Row, int32 Col) const
{
    return (Row == 0 || Row == NumRows - 1 || Col == 0 || Col == NumCols - 1);
}

int32 FIPGDistrictProcessor::Index(int32 Row, int32 Col) const
{
    return Row * NumCols + Col;
}

void FIPGDistrictProcessor::FSectionTask::DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent)
{
    if (Owner && Owner->SectionProcessor.IsValid())
    {
        Owner->SectionProcessor->ProcessSection(FDistrictSection(Row, Col), DeltaTime);
    }
}
