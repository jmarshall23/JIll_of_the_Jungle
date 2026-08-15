/*
===========================================================================

Jill of the Jungle Reconstructed
Copyright (C) 2026 Justin Marshall(IceColdDuke).

This file is part of the Jill of the Jungle Reconstructed Source Code.

===========================================================================
*/

#ifndef JILL_EPISODE_H
#define JILL_EPISODE_H

/* Every game target must select exactly one original Jill episode. */
#if (defined(JILL_EP1) + defined(JILL_EP2) + defined(JILL_EP3)) != 1
#  error Exactly one of JILL_EP1, JILL_EP2, or JILL_EP3 must be defined
#endif

#if defined(JILL_EP1)

#  define JILL_EPISODE_NUMBER       1
#  define JILL_PROGRAM_NAME         "JILL1"
#  define JILL_GAME_TITLE           "Jill of the Jungle"
#  define JILL_WINDOW_TITLE         "Jill of the Jungle - SHAREWARE"
#  define JILL_CONFIG_FILE          "JILL1.CFG"
#  define JILL_SHAPE_FILE           "JILL1.SHA"
#  define JILL_SOUND_FILE           "JILL1.VCL"
#  define JILL_INTRO_LEVEL          "intro.jn1"
#  define JILL_START_LEVEL          "map.jn1"
#  define JILL_START_LEVEL_NUMBER   127
#  define JILL_MENU_MUSIC           "funky.ddt"
#  define JILL_SAVE_PREFIX          "jn1save"

#elif defined(JILL_EP2)

#  define JILL_EPISODE_NUMBER       2
#  define JILL_PROGRAM_NAME         "JILL2"
#  define JILL_GAME_TITLE           "Jill Goes Underground"
#  define JILL_WINDOW_TITLE         "Jill Goes Underground"
#  define JILL_CONFIG_FILE          "jill2.cfg"
#  define JILL_SHAPE_FILE           "jill2.sha"
#  define JILL_SOUND_FILE           "jill2.vcl"
#  define JILL_INTRO_LEVEL          "intro.jn2"
#  define JILL_START_LEVEL          "0.jn2"
#  define JILL_START_LEVEL_NUMBER   0
#  define JILL_MENU_MUSIC           "peaks.ddt"
#  define JILL_SAVE_PREFIX          "jn2save"

#else

/* Episode 3 is deliberately not configured until its executable is mapped. */
#  error JILL_EP3 is reserved but Episode 3 has not been reconstructed yet

#endif

#endif
