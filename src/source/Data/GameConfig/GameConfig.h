#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "Core/Platform/WinCompat.h"

class GameConfig
{
public:
    static GameConfig& GetInstance();

    void Load();
    void Save();

    // Window
    int  GetWindowWidth()  const { return m_windowWidth; }
    int  GetWindowHeight() const { return m_windowHeight; }
    bool GetWindowMode()   const { return m_windowMode; }

    void SetWindowSize(int width, int height);
    void SetWindowMode(bool windowed);

    // Audio — volume 0 = off, >0 = on. No separate Enabled flag.
    int  GetSoundVolume()  const { return m_soundVolume; }
    int  GetMusicVolume()  const { return m_musicVolume; }

    void SetSoundVolume(int level);
    void SetMusicVolume(int level);

    // Login
    std::wstring GetLanguageSelection() const { return m_languageSelection; }
    void SetLanguageSelection(const std::wstring& lang);

    // Launcher-saved accounts, read from the numbered EncryptedUsername{n}/
    // EncryptedPassword{n} slots at load time (decrypted, ready to log in with).
    // Order follows the slot numbers; sparse slots are skipped.
    struct SavedAccount
    {
        std::wstring username;
        std::wstring password;
    };
    const std::vector<SavedAccount>& GetSavedAccounts() const { return m_savedAccounts; }

    // Connection
    std::wstring GetServerIP() const { return m_serverIP; }
    int GetServerPort() const { return m_serverPort; }

    void SetServerIP(const std::wstring& ip);
    void SetServerPort(int port);

    // UI — I18N locale code ("en", "de", ...) for the typed translation
    // accessors. Distinct from GetLanguageSelection above, which is the
    // legacy "Eng"/"Por"/"Spn" data-dir prefix used by .bmd asset loaders.
    std::wstring GetUILocale() const { return m_uiLocale; }
    void SetUILocale(const std::wstring& locale);

    // Main UI font size in px; 0 = automatic (scales with resolution).
    int  GetFontSize() const { return m_fontSize; }
    void SetFontSize(int size);

    // Camera
    int GetZoom() const { return m_zoom; }
    void SetZoom(int zoom);

    // Helpers
    static std::wstring BinaryToHex(const BYTE* data, DWORD size);
    static std::vector<BYTE> HexToBinary(const std::wstring& hex);

private:
    GameConfig();
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;

    std::filesystem::path m_configPath;

    int  m_windowWidth;
    int  m_windowHeight;
    bool m_windowMode;

    int  m_soundVolume;
    int  m_musicVolume;

    std::wstring m_languageSelection;
    std::vector<SavedAccount> m_savedAccounts;

    std::wstring m_serverIP;
    int m_serverPort;

    std::wstring m_uiLocale;
    int m_fontSize;

    int m_zoom;

    int ReadInt(const wchar_t* section, const wchar_t* key, int defaultValue);
    void WriteInt(const wchar_t* section, const wchar_t* key, int value);

    bool ReadBool(const wchar_t* section, const wchar_t* key, bool defaultValue);
    void WriteBool(const wchar_t* section, const wchar_t* key, bool value);

    std::wstring ReadString(const wchar_t* section, const wchar_t* key, const std::wstring& defaultValue);
    void WriteString(const wchar_t* section, const wchar_t* key, const std::wstring& value);

    void RemoveObsoleteKey(const wchar_t* section, const wchar_t* key);
    void RemoveObsoleteSection(const wchar_t* section);

    std::wstring DecryptSetting(const std::wstring& hexInput);
    std::wstring EncryptSetting(const wchar_t* input);
};
