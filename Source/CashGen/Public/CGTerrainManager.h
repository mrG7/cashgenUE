#pragma once
#include "GameFramework/Actor.h"
#include "Struct/CGJob.h"
#include "Struct/CGTerrainConfig.h"
#include "Struct/CGMeshData.h"
#include "Struct/CGPoint.h"
#include "Struct/CGLODMeshData.h"
#include "CGTerrainManager.generated.h"

class ACGTile;

UCLASS(BlueprintType, Blueprintable)
class ACGTerrainManager : public AActor
{
	GENERATED_BODY()

	bool isSetup = false;
	CGPoint currentPlayerZone = CGPoint(0,0);
	TArray<FRunnableThread*> WorkerThreads;

	UPROPERTY()
	TArray<FCGLODMeshData> MeshData;
	TArray<TSet<FCGMeshData*>> FreeMeshData;
	TArray<TSet<FCGMeshData*>> InUseMeshData;
	FVector WorldOffset;
	TArray<ACGTile*> Tiles;

	bool GetFreeMeshData(FCGJob& aJob);
	void ReleaseMeshData(uint8 aLOD, FCGMeshData* aDataToRelease);
	void AllocateAllMeshDataStructures();
	bool AllocateDataStructuresForLOD(FCGMeshData* aData, FCGTerrainConfig* aConfig, const uint8 aLOD);
	void CreateTileRefreshJob(FCGJob aJob);
	void SweepLODs();
	uint8 GetLODForTile(ACGTile* aTile);

	CGPoint GetXYfromIdx(const int32 idx) { return CGPoint(idx % XTiles, idx / YTiles); }
	float TimeSinceLastSweep;
	const float SweepInterval = 0.1f;

	bool isFirstDraw = true;

public:
	ACGTerrainManager();
	~ACGTerrainManager();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void HandleTileFlip(int32 sectorX, int32 sectorY);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCallTileFlip(int32 sectorX, int32 sectorY);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTileMeshUpdated, ACGTile*, Tile);
	UPROPERTY(BlueprintAssignable)
		FTileMeshUpdated OnTileMeshUpdated;

	UFUNCTION(BlueprintImplementableEvent, Category = "CGWorld")
	void OnInitialTileDrawComplete();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CashGen")
	AActor* TrackingActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CashGen")
	int32 XTiles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CashGen")
	int32 YTiles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CashGen")
	FCGTerrainConfig TerrainConfig;

	/* Spawn the terrain */
	UFUNCTION(BlueprintCallable, Category = "CashGen")
		void SetupTerrainClient(UUFNNoiseGenerator* aNoiseGen, UUFNNoiseGenerator* aBiomeBlendGen, AActor* aTrackingActor);

	TQueue<FCGJob, EQueueMode::Spsc> PendingJobs;
	TArray<TQueue<FCGJob, EQueueMode::Spsc>> GeometryJobs;
	TQueue<FCGJob, EQueueMode::Mpsc> UpdateJobs;

	TSet<ACGTile*> QueuedTiles;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//void SpawnTiles(AActor* aTrackingActor, const FCGTerrainConfig aTerrainConfig, const int32 aXTiles, const int32 aYTiles);
	//UFUNCTION(NetMulticast, Reliable, WithValidation)
	UFUNCTION(BlueprintCallable, Category = "CashGen")
		void SpawnTiles(const int32 aXTiles, const int32 aYTiles) { ServerCallSpawnTiles(aXTiles, aYTiles); };

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void HandleSpawnTiles(const int32 aXTiles, const int32 aYTiles);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerCallSpawnTiles(const int32 aXTiles, const int32 aYTiles);

	void BeginDestroy() override;
};