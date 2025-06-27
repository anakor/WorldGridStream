// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridStreamSettings.h"
#include "WorldGridStreamBuilder.h"
#include "WorldGridStreamInstancesActor.h"

BEGIN_FUNCTION_BUILD_OPTIMIZATION

AWorldGridStreamSettings::AWorldGridStreamSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
	, bIncludeZDistance(false)
	, bStreamingOn(true)
	, WorldScale(100.0f)
#if WITH_EDITORONLY_DATA
	, DivideDistancePowerOfTwo(EPowerOfTwo::Power256)
	, bVisualizeDivideRect(false)
#endif //WITH_EDITORONLY_DATA
{
#if WITH_EDITORONLY_DATA
	DivideDistance = static_cast<float>(static_cast<uint16>(DivideDistancePowerOfTwo)) * WorldScale;
	VisibilityDistance = DivideDistance;
#endif //WITH_EDITORONLY_DATA
}

AWorldGridStreamSettings::~AWorldGridStreamSettings()
{

}

void AWorldGridStreamSettings::PostActorCreated()
{
	Super::PostActorCreated();
	// This function is called after the actor has been created in the world.
	// You can perform any initialization or setup here if needed.
	
	ResiterDelegate();
}
void AWorldGridStreamSettings::PostLoad()
{
	Super::PostLoad();

	ResiterDelegate();

#if WITH_EDITORONLY_DATA
	DivideDistance = static_cast<float>(static_cast<uint16>(DivideDistancePowerOfTwo)) * WorldScale;
	VisibilityDistance = DivideDistance;
#endif //WITH_EDITORONLY_DATA
}

void AWorldGridStreamSettings::BeginDestroy()
{
	Super::BeginDestroy();
	// Clean up any resources or references if necessary
}

#if WITH_EDITOR
void AWorldGridStreamSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AWorldGridStreamSettings, DivideDistancePowerOfTwo))
	{
		DivideDistance = static_cast<float>(static_cast<uint16>(DivideDistancePowerOfTwo)) * WorldScale;
		VisibilityDistance = DivideDistance;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AWorldGridStreamSettings, bVisualizeDivideRect))
	{
		//Volume을 보여줘야 함.
		ShowDivideRect(bVisualizeDivideRect);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AWorldGridStreamSettings, bBuildWorldGridAssets))
	{
		//Actor를 가상의 Grid 단위로 나누어주는 연산
		BuildWorldGridAssets(bBuildWorldGridAssets);
	}
}

void AWorldGridStreamSettings::OnActorDeleted(AActor* ActorDeleted)
{
	// 이 부분은 WorldGridStreamInstancesActor가 여러개 생성되는 것을 방지하기 위함이다.
	UWorld* World = GetWorld();
	if(nullptr == World)
	{
		return;
	}
	if(ActorDeleted->GetClass() == GetClass())
	{
		// 월드에 있는 AWorldGridStreamInstancesActor를 찾는다.
		AWorldGridStreamInstancesActor* WorldGridStreamInstancesActor = AWorldGridStreamInstancesActor::GetWorldGridStreamInstancesActor(World);
		// 너무많이 호출되어 주석 처리함.
		//FMessageDialog::Open( EAppMsgType::Ok, NSLOCTEXT("ActorStreamingInWorld", "Already Spwaned", "ASW Settings Already Spwaned. Please check Outerline.") );
		if(nullptr != GEditor)
		{
			GEditor->SelectNone(false, false, false);
			GEditor->SelectActor(WorldGridStreamInstancesActor, true, false, true);
			if(nullptr != World)
			{
				GEditor->edactDeleteSelected(World);
			}
		}
	}
}
#endif //WITH_EDITOR


void AWorldGridStreamSettings::SetWorldScale(float InWorldScale)
{
	WorldScale = InWorldScale;
#if WITH_EDITORONLY_DATA
	DivideDistance = static_cast<float>(static_cast<uint16>(DivideDistancePowerOfTwo)) * WorldScale;
	VisibilityDistance = DivideDistance;
#endif //WITH_EDITORONLY_DATA
}

#if WITH_EDITOR
void AWorldGridStreamSettings::ShowDivideRect(bool InbVisualizeDivideRect)
{
}

void AWorldGridStreamSettings::BuildWorldGridAssets(bool InbBuildWorldGridAssets)
{
	FWorldGridStreamBuilder WorldGridStreamBuilder;
	WorldGridStreamBuilder.RunBuilder(GetWorld(), DivideDistance, bIncludeZDistance);
}
#endif //WITH_EDITOR

void AWorldGridStreamSettings::ResiterDelegate()
{
#if WITH_EDITOR
	if(nullptr != GEngine)
	{
		OnActorDeletedDelegateHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &AWorldGridStreamSettings::OnActorDeleted);
	}
#endif //WITH_EDITOR
}

END_FUNCTION_BUILD_OPTIMIZATION