//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef PREFERENCEKEYS_H
#define PREFERENCEKEYS_H

//
// Defines for all preferences
// Every preference should have a define to ease the usage of the preference
// Make sure the string key has a sensible grouping - use / for grouping
//
#define PREF_APP_AUTOSAVE_AUTOSAVETIME                      "application/autosave/autosaveTime"
#define PREF_APP_AUTOSAVE_USEAUTOSAVE                       "application/autosave/useAutosave"
#define PREF_APP_KEYBOARDLAYOUT                             "application/keyboardLayout"
// file path of instrument templates
#define PREF_APP_PATHS_INSTRUMENTLIST1                      "application/paths/instrumentList1"
#define PREF_APP_PATHS_INSTRUMENTLIST2                      "application/paths/instrumentList2"
#define PREF_APP_PATHS_MYIMAGES                             "application/paths/myImages"
#define PREF_APP_PATHS_MYPLUGINS                            "application/paths/myPlugins"
#define PREF_APP_PATHS_MYSCORES                             "application/paths/myScores"
#define PREF_APP_PATHS_MYSHORTCUTS                          "application/paths/myShortcuts"
#define PREF_APP_PATHS_MYSOUNDFONTS                         "application/paths/mySoundfonts"
#define PREF_APP_PATHS_MYSTYLES                             "application/paths/myStyles"
#define PREF_APP_PATHS_MYTEMPLATES                          "application/paths/myTemplates"
#define PREF_APP_PATHS_MYEXTENSIONS                         "application/paths/myExtensions"
#define PREF_APP_PLAYBACK_FOLLOWSONG                        "application/playback/followSong"
#define PREF_APP_PLAYBACK_PANPLAYBACK                       "application/playback/panPlayback"
#define PREF_APP_PLAYBACK_PLAYREPEATS                       "application/playback/playRepeats"
#define PREF_APP_PLAYBACK_LOOPTOSELECTIONONPLAY             "application/playback/setLoopToSelectionOnPlay"
#define PREF_APP_PLAYBACK_PLAY_SELECTED_STAVES_ONLY         "application/playback/playSelectedStavesOnly"
#define PREF_APP_USESINGLEPALETTE                           "application/useSinglePalette"
#define PREF_APP_PALETTESCALE                               "application/paletteScale"
#define PREF_APP_STARTUP_FIRSTSTART                         "application/startup/firstStart"
#define PREF_APP_STARTUP_SESSIONSTART                       "application/startup/sessionStart"
#define PREF_APP_STARTUP_STARTSCORE                         "application/startup/startScore"
#define PREF_APP_WORKSPACE                                  "application/workspace"
#define PREF_APP_STARTUP_TELEMETRY_ACCESS_REQUESTED         "application/startup/telemetry_access_requested"
#define PREF_APP_TELEMETRY_ALLOWED                          "application/telemetry/allowed"
#define PREF_APP_BACKUP_GENERATE_BACKUP                     "application/backup/generateBackup"
#define PREF_APP_BACKUP_SUBFOLDER                           "application/backup/subfolder"
#define PREF_EXPORT_AUDIO_NORMALIZE                         "export/audio/normalize"
#define PREF_EXPORT_AUDIO_SAMPLERATE                        "export/audio/sampleRate"
#define PREF_EXPORT_AUDIO_PCMRATE                           "export/audio/PCMRate"
#define PREF_EXPORT_MP3_BITRATE                             "export/mp3/bitRate"
#define PREF_EXPORT_MUSICXML_EXPORTLAYOUT                   "export/musicXML/exportLayout"
#define PREF_EXPORT_MUSICXML_EXPORTBREAKS                   "export/musicXML/exportBreaks"
#define PREF_EXPORT_PDF_DPI                                 "export/pdf/dpi"
#define PREF_EXPORT_PNG_RESOLUTION                          "export/png/resolution"
#define PREF_EXPORT_PNG_USETRANSPARENCY                     "export/png/useTransparency"
#define PREF_IMPORT_GUITARPRO_CHARSET                       "import/guitarpro/charset"
#define PREF_IMPORT_MUSICXML_IMPORTBREAKS                   "import/musicXML/importBreaks"
#define PREF_IMPORT_MUSICXML_IMPORTLAYOUT                   "import/musicXML/importLayout"
#define PREF_IMPORT_AVSOMR_USELOCAL                         "import/avsomr/useLocalEngine"
#define PREF_IMPORT_OVERTURE_CHARSET                        "import/overture/charset"
#define PREF_IMPORT_STYLE_STYLEFILE                         "import/style/styleFile"
#define PREF_IMPORT_COMPATIBILITY_RESET_ELEMENT_POSITIONS   "import/compatibility/resetElementPositions"
#define PREF_APP_PALETTESCALE                               "application/paletteScale"
#define PREF_IO_ALSA_DEVICE                                 "io/alsa/device"
#define PREF_IO_ALSA_FRAGMENTS                              "io/alsa/fragments"
#define PREF_IO_ALSA_PERIODSIZE                             "io/alsa/periodSize"
#define PREF_IO_ALSA_SAMPLERATE                             "io/alsa/sampleRate"
#define PREF_IO_ALSA_USEALSAAUDIO                           "io/alsa/useAlsaAudio"
#define PREF_IO_JACK_REMEMBERLASTCONNECTIONS                "io/jack/rememberLastConnections"
#define PREF_IO_JACK_TIMEBASEMASTER                         "io/jack/timebaseMaster"
#define PREF_IO_JACK_USEJACKAUDIO                           "io/jack/useJackAudio"
#define PREF_IO_JACK_USEJACKMIDI                            "io/jack/useJackMIDI"
#define PREF_IO_JACK_USEJACKTRANSPORT                       "io/jack/useJackTransport"
#define PREF_IO_MIDI_ADVANCEONRELEASE                       "io/midi/advanceOnRelease"
#define PREF_IO_MIDI_ENABLEINPUT                            "io/midi/enableInput"
#define PREF_IO_MIDI_EXPANDREPEATS                          "io/midi/expandRepeats"
#define PREF_IO_MIDI_EXPORTRPNS                             "io/midi/exportRPNs"
#define PREF_IO_MIDI_PEDAL_EVENTS_MIN_TICKS                 "io/midi/pedalEventsMinTicks"
#define PREF_IO_MIDI_REALTIMEDELAY                          "io/midi/realtimeDelay"
#define PREF_IO_MIDI_REMOTE                                 "io/midi/remote"
#define PREF_IO_MIDI_SHORTESTNOTE                           "io/midi/shortestNote"
#define PREF_IO_MIDI_SHOWCONTROLSINMIXER                    "io/midi/showControlsInMixer"
#define PREF_IO_MIDI_USEREMOTECONTROL                       "io/midi/useRemoteControl"
#define PREF_IO_OSC_PORTNUMBER                              "io/osc/portNumber"
#define PREF_IO_OSC_USEREMOTECONTROL                        "io/osc/useRemoteControl"
#define PREF_IO_PORTAUDIO_DEVICE                            "io/portAudio/device"
#define PREF_IO_PORTAUDIO_USEPORTAUDIO                      "io/portAudio/usePortAudio"
#define PREF_IO_PORTMIDI_INPUTBUFFERCOUNT                   "io/portMidi/inputBufferCount"
#define PREF_IO_PORTMIDI_INPUTDEVICE                        "io/portMidi/inputDevice"
#define PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT                  "io/portMidi/outputBufferCount"
#define PREF_IO_PORTMIDI_OUTPUTDEVICE                       "io/portMidi/outputDevice"
#define PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS          "io/portMidi/outputLatencyMilliseconds"
#define PREF_IO_PULSEAUDIO_USEPULSEAUDIO                    "io/pulseAudio/usePulseAudio"
#define PREF_SCORE_CHORD_PLAYONADDNOTE                      "score/chord/playOnAddNote"
#define PREF_SCORE_HARMONY_PLAY                             "score/harmony/play"
#define PREF_SCORE_HARMONY_PLAY_ONEDIT                      "score/harmony/play/onedit"
#define PREF_SCORE_MAGNIFICATION                            "score/magnification"
#define PREF_SCORE_NOTE_PLAYONCLICK                         "score/note/playOnClick"
#define PREF_SCORE_NOTE_DEFAULTPLAYDURATION                 "score/note/defaultPlayDuration"
#define PREF_SCORE_NOTE_WARNPITCHRANGE                      "score/note/warnPitchRange"
#define PREF_SCORE_STYLE_DEFAULTSTYLEFILE                   "score/style/defaultStyleFile"
#define PREF_SCORE_STYLE_PARTSTYLEFILE                      "score/style/partStyleFile"
#define PREF_UI_CANVAS_BG_USECOLOR                          "ui/canvas/background/useColor"
#define PREF_UI_CANVAS_FG_USECOLOR                          "ui/canvas/foreground/useColor"
#define PREF_UI_CANVAS_FG_USECOLOR_IN_PALETTES              "ui/canvas/foreground/useColorInPalettes"
#define PREF_UI_CANVAS_BG_COLOR                             "ui/canvas/background/color"
#define PREF_UI_CANVAS_FG_COLOR                             "ui/canvas/foreground/color"
#define PREF_UI_CANVAS_BG_WALLPAPER                         "ui/canvas/background/wallpaper"
#define PREF_UI_CANVAS_FG_WALLPAPER                         "ui/canvas/foreground/wallpaper"
#define PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING              "ui/canvas/misc/antialiasedDrawing"
#define PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY              "ui/canvas/misc/selectionProximity"
#define PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION           "ui/canvas/scroll/verticalOrientation"
#define PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA               "ui/canvas/scroll/limitScrollArea"
#define PREF_UI_APP_STARTUP_CHECKUPDATE                     "ui/application/startup/checkUpdate"
#define PREF_UI_APP_STARTUP_CHECK_EXTENSIONS_UPDATE         "ui/application/startup/checkExtensionsUpdate"
#define PREF_UI_APP_STARTUP_SHOWNAVIGATOR                   "ui/application/startup/showNavigator"
#define PREF_UI_APP_STARTUP_SHOWPLAYPANEL                   "ui/application/startup/showPlayPanel"
#define PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN                "ui/application/startup/showSplashScreen"
#define PREF_UI_APP_STARTUP_SHOWSTARTCENTER                 "ui/application/startup/showStartCenter"
#define PREF_UI_APP_STARTUP_SHOWTOURS                       "ui/application/startup/showTours"
#define PREF_UI_APP_GLOBALSTYLE                             "ui/application/globalStyle"
#define PREF_UI_APP_LANGUAGE                                "ui/application/language"
#define PREF_UI_APP_RASTER_HORIZONTAL                       "ui/application/raster/horizontal"
#define PREF_UI_APP_RASTER_VERTICAL                         "ui/application/raster/vertical"
#define PREF_UI_APP_SHOWSTATUSBAR                           "ui/application/showStatusBar"
#define PREF_UI_APP_USENATIVEDIALOGS                        "ui/application/useNativeDialogs"
#define PREF_UI_PIANO_HIGHLIGHTCOLOR                        "ui/piano/highlightColor"
#define PREF_UI_SCORE_NOTE_DROPCOLOR                        "ui/score/note/dropColor"
#define PREF_UI_SCORE_DEFAULTCOLOR                          "ui/score/defaultColor"
#define PREF_UI_SCORE_FRAMEMARGINCOLOR                      "ui/score/frameMarginColor"
#define PREF_UI_SCORE_LAYOUTBREAKCOLOR                      "ui/score/layoutBreakColor"
#define PREF_UI_SCORE_VOICE1_COLOR                          "ui/score/voice1/color"
#define PREF_UI_SCORE_VOICE2_COLOR                          "ui/score/voice2/color"
#define PREF_UI_SCORE_VOICE3_COLOR                          "ui/score/voice3/color"
#define PREF_UI_SCORE_VOICE4_COLOR                          "ui/score/voice4/color"
#define PREF_UI_THEME_ICONHEIGHT                            "ui/theme/iconHeight"
#define PREF_UI_THEME_ICONWIDTH                             "ui/theme/iconWidth"
#define PREF_UI_THEME_FONTFAMILY                            "ui/theme/fontFamily"
#define PREF_UI_THEME_FONTSIZE                              "ui/theme/fontSize"
#define PREF_UI_PIANOROLL_DARK_SELECTION_BOX_COLOR          "ui/pianoroll/dark/selectionBox/color"
#define PREF_UI_PIANOROLL_DARK_NOTE_UNSEL_COLOR             "ui/pianoroll/dark/note/unselected/color"
#define PREF_UI_PIANOROLL_DARK_NOTE_SEL_COLOR               "ui/pianoroll/dark/note/selected/color"
#define PREF_UI_PIANOROLL_DARK_NOTE_DRAG_COLOR              "ui/pianoroll/dark/note/drag/color"
#define PREF_UI_PIANOROLL_DARK_BG_BASE_COLOR                "ui/pianoroll/dark/background/base/color"
#define PREF_UI_PIANOROLL_DARK_BG_KEY_HIGHLIGHT_COLOR       "ui/pianoroll/dark/background/keys/highlight/color"
#define PREF_UI_PIANOROLL_DARK_BG_KEY_WHITE_COLOR           "ui/pianoroll/dark/background/keys/white/color"
#define PREF_UI_PIANOROLL_DARK_BG_KEY_BLACK_COLOR           "ui/pianoroll/dark/background/keys/black/color"
#define PREF_UI_PIANOROLL_DARK_BG_GRIDLINE_COLOR            "ui/pianoroll/dark/background/gridLine/color"
#define PREF_UI_PIANOROLL_DARK_BG_TEXT_COLOR                "ui/pianoroll/dark/background/text/color"
#define PREF_UI_PIANOROLL_DARK_BG_TIE_COLOR                 "ui/pianoroll/dark/background/tie/color"
#define PREF_UI_PIANOROLL_LIGHT_SELECTION_BOX_COLOR         "ui/pianoroll/light/selectionBox/color"
#define PREF_UI_PIANOROLL_LIGHT_NOTE_UNSEL_COLOR            "ui/pianoroll/light/note/unselected/color"
#define PREF_UI_PIANOROLL_LIGHT_NOTE_SEL_COLOR              "ui/pianoroll/light/note/selected/color"
#define PREF_UI_PIANOROLL_LIGHT_NOTE_DRAG_COLOR             "ui/pianoroll/light/note/drag/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_BASE_COLOR               "ui/pianoroll/light/background/base/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_KEY_HIGHLIGHT_COLOR      "ui/pianoroll/light/background/keys/highlight/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_KEY_WHITE_COLOR          "ui/pianoroll/light/background/keys/white/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_KEY_BLACK_COLOR          "ui/pianoroll/light/background/keys/black/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_GRIDLINE_COLOR           "ui/pianoroll/light/background/gridLine/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_TEXT_COLOR               "ui/pianoroll/light/background/text/color"
#define PREF_UI_PIANOROLL_LIGHT_BG_TIE_COLOR                "ui/pianoroll/light/background/tie/color"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_ON     "ui/button/highlight/color/disabled/dark/on"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_DARK_OFF    "ui/button/highlight/color/disabled/dark/off"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_ON    "ui/button/highlight/color/disabled/light/on"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_DISABLED_LIGHT_OFF   "ui/button/highlight/color/disabled/light/off"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_ON      "ui/button/highlight/color/enabled/dark/on"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_DARK_OFF     "ui/button/highlight/color/enabled/dark/off"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_ON     "ui/button/highlight/color/enabled/light/on"
#define PREF_UI_BUTTON_HIGHLIGHT_COLOR_ENABLED_LIGHT_OFF    "ui/button/highlight/color/enabled/light/off"
#define PREF_UI_INSPECTOR_STYLED_TEXT_COLOR_LIGHT           "ui/inspector/styledtext/color/light"
#define PREF_UI_INSPECTOR_STYLED_TEXT_COLOR_DARK            "ui/inspector/styledtext/color/dark"
#define PREF_UI_AVSOMR_RECOGNITION_COLOR                    "ui/avsomr/recognition/valid/color"
#define PREF_UI_AVSOMR_NOT_RECOGNITION_COLOR                "ui/avsomr/recognition/notValid/color"
#define PREF_PAN_SMOOTHLY_ENABLED                           "smoothPan/enabled"
#define PREF_PAN_MODIFIER_BASE                              "smoothPan/modifier/baseSpeed"
#define PREF_PAN_MODIFIER_STEP                              "smoothPan/modifier/step"
#define PREF_PAN_MODIFIER_MIN                               "smoothPan/modifier/minSpeed"
#define PREF_PAN_MODIFIER_MAX                               "smoothPan/modifier/maxSpeed"
#define PREF_PAN_CURSOR_POS                                 "smoothPan/cursor/position"
#define PREF_PAN_CURSOR_VISIBLE                             "smoothPan/cursor/visible"
//#define PREF_PAN_DISTANCE_LEFT                              "smoothPan/distance/left"
//#define PREF_PAN_DISTANCE_LEFT1                             "smoothPan/distance/left1"
//#define PREF_PAN_DISTANCE_LEFT2                             "smoothPan/distance/left2"
//#define PREF_PAN_DISTANCE_LEFT3                             "smoothPan/distance/left3"
//#define PREF_PAN_DISTANCE_RIGHT                             "smoothPan/distance/right"
//#define PREF_PAN_DISTANCE_RIGHT1                            "smoothPan/distance/right1"
//#define PREF_PAN_DISTANCE_RIGHT2                            "smoothPan/distance/right2"
//#define PREF_PAN_DISTANCE_RIGHT3                            "smoothPan/distance/right3"
//#define PREF_PAN_MODIFIER_LEFT1                             "smoothPan/modifier/left1"
//#define PREF_PAN_MODIFIER_LEFT2                             "smoothPan/modifier/left2"
//#define PREF_PAN_MODIFIER_LEFT3                             "smoothPan/modifier/left3"
//#define PREF_PAN_MODIFIER_RIGHT1                            "smoothPan/modifier/right1"
//#define PREF_PAN_MODIFIER_RIGHT2                            "smoothPan/modifier/right2"
//#define PREF_PAN_MODIFIER_RIGHT3                            "smoothPan/modifier/right3"
//#define PREF_PAN_WEIGHT_NORMAL                              "smoothPan/weight/normal"
//#define PREF_PAN_WEIGHT_SMART                               "smoothPan/weight/smart"
//#define PREF_PAN_WEIGHT_ADVANCED                            "smoothPan/weight/advanced"
//#define PREF_PAN_SMART_TIMER_DURATION                       "smoothPan/smart/timer/duration"

#endif // PREFERENCEKEYS_H
