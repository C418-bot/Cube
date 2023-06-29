// Fill out your copyright notice in the Description page of Project Settings.
#include "GamiChunk.h"
#include "include/FastNoise/FastNoise.h"
#pragma comment(lib ,R"(D:\work\ue\Minecraft\Source\Minecraft\libs\FastNoiseD.lib)")
// Sets default values



#define DEBUG_END char output[128] = { 0 }; \
                  sprintf(output, "message > %s => yes", __FUNCTION__); \
                  GEngine->AddOnScreenDebugMessage(0, 5000, FColor::Green, output);

static const int          CHUNK_DEFAULT_SIZE = CHUNK_DEFAULT_W * CHUNK_DEFAULT_W;
static const int          CHUNK_DEFAULT_VOLUME = CHUNK_DEFAULT_SIZE * CHUNK_DEFAULT_H;
AGamiChunk::AGamiChunk() :
_procedural_mesh(nullptr),
_data{0}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


void GeneratePlane(int32 NumX, int32 NumY, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs, TArray<FVector>& Normals, TArray<FVector>& Tangents)
{
    int32 NumVertsX = NumX + 1;
    int32 NumVertsY = NumY + 1;

    Vertices.Empty();
    UVs.Empty();
    Normals.Empty();
    Tangents.Empty();
    Triangles.Empty();

    // Generate vertices and UVs
    for (int32 y = 0; y < NumVertsY; ++y)
    {
        for (int32 x = 0; x < NumVertsX; ++x)
        {
            FVector VertexPos(static_cast<float>(x) / static_cast<float>(NumX) - 0.5f, static_cast<float>(y) / static_cast<float>(NumY) - 0.5f, 0.0f);
            Vertices.Add(VertexPos);

            UVs.Add(FVector2D(static_cast<float>(x) / static_cast<float>(NumX), static_cast<float>(y) / static_cast<float>(NumY)));
        }
    }

    // Generate triangles
    for (int32 y = 0; y < NumY; ++y)
    {
        for (int32 x = 0; x < NumX; ++x)
        {
            int32 Index0 = y * NumVertsX + x;
            int32 Index1 = y * NumVertsX + x + 1;
            int32 Index2 = (y + 1) * NumVertsX + x;
            int32 Index3 = (y + 1) * NumVertsX + x + 1;

            Triangles.Add(Index0);
            Triangles.Add(Index1);
            Triangles.Add(Index2);

            Triangles.Add(Index2);
            Triangles.Add(Index1);
            Triangles.Add(Index3);
        }
    }

    // Generate normals and tangents
    for (int32 i = 0; i < Vertices.Num(); ++i)
    {
        Normals.Add(FVector::UpVector);
        Tangents.Add(FVector::RightVector);
    }
}

void AGamiChunk::GeneratePlaneMesh(const FVector& Center, const FVector2D& Size, const FVector& Normal, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<int32>& Triangles){
    int vertices_size = Vertices.Num();

    // Calculate the tangents
    FVector AxisX, AxisY;
    Normal.FindBestAxisVectors(AxisX, AxisY);
    const FVector TangentX = AxisY;
    const FVector TangentY = -AxisX;

    // Generate vertices and tangents
    const FVector2D HalfSize = Size / 2.f;
    const FVector Corner00 = Center - TangentX * HalfSize.X - TangentY * HalfSize.Y;
    const FVector Corner01 = Center + TangentX * HalfSize.X - TangentY * HalfSize.Y;
    const FVector Corner10 = Center - TangentX * HalfSize.X + TangentY * HalfSize.Y;
    const FVector Corner11 = Center + TangentX * HalfSize.X + TangentY * HalfSize.Y;
    const FProcMeshTangent ProcTangent(TangentX, true);

    Vertices.Add(Corner00);
    Vertices.Add(Corner01);
    Vertices.Add(Corner10);
    Vertices.Add(Corner11);

    for (int i = 0; i < 4; i++) {
        Normals.Add(Normal);
        Tangents.Add(ProcTangent);
    }
    
    UVs.Add(FVector2D(0.f, 0.f));
    UVs.Add(FVector2D(0.f, 1.f));
    UVs.Add(FVector2D(1.f, 0.f));
    UVs.Add(FVector2D(1.f, 1.f));

    Triangles.Add(0 + vertices_size);
    Triangles.Add(1 + vertices_size);
    Triangles.Add(2 + vertices_size);
    Triangles.Add(2 + vertices_size);
    Triangles.Add(1 + vertices_size);
    Triangles.Add(3 + vertices_size);
}
void AGamiChunk::GenerateCube(FVector Center,  TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents, TArray<int32>& Triangles){
    static float                                                CHUNK_SIZE_HALF = floor(CHUNK_SIZE / 2.f);
    FVector2D Size(CHUNK_SIZE);
    FVector   Normal(0.f, 0.f, 1.f);
    GeneratePlaneMesh(Center + FVector(0.f, 0.f, CHUNK_SIZE), Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
    Normal = FVector(0.f, 0.f, -1.f);
    GeneratePlaneMesh(Center, Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
    Normal = FVector(1.f, 0.f, 0.f);
    GeneratePlaneMesh(Center+ FVector(CHUNK_SIZE_HALF, 0.f, CHUNK_SIZE_HALF), Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
    Normal = FVector(-1.f, 0.f, 0.f);
    GeneratePlaneMesh(Center+ FVector(-CHUNK_SIZE_HALF, 0.f, CHUNK_SIZE_HALF), Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
    Normal = FVector(0.f, 1.f, 0.f);
    GeneratePlaneMesh(Center+ FVector(0.f, CHUNK_SIZE_HALF, CHUNK_SIZE_HALF), Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
    Normal = FVector(0.f, -1.f, 0.f);
    GeneratePlaneMesh(Center+ FVector(0.f, -CHUNK_SIZE_HALF, CHUNK_SIZE_HALF), Size, Normal, Vertices, Normals, UVs, Tangents, Triangles);
}

void AGamiChunk::initialize(){
    _procedural_mesh = NewObject<UProceduralMeshComponent >(this, "chunk");
    _procedural_mesh->RegisterComponent();
    _procedural_mesh->SetupAttachment(RootComponent);
    DEBUG_END
}
void AGamiChunk::caculateShape(){
#define IS_VALID(INDEX) if(INDEX >= 0 && INDEX < CHUNK_DEFAULT_SIZE)
    static int _seed_a = 25565;
    static int _seed_b = std::rand();
    static int _seed_c = std::rand();
    static int _seed_d = std::rand();
    static int _seed_e = std::rand();
    static FastNoise::SmartNode<> _perlin_noise_a = FastNoise::New<FastNoise::Perlin>();
    static FastNoise::SmartNode<> _simple_noise_a = FastNoise::New<FastNoise::Simplex>();
    static FastNoise::SmartNode<> _cellularValue_noise_a = FastNoise::New<FastNoise::CellularValue>();

    FVector Center(0.f);
    FVector2D Size(CHUNK_SIZE);
    FVector Normal(0.f, 0.f, 1.f);
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FProcMeshTangent> Tangents;
    TArray<int32> Triangles;

    for ( int index = 0; index < CHUNK_DEFAULT_VOLUME; index++) {
        if (_data[index].page != 0)
            continue;
        FVector myself_position = std::move(convert1dto3d(index));
        
        /****
        FVector top = myself_position + FVector(0, 0, CHUNK_SIZE);
        FVector bottom = myself_position + FVector(0, 0, -CHUNK_SIZE);
        FVector left = myself_position + FVector(0, CHUNK_SIZE, 0);
        FVector right = myself_position + FVector(0, -CHUNK_SIZE, 0);
        FVector forward = myself_position + FVector(CHUNK_SIZE, 0, 0);
        FVector backward = myself_position + FVector(-CHUNK_SIZE, 0, 0);

        int top_index = std::move(covert3dto1d(top));
        int bottom_index = std::move(covert3dto1d(bottom));
        int left_index = std::move(covert3dto1d(left));
        int right_index = std::move(covert3dto1d(right));
        int forward_index = std::move(covert3dto1d(forward));
        int backward_index = std::move(covert3dto1d(backward));
        */

        auto caculate2DNoise = [=](FVector myself_position) ->int{
            int depth = (_perlin_noise_a->GenSingle2D(myself_position.X * 0.000002, myself_position.Y * 0.000002, _seed_a) * CHUNK_DEFAULT_H); /* +
                _perlin_noise_a->GenSingle2D(myself_position.X * 0.00001, myself_position.Y * 0.00001, _seed_b) +
                _simple_noise_a->GenSingle2D(myself_position.X * 0.0001, myself_position.Y * 0.0001, _seed_c)) *
                ((_perlin_noise_a->GenSingle2D(myself_position.X * 0.00001, myself_position.Y * 0.00001, _seed_d + _seed_e) +
                  _perlin_noise_a->GenSingle2D(myself_position.X * 0.0001, myself_position.Y * 0.0001, _seed_d)) * CHUNK_DEFAULT_H)*/;
            return depth;
        };
        auto caculate3DNoise = [=](FVector myself_position) ->float {
            float depth3d = _cellularValue_noise_a->GenSingle3D(myself_position.X * 0.001, myself_position.Y * 0.001, myself_position.Z * 0.001, _seed_a);
            return depth3d;
        };

        auto depth =caculate2DNoise(myself_position);
        auto depth3d = caculate3DNoise(myself_position);

        static int half_height= int((CHUNK_SIZE * CHUNK_DEFAULT_H) / 2);
        depth = floor(depth) * CHUNK_SIZE;

        
        if (myself_position.Z == (half_height + depth) && depth3d > 1.0) {
            _data[index].page = 1;
            //GenerateCube(myself_position, Vertices, Normals, UVs, Tangents, Triangles);
        }else if (myself_position.Z <= (half_height + depth) && depth3d < 0.5) {
            _data[index].page = 1;
            //GenerateCube(myself_position, Vertices, Normals, UVs, Tangents, Triangles);
        }
        else
            _data[index].page = 0;

        /******
        IS_VALID(top_index){
            _data[top_index].page = 1;            
        }
        IS_VALID(bottom_index){
            _data[bottom_index].page = 1;
        }
        IS_VALID(left_index) {
            _data[left_index].page = 1;
        }
        IS_VALID(right_index) {
            _data[right_index].page = 1;
        }
        IS_VALID(forward_index) {
            _data[forward_index].page = 1;
        }
        IS_VALID(backward_index) {
            _data[backward_index].page = 1;
        }
        */
    }





    _procedural_mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);
}
inline FVector AGamiChunk::convert1dto3d( int index){
    //一维数组升三维,这里的坐标全部为“单位化”的数据
    int z = index / CHUNK_DEFAULT_SIZE;
    int y = (index - (z * CHUNK_DEFAULT_SIZE)) / CHUNK_DEFAULT_W;
    int x = (index - (z * CHUNK_DEFAULT_SIZE)) - y * CHUNK_DEFAULT_W ;
    return FVector(x, y, z) * CHUNK_SIZE;
}
inline unsigned int AGamiChunk::covert3dto1d(FVector position){
    //三维降维打击=>一维
    position = position / CHUNK_SIZE;
    int x = position.X;
    int y = position.Y;
    int z = position.Z;
    return y * CHUNK_DEFAULT_W + x + (z * CHUNK_DEFAULT_SIZE);
}
// Called when the game starts or when spawned
void AGamiChunk::BeginPlay(){
	Super::BeginPlay();
    initialize();
    caculateShape();
}
// Called every frame
void AGamiChunk::Tick(float DeltaTime){
	Super::Tick(DeltaTime);
}

