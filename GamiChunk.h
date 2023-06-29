// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "GamiChunk.generated.h"


#define CHUNK_DEFAULT_W 50
#define CHUNK_DEFAULT_H 100
#define CHUNK_SIZE		100
UCLASS()
class MINECRAFT_API AGamiChunk : public AActor
{
	GENERATED_BODY()
public:
	typedef struct {
		//unsigned short > 2byte/16bit
		//前8位作为方块耐久度, 后8位作为方块ID
		unsigned short page;
	}stChunkInfo;
public:	
	// Sets default values for this actor's properties
	AGamiChunk();
protected:
	void initialize();
	void caculateShape();
	
	inline FVector convert1dto3d( int index);
	inline unsigned int covert3dto1d( FVector position);
private:
	void GenerateCube(FVector Center, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<int32>& Triangles);
	void GeneratePlaneMesh(const FVector& Center, const FVector2D& Size, const FVector& Normal, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<int32>& Triangles);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UProceduralMeshComponent* _procedural_mesh;
private:
	stChunkInfo _data[CHUNK_DEFAULT_W * CHUNK_DEFAULT_W * CHUNK_DEFAULT_H];
};
