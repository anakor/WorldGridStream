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
	/* * DivideDistancePowerOfTwo * Landscape�� ScaleX����� ���ؼ� ������ ��.
	 * �ϴ� Landscape ScaleX�� ���� 100.0f�� �����ϰ� ������.
	 * Default is set to Power256 (25600 uu).
	 */
	UPROPERTY(VisibleAnywhere, Category="World Grid Stream Settings", meta=(DisplayAfter="Visibility Distance"))
	float VisibilityDistance;
	
	/* * Streaming�ÿ� Z���� �Ÿ��� �������� ���θ� �����ϴ� ����.
	 * true�� ��쿡�� Z���� �Ÿ��� ����.
	 * Default is set to false.
	 */
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Include Z-Axis Distance"))
	bool bIncludeZDistance; 
	
	/* * Streaming�� ���� �ɼ�
	 * true�� ��쿡�� Streaming�� ��
	 * Default is set to true.
	 */
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Streaming Off"))
	bool bStreamingOn;

	/* * Landscape�� Scale������� �����ϰ� �� ������
	 * �ʿ������� �ϸ鼭 ���� ��.
	 * Default is set to 100.0f.
	 */
	float WorldScale;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category="World Grid Stream Settings", meta=(DisplayAfter="Divide Distance"))
	EPowerOfTwo DivideDistancePowerOfTwo; //���Ŀ� custom ui�� �����ؼ� �Ʒ��� DivideDistance�� ���ľ� ��.

	float DivideDistance; //DivideDistancePowerOfTwo * Landscape�� ScaleX����� ���ؼ� ������ ��. �ϴ� Landscape ScaleX�� ���� 100.0f�� �����ϰ� ������.

	/* * ��� �����Ͽ� Asset���� �����ߴ����� ���������� �����ϴ� ����. Debugging������ Shipping�ÿ� false�� ������
	 * true�� ��쿡�� VisibilityDistance�� �ش��ϴ� ũ���� Rect�� Volume���� ������
	 * Default is set to false.
	 */
	UPROPERTY(EditAnywhere, Transient, Category="World Grid Stream Settings", AdvancedDisplay, meta=(DisplayAfter="Show Divide Rect"))
	bool bVisualizeDivideRect;
	
	/* * Distance�� ���� Grid�� �����Ͽ� Asset�� ����
	 * true�� ��쿡�� Asset���� ������
	 * Default is set to true.
	 */
	UPROPERTY(EditAnywhere, Transient, Category="World Grid Stream Settings", meta=(DisplayAfter="Build World Grid Assets"))
	bool bBuildWorldGridAssets; //���Ŀ� custom ui�� �����ؼ� ��ư Ÿ������ �ٲ��ְ� �� ������ ���� �ؾ� �� ���� ����.
	
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