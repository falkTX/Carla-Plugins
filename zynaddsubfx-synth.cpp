/*
 * Carla Native Plugins
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
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

#include "CarlaNativeExtUI.hpp"
#include "CarlaMIDI.h"
#include "CarlaThread.hpp"
#include "LinkedList.hpp"

#include "CarlaMathUtils.hpp"

#include <ctime>
#include <set>
#include <string>

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Weffc++"
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Weffc++"
# pragma GCC diagnostic ignored "-Wconversion"
# pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
// # pragma GCC diagnostic ignored "-Wsign-conversion"
# pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#include "Misc/Master.h"
#include "Misc/MiddleWare.h"
#include "Misc/Part.h"
#include "Misc/Util.h"

#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
# pragma GCC diagnostic pop
#endif

#include "water/maths/MathsFunctions.h"

using water::roundToIntAccurate;

using namespace zyncarla;

// #define ZYN_MSG_ANYWHERE

// -----------------------------------------------------------------------

class ZynAddSubFxPrograms
{
public:
    ZynAddSubFxPrograms() noexcept
        : fInitiated(false),
#ifdef CARLA_PROPER_CPP11_SUPPORT
          fRetProgram({0, 0, nullptr}),
#endif
          fProgramCount(0),
          fPrograms(nullptr) {}

    ~ZynAddSubFxPrograms() noexcept
    {
        if (! fInitiated)
            return;

        for (uint32_t i=0; i<fProgramCount; ++i)
            delete fPrograms[i];

        delete[] fPrograms;
    }

    void initIfNeeded()
    {
        if (fInitiated)
            return;
        fInitiated = true;

        std::vector<const ProgramInfo*> programs;
        programs.push_back(new ProgramInfo(0, 0, "default", ""));

        Config config;
        config.init();

        SYNTH_T synth;
        Master  master(synth, &config);

        // refresh banks
        master.bank.rescanforbanks();

        for (std::uint32_t i=0, size=static_cast<uint32_t>(master.bank.banks.size()); i<size; ++i)
        {
            const std::string dir(master.bank.banks[i].dir);

            if (dir.empty())
                continue;

            master.bank.loadbank(dir);

            for (uint ninstrument = 0; ninstrument < BANK_SIZE; ++ninstrument)
            {
                const Bank::ins_t& instrument(master.bank.ins[ninstrument]);

                if (instrument.name.empty() || instrument.name[0] == ' ')
                    continue;

                programs.push_back(new ProgramInfo(i+1U, ninstrument, instrument.name.c_str(), instrument.filename.c_str()));
            }
        }

        fPrograms = new const ProgramInfo*[programs.size()];

        for (const ProgramInfo* p : programs)
            fPrograms[fProgramCount++] = p;
    }

    uint32_t getNativeMidiProgramCount() const noexcept
    {
        return fProgramCount;
    }

    const NativeMidiProgram* getNativeMidiProgramInfo(const uint32_t index) const noexcept
    {
        if (index >= fProgramCount)
            return nullptr;

        const ProgramInfo* const pInfo(fPrograms[index]);
        CARLA_SAFE_ASSERT_RETURN(pInfo != nullptr, nullptr);

        fRetProgram.bank    = pInfo->bank;
        fRetProgram.program = pInfo->prog;
        fRetProgram.name    = pInfo->name;

        return &fRetProgram;
    }

    const char* getZynProgramFilename(const uint32_t bank, const uint32_t program) const noexcept
    {
        for (uint32_t i=0; i<fProgramCount; ++i)
        {
            const ProgramInfo* const pInfo(fPrograms[i]);

            if (pInfo->bank != bank)
                continue;
            if (pInfo->prog != program)
                continue;

            return pInfo->filename;
        }

        return nullptr;
    }

private:
  struct ProgramInfo {
      uint32_t bank;
      uint32_t prog;
      const char* name;
      const char* filename;

      ProgramInfo(uint32_t b, uint32_t p, const char* n, const char* fn) noexcept
        : bank(b),
          prog(p),
          name(carla_strdup_safe(n)),
          filename(carla_strdup_safe(fn)) {}

      ~ProgramInfo() noexcept
      {
          if (name != nullptr)
          {
              delete[] name;
              name = nullptr;
          }

          if (filename != nullptr)
          {
              delete[] filename;
              filename = nullptr;
          }
      }

#ifdef CARLA_PROPER_CPP11_SUPPORT
      ProgramInfo() = delete;
      ProgramInfo(ProgramInfo&) = delete;
      ProgramInfo(const ProgramInfo&) = delete;
      ProgramInfo& operator=(ProgramInfo&);
      ProgramInfo& operator=(const ProgramInfo&);
#endif
    };

    bool fInitiated;
    mutable NativeMidiProgram fRetProgram;
    uint32_t fProgramCount;
    const ProgramInfo** fPrograms;

    CARLA_PREVENT_HEAP_ALLOCATION
    CARLA_DECLARE_NON_COPY_CLASS(ZynAddSubFxPrograms)
};

static ZynAddSubFxPrograms sPrograms;

// -----------------------------------------------------------------------

class MiddleWareThread : private CarlaThread
{
public:
      class ScopedStopper
      {
      public:
          ScopedStopper(MiddleWareThread& mwt) noexcept
              : wasRunning(mwt.isThreadRunning()),
                thread(mwt),
                middleWare(mwt.fMiddleWare)
          {
              if (wasRunning)
                  thread.stop();
          }

          ~ScopedStopper() noexcept
          {
              if (wasRunning)
                  thread.start(middleWare);
          }

          void updateMiddleWare(MiddleWare* const mw) noexcept
          {
              middleWare = mw;
          }

      private:
          const bool wasRunning;
          MiddleWareThread& thread;
          MiddleWare* middleWare;

          CARLA_PREVENT_HEAP_ALLOCATION
          CARLA_DECLARE_NON_COPY_CLASS(ScopedStopper)
      };

      MiddleWareThread()
          : CarlaThread("ZynMiddleWare"),
            fMiddleWare(nullptr) {}

      void start(MiddleWare* const mw) noexcept
      {
          fMiddleWare = mw;
          startThread();
      }

      void stop() noexcept
      {
          stopThread(1000);
          fMiddleWare = nullptr;
      }

private:
    MiddleWare* fMiddleWare;

    void run() noexcept override
    {
        for (; ! shouldThreadExit();)
        {
            CARLA_SAFE_ASSERT_RETURN(fMiddleWare != nullptr,);

            try {
                fMiddleWare->tick();
            } CARLA_SAFE_EXCEPTION("ZynAddSubFX MiddleWare tick");

            carla_msleep(1);
        }
    }

    CARLA_DECLARE_NON_COPY_CLASS(MiddleWareThread)
};

// -----------------------------------------------------------------------

class ZynAddSubFxPlugin : public NativePluginAndUiClass
{
public:
    enum Parameters {
        kParamPart01Enabled ,
        kParamPart16Enabled = kParamPart01Enabled + 15,
        kParamPart01Volume,
        kParamPart16Volume = kParamPart01Volume + 15,
        kParamPart01Panning,
        kParamPart16Panning = kParamPart01Panning + 15,
        kParamFilterCutoff,  // Filter Frequency
        kParamFilterQ,       // Filter Resonance
        kParamBandwidth,     // Bandwidth
        kParamModAmp,        // FM Gain
        kParamResCenter,     // Resonance center frequency
        kParamResBandwidth,  // Resonance bandwidth
        kParamCount
    };

    ZynAddSubFxPlugin(const NativeHostDescriptor* const host)
        : NativePluginAndUiClass(host, "zynaddsubfx-ui"),
          fMiddleWare(nullptr),
          fMaster(nullptr),
          fSynth(),
          fConfig(),
          fDefaultState(nullptr),
          fMutex(),
          fMiddleWareThread(new MiddleWareThread())
    {
        isPlugin = true;

        sPrograms.initIfNeeded();
        fConfig.init();

        // init parameters to default
        fParameters[kParamPart01Enabled] = 1.0f;

        for (int i=kParamPart16Enabled+1; --i>kParamPart01Enabled;)
            fParameters[i] = 0.0f;

        for (int i=kParamPart16Volume+1; --i>=kParamPart01Volume;)
            fParameters[i] = 100.0f;

        for (int i=kParamPart16Panning+1; --i>=kParamPart01Panning;)
            fParameters[i] = 64.0f;

        fParameters[kParamFilterCutoff] = 64.0f;
        fParameters[kParamFilterQ]      = 64.0f;
        fParameters[kParamBandwidth]    = 64.0f;
        fParameters[kParamModAmp]       = 127.0f;
        fParameters[kParamResCenter]    = 64.0f;
        fParameters[kParamResBandwidth] = 64.0f;

        fSynth.buffersize = static_cast<int>(getBufferSize());
        fSynth.samplerate = static_cast<uint>(getSampleRate());

        if (fSynth.buffersize > 32)
            fSynth.buffersize = 32;

        fSynth.alias();

        _initMaster();
        _setMasterParameters();

        fMaster->getalldata(&fDefaultState);

        fMiddleWareThread->start(fMiddleWare);
    }

    ~ZynAddSubFxPlugin() override
    {
        fMiddleWareThread->stop();
        _deleteMaster();
        std::free(fDefaultState);
    }

protected:
    // -------------------------------------------------------------------
    // Plugin parameter calls

    uint32_t getParameterCount() const final
    {
        return kParamCount;
    }

    const NativeParameter* getParameterInfo(const uint32_t index) const override
    {
        CARLA_SAFE_ASSERT_RETURN(index < kParamCount, nullptr);

        static NativeParameter param;

        int hints = NATIVE_PARAMETER_IS_ENABLED|NATIVE_PARAMETER_IS_AUTOMABLE;

        param.name = nullptr;
        param.unit = nullptr;
        param.ranges.def       = 64.0f;
        param.ranges.min       = 0.0f;
        param.ranges.max       = 127.0f;
        param.ranges.step      = 1.0f;
        param.ranges.stepSmall = 1.0f;
        param.ranges.stepLarge = 20.0f;
        param.scalePointCount  = 0;
        param.scalePoints      = nullptr;

        if (index <= kParamPart16Enabled)
        {
            hints |= NATIVE_PARAMETER_IS_BOOLEAN;
            param.ranges.def       = 0.0f;
            param.ranges.min       = 0.0f;
            param.ranges.max       = 1.0f;
            param.ranges.step      = 1.0f;
            param.ranges.stepSmall = 1.0f;
            param.ranges.stepLarge = 1.0f;

            #define PARAM_PART_ENABLE_DESC(N) \
            case kParamPart01Enabled + N - 1: \
                param.name = "Part " #N " Enabled"; break;

            switch (index)
            {
            PARAM_PART_ENABLE_DESC( 1)
            PARAM_PART_ENABLE_DESC( 2)
            PARAM_PART_ENABLE_DESC( 3)
            PARAM_PART_ENABLE_DESC( 4)
            PARAM_PART_ENABLE_DESC( 5)
            PARAM_PART_ENABLE_DESC( 6)
            PARAM_PART_ENABLE_DESC( 7)
            PARAM_PART_ENABLE_DESC( 8)
            PARAM_PART_ENABLE_DESC( 9)
            PARAM_PART_ENABLE_DESC(10)
            PARAM_PART_ENABLE_DESC(11)
            PARAM_PART_ENABLE_DESC(12)
            PARAM_PART_ENABLE_DESC(13)
            PARAM_PART_ENABLE_DESC(14)
            PARAM_PART_ENABLE_DESC(15)
            PARAM_PART_ENABLE_DESC(16)
            }

            #undef PARAM_PART_ENABLE_DESC
        }
        else if (index <= kParamPart16Volume)
        {
            hints |= NATIVE_PARAMETER_IS_INTEGER;
            param.ranges.def = 100.0f;

            #define PARAM_PART_ENABLE_DESC(N) \
            case kParamPart01Volume + N - 1: \
                param.name = "Part " #N " Volume"; break;

            switch (index)
            {
            PARAM_PART_ENABLE_DESC( 1)
            PARAM_PART_ENABLE_DESC( 2)
            PARAM_PART_ENABLE_DESC( 3)
            PARAM_PART_ENABLE_DESC( 4)
            PARAM_PART_ENABLE_DESC( 5)
            PARAM_PART_ENABLE_DESC( 6)
            PARAM_PART_ENABLE_DESC( 7)
            PARAM_PART_ENABLE_DESC( 8)
            PARAM_PART_ENABLE_DESC( 9)
            PARAM_PART_ENABLE_DESC(10)
            PARAM_PART_ENABLE_DESC(11)
            PARAM_PART_ENABLE_DESC(12)
            PARAM_PART_ENABLE_DESC(13)
            PARAM_PART_ENABLE_DESC(14)
            PARAM_PART_ENABLE_DESC(15)
            PARAM_PART_ENABLE_DESC(16)
            }

            #undef PARAM_PART_ENABLE_DESC
        }
        else if (index <= kParamPart16Panning)
        {
            hints |= NATIVE_PARAMETER_IS_INTEGER;

            #define PARAM_PART_ENABLE_DESC(N) \
            case kParamPart01Panning + N - 1: \
                param.name = "Part " #N " Panning"; break;

            switch (index)
            {
            PARAM_PART_ENABLE_DESC( 1)
            PARAM_PART_ENABLE_DESC( 2)
            PARAM_PART_ENABLE_DESC( 3)
            PARAM_PART_ENABLE_DESC( 4)
            PARAM_PART_ENABLE_DESC( 5)
            PARAM_PART_ENABLE_DESC( 6)
            PARAM_PART_ENABLE_DESC( 7)
            PARAM_PART_ENABLE_DESC( 8)
            PARAM_PART_ENABLE_DESC( 9)
            PARAM_PART_ENABLE_DESC(10)
            PARAM_PART_ENABLE_DESC(11)
            PARAM_PART_ENABLE_DESC(12)
            PARAM_PART_ENABLE_DESC(13)
            PARAM_PART_ENABLE_DESC(14)
            PARAM_PART_ENABLE_DESC(15)
            PARAM_PART_ENABLE_DESC(16)
            }

            #undef PARAM_PART_ENABLE_DESC
        }
        else if (index <= kParamResBandwidth)
        {
            hints |= NATIVE_PARAMETER_IS_INTEGER;

            switch (index)
            {
            case kParamFilterCutoff:
                param.name = "Filter Cutoff";
                break;
            case kParamFilterQ:
                param.name = "Filter Q";
                break;
            case kParamBandwidth:
                param.name = "Bandwidth";
                break;
            case kParamModAmp:
                param.name = "FM Gain";
                param.ranges.def = 127.0f;
                break;
            case kParamResCenter:
                param.name = "Res Center Freq";
                break;
            case kParamResBandwidth:
                param.name = "Res Bandwidth";
                break;
            }
        }

        param.hints = static_cast<NativeParameterHints>(hints);

        return &param;
    }

    float getParameterValue(const uint32_t index) const final
    {
        CARLA_SAFE_ASSERT_RETURN(index < kParamCount, 0.0f);

        return fParameters[index];
    }

    // -------------------------------------------------------------------
    // Plugin midi-program calls

    uint32_t getMidiProgramCount() const noexcept override
    {
        return sPrograms.getNativeMidiProgramCount();
    }

    const NativeMidiProgram* getMidiProgramInfo(const uint32_t index) const noexcept override
    {
        return sPrograms.getNativeMidiProgramInfo(index);
    }

    // -------------------------------------------------------------------
    // Plugin state calls

    void setParameterValue(const uint32_t index, const float value) final
    {
        CARLA_SAFE_ASSERT_RETURN(index < kParamCount,);

        if (index <= kParamPart16Enabled)
        {
            fParameters[index] = (value >= 0.5f) ? 1.0f : 0.0f;

            char msg[24];
            std::sprintf(msg, "/part%i/Penabled", index-kParamPart01Enabled);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, (value >= 0.5f) ? "T" : "F");
#else
            fMiddleWare->transmitMsg("/echo", "ss", "OSC_URL", "");
            fMiddleWare->activeUrl("");
            fMiddleWare->transmitMsg(msg, (value >= 0.5f) ? "T" : "F");
#endif
        }
        else if (index <= kParamPart16Volume)
        {
            if (carla_isEqual(fParameters[index], value))
                return;

            fParameters[index] = std::round(carla_fixedValue(0.0f, 127.0f, value));

            char msg[24];
            std::sprintf(msg, "/part%i/Pvolume", index-kParamPart01Volume);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, "i", static_cast<int>(fParameters[index]));
#else
            fMiddleWare->transmitMsg("/echo", "ss", "OSC_URL", "");
            fMiddleWare->activeUrl("");
            fMiddleWare->transmitMsg(msg, "i", static_cast<int>(fParameters[index]));
#endif
        }
        else if (index <= kParamPart16Panning)
        {
            if (carla_isEqual(fParameters[index], value))
                return;

            fParameters[index] = std::round(carla_fixedValue(0.0f, 127.0f, value));

            char msg[24];
            std::sprintf(msg, "/part%i/Ppanning", index-kParamPart01Panning);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, "i", static_cast<int>(fParameters[index]));
#else
            fMiddleWare->transmitMsg("/echo", "ss", "OSC_URL", "");
            fMiddleWare->activeUrl("");
            fMiddleWare->transmitMsg(msg, "i", static_cast<int>(fParameters[index]));
#endif
        }
        else if (index <= kParamResBandwidth)
        {
            const MidiControllers zynControl(getZynControlFromIndex(index));
            CARLA_SAFE_ASSERT_RETURN(zynControl != C_NULL,);

            fParameters[index] = std::round(carla_fixedValue(0.0f, 127.0f, value));

            for (int npart=0; npart<NUM_MIDI_PARTS; ++npart)
            {
                if (fMaster->part[npart] != nullptr)
                    fMaster->part[npart]->SetController(zynControl, static_cast<int>(value));
            }
        }
    }

    void setMidiProgram(const uint8_t channel, const uint32_t bank, const uint32_t program) override
    {
        CARLA_SAFE_ASSERT_RETURN(program < BANK_SIZE,);

        if (bank == 0)
        {
            // reset part to default
            setState(fDefaultState);
            return;
        }

        const char* const filename(sPrograms.getZynProgramFilename(bank, program));
        CARLA_SAFE_ASSERT_RETURN(filename != nullptr && filename[0] != '\0',);

#ifdef ZYN_MSG_ANYWHERE
        fMiddleWare->messageAnywhere("/load-part", "is", channel, filename);
#else
        fMiddleWare->transmitMsg("/load-part", "is", channel, filename);
#endif
    }

    void setCustomData(const char* const key, const char* const value) override
    {
        CARLA_SAFE_ASSERT_RETURN(key != nullptr && key[0] != '\0',);
        CARLA_SAFE_ASSERT_RETURN(value != nullptr,);

        /**/ if (std::strcmp(key, "CarlaAlternateFile1") == 0) // xmz
        {
#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere("/load_xmz", "s", value);
#else
            fMiddleWare->transmitMsg("/load_xmz", "s", value);
#endif
        }
        else if (std::strcmp(key, "CarlaAlternateFile2") == 0) // xiz
        {
#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere("/load_xiz", "is", 0, value);
#else
            fMiddleWare->transmitMsg("/load_xiz", "is", 0, value);
#endif
        }
    }

    // -------------------------------------------------------------------
    // Plugin process calls

    void process(const float* const*, float** const outBuffer, const uint32_t frames,
                 const NativeMidiEvent* const midiEvents, const uint32_t midiEventCount) override
    {
        if (! fMutex.tryLock())
        {
            if (! isOffline())
            {
                carla_zeroFloats(outBuffer[0], frames);
                carla_zeroFloats(outBuffer[1], frames);
                return;
            }

            fMutex.lock();
        }

        uint32_t framesOffset = 0;

        for (uint32_t i=0; i < midiEventCount; ++i)
        {
            const NativeMidiEvent* const midiEvent(&midiEvents[i]);

            if (midiEvent->time >= frames)
                continue;

            if (midiEvent->time > framesOffset)
            {
                fMaster->GetAudioOutSamples(midiEvent->time-framesOffset, fSynth.samplerate, outBuffer[0]+framesOffset,
                                                                                             outBuffer[1]+framesOffset);
                framesOffset = midiEvent->time;
            }

            const uint8_t status  = MIDI_GET_STATUS_FROM_DATA(midiEvent->data);
            const char    channel = MIDI_GET_CHANNEL_FROM_DATA(midiEvent->data);

            if (MIDI_IS_STATUS_NOTE_OFF(status))
            {
                const char note = static_cast<char>(midiEvent->data[1]);

                fMaster->noteOff(channel, note);
            }
            else if (MIDI_IS_STATUS_NOTE_ON(status))
            {
                const char note = static_cast<char>(midiEvent->data[1]);
                const char velo = static_cast<char>(midiEvent->data[2]);

                fMaster->noteOn(channel, note, velo);
            }
            else if (MIDI_IS_STATUS_POLYPHONIC_AFTERTOUCH(status))
            {
                const char note     = static_cast<char>(midiEvent->data[1]);
                const char pressure = static_cast<char>(midiEvent->data[2]);

                fMaster->polyphonicAftertouch(channel, note, pressure);
            }
            else if (MIDI_IS_STATUS_CONTROL_CHANGE(status))
            {
                // skip controls which we map to parameters
                if (getIndexFromZynControl(midiEvent->data[1]) != kParamCount)
                    continue;

                const int control = midiEvent->data[1];
                const int value   = midiEvent->data[2];

                fMaster->setController(channel, control, value);
            }
            else if (MIDI_IS_STATUS_PITCH_WHEEL_CONTROL(status))
            {
                const uint8_t lsb = midiEvent->data[1];
                const uint8_t msb = midiEvent->data[2];
                const int   value = ((msb << 7) | lsb) - 8192;

                fMaster->setController(channel, C_pitchwheel, value);
            }
        }

        if (frames > framesOffset)
            fMaster->GetAudioOutSamples(frames-framesOffset, fSynth.samplerate, outBuffer[0]+framesOffset,
                                                                                outBuffer[1]+framesOffset);

        fMutex.unlock();
    }

    // -------------------------------------------------------------------
    // Plugin UI calls

#ifdef HAVE_ZYN_UI_DEPS
    void uiShow(const bool show) override
    {
        if (show)
        {
            if (isPipeRunning())
            {
                writeFocusMessage();
                return;
            }

            carla_stdout("Trying to start UI using \"%s\"", getExtUiPath());

            CarlaExternalUI::setData(getExtUiPath(), fMiddleWare->getServerAddress(), getUiName());

            if (! CarlaExternalUI::startPipeServer(true))
            {
                uiClosed();
                hostUiUnavailable();
            }
        }
        else
        {
            CarlaExternalUI::stopPipeServer(2000);
        }
    }
#endif

    // -------------------------------------------------------------------
    // Plugin state calls

    char* getState() const override
    {
        const MiddleWareThread::ScopedStopper mwss(*fMiddleWareThread);

        char* data = nullptr;
        fMaster->getalldata(&data);
        return data;
    }

    void setState(const char* const data) override
    {
        CARLA_SAFE_ASSERT_RETURN(data != nullptr,);

        const MiddleWareThread::ScopedStopper mwss(*fMiddleWareThread);
        const CarlaMutexLocker cml(fMutex);

        fMaster->defaults();
        fMaster->putalldata(data);
        fMaster->applyparameters();
        fMaster->initialize_rt();

        fMiddleWare->updateResources(fMaster);

        _setMasterParameters();
    }

    // -------------------------------------------------------------------
    // Plugin dispatcher

    void bufferSizeChanged(const uint32_t bufferSize) final
    {
        MiddleWareThread::ScopedStopper mwss(*fMiddleWareThread);

        char* const state(getState());

        _deleteMaster();

        fSynth.buffersize = static_cast<int>(bufferSize);

        if (fSynth.buffersize > 32)
            fSynth.buffersize = 32;

        fSynth.alias();

        _initMaster();
        mwss.updateMiddleWare(fMiddleWare);

        setState(state);
        std::free(state);
    }

    void sampleRateChanged(const double sampleRate) final
    {
        MiddleWareThread::ScopedStopper mwss(*fMiddleWareThread);

        char* const state(getState());

        _deleteMaster();

        fSynth.samplerate = static_cast<uint>(sampleRate);
        fSynth.alias();

        _initMaster();
        mwss.updateMiddleWare(fMiddleWare);

        setState(state);
        std::free(state);
    }

    // -------------------------------------------------------------------

private:
    MiddleWare* fMiddleWare;
    Master*     fMaster;
    SYNTH_T     fSynth;
    Config      fConfig;
    char*       fDefaultState;

    float fParameters[kParamCount];

    CarlaMutex fMutex;
    CarlaScopedPointer<MiddleWareThread> fMiddleWareThread;

    static MidiControllers getZynControlFromIndex(const uint index)
    {
        switch (index)
        {
        case kParamFilterCutoff:
            return C_filtercutoff;
        case kParamFilterQ:
            return C_filterq;
        case kParamBandwidth:
            return C_bandwidth;
        case kParamModAmp:
            return C_fmamp;
        case kParamResCenter:
            return C_resonance_center;
        case kParamResBandwidth:
            return C_resonance_bandwidth;
        default:
            return C_NULL;
        }
    }

    static Parameters getIndexFromZynControl(const uint8_t control)
    {
        switch (control)
        {
        case C_filtercutoff:
            return kParamFilterCutoff;
        case C_filterq:
            return kParamFilterQ;
        case C_bandwidth:
            return kParamBandwidth;
        case C_fmamp:
            return kParamModAmp;
        case C_resonance_center:
            return kParamResCenter;
        case C_resonance_bandwidth:
            return kParamResBandwidth;
        default:
            return kParamCount;
        }
    }

    // -------------------------------------------------------------------

    void _initMaster()
    {
        fMiddleWare = new MiddleWare(std::move(fSynth), &fConfig);
        fMiddleWare->setUiCallback(__uiCallback, this);
        fMiddleWare->setIdleCallback(_idleCallback, this);
        _masterChangedCallback(fMiddleWare->spawnMaster());
    }

    void _setMasterParameters()
    {
#ifndef ZYN_MSG_ANYWHERE
        fMiddleWare->transmitMsg("/echo", "ss", "OSC_URL", "");
        fMiddleWare->activeUrl("");
#endif
        char msg[24];

        for (int i=kParamPart16Enabled+1; --i>=kParamPart01Enabled;)
        {
            std::sprintf(msg, "/part%i/Penabled", i-kParamPart01Enabled);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, (fParameters[i] >= 0.5f) ? "T" : "F");
#else
            fMiddleWare->transmitMsg(msg, (fParameters[i] >= 0.5f) ? "T" : "F");
#endif
        }

        for (int i=kParamPart16Volume+1; --i>=kParamPart01Volume;)
        {
            std::sprintf(msg, "/part%i/Pvolume", i-kParamPart01Volume);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, "i", static_cast<int>(fParameters[i]));
#else
            fMiddleWare->transmitMsg(msg, "i", static_cast<int>(fParameters[i]));
#endif
        }

        for (int i=kParamPart16Panning+1; --i>=kParamPart01Panning;)
        {
            std::sprintf(msg, "/part%i/Ppanning", i-kParamPart01Panning);

#ifdef ZYN_MSG_ANYWHERE
            fMiddleWare->messageAnywhere(msg, "i", static_cast<int>(fParameters[i]));
#else
            fMiddleWare->transmitMsg(msg, "i", static_cast<int>(fParameters[i]));
#endif
        }

        for (int i=0; i<NUM_MIDI_PARTS; ++i)
        {
            fMaster->part[i]->SetController(C_filtercutoff,        static_cast<int>(fParameters[kParamFilterCutoff]));
            fMaster->part[i]->SetController(C_filterq,             static_cast<int>(fParameters[kParamFilterQ]));
            fMaster->part[i]->SetController(C_bandwidth,           static_cast<int>(fParameters[kParamBandwidth]));
            fMaster->part[i]->SetController(C_fmamp,               static_cast<int>(fParameters[kParamModAmp]));
            fMaster->part[i]->SetController(C_resonance_center,    static_cast<int>(fParameters[kParamResCenter]));
            fMaster->part[i]->SetController(C_resonance_bandwidth, static_cast<int>(fParameters[kParamResBandwidth]));
        }
    }

    void _deleteMaster()
    {
        fMaster = nullptr;
        delete fMiddleWare;
        fMiddleWare = nullptr;
    }

    void _masterChangedCallback(Master* m)
    {
        fMaster = m;
        fMaster->setMasterChangedCallback(__masterChangedCallback, this);
    }

    static void __masterChangedCallback(void* ptr, Master* m)
    {
        ((ZynAddSubFxPlugin*)ptr)->_masterChangedCallback(m);
    }

    void _uiCallback(const char* const msg)
    {
        if (std::strncmp(msg, "/part", 5) != 0)
            return;

        const char* msgtmp = msg;
        msgtmp += 5;
        CARLA_SAFE_ASSERT_RETURN( msgtmp[0] >= '0' && msgtmp[0] <= '9',);
        CARLA_SAFE_ASSERT_RETURN((msgtmp[1] >= '0' && msgtmp[1] <= '9') || msgtmp[1] == '/',);

        char partnstr[3] = { '\0', '\0', '\0' };

        partnstr[0] = msgtmp[0];
        ++msgtmp;

        if (msgtmp[0] >= '0' && msgtmp[0] <= '9')
        {
            partnstr[1] = msgtmp[0];
            ++msgtmp;
        }

        const int ipartn = std::atoi(partnstr);
        CARLA_SAFE_ASSERT_RETURN(ipartn >= 0,);

        const uint partn = static_cast<uint>(ipartn);
        ++msgtmp;

        /**/ if (std::strcmp(msgtmp, "Penabled") == 0)
        {
            const uint index  = kParamPart01Enabled+partn;
            const bool enable = rtosc_argument(msg,0).T;

            fParameters[index] = enable ? 1.0f : 0.0f;
            uiParameterChanged(kParamPart01Enabled+partn, enable ? 1.0f : 0.0f);
        }
        else if (std::strcmp(msgtmp, "Pvolume") == 0)
        {
            const uint index  = kParamPart01Volume+partn;
            const int  ivalue = rtosc_argument(msg,0).i;
            const float value = static_cast<float>(ivalue);

            fParameters[index] = value;
            uiParameterChanged(kParamPart01Volume+partn, value);
        }
        else if (std::strcmp(msgtmp, "Ppanning") == 0)
        {
            const uint index  = kParamPart01Panning+partn;
            const int  ivalue = rtosc_argument(msg,0).i;
            const float value = static_cast<float>(ivalue);

            fParameters[index] = value;
            uiParameterChanged(kParamPart01Panning+partn, value);
        }
    }

    static void __uiCallback(void* ptr, const char* msg)
    {
        ((ZynAddSubFxPlugin*)ptr)->_uiCallback(msg);
    }

    static void _idleCallback(void* ptr)
    {
        ((ZynAddSubFxPlugin*)ptr)->hostGiveIdle();
    }

    // -------------------------------------------------------------------

    PluginClassEND(ZynAddSubFxPlugin)
    CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZynAddSubFxPlugin)
};

// -----------------------------------------------------------------------

static const NativePluginDescriptor zynaddsubfxDesc = {
    /* category  */ NATIVE_PLUGIN_CATEGORY_SYNTH,
    /* hints     */ static_cast<NativePluginHints>(NATIVE_PLUGIN_IS_SYNTH
#ifdef HAVE_ZYN_UI_DEPS
                                                  |NATIVE_PLUGIN_HAS_UI
#endif
                                                  |NATIVE_PLUGIN_USES_MULTI_PROGS
                                                  |NATIVE_PLUGIN_USES_STATE),
    /* supports  */ static_cast<NativePluginSupports>(NATIVE_PLUGIN_SUPPORTS_CONTROL_CHANGES
                                                     |NATIVE_PLUGIN_SUPPORTS_NOTE_AFTERTOUCH
                                                     |NATIVE_PLUGIN_SUPPORTS_PITCHBEND
                                                     |NATIVE_PLUGIN_SUPPORTS_ALL_SOUND_OFF),
    /* audioIns  */ 0,
    /* audioOuts */ 2,
    /* midiIns   */ 1,
    /* midiOuts  */ 0,
    /* paramIns  */ ZynAddSubFxPlugin::kParamCount,
    /* paramOuts */ 0,
    /* name      */ "ZynAddSubFX",
    /* label     */ "zynaddsubfx",
    /* maker     */ "falkTX, Mark McCurry, Nasca Octavian Paul",
    /* copyright */ "GNU GPL v2+",
    PluginDescriptorFILL(ZynAddSubFxPlugin)
};

// -----------------------------------------------------------------------

CARLA_EXPORT
void carla_register_native_plugin_zynaddsubfx_synth();

CARLA_EXPORT
void carla_register_native_plugin_zynaddsubfx_synth()
{
    carla_register_native_plugin(&zynaddsubfxDesc);
}

// -----------------------------------------------------------------------
