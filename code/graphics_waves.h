#if !defined(GRAPHICS_H)

#if SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

/* NOTE: Application specific structs */

/* NOTE: Platform/Application shared structs */

struct screen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

struct button
{
    bool32 IsDown;
    bool32 WasDown;
};

struct input
{
    button Up;
    button Down;
    button Right;
    button Left;
    button Space;
};

struct image
{
    int Width;
    int Height;
    void *Memory;
};

/* NOTE: Platform services */

internal image LoadBMP(char *FileName);

struct file
{
    uint32 ContentsSize;
    void *Contents;
};

internal file PlatformReadEntireFile(char *FileName);
internal bool32 PlatformWriteEntireFile(char *FileName, uint32 MemorySize, void *Memory);
internal void PlatformFreeFileMemory(void *Memory);

/* NOTE: Application Services */

internal void UpdateAndRender(input *Input, screen_buffer *Buffer);

#define GRAPHICS_H
#endif
