#include "graphics.h"
#include "graphics_math.cpp"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
// NOTE: Side length of cubes
#define CUBE_SIDE 16.0f
#define SPEED 1.0f
#define ANGULAR_SPEED 5.0f
#define PLANE_WIDTH 640
#define PLANE_HEIGHT 320
#define FOV 60.0f
#define WALL_VIEW_DISTANCE 3.0f
#define FLOOR_VIEW_DISTANCE 3.5f

// NOTE: Zero is empty.
bool32 Map[MAP_WIDTH][MAP_HEIGHT] = 
{   {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {2,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,1},
    {2,0,1,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {2,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,0,2,0,0,1},
    {1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,2,0,0,1},
    {1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,2,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,3,0,2,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,3,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,2,1,0,2,3,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};

global_variable v2 Position = {24.0f, 24.0f};
global_variable v2 Velocity = {};
global_variable real32 Angle = -90.0f;
global_variable real32 DistanceToPlane = (PLANE_WIDTH / 2.0f) / tanf(DegreesToRadians(FOV / 2.0f));
global_variable real32 AngleBetweenRays = FOV / PLANE_WIDTH;

internal int
GetCube(v2 Position)
{
    int CubeX = (int)(Position.X/CUBE_SIDE);
    int CubeY = (int)(Position.Y/CUBE_SIDE);
    if(CubeX < 0 || CubeY < 0 || CubeX >= MAP_WIDTH || CubeY >= MAP_HEIGHT)
    {
        return 1;
    }
    return Map[CubeY][CubeX];
}

internal uint32
ShadeColor(uint32 Color, real32 Shading)
{
    uint8 Blue = (uint8)Color;
    uint8 Green = (uint8)(Color >> 8);
    uint8 Red = (uint8)(Color >> 16);
    Blue = (uint8)((real32)Blue * Shading);
    Red = (uint8)((real32)Red * Shading);
    Green = (uint8)((real32)Green * Shading);
    return (Blue | (Green << 8)) | (Red << 16);
}

// NOTE: X and Y are between 0 and 1
internal uint32
GetPixelFromImage(image Image, real32 X, real32 Y)
{
    int Offset = (int)(Y * Image.Height) * Image.Width + (int)(X * Image.Width);
    if(Offset < 0 || Offset >= Image.Width * Image.Height)
    {
        return 0;
    }
    return *((uint32 *)Image.Memory + Offset);
}

internal ray_hit
CastRay(v2 Position, real32 Angle)
{
    v2 CubeCordinates = {Floor(Position.X / CUBE_SIDE), Floor(Position.Y / CUBE_SIDE)};
    v2 Direction = {Cos(Angle), -Sin(Angle)};
    ray_hit Hit;
    
    {
        v2 Initial = {};
        v2 Delta = {};
        
        if(Direction.Y < 0)
        {
            Initial.Y = CUBE_SIDE * CubeCordinates.Y - 0.001f;
            Delta.Y = -CUBE_SIDE;
        }
        else
        {
            Initial.Y = CUBE_SIDE * CubeCordinates.Y + CUBE_SIDE;
            Delta.Y = CUBE_SIDE;
        }
        
        if(abs(Direction.Y) < 0.0001f)
        {
            Direction.Y = 0.0001f;
        }
        
        Initial.X = abs((Initial.Y - Position.Y) / Direction.Y) * Direction.X + Position.X;
        
        Delta.X = abs(CUBE_SIDE / Direction.Y) * Direction.X;
        
        v2 PotentialIntersection = Initial;
        while(GetCube(PotentialIntersection) == 0)
        {
            PotentialIntersection = PotentialIntersection + Delta;
        }
        Hit.Distance = DistanceBetweenVectors(Position, PotentialIntersection);
        Hit.Object = GetCube(PotentialIntersection);
        Hit.X = (PotentialIntersection.X - CUBE_SIDE *
                 (int)(PotentialIntersection.X / CUBE_SIDE))/CUBE_SIDE;
    }
    
    {
        v2 Initial = {};
        v2 Delta = {};
        
        if(Direction.X < 0)
        {
            Initial.X = CUBE_SIDE * CubeCordinates.X - 0.001f;
            Delta.X = -CUBE_SIDE;
        }
        else
        {
            Initial.X = CUBE_SIDE * CubeCordinates.X + CUBE_SIDE;
            Delta.X = CUBE_SIDE;
        }
        
        if(abs(Direction.X) < 0.0001f)
        {
            Direction.X = -0.0001f;
        }
        
        Initial.Y = abs((Initial.X - Position.X) / Direction.X) * Direction.Y + Position.Y;
        Delta.Y = abs(CUBE_SIDE / Direction.X) * Direction.Y;
        
        v2 PotentialIntersection = Initial;
        while(GetCube(PotentialIntersection) == 0)
        {
            PotentialIntersection = PotentialIntersection + Delta;
        }
        real32 Distance = DistanceBetweenVectors(Position, PotentialIntersection);
        
        if(Distance < Hit.Distance)
        {
            Hit.Distance = Distance;
            Hit.Object = GetCube(PotentialIntersection);
            Hit.X = (PotentialIntersection.Y - CUBE_SIDE * 
                     (int)(PotentialIntersection.Y / CUBE_SIDE))/CUBE_SIDE;            
        }
    }
    
    return Hit;
}

// TODO: Horizontal wall bug
//       360 degrees bug
internal void
RenderPlane(screen_buffer *Buffer, image Image, v2 PlanePosition)
{
    //real32 PlaneWidth = 16.0f;
    real32 PlaneHeight = 8.0f;
    real32 Distance = DistanceBetweenVectors(Position, PlanePosition);
    int Height = (int)((PlaneHeight / Distance) * DistanceToPlane);
    int Width = Height;
    
    if(Width > Buffer->Width)
    {
        Width = Buffer->Width;
        Height = Width;
    }
    
    uint32 *Pixel = (uint32 *)Buffer->Memory;
    Pixel += (Buffer->Width / 2) - (Width / 2);
    Pixel += ((Buffer->Height / 2) - (Height / 2)) * Buffer->Width;
    
    v2 Direction = Normalize(PlanePosition - Position);
    
    // TODO: Don't use math.h trig functions outside of initialization, it's slow.
    ray_hit Hit = CastRay(Position, RadiansToDegrees(atan2f(-Direction.Y, Direction.X)));
    real32 WallDistance = Hit.Distance;
    real32 PlaneDistance = VectorMagnitude(PlanePosition - Position);
    if(WallDistance < PlaneDistance)
    {
        return;
    }
    
    v2 Plane = {Cos(Angle), -Sin(Angle)};
    Plane = DistanceToPlane * Plane;
    
    real32 AngleBetween = abs(RadiansToDegrees(acosf(DotProduct(Plane, Direction) / VectorMagnitude(Plane))));
    if(AngleBetween > (FOV / 2.0f))
    {
        return;
    }
    
    v2 PlaneOffset = ((VectorMagnitude(Plane) / (DotProduct(Direction, Plane) / VectorMagnitude(Plane))) * Direction) - Plane;
    
    int Offset = (int)VectorMagnitude(PlaneOffset);
    
    // TODO: Think about how to get horizontal offset better. There seems to be a cleaner way to do this.
    if(Plane.Y > 0 && PlaneOffset.X > 0)
    {
        Offset = -Offset;
    }
    else if(Plane.Y < 0 && PlaneOffset.X < 0)
    {
        Offset = -Offset;
    }
    
    Pixel += Offset;
    
    for(int Y = 0; Y < Height; Y++)
    {
        uint32 *RowPixel = Pixel;
        for(int X = 0; X < Width; X++)
        {
            if(RowPixel < Buffer->Memory || RowPixel >= ((uint32 *)Buffer->Memory + Buffer->Width * Buffer->Height))
            {
                continue;
            }
            
            if(Buffer->Width / 2 - Width / 2 + Offset + X >= Buffer->Width)
            {
                break;
            }
            
            if(Buffer->Width / 2 - Width / 2 + Offset + X < 0)
            {
                RowPixel++;
                continue;
            }
            
            uint32 Color = GetPixelFromImage(Image, (real32)X/Width, (real32)Y/Height);
            if(Color == 0xff00ff)
            {
                RowPixel++;
                continue;
            }
            
            real32 Shading = (32.0f - VectorMagnitude(PlanePosition - Position)) / 32.0f;
            if(Shading > 1.0f)
            {
                Shading = 1.0f;
            }
            if(Shading < 0.0f)
            {
                Shading = 0.0f;
            }
            *RowPixel++ = ShadeColor(Color, 1.0f);
        }
        Pixel += Buffer->Width;
    }
}

internal void
ClearScreen(screen_buffer *Buffer)
{
    uint32 *Pixel = (uint32 *)Buffer->Memory;
    for(int PixelIndex = 0; PixelIndex < Buffer->Width * Buffer->Height; PixelIndex++)
    {
        *Pixel++ = 0x000000;
    }
}

// TODO: Replace with faster sort
internal void
SortPositionsByFarthest(v2 *Pos, const int NumPositions)
{
    for(int SmallestIndex = 0;
        SmallestIndex < NumPositions;
        SmallestIndex++)
    {
        real32 CurrentSmallest = DistanceBetweenVectors(Position, *(Pos + SmallestIndex));
        int CurrentSmallestIndex = SmallestIndex;
        for(int CandidateIndex = SmallestIndex + 1;
            CandidateIndex < NumPositions;
            CandidateIndex++)
        {
            real32 DistanceToPlayer = DistanceBetweenVectors(Position, *(Pos + CandidateIndex));
            if(DistanceToPlayer > CurrentSmallest)
            {
                CurrentSmallest = DistanceToPlayer;
                CurrentSmallestIndex = CandidateIndex;
            }
        }
        v2 Temp = *(Pos + SmallestIndex);
        *(Pos + SmallestIndex) = *(Pos + CurrentSmallestIndex);
        *(Pos + CurrentSmallestIndex) = Temp;
    }
}

global_variable image Images[5];
global_variable image CeilingImage;
global_variable image FloorImage;
global_variable image BulletImage;
global_variable image EnemyImage;
global_variable bool32 Initialized = false;

internal void 
UpdateAndRender(input *Input,
                screen_buffer *Buffer)
{
    if(!Initialized)
    {
        PrecomputeTrig();
        // NOTE: Images indicies correspond to Hit.Object values
        Images[1] = LoadBMP("textures/gray.bmp");
        Images[2] = LoadBMP("textures/cyan.bmp");
        Images[3] = LoadBMP("textures/red.bmp");
        Images[4] = LoadBMP("textures/enemy.bmp");
        CeilingImage = Images[2];
        FloorImage = Images[3];
        EnemyImage = Images[4];
        BulletImage = LoadBMP("textures/bullet.bmp");
        Initialized = true;
    }
    
    // NOTE: Updating
    
    if(Input->Left.IsDown)
    {
        Angle += ANGULAR_SPEED;
    }
    
    if(Input->Right.IsDown)
    {
        Angle -= ANGULAR_SPEED;
    }
    
    if(Input->Up.IsDown)
    {
        Velocity.X = SPEED * Cos(Angle);
        Velocity.Y = SPEED * -Sin(Angle);
    }
    else if(Input->Down.IsDown)
    {
        Velocity.X = SPEED * -Cos(Angle);
        Velocity.Y = SPEED * Sin(Angle);
    }
    else
    {
        Velocity.X = 0.0f;
        Velocity.Y = 0.0f;
    }
    
    // TODO: Fix bug in movement code, can move diagonally through walls
    if(GetCube(Position + Velocity) == 0)
    {
        Position = Position + Velocity;
    }
    else
    {
        v2 DeltaX = {Velocity.X, 0};
        v2 DeltaY = {0, Velocity.Y};
        
        if(GetCube(Position + DeltaX) == 0)
        {
            Position = Position + DeltaX;
        }
        else if(GetCube(Position + DeltaY) == 0)
        {
            Position = Position + DeltaY;
        }
    }
    
    
    local_persist v2 PlanePosition = {24.0f, 24.0f + 64.0f - 8.0f};
    v2 DirectionToPlayer = Normalize(Position - PlanePosition);
    
    if(DistanceBetweenVectors(PlanePosition, Position) > 32.0f)
    {
        PlanePosition = PlanePosition + 0.5f * DirectionToPlayer;    
    }
    
    
    local_persist v2 BulletPosition = {};
    local_persist v2 BulletVelocity = {};
    local_persist bool32 BulletActive = false;
    local_persist real32 BulletSpeed = 5.0f;
    
    if(Input->Space.IsDown && !Input->Space.WasDown)
    {
        BulletActive = true;
        BulletPosition = Position;
        BulletVelocity.X = BulletSpeed * Cos(Angle);
        BulletVelocity.Y = BulletSpeed * -Sin(Angle);
    }    
    
    BulletPosition = BulletPosition + BulletVelocity;
    
    // NOTE: Rendering

    ClearScreen(Buffer);

    real32 StartAngle = Angle + FOV/2.0f;
    real32 CurrentAngle = StartAngle;
    real32 AngleStep = AngleBetweenRays;
    uint32 *Pixel = (uint32 *)Buffer->Memory;

    for(int Column = 0; Column < PLANE_WIDTH; Column++)
    {
        ray_hit Hit = CastRay(Position, CurrentAngle);
        real32 UncurvedDistance = Hit.Distance * Cos(abs(CurrentAngle - Angle));

        int ColumnHeight = (int)((DistanceToPlane * CUBE_SIDE) / UncurvedDistance);
        int OriginalHeight = ColumnHeight;
        if(ColumnHeight > PLANE_HEIGHT) ColumnHeight = PLANE_HEIGHT;

        uint32 *RowPixel = Pixel;
        int WallOffset = (PLANE_HEIGHT - ColumnHeight) / 2;
        int Row = 0;
        
        for(int RowIndex = 0; RowIndex < WallOffset; RowIndex++)
        {
            int RowsFromCenter = abs((int)(PLANE_HEIGHT/2.0f) - RowIndex);
            
            real32 Dx = Cos(CurrentAngle);
            real32 Dy = -Sin(CurrentAngle);
            real32 Dz = (RowsFromCenter) / DistanceToPlane;

            real32 Distance = 8.0f / Dz;
            real32 X = Position.X + Dx * Distance;
            real32 Y = Position.Y + Dy * Distance;
            X = (X - 16.0f * (int)(X / 16.0f)) / 16.0f;
            Y = (Y - 16.0f * (int)(Y / 16.0f)) / 16.0f;
            
            real32 Shading = (FLOOR_VIEW_DISTANCE - Distance / CUBE_SIDE) / FLOOR_VIEW_DISTANCE;
            if(Shading < 0) Shading = 0.0f;
            uint32 Color = GetPixelFromImage(CeilingImage, X, Y);
            *RowPixel = ShadeColor(Color, Shading + 0.05f);

            RowPixel += Buffer->Width;
            Row++;
        }

        image WallImage = Images[Hit.Object];

        for(int RowIndex = 0; RowIndex < ColumnHeight; RowIndex++)
        {
            real32 X = Hit.X;
            real32 Y = (real32)RowIndex / (real32)OriginalHeight;

            if(ColumnHeight < OriginalHeight)
            {
                Y += ((OriginalHeight - ColumnHeight) / 2.0f) / OriginalHeight;
            }
            
            real32 Shading = (WALL_VIEW_DISTANCE - Hit.Distance / CUBE_SIDE) / WALL_VIEW_DISTANCE;
            if(Shading < 0) Shading = 0.0f;

            *RowPixel = ShadeColor(GetPixelFromImage(WallImage, X, Y), Shading + 0.0f);

            RowPixel += Buffer->Width;
            Row++;
        }

        // TODO: Repetative
        for(;Row < PLANE_HEIGHT; Row++)
        {
            int RowIndex = PLANE_HEIGHT - Row;
            int RowsFromCenter = abs((int)(PLANE_HEIGHT/2.0f) - RowIndex);
            
            real32 Dx = Cos(CurrentAngle);
            real32 Dy = -Sin(CurrentAngle);
            real32 Dz = (RowsFromCenter) / DistanceToPlane;

            real32 Distance = 8.0f / Dz;
            real32 X = Position.X + Dx * Distance;
            real32 Y = Position.Y + Dy * Distance;
            X = (X - 16.0f * (int)(X / 16.0f)) / 16.0f;
            Y = (Y - 16.0f * (int)(Y / 16.0f)) / 16.0f;

            real32 Shading = (FLOOR_VIEW_DISTANCE - Distance / CUBE_SIDE) / FLOOR_VIEW_DISTANCE;
            if(Shading < 0) Shading = 0.0f;
            uint32 Color = GetPixelFromImage(FloorImage, X, Y);
            *RowPixel = ShadeColor(Color, Shading + 0.05f);
            
            RowPixel += Buffer->Width;
        }

        Pixel++;
        CurrentAngle -= AngleStep;
    }

    RenderPlane(Buffer, EnemyImage, PlanePosition);
    //RenderPlane(Buffer, BulletImage, BulletPosition);
}