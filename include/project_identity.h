#pragma once

namespace ProjectIdentity {
#if defined(PS3_GAME_ORBIT_FIX30)
inline constexpr const char* Name = "PS3 Game Orbit";
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX32
#ifdef PS3_GAME_ORBIT_FIX33
#ifdef PS3_GAME_ORBIT_FIX39
inline constexpr const char* Version = "1.4.2";
#elif defined(PS3_GAME_ORBIT_FIX38)
inline constexpr const char* Version = "1.4.1 TESTE";
#elif defined(PS3_GAME_ORBIT_FIX37)
inline constexpr const char* Version = "1.4 TESTE";
#elif defined(PS3_GAME_ORBIT_FIX36)
inline constexpr const char* Version = "1.3.3 TESTE FIX36";
#elif defined(PS3_GAME_ORBIT_FIX35)
inline constexpr const char* Version = "1.3.2 TESTE FIX35";
#elif defined(PS3_GAME_ORBIT_FIX34)
inline constexpr const char* Version = "1.3.1 TESTE FIX34";
#else
inline constexpr const char* Version = "1.3 TESTE FIX33";
#endif
#else
inline constexpr const char* Version = "1.2 TESTE FIX32";
#endif
#else
inline constexpr const char* Version = "1.1 TESTE FIX31";
#endif
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr const char* LayoutPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_LAYOUT.dat";
inline constexpr const char* FallbackLayoutPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_LAYOUT.dat";
#else
inline constexpr const char* LayoutPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_LAYOUT.dat";
inline constexpr const char* FallbackLayoutPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_LAYOUT.dat";
#endif
#else
inline constexpr const char* Version = "FIX30";
#endif
inline constexpr const char* DisplayTitle = "PS3 Game Orbit";
inline constexpr const char* AppId = "PGORBT301";
#else
inline constexpr const char* Name = "PS3_SP_LOADER";
#if defined(PS3_SP_LOADER_FIX29)
inline constexpr const char* Version = "FIX29";
inline constexpr const char* DisplayTitle = "PS3_SP_LOADER FIX29 JFX WEBMAN";
inline constexpr const char* AppId = "PSSPF2901";
#elif defined(PS3_SP_LOADER_FIX28)
inline constexpr const char* Version = "FIX28";
inline constexpr const char* DisplayTitle = "PS3_SP_LOADER FIX28 DUAS CAPAS";
inline constexpr const char* AppId = "PSSPF2801";
#elif defined(PS3_SP_LOADER_FIX20)
inline constexpr const char* Version = "FIX20";
inline constexpr const char* DisplayTitle = "PS3_SP_LOADER FIX20 RENDERER";
inline constexpr const char* AppId = "PSSPF2001";
#else
inline constexpr const char* Version = "V13";
inline constexpr const char* DisplayTitle = "PS3_SP_LOADER V13";
inline constexpr const char* AppId = "PSSP00001";
#endif
#endif
#ifdef PS3_GAME_ORBIT_FIX30
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr const char* PreferencesPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_STATE.dat";
inline constexpr const char* FallbackPreferencesPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_STATE.dat";
#else
inline constexpr const char* PreferencesPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_STATE.dat";
inline constexpr const char* FallbackPreferencesPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_STATE.dat";
#endif
inline constexpr const char* LegacyPreferencesPath = "/dev_hdd0/tmp/PS3_SP_LOADER_STATE.dat";
inline constexpr const char* LegacyFallbackPreferencesPath = "/dev_hdd0/game/PSSPF2901/USRDIR/PS3_SP_LOADER_STATE.dat";
#else
inline constexpr const char* PreferencesPath = "/dev_hdd0/tmp/PS3_SP_LOADER_STATE.dat";
inline constexpr const char* FallbackPreferencesPath = "/dev_hdd0/game/PSSPF2901/USRDIR/PS3_SP_LOADER_STATE.dat";
#endif
#ifdef PS3_GAME_ORBIT_FIX35
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr const char* NamesPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_NAMES.dat";
inline constexpr const char* FallbackNamesPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_NAMES.dat";
#else
inline constexpr const char* NamesPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_NAMES.dat";
inline constexpr const char* FallbackNamesPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_NAMES.dat";
#endif
#endif
#ifdef PS3_GAME_ORBIT_FIX36
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr const char* BackgroundPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_BACKGROUND.dat";
inline constexpr const char* FallbackBackgroundPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_BACKGROUND.dat";
#else
inline constexpr const char* BackgroundPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_BACKGROUND.dat";
inline constexpr const char* FallbackBackgroundPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_BACKGROUND.dat";
#endif
#endif
inline constexpr const char* CoverDirectory = "/dev_hdd0/PS3COVERS";
#ifdef PS3_GAME_ORBIT_FIX37
#ifdef PS3_GAME_ORBIT_FIX38
inline constexpr const char* SettingsPath="/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_SETTINGS.dat";
inline constexpr const char* FallbackSettingsPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_SETTINGS.dat";
#else
inline constexpr const char* SettingsPath="/dev_hdd0/tmp/PS3_GAME_ORBIT_SETTINGS.dat";
inline constexpr const char* FallbackSettingsPath="/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_SETTINGS.dat";
#endif
#endif
#if defined(__PSL1GHT__) && defined(PS3_GAME_ORBIT_FIX30)
#ifdef PS3_GAME_ORBIT_FIX31
#ifdef PS3_GAME_ORBIT_FIX32
#ifdef PS3_GAME_ORBIT_FIX33
#ifdef PS3_GAME_ORBIT_FIX39
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX39.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX39.log";
#elif defined(PS3_GAME_ORBIT_FIX38)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX38.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX38.log";
#elif defined(PS3_GAME_ORBIT_FIX37)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX37.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX37.log";
#elif defined(PS3_GAME_ORBIT_FIX36)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX36.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX36.log";
#elif defined(PS3_GAME_ORBIT_FIX35)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX35.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX35.log";
#elif defined(PS3_GAME_ORBIT_FIX34)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX34.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX34.log";
#else
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX33.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX33.log";
#endif
#else
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX32.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX32.log";
#endif
#else
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX31.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX31.log";
#endif
#else
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX30.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX30.log";
#endif
inline constexpr const char* SafeBootCoverPath = "/dev_hdd0/game/PGORBT301/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
inline constexpr const char* SecondDemoCoverPath = "/dev_hdd0/game/PGORBT301/USRDIR/FULL_COVER_FIX28_SEGUNDA.png";
inline constexpr const char* SplashPath = "/dev_hdd0/game/PGORBT301/USRDIR/ORBIT_SPLASH.png";
#elif defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX29)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_SP_LOADER_FIX29.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PSSPF2901/USRDIR/PS3_SP_LOADER_FIX29.log";
inline constexpr const char* SafeBootCoverPath = "/dev_hdd0/game/PSSPF2901/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
inline constexpr const char* SecondDemoCoverPath = "/dev_hdd0/game/PSSPF2901/USRDIR/FULL_COVER_FIX28_SEGUNDA.png";
#elif defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX28)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_SP_LOADER_FIX28.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PSSPF2801/USRDIR/PS3_SP_LOADER_FIX28.log";
inline constexpr const char* SafeBootCoverPath = "/dev_hdd0/game/PSSPF2801/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
inline constexpr const char* SecondDemoCoverPath = "/dev_hdd0/game/PSSPF2801/USRDIR/FULL_COVER_FIX28_SEGUNDA.png";
#elif defined(__PSL1GHT__) && defined(PS3_SP_LOADER_FIX20)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_SP_LOADER_FIX20.log";
inline constexpr const char* FallbackLogPath = "/dev_hdd0/game/PSSPF2001/USRDIR/PS3_SP_LOADER_FIX20.log";
inline constexpr const char* SafeBootCoverPath = "/dev_hdd0/game/PSSPF2001/USRDIR/FULL_COVER_TEST_275x147.png";
#elif defined(__PSL1GHT__)
inline constexpr const char* LogPath = "/dev_hdd0/tmp/PS3_SP_LOADER_V13.log";
inline constexpr const char* SafeBootCoverPath = "/dev_hdd0/game/PSSP00001/USRDIR/FULL_COVER_TEST_275x147.png";
#else
#ifdef PS3_GAME_ORBIT_FIX30
#ifdef PS3_GAME_ORBIT_FIX39
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX39.log";
#elif defined(PS3_GAME_ORBIT_FIX38)
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX38.log";
#elif defined(PS3_GAME_ORBIT_FIX37)
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX37.log";
#elif defined(PS3_GAME_ORBIT_FIX36)
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX36.log";
#elif defined(PS3_GAME_ORBIT_FIX35)
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX35.log";
#else
inline constexpr const char* LogPath = "/tmp/PS3_GAME_ORBIT_FIX30.log";
#endif
inline constexpr const char* SplashPath = "pkgfiles/USRDIR/ORBIT_SPLASH.png";
#else
inline constexpr const char* LogPath = "/tmp/PS3_SP_LOADER_V13.log";
#endif
#if defined(PS3_SP_LOADER_FIX28)
inline constexpr const char* SafeBootCoverPath = "pkgfiles/USRDIR/FULL_COVER_FIX26_CONTINUA.png";
inline constexpr const char* SecondDemoCoverPath = "pkgfiles/USRDIR/FULL_COVER_FIX28_SEGUNDA.png";
#else
inline constexpr const char* SafeBootCoverPath = "pkgfiles/USRDIR/FULL_COVER_TEST_275x147.png";
#endif
#endif
}
