#define Pi32 3.14159265359f

struct v2
{
    real32 X;
    real32 Y;
};

v2 operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

v2 operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

v2 operator*(real32 C, v2 B)
{
    v2 Result;
    Result.X = C * B.X;
    Result.Y = C * B.Y;
    return Result;
}

v2 operator/(v2 A, real32 C)
{
    v2 Result;
    Result.X = A.X / C;
    Result.Y = A.Y / C;
    return Result;
}

real32 Floor(real32 Number)
{
    return (real32)((int)Number);
}

real32 Ceil(real32 Number)
{
    return (real32)((int)(Number + 1));
}

real32 DegreesToRadians(real32 Degrees)
{
    return (Degrees / 360.0f) * 2 * Pi32;
}

real32 RadiansToDegrees(real32 Radians)
{
    return (Radians / (2.0f * Pi32)) * 360.0f;
}

real32 DistanceBetweenVectors(v2 A, v2 B)
{
    real32 Dx = A.X - B.X;
    real32 Dy = A.Y - B.Y;
    return sqrtf(Dx * Dx + Dy * Dy);
}

real32 VectorMagnitude(v2 Vector)
{
    return sqrtf(Vector.X * Vector.X + Vector.Y * Vector.Y);
}

v2 Normalize(v2 Vector)
{
    real32 Mag = VectorMagnitude(Vector);
    if(Mag == 0)
    {
        Mag = .00001f;
    }
    return Vector / Mag;
}

real32 DotProduct(v2 A, v2 B)
{
    return ((A.X * B.X) + (A.Y * B.Y));
}

global_variable real32 SinTable[360 * 100];
global_variable real32 CosTable[360 * 100];
void PrecomputeTrig()
{
    for(int Angle = 0; Angle < 360 * 100; Angle++)
    {
        SinTable[Angle] = sinf(DegreesToRadians((real32)Angle/100.0f));
        CosTable[Angle] = cosf(DegreesToRadians((real32)Angle/100.0f));
    }
}

real32 Sin(real32 Angle)
{
    int AngleIndex = (int)(Angle * 100.0f) % (360 * 100);
    if(AngleIndex < 0) AngleIndex = (360 * 100) + AngleIndex;
    return SinTable[AngleIndex];
}

real32 Cos(real32 Angle)
{
    int AngleIndex = (int)(Angle * 100.0f) % (360 * 100);
    if(AngleIndex < 0) AngleIndex = (360 * 100) + AngleIndex;
    return CosTable[AngleIndex];
}

int IntegerSign(int Integer)
{
    if(Integer < 0)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}
