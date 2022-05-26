/*
 * Carla Native Plugins
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

// config fix
#include "distrho-kars/DistrhoPluginInfo.h"
#include "CarlaJuceUtils.hpp"

// Plugin Code
#include "distrho-kars/DistrhoPluginKars.cpp"

// DISTRHO Code
#define DISTRHO_PLUGIN_TARGET_CARLA
#include "DistrhoPluginMain.cpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

static const NativePluginDescriptor karsDesc = {
    /* category  */ NATIVE_PLUGIN_CATEGORY_SYNTH,
    /* hints     */ static_cast<NativePluginHints>(NATIVE_PLUGIN_IS_RTSAFE
                                                  |NATIVE_PLUGIN_IS_SYNTH),
    /* supports  */ NATIVE_PLUGIN_SUPPORTS_NOTHING,
    /* audioIns  */ DISTRHO_PLUGIN_NUM_INPUTS,
    /* audioOuts */ DISTRHO_PLUGIN_NUM_OUTPUTS,
    /* midiIns   */ 1,
    /* midiOuts  */ 0,
    /* paramIns  */ DistrhoPluginKars::paramCount,
    /* paramOuts */ 0,
    /* name      */ DISTRHO_PLUGIN_NAME,
    /* label     */ "kars",
    /* maker     */ "falkTX, Chris Cannam",
    /* copyright */ "ISC",
    PluginDescriptorFILL(PluginCarla)
};

END_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

CARLA_API_EXPORT
void carla_register_native_plugin_distrho_kars();

CARLA_API_EXPORT
void carla_register_native_plugin_distrho_kars()
{
    USE_NAMESPACE_DISTRHO
    carla_register_native_plugin(&karsDesc);
}

// -----------------------------------------------------------------------
