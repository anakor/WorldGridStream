#include "AssetStreamingTest.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"

static const FSoftObjectPath TestAssetPath(TEXT("/Game/TestContents/Textures/T_UEFN_Mannequin_D.T_UEFN_Mannequin_D"));

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAssetStreamingSubsystem_BasicTest, "AssetStreaming.Basic.RequestAndRelease", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);


bool FAssetStreamingSubsystem_BasicTest::RunTest(const FString& Parameters)
{
    UAssetStreamingSubsystem* Subsystem = GEngine->GetEngineSubsystem<UAssetStreamingSubsystem>();
    TestNotNull(TEXT("Subsystem should not be null"), Subsystem);

    // 1. 기본 요청 (Default Queue)
    FGuid RequestId;
    bool bRequested = Subsystem->RequestAssetStreaming(TestAssetPath, RequestId, 0);
    TestTrue(TEXT("RequestAssetStreaming(Default) returns true"), bRequested);
    TestTrue(TEXT("RequestId is valid"), RequestId.IsValid());

    // 2. 우선순위 요청 (Priority Queue)
    FGuid PriorityRequestId;
    int32 Priority = 10;
    bool bPriorityRequested = Subsystem->RequestAssetStreaming(TestAssetPath, PriorityRequestId, Priority);
    TestTrue(TEXT("RequestAssetStreaming(Priority) returns true"), bPriorityRequested);
    TestTrue(TEXT("PriorityRequestId is valid"), PriorityRequestId.IsValid());

    // 3. 일괄 요청
    TArray<FSoftObjectPath> AssetPaths = { TestAssetPath, TestAssetPath };
    TArray<FGuid> BatchRequestIds;
    bool bBatchRequested = Subsystem->RequestAssetsStreaming(AssetPaths, BatchRequestIds, 0);
    TestTrue(TEXT("RequestAssetsStreaming returns true"), bBatchRequested);
    TestEqual(TEXT("BatchRequestIds count"), BatchRequestIds.Num(), 2);

    // 4. 델리게이트 바인딩 및 호출 테스트
    UAssetStreamingTestHelper* Helper = NewObject<UAssetStreamingTestHelper>();
    Helper->AddToRoot();
    Subsystem->OnAssetLoaded.AddDynamic(Helper, &UAssetStreamingTestHelper::OnAssetLoaded);

    for (int i = 0; i < 100; ++i)
    {
        Subsystem->Tick(0.01f); // 더 자주 호출
        if (Helper->bDelegateCalled)
        {
            UE_LOG(LogTemp, Warning, TEXT("Delegate triggered at tick %d"), i);
            break;
        }
    }
    TestTrue(TEXT("OnAssetLoaded delegate should be called"), Helper->bDelegateCalled);

    Subsystem->OnAssetLoaded.RemoveDynamic(Helper, &UAssetStreamingTestHelper::OnAssetLoaded);
    
    // 5. 해제 테스트
    bool bReleased = Subsystem->ReleaseAsset(RequestId);
    TestTrue(TEXT("ReleaseAsset returns true"), bReleased);

    return true;
}
