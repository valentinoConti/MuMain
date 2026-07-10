//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/Legacy/UIControls.h"
#include "Scenes/SceneCore.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"


#include "Network/Server/ServerListManager.h"

#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"

#define	LIW_ACCOUNT		0
#define	LIW_PASSWORD	1

#define LIW_OK			0
#define LIW_CANCEL		1

// Quick-login (saved account) button size. Narrow enough to fit a row of them
// across the 329px-wide login window; a 10-char username fits at the fix font.
static constexpr int kAccountBtnWidth = 92;
static constexpr int kAccountBtnHeight = 24;
// Button row top, relative to the window's top-left. Sits just below the 245px
// login box. The window's clickable/active rect is extended to include this row
// (see Create) so the buttons actually receive clicks.
static constexpr int kAccountRowYLocal = 262;



extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

CLoginWin::CLoginWin()
{
    m_pUsernameInputBox = NULL;
    m_pPasswordInputBox = NULL;
}

CLoginWin::~CLoginWin()
{
    SAFE_DELETE(m_pUsernameInputBox);
    SAFE_DELETE(m_pPasswordInputBox);
}

void CLoginWin::Create()
{
    // Manual login fields start empty; saved accounts are quick-login buttons.
    m_Username[0] = L'\0';
    m_Password[0] = L'\0';

    CWin::Create(329, 245, BITMAP_LOG_IN + 7);

    m_asprInputBox[LIW_ACCOUNT].Create(156, 23, BITMAP_LOG_IN + 8);
    m_asprInputBox[LIW_PASSWORD].Create(156, 23, BITMAP_LOG_IN + 8);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    // One quick-login button per launcher-saved account, captioned with the
    // username. Colors mirror the other text buttons (up / down / active / off).
    const auto& accounts = GameConfig::GetInstance().GetSavedAccounts();
    m_accountBtnCount = static_cast<int>(accounts.size());
    if (m_accountBtnCount > kMaxSavedAccounts)
        m_accountBtnCount = kMaxSavedAccounts;

    DWORD adwAccountClr[4] = { CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_WHITE, 0 };
    for (int i = 0; i < m_accountBtnCount; ++i)
    {
        m_accountBtns[i].Create(kAccountBtnWidth, kAccountBtnHeight, BITMAP_TEXT_BTN, 4, 2, 1);
        m_accountBtns[i].SetText(accounts[i].username.c_str(), adwAccountClr);
        CWin::RegisterButton(&m_accountBtns[i]);
    }

    // Extend the window's active/hit rect downward so the button row (which
    // renders below the 245px login box) still counts as "inside the window".
    // Otherwise clicking it deactivates the window and the click is dropped.
    // The background sprite is unaffected (it keeps its 245px height).
    if (m_accountBtnCount > 0)
        m_Size.cy = kAccountRowYLocal + kAccountBtnHeight + 4;

    SAFE_DELETE(m_pUsernameInputBox);

    m_pUsernameInputBox = new CUITextInputBox;
    m_pUsernameInputBox->Init(g_hWnd, 140, 14, MAX_USERNAME_SIZE);
    m_pUsernameInputBox->SetBackColor(0, 0, 0, 25);
    m_pUsernameInputBox->SetTextColor(255, 255, 230, 210);
    m_pUsernameInputBox->SetFont(g_hFixFont);
    m_pUsernameInputBox->SetState(UISTATE_NORMAL);

    SAFE_DELETE(m_pPasswordInputBox);

    m_pPasswordInputBox = new CUITextInputBox;
    m_pPasswordInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
    m_pPasswordInputBox->SetBackColor(0, 0, 0, 25);
    m_pPasswordInputBox->SetTextColor(255, 255, 230, 210);
    m_pPasswordInputBox->SetFont(g_hFixFont);
    m_pPasswordInputBox->SetState(UISTATE_NORMAL);

    m_pUsernameInputBox->SetTabTarget(m_pPasswordInputBox);
    m_pPasswordInputBox->SetTabTarget(m_pUsernameInputBox);

    this->FirstLoad = 1;
}

void CLoginWin::PreRelease()
{
    for (int i = 0; i < 2; ++i)
        m_asprInputBox[i].Release();
}

void CLoginWin::SetPosition(int x, int y)
{
	CWin::SetPosition(x, y);

	const int boxOffsetX = x + 109;
	m_asprInputBox[LIW_ACCOUNT].SetPosition(boxOffsetX, y + 106);
	m_asprInputBox[LIW_PASSWORD].SetPosition(boxOffsetX, y + 131);

	if (g_iChatInputType == 1)
	{
		const int boxX = int((x + 115) / g_fScreenRate_x);
		m_pUsernameInputBox->SetPosition(boxX, int((y + 112) / g_fScreenRate_y));
		m_pPasswordInputBox->SetPosition(boxX, int((y + 137) / g_fScreenRate_y));
	}

	m_aBtn[LIW_OK].SetPosition(x + 150, y + 178);
	m_aBtn[LIW_CANCEL].SetPosition(x + 211, y + 178);

	// Quick-login buttons laid out horizontally in a row below the login box,
	// centered on the 329px-wide window.
	const int rowY = y + kAccountRowYLocal;
	const int gap = 6;
	const int rowWidth = m_accountBtnCount * kAccountBtnWidth + (m_accountBtnCount - 1) * gap;
	int accountX = x + (329 - rowWidth) / 2;
	for (int i = 0; i < m_accountBtnCount; ++i)
	{
		m_accountBtns[i].SetPosition(accountX, rowY);
		accountX += kAccountBtnWidth + gap;
	}
}

void CLoginWin::Show(bool bShow)
{
    CWin::Show(bShow);

    for (int i = 0; i < 2; ++i)
    {
        m_asprInputBox[i].Show(bShow);
        m_aBtn[i].Show(bShow);
    }
    for (int i = 0; i < m_accountBtnCount; ++i)
        m_accountBtns[i].Show(bShow);

    // Drive the text fields' state so a hidden login screen releases keyboard
    // focus (portable fields stop SDL text input when hidden, #447).
    const int iState = bShow ? UISTATE_NORMAL : UISTATE_HIDE;
    if (m_pUsernameInputBox) m_pUsernameInputBox->SetState(iState);
    if (m_pPasswordInputBox) m_pPasswordInputBox->SetState(iState);
}

bool CLoginWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void CLoginWin::UpdateWhileActive(double)
{
	if (m_aBtn[LIW_OK].IsClick() || CInput::Instance().IsKeyDown(VK_RETURN))
	{
		PlayBuffer(SOUND_CLICK01);
		RequestLogin();
		return;
	}

	if (m_aBtn[LIW_CANCEL].IsClick() || CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		PlayBuffer(SOUND_CLICK01);
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
		return;
	}

	// A saved-account button: log straight in with that account.
	for (int i = 0; i < m_accountBtnCount; ++i)
	{
		if (m_accountBtns[i].IsClick())
		{
			PlayBuffer(SOUND_CLICK01);
			QuickLogin(i);
			return;
		}
	}
}

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
    m_pUsernameInputBox->DoAction();
    m_pPasswordInputBox->DoAction();
}

void CLoginWin::RenderControls()
{
    if (FirstLoad)
    {
        (wcslen(m_Username) > 0 ? m_pPasswordInputBox : m_pUsernameInputBox)->GiveFocus();
        FirstLoad = 0;
    }

    CWin::RenderButtons();
    m_asprInputBox[LIW_ACCOUNT].Render();
    m_asprInputBox[LIW_PASSWORD].Render();
    m_pUsernameInputBox->Render();
    m_pPasswordInputBox->Render();

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(CLRDW_WHITE);

    const int baseX = GetXPos();
    const int baseY = GetYPos();

    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 113) / g_fScreenRate_y), I18N::Game::Account);
    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 139) / g_fScreenRate_y), I18N::Game::Password);

    wchar_t szServerName[MAX_TEXT_LENGTH] = {};
    const wchar_t* pServerStatus = g_ServerListManager->GetNonPVPInfo() ? I18N::Game::SDServer : I18N::Game::SDNonPvPServer;
    mu_swprintf(szServerName, pServerStatus, g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
    g_pRenderText->RenderText(int((baseX + 111) / g_fScreenRate_x), int((baseY + 80) / g_fScreenRate_y), szServerName);

    // Header above the quick-login account button row (only when there are any).
    if (m_accountBtnCount > 0)
        g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + kAccountRowYLocal - 14) / g_fScreenRate_y), L"Cuentas guardadas:");
}

void CLoginWin::RequestLogin()
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    // Manual login: take whatever is typed in the fields. The client no longer
    // saves credentials here -- the launcher manages the saved-account slots.
    m_pUsernameInputBox->GetText(m_Username, _countof(m_Username));
    m_pPasswordInputBox->GetText(m_Password, _countof(m_Password));

    DoLogin();
}

void CLoginWin::QuickLogin(int index)
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    const auto& accounts = GameConfig::GetInstance().GetSavedAccounts();
    if (index < 0 || index >= static_cast<int>(accounts.size()))
        return;

    wcsncpy_s(m_Username, _countof(m_Username), accounts[index].username.c_str(), _TRUNCATE);
    wcsncpy_s(m_Password, _countof(m_Password), accounts[index].password.c_str(), _TRUNCATE);

    DoLogin();
}

void CLoginWin::DoLogin()
{
    CUIMng::Instance().HideWin(this);

    if (wcslen(m_Username) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
    else
    {
        if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Login with the following account: %ls", m_Username);

            g_ErrorReport.Write(L"> Login Request.\r\n");
            g_ErrorReport.Write(L"> Try to Login \"%ls\"\r\n", m_Username);

            LogIn = 1;
            wcscpy(LogInID, (m_Username));
            CurrentProtocolState = REQUEST_LOG_IN;

            SocketClient->ToGameServer()->SendLogin(m_Username, m_Password, Version, Serial);

            // Keep the credentials in memory so auto-reconnect can re-login
            // without prompting after an in-game disconnect.
            ReconnectManager::Instance().CacheCredentials(m_Username, m_Password);

            g_pSystemLogBox->AddText(I18N::Game::VerifyingYourAccount, SEASON3B::TYPE_SYSTEM_MESSAGE);
            g_pSystemLogBox->AddText(I18N::Game::PleaseWait, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
    }
}

void CLoginWin::CancelLogin()
{
    ConnectConnectionServer();
    CUIMng::Instance().HideWin(this);
}

void CLoginWin::ConnectConnectionServer()
{
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    // Return to the connect server (server list) rather than whatever endpoint a
    // prior auto-reconnect left in szServerIpAddress (possibly the game server).
    szServerIpAddress = szConnectServerIpAddress;
    g_ServerPort = g_ConnectServerPort;
    CreateSocket(szServerIpAddress, g_ServerPort);
}