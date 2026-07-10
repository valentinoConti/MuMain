#include "stdafx.h"
#include "GameConfig.h"

#ifdef _WIN32
#include <imagehlp.h>
#endif

#include <cstdint>

#include "GameConfigConstants.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"  // private-profile (.ini) API

GameConfig& GameConfig::GetInstance()
{
    static GameConfig instance;
    return instance;
}

GameConfig::GameConfig()
{
    // Get executable directory and construct config path
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    // Find the last path separator to get the directory. GetModuleFileNameW
    // returns backslashes on Windows but forward slashes on Linux (issue #462),
    // so accept either; truncating on the wrong one leaves the whole exe path
    // glued to "config.ini", and the file is silently never found (every
    // setting then falls back to its default).
    wchar_t* lastBackslash = wcsrchr(exePath, L'\\');
    wchar_t* lastForwardSlash = wcsrchr(exePath, L'/');
    // Relational comparison of pointers into different arrays (or with null) is
    // undefined behavior, and on Linux one of these is always null, so pick the
    // later separator only when both exist.
    wchar_t* lastSlash = nullptr;
    if (lastBackslash && lastForwardSlash)
        lastSlash = (lastBackslash > lastForwardSlash) ? lastBackslash : lastForwardSlash;
    else
        lastSlash = lastBackslash ? lastBackslash : lastForwardSlash;
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';  // Keep the trailing separator
    }

    m_configPath = exePath;
    m_configPath += L"config.ini";

    Load();
}

void GameConfig::Load()
{
    using namespace CfgSections;
    using namespace CfgKeys;
    using namespace CfgDefaults;

    m_windowWidth  = ReadInt(CfgSectionWindow, CfgKeyWidth, CfgDefaultWindowWidth);
    m_windowHeight = ReadInt(CfgSectionWindow, CfgKeyHeight, CfgDefaultWindowHeight);
    m_windowMode   = ReadBool(CfgSectionWindow, CfgKeyWindowed, CfgDefaultWindowed);

    m_soundVolume  = ReadInt(CfgSectionAudio, CfgKeySoundVolume, CfgDefaultSoundVolume);
    m_musicVolume  = ReadInt(CfgSectionAudio, CfgKeyMusicVolume, CfgDefaultMusicVolume);

    m_rememberMe        = ReadBool(CfgSectionLogin, CfgKeyRememberMe, CfgDefaultRememberMe);
    m_languageSelection = ReadString(CfgSectionLogin, CfgKeyLanguage, CfgDefaultLanguage);
    m_encryptedUsername = ReadString(CfgSectionLogin, CfgKeyEncryptedUsername, CfgDefaultEncryptedUsername);
    m_encryptedPassword = ReadString(CfgSectionLogin, CfgKeyEncryptedPassword, CfgDefaultEncryptedPassword);

    m_serverIP   = ReadString(CfgSectionConnectionSettings, CfgKeyServerIP, CfgDefaultServerIP);
    m_serverPort = ReadInt(CfgSectionConnectionSettings, CfgKeyServerPort, CfgDefaultServerPort);

    m_uiLocale = ReadString(CfgSectionUI, CfgKeyUILocale, CfgDefaultUILocale);
    m_fontSize = ReadInt(CfgSectionUI, CfgKeyFontSize, CfgDefaultFontSize);

    m_zoom = ReadInt(CfgSectionCamera, CfgKeyZoom, CfgDefaultZoom);

    // Strip keys/sections we used to write but no longer use, so user config
    // files don't accumulate orphans. Append one line per retired key — no
    // central registry of valid keys to keep in sync.
    RemoveObsoleteKey(CfgSectionGraphics, L"RenderTextType");
    RemoveObsoleteKey(CfgSectionGraphics, L"ColorDepth");      // 16/32bpp toggle, dead since fullscreen uses GetDesktopBitsPerPel
    RemoveObsoleteKey(CfgSectionAudio,    L"SoundEnabled");   // replaced by SoundVolume==0
    RemoveObsoleteKey(CfgSectionAudio,    L"MusicEnabled");   // replaced by MusicVolume==0
    RemoveObsoleteKey(CfgSectionAudio,    L"VolumeLevel");    // legacy single-volume key
    RemoveObsoleteKey(CfgSectionLogin,    L"Version");        // launcher metadata, never read by client
    RemoveObsoleteKey(CfgSectionLogin,    L"TestVersion");    // launcher metadata, never read by client
    RemoveObsoleteSection(CfgSectionGraphics);                // empty after RenderTextType + ColorDepth removal
    RemoveObsoleteSection(L"PARTITION");                      // launcher metadata, never read by client
}

void GameConfig::Save()
{
    using namespace CfgSections;
    using namespace CfgKeys;

    WriteInt(CfgSectionWindow, CfgKeyWidth, m_windowWidth);
    WriteInt(CfgSectionWindow, CfgKeyHeight, m_windowHeight);
    WriteBool(CfgSectionWindow, CfgKeyWindowed, m_windowMode);

    WriteInt(CfgSectionAudio, CfgKeySoundVolume, m_soundVolume);
    WriteInt(CfgSectionAudio, CfgKeyMusicVolume, m_musicVolume);

    WriteBool(CfgSectionLogin, CfgKeyRememberMe, m_rememberMe);
    WriteString(CfgSectionLogin, CfgKeyLanguage, m_languageSelection);
    WriteString(CfgSectionLogin, CfgKeyEncryptedUsername, m_encryptedUsername);
    WriteString(CfgSectionLogin, CfgKeyEncryptedPassword, m_encryptedPassword);

    WriteString(CfgSectionConnectionSettings, CfgKeyServerIP, m_serverIP);
    WriteInt(CfgSectionConnectionSettings, CfgKeyServerPort, m_serverPort);

    WriteString(CfgSectionUI, CfgKeyUILocale, m_uiLocale);
    WriteInt(CfgSectionUI, CfgKeyFontSize, m_fontSize);

    WriteInt(CfgSectionCamera, CfgKeyZoom, m_zoom);
}

void GameConfig::SetWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
}

void GameConfig::SetWindowMode(bool windowed)
{
    m_windowMode = windowed;
}

void GameConfig::SetSoundVolume(int level)
{
    m_soundVolume = level;
}

void GameConfig::SetMusicVolume(int level)
{
    m_musicVolume = level;
}

void GameConfig::SetRememberMe(bool remember)
{
    m_rememberMe = remember;
}

void GameConfig::SetLanguageSelection(const std::wstring& lang)
{
    m_languageSelection = lang;
}

void GameConfig::SetUILocale(const std::wstring& locale)
{
    m_uiLocale = locale;
}

void GameConfig::SetEncryptedUsername(const std::wstring& encryptedUsername)
{
    m_encryptedUsername = encryptedUsername;
}

void GameConfig::SetEncryptedPassword(const std::wstring& encryptedPassword)
{
    m_encryptedPassword = encryptedPassword;
}

void GameConfig::SetServerIP(const std::wstring& ip)
{
    m_serverIP = ip;
}

void GameConfig::SetServerPort(int port)
{
    m_serverPort = port;
}

void GameConfig::SetZoom(int zoom)
{
    m_zoom = zoom;
}

void GameConfig::SetFontSize(int size)
{
    m_fontSize = size;
}

// Helper function to convert binary data to hex string
std::wstring GameConfig::BinaryToHex(const BYTE* data, DWORD size)
{
    std::wstring hex;
    hex.reserve(size * 2);

    const wchar_t hexChars[] = L"0123456789ABCDEF";
    for (DWORD i = 0; i < size; ++i)
    {
        hex += hexChars[(data[i] >> 4) & 0x0F];
        hex += hexChars[data[i] & 0x0F];
    }

    return hex;
}

// Helper function to convert hex string to binary data
std::vector<BYTE> GameConfig::HexToBinary(const std::wstring& hex)
{
    std::vector<BYTE> binary;

    if (hex.empty() || hex.length() % 2 != 0)
        return binary;

    binary.reserve(hex.length() / 2);

    auto hex_char_to_byte = [](wchar_t c) -> BYTE {
        if (c >= L'0' && c <= L'9') return (c - L'0');
        if (c >= L'a' && c <= L'f') return (c - L'a' + 10);
        return (c - L'A' + 10);
    };

    for (size_t i = 0; i < hex.length(); i += 2)
    {
        wchar_t high = hex[i];
        wchar_t low = hex[i + 1];

        if (!iswxdigit(high) || !iswxdigit(low))
        {
            // Invalid hex character detected, return empty vector
            return {};
        }

        binary.push_back((hex_char_to_byte(high) << 4) | hex_char_to_byte(low));
    }

    return binary;
}

void GameConfig::DecryptCredentials(wchar_t* outUser, wchar_t* outPass, size_t userBufSize, size_t passBufSize)
{
    // Decrypt Username
    std::wstring user = DecryptSetting(GetEncryptedUsername());
    if (!user.empty()) {
        wcsncpy_s(outUser, userBufSize, user.c_str(), _TRUNCATE);
    }

    // Decrypt Password
    std::wstring pass = DecryptSetting(GetEncryptedPassword());
    if (!pass.empty()) {
        wcsncpy_s(outPass, passBufSize, pass.c_str(), _TRUNCATE);
    }
}

// Helper functions using Windows INI API
int GameConfig::ReadInt(const wchar_t* section, const wchar_t* key, int defaultValue)
{
    return GetPrivateProfileIntW(section, key, defaultValue, m_configPath.wstring().c_str());
}

void GameConfig::WriteInt(const wchar_t* section, const wchar_t* key, int value)
{
    wchar_t buffer[32];
    swprintf_s(buffer, L"%d", value);

    WritePrivateProfileStringW(section, key, buffer, m_configPath.wstring().c_str());
}

bool GameConfig::ReadBool(const wchar_t* section, const wchar_t* key, bool defaultValue)
{
    return GetPrivateProfileIntW(section, key, defaultValue ? 1 : 0, m_configPath.wstring().c_str()) != 0;
}

void GameConfig::WriteBool(const wchar_t* section, const wchar_t* key, bool value)
{
    WritePrivateProfileStringW(section, key, value ? L"1" : L"0", m_configPath.wstring().c_str());
}

std::wstring GameConfig::ReadString(const wchar_t* section, const wchar_t* key, const std::wstring& defaultValue)
{
    std::vector<wchar_t> buffer(2048);
    while (true)
    {
        DWORD charsRead = GetPrivateProfileStringW(section, key, defaultValue.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()), m_configPath.wstring().c_str());
        if (charsRead < buffer.size() - 1)
        {
            return std::wstring(buffer.data());
        }
        buffer.resize(buffer.size() * 2);
    }
}

void GameConfig::WriteString(const wchar_t* section, const wchar_t* key, const std::wstring& value)
{
    WritePrivateProfileStringW(section, key, value.c_str(), m_configPath.wstring().c_str());
}

void GameConfig::RemoveObsoleteKey(const wchar_t* section, const wchar_t* key)
{
    // Passing nullptr as the value deletes the key (Windows INI API).
    WritePrivateProfileStringW(section, key, nullptr, m_configPath.wstring().c_str());
}

void GameConfig::RemoveObsoleteSection(const wchar_t* section)
{
    // Passing nullptr as the key deletes the entire section.
    WritePrivateProfileStringW(section, nullptr, nullptr, m_configPath.wstring().c_str());
}

// ---------------------------------------------------------------------------
// Credential obfuscation (replaces the previous Windows DPAPI encryption).
//
// Saved login/password are NO LONGER machine-bound. They are obfuscated with a
// FIXED key so the value is identical on every install and an external account-
// manager tool can produce/read them. The transform is deliberately reversible
// (the client must recover the real password to send it to the game server) and
// only keeps literal plaintext out of config.ini -- this is obfuscation, not
// security.
//
// Stored format of [LOGIN] EncryptedUsername / EncryptedPassword:
//   plain  = ASCII/Latin-1 bytes of the string (one byte per character)
//   framed = 0x4D 0x55  ("MU" marker)  followed by plain
//   cipher = framed XOR SplitMix64-keystream(seed = kCredKeySeed)
//   value  = uppercase hex of cipher
// To decode, reverse it and require the "MU" marker -- so a pre-existing DPAPI
// blob (which won't carry the marker) is treated as "no saved credentials" and
// the user simply re-enters once.
// ---------------------------------------------------------------------------
namespace
{
    // Fixed, non-machine-bound key ("MuDefini" in ASCII). If you change this,
    // every saved credential AND the account-manager tool must use the new value.
    constexpr std::uint64_t kCredKeySeed = 0x4D75446566696E69ull;
    constexpr BYTE kCredMarker0 = 0x4D; // 'M'
    constexpr BYTE kCredMarker1 = 0x55; // 'U'

    // SplitMix64 keystream XOR seeded from the fixed key. Symmetric: applying it
    // once encrypts, applying it again decrypts.
    void CredKeystreamXor(BYTE* data, size_t len)
    {
        std::uint64_t s = kCredKeySeed;
        for (size_t i = 0; i < len; )
        {
            s += 0x9E3779B97F4A7C15ull;
            std::uint64_t z = s;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
            z = z ^ (z >> 31);
            for (int b = 0; b < 8 && i < len; ++b, ++i)
                data[i] ^= static_cast<BYTE>(z >> (b * 8));
        }
    }
}

std::wstring GameConfig::DecryptSetting(const std::wstring& hexInput)
{
    if (hexInput.empty()) return L"";

    std::vector<BYTE> data = HexToBinary(hexInput);
    if (data.size() < 2) return L"";  // too short to hold the marker

    CredKeystreamXor(data.data(), data.size());

    // Require the "MU" marker. Anything else (e.g. an old machine-bound DPAPI
    // blob) is treated as "no saved credentials".
    if (data[0] != kCredMarker0 || data[1] != kCredMarker1)
        return L"";

    std::wstring result;
    result.reserve(data.size() - 2);
    for (size_t i = 2; i < data.size(); ++i)
        result.push_back(static_cast<wchar_t>(data[i]));
    return result;
}

std::wstring GameConfig::EncryptSetting(const wchar_t* input)
{
    if (!input || wcslen(input) == 0) return L"";

    std::vector<BYTE> data;
    data.push_back(kCredMarker0);
    data.push_back(kCredMarker1);
    for (const wchar_t* p = input; *p != L'\0'; ++p)
        data.push_back(static_cast<BYTE>(*p & 0xFF));  // ASCII/Latin-1 credentials

    CredKeystreamXor(data.data(), data.size());
    return BinaryToHex(data.data(), static_cast<DWORD>(data.size()));
}

void GameConfig::EncryptAndSaveCredentials(const wchar_t* user, const wchar_t* pass)
{
    std::wstring encUser = EncryptSetting(user);
    std::wstring encPass = EncryptSetting(pass);

    if (!encUser.empty() && !encPass.empty())
    {
        SetEncryptedUsername(encUser);
        SetEncryptedPassword(encPass);
        Save(); // Actually write to the .ini file
    }
}
