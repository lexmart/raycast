// next day to watch 015

#include <stdint.h>
#include <math.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;

#include "graphics.h"
#include "graphics.cpp"

#include "windows.h"
#include "stdio.h"
#include "malloc.h"

#include "win32_graphics.h"

global_variable bool32 GlobalRunning;
global_variable win32_screen_buffer GlobalBackBuffer;
global_variable int64 GlobalPerfCountFrequency;

internal image 
LoadBMP(char *FileName)
{
    image Image = {};

    file File = PlatformReadEntireFile(FileName);
    if(File.Contents)
    {
        uint8 *Offset = (uint8 *)File.Contents;
        BITMAPFILEHEADER *BitmapHeader = (BITMAPFILEHEADER *)Offset;
        Offset += sizeof(BITMAPFILEHEADER);
        BITMAPINFOHEADER BitmapInfo = ((BITMAPINFO *)(Offset))->bmiHeader;
        
        Image.Width = BitmapInfo.biWidth;
        Image.Height = BitmapInfo.biHeight;
        int ImageMemorySize = Image.Width * Image.Height * 4;
        Image.Memory = VirtualAlloc(0, ImageMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
        if(Image.Memory)
        {
            uint8 *ImageMemory = (uint8 *)Image.Memory;
            uint8 *Pixel = ((uint8 *)File.Contents + BitmapHeader->bfOffBits);
        
            for(int PixelIndex = 0; PixelIndex < Image.Width * Image.Height; PixelIndex++)
            {
                if(PixelIndex % Image.Width == 0 && PixelIndex != 0)
                {
                    Pixel += Image.Width % 4;
                }

                uint8 Blue = *Pixel++;
                uint8 Green = *Pixel++;
                uint8 Red = *Pixel++;
                *ImageMemory++ = Blue;
                *ImageMemory++ = Green;
                *ImageMemory++ = Red;
                *ImageMemory++ = 0;               
/*                
                char OutputBuffer[256];
                sprintf_s(OutputBuffer, "%d %d %d\n", Blue, Green, Red);
                OutputDebugString(OutputBuffer);
*/
            }
            
            uint32 *TopRowStart = ((uint32 *)Image.Memory)+(Image.Width * Image.Height)-Image.Width;
            uint32 *BottomRowStart = (uint32 *)Image.Memory;
            while(TopRowStart > BottomRowStart)
            {
                uint32 *TopRow = TopRowStart;
                uint32 *BottomRow = BottomRowStart;
                for(int X = 0; X < Image.Width; X++)
                {
                    uint32 Temp = *TopRow;
                    *TopRow = *BottomRow;
                    *BottomRow = Temp;
                    TopRow++;
                    BottomRow++;
                }

                TopRowStart -= Image.Width;
                BottomRowStart += Image.Width;
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    return Image;
}

internal file
PlatformReadEntireFile(char *FileName)
{
    file Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = (uint32)(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                    (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    PlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
            }
        }
        else
        {
        }

        CloseHandle(FileHandle);
    }
    else
    {
    }
    return Result;
}

internal void
PlatformFreeFileMemory(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal bool32
PlatformWriteEntireFile(char *FileName, uint32 MemorySize, void *Memory)
{
    bool32 Result = false;

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            Result = (BytesWritten == MemorySize);
        }
        else
        {
        }
        CloseHandle(FileHandle);
    }
    else
    {
    }

    return Result;
}


internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    win32_window_dimension WindowDimension;
    WindowDimension.Width = ClientRect.right - ClientRect.left;
    WindowDimension.Height = ClientRect.bottom - ClientRect.top;
    return WindowDimension;
}

// DIB = Device Independent Bitmap
internal void
Win32ResizeDIBSection(win32_screen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
 
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
}

internal void
Win32DisplayBufferToWindow(win32_screen_buffer *Buffer,
                           HDC DeviceContext,
                           int WindowWidth, int WindowHeight)
{
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;

    switch(Message)
    {

        case WM_SIZE:
        {
            /*
              win32_window_dimension Dimension = Win32GetWindowDimension(Window);
              Win32ResizeDIBSection(&GlobalBackBuffer, Dimension.Width, Dimension.Height);
            */
        } break;

        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP");
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Bad keyboard input!");
        } break;    

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferToWindow(&GlobalBackBuffer, DeviceContext, 
                                       Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

internal void
Win32ProcessPendingMessages(input *Input)
{
    // process all Windows messages
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        if(Message.message == WM_QUIT)
        {
            GlobalRunning = false;
        }

        switch(Message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 WasDown = (Message.lParam & (1 << 30)) != 0;
                bool32 IsDown = (Message.lParam & (1 << 31)) == 0;
        
                if (WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Input->Up.IsDown = IsDown;
                        Input->Up.WasDown = WasDown;
                    }
                    else if(VKCode == 'A')
                    {
                        Input->Left.IsDown = IsDown;
                        Input->Left.WasDown = WasDown;
                    }
                    else if(VKCode == 'S')
                    {
                        Input->Down.IsDown = IsDown;
                        Input->Down.WasDown = WasDown;
                    }
                    else if(VKCode == 'D')
                    {
                        Input->Right.IsDown = IsDown;
                        Input->Right.WasDown = WasDown;
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        Input->Space.IsDown = IsDown;
                        Input->Space.WasDown = WasDown;
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        GlobalRunning = false;
                    }
                }

                // NOTE: alt-f4 window closing
                bool32 AltKeyWasDown = (Message.lParam & (1 << 29)) != 0;
                if ((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = false;
                }

            } break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    return ((real32)(End.QuadPart - Start.QuadPart)) / (real32)GlobalPerfCountFrequency;        
}

inline LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

int CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);


    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "GraphicsWindowClass";
 
    Win32ResizeDIBSection(&GlobalBackBuffer, 640, 320);

    int FramesPerSecond = 30;
    real32 TargetSecondsPerFrame = 1.0f / (real32)FramesPerSecond;

    if(RegisterClass(&WindowClass))
    {
        HWND Window =
            CreateWindowEx(
                0,
                WindowClass.lpszClassName,
                "Graphics",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                1280,
                640,
                0,
                0,
                Instance,
                0);
       
        if(Window)
        {
            HDC DeviceContext = GetDC(Window);           
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            int64 LastCycleCount = __rdtsc();

            input Input = {};
            
            GlobalRunning = true;
            while(GlobalRunning) 
            {
                Win32ProcessPendingMessages(&Input);
                
                screen_buffer Buffer = {};
                Buffer.Memory = GlobalBackBuffer.Memory;
                Buffer.Width = GlobalBackBuffer.Width;
                Buffer.Height = GlobalBackBuffer.Height;
                Buffer.Pitch = GlobalBackBuffer.Pitch;
                UpdateAndRender(&Input, &Buffer);

                LARGE_INTEGER WorkCounter = Win32GetWallClock();
                real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                    
                real32 SecondsElapsedForFrame = WorkSecondsElapsed;
                if (WorkSecondsElapsed < TargetSecondsPerFrame)
                {
                    while (SecondsElapsedForFrame < TargetSecondsPerFrame)
                    {
                        DWORD SleepMS = (DWORD)((TargetSecondsPerFrame - SecondsElapsedForFrame) * 1000.0f);
                        if(SleepIsGranular)
                        {
                            Sleep(SleepMS);
                        }
                        SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                        Win32GetWallClock());
                    }
                }

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferToWindow(&GlobalBackBuffer, DeviceContext,
                                           Dimension.Width, Dimension.Height);

                    
                LARGE_INTEGER EndCounter = Win32GetWallClock();

                char FPSBuffer[256];
                sprintf_s(FPSBuffer, "FPS: %f\n", 1.0f / Win32GetSecondsElapsed(LastCounter, EndCounter));
                OutputDebugString(FPSBuffer);
                LastCounter = EndCounter;
                   
                int64 EndCycleCount = __rdtsc();
                int64 CyclesElapsed = EndCycleCount - LastCycleCount;
                LastCycleCount = EndCycleCount;
            }
            
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }

    return 0;
}
