#include "speedhack.h"
#include "Functions.h"
#include <Windows.h>
 
bool SpeedHackEnabled, SpeedHackInitialized;
int TickCount;
long long PerformanceCount, PerformanceFrequency;
float Acceleration = 1;
 
typedef MMRESULT (__stdcall *timebeginperiod) (UINT Period);
timebeginperiod TimeBeginPeriod = (timebeginperiod)GetProcAddress(LoadLibrary("winmm.dll"), "timeBeginPeriod");
 
typedef MMRESULT (__stdcall *timeendperiod) (UINT Period);
timeendperiod TimeEndPeriod = (timeendperiod)GetProcAddress(LoadLibrary("winmm.dll"), "timeEndPeriod");
 
void sleep(int MiliSec)
{
    TimeBeginPeriod(1);
    Sleep(MiliSec);
    TimeEndPeriod(1);
}
 
void Tick()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    const int SleepTime = 30;
 
    while (true)
    {
        sleep(SleepTime);
        if (SpeedHackEnabled)
        {
            TickCount += (int)(SleepTime * Acceleration);
            PerformanceCount += (long long)((SleepTime * PerformanceFrequency / 1000) * Acceleration);
        }
        else
        {
            TickCount += SleepTime;
            PerformanceCount += (long long)(SleepTime * PerformanceFrequency / 1000);
        }
    }
}
 
int __declspec(naked) NewTickCount()
{
    __asm
    {
        mov eax,[TickCount]
        ret
    }
}
 
BOOL WINAPI NewQueryPerformanceCounter(LARGE_INTEGER* Count)
{
    Count->QuadPart = PerformanceCount;
    return TRUE;
}
 
void SetSpeed(float Speed)
{
    Acceleration = Speed;
}
 
void SpeedHack(bool Enable)
{
    SpeedHackEnabled = Enable;
    if (Enable == true)
    {
        TickCount = GetTickCount();
        LARGE_INTEGER Ref;
        QueryPerformanceFrequency(&Ref);
        PerformanceFrequency = Ref.QuadPart;
 
        QueryPerformanceCounter(&Ref);
        PerformanceCount = Ref.QuadPart;
 
        if (SpeedHackInitialized == false)
        {
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Tick, NULL, 0, NULL);
            SpeedHackInitialized = true;
        }
 
        DWORD GetTickCount = (DWORD)GetProcAddress(LoadLibrary("kernel32.dll"), "GetTickCount");
        Jump(GetTickCount, NewTickCount, 0);
        DWORD TimeGetTime = (DWORD)GetProcAddress(LoadLibrary("winmm.dll"), "timeGetTime");
        Jump(TimeGetTime, NewTickCount, 0);
        DWORD QueryPerformanceCounter = (DWORD)GetProcAddress(LoadLibrary("kernel32.dll"), "QueryPerformanceCounter") + 6;
        Jump(QueryPerformanceCounter, NewQueryPerformanceCounter, 0); //QueryPerformanceCounter does not seems to be really needed for MS but i did it anyway
    }
}