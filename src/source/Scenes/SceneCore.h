#pragma once

#include "Engine/Object/ZzzInfomation.h"
#include "Core/Utilities/SpinLock.h"

// Include refactored scene headers
#ifdef USE_REFACTORED_SCENES
#include "Scenes/SceneCommon.h"
#include "Scenes/SceneManager.h"
#include "Camera/CameraUtility.h"
#endif

extern EGameScene SceneFlag;

extern int  ErrorMessage;
extern bool InitServerList;
extern const wchar_t* szServerIpAddress;
extern unsigned short g_ServerPort;

// The connect-server (server-list) endpoint, preserved separately so it is never
// lost. ReconnectManager::Begin() repoints szServerIpAddress at the game server
// during an auto-reconnect and never restores it; keeping the original here lets
// "Select Server" and the login Cancel button return to the server list.
extern const wchar_t* szConnectServerIpAddress;
extern unsigned short g_ConnectServerPort;
extern int g_iLengthAuthorityCode;

inline SpinLock* g_render_lock = new SpinLock();

extern bool CheckRenderNextFrame();
extern void WaitForNextActivity(bool usePreciseSleep);
extern void UpdateSceneState();
extern void LoadingScene(HDC hDC);
extern void RenderScene(HDC Hdc);
extern bool CheckName();
void    StartGame();
void SetTargetFps(double targetFps);
double GetTargetFps();

BOOL	ShowCheckBox(int num, int index, int message = MESSAGE_TRADE_CHECK);

int	SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize);

bool	GetTimeCheck(int DelayTime);
void	SetEffectVolumeLevel(int level);

bool IsEnterPressed();
void SetEnterPressed(bool enterpressed);
