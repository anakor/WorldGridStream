// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "UObject/ObjectMacros.h"
//#include "Templates/SubclassOf.h"
#include "Interfaces/Interface_AssetUserData.h"
#include "GameFramework/Actor.h"

#include "WorldGridStreamSettings.generated.h"

UENUM()
enum class EPowerOfTwo : uint16
{
    Power1 = 1     UMETA(DisplayName = "100 uu"),
    Power2 = 2     UMETA(DisplayName = "200 uu"),
    Power4 = 4     UMETA(DisplayName = "400 uu"),
    Power8 = 8     UMETA(DisplayName = "800 uu"),
    Power16 = 16   UMETA(DisplayName = "1600 uu"),
    Power32 = 32   UMETA(DisplayName = "3200 uu"),
    Power64 = 64   UMETA(DisplayName = "6400 uu"),
    Power128 = 128 UMETA(DisplayName = "12800 uu"),
    Power256 = 256 UMETA(DisplayName = "25600 uu"),
    Power512 = 512 UMETA(DisplayName = "51200 uu"),
};

UCLASS(ClassGroup=(Custom), hidecategories=(Actor, Display, Advanced, Physics, Networking, Replication, LevelInstance, WorldPartition, Transform, Cooking), NotBlueprintable, MinimalAPI, meta = (DisplayName = "World Grid Stream Settings"))
class AWorldGridStreamSettings : public AInfo, public IInterface_AssetUserData
{
	GENERATED_BODY()

public:	
	/* * DivideDistancePowerOfTwo * Landscape의 ScaleX사이즈를 곱해서 나오는 값.
	 * 일단 Landscape ScaleX의 값은 100.0f로 가정하고 시작함.
	 * Default is set to Power256 (25600 uu).
	 */
	UPROPERTY(VisibleAnywhere, Category="World Grid Stream Settings", meta=(DisplayAfter="Visibility Distance"))
	float VisibilityDistance;
	
	/* * Streaming시에 Z축의 거리를 포함할지 여부를 결정하는 변수.
	 * true일 경우에는 Z축의 거리를 포함.
	 * Default is set to false.
	 */
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Include Z-Axis Distance"))
	bool bIncludeZDistance; 
	
	/* * Streaming을 끄는 옵션
	 * true일 경우에는 Streaming을 켬
	 * Default is set to true.
	 */
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Streaming Off"))
	bool bStreamingOn;

	/* * Landscape의 Scale사이즈와 동일하게 할 예정임
	 * 필요할지는 하면서 봐야 함.
	 * Default is set to 100.0f.
	 */
	float WorldScale;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", meta=(DisplayAfter="Divide Distance"))
	EPowerOfTwo DivideDistancePowerOfTwo; //추후에 custom ui를 생성해서 아래의 DivideDistance와 합쳐야 함.

	float DivideDistance; //DivideDistancePowerOfTwo * Landscape의 ScaleX사이즈를 곱해서 나오는 값. 일단 Landscape ScaleX의 값은 100.0f로 가정하고 시작함.

	/* * 어떻게 분할하여 Asset들을 저장했는지를 보여줄지를 결정하는 변수. Debugging용으로 Shipping시에 false로 설정함
	 * true일 경우에는 VisibilityDistance에 해당하는 크기의 Rect를 Volume으로 보여줌
	 * Default is set to false.
	 */
	UPROPERTY(EditAnywhere, Transient, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Show Divide Rect"))
	bool bVisualizeDivideRect;
	
	/* * Distance에 따라 Grid를 분할하여 Asset을 저장
	 * true일 경우에는 Asset들을 빌드함
	 * Default is set to true.
	 */
	UPROPERTY(EditAnywhere, Transient, Category="World Grid Stream Settings", meta=(DisplayAfter="Build World Grid Assets"))
	bool bBuildWorldGridAssets; //추후에 custom ui를 생성해서 버튼 타입으로 바꿔주고 이 변수는 삭제 해야 할 수도 있음.
	
	FDelegateHandle OnActorDeletedDelegateHandle;
#endif //WITH_EDITORONLY_DATA

protected:
private:

public:
	AWorldGridStreamSettings(const FObjectInitializer& ObjectInitializer);
	virtual ~AWorldGridStreamSettings();
	
	//~ Begin UObject Interface.
	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual void PostActorCreated() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void OnActorDeleted(AActor* ActorDeleted);
#endif //WITH_EDITOR

	//~ End UObject Interface.

	void SetWorldScale(float InWorldScale);
	float GetWorldScale() { return WorldScale; }

protected:
#if WITH_EDITOR
	void ShowDivideRect(bool InbVisualizeDivideRect);
	void BuildWorldGridAssets(bool InbBuildWorldGridAssets);
#endif //WITH_EDITOR

	void ResiterDelegate();

private:
};