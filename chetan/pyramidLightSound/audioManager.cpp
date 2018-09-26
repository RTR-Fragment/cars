#include "audioManager.h"

AudioManager::AudioManager()
{
    if (fopen_s(&logFile, "audio_manager_debug.log", "w") != 0)
	{
        printf("\nNot able to open audio manager debug log.\n");
	}

    log("---------- AudioManager | OpenAL Debug Logs Starts ----------\n");

    waveLoader = new CWaves();

    if(waveLoader == NULL)
    {
        log("[Error] | Not able to create wave loader.");
    }
}

ALboolean AudioManager::initialize()
{
    ALboolean initializationCompleted = AL_FALSE;
    device = alcOpenDevice(NULL);

    if(device)
    {
        log("[Info]: Audio device created: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
        context = alcCreateContext(device, NULL);

        if(context)
        {
            alcMakeContextCurrent(context);
            initializationCompleted = AL_TRUE;
            log("[Info]: Context created.\n");
        }
        else
        {
            alcCloseDevice(device);
            log("[Error]: Cannot create context.\n");
        }
    }
    else
    {
        log("[Error]: Cannot create audio device.\n");
    }

    return initializationCompleted;
}

ALboolean AudioManager::loadWaveAudio(const char* filePath, ALuint bufferId)
{
    ALboolean loaded = AL_FALSE;
	WAVEID waveId = 0;
	ALint dataSize = 0;
    ALint frequency = 0;
	ALenum bufferFormat;
	ALchar *data = NULL;

    if(waveLoader)
    {
        WAVERESULT loadWaveResult = waveLoader->LoadWaveFile(filePath, &waveId);

        if(SUCCEEDED(loadWaveResult))
        {
            WAVERESULT waveSizeResult = waveLoader->GetWaveSize(waveId, (unsigned long *)&dataSize);
            WAVERESULT waveDataResult = waveLoader->GetWaveData(waveId, (void **)&data);
            WAVERESULT waveFrequencyResult = waveLoader->GetWaveFrequency(waveId, (unsigned long *)&frequency);
            WAVERESULT waveBufferFormatResult = waveLoader->GetWaveALBufferFormat(waveId, &alGetEnumValue, (unsigned long *)&bufferFormat);

            log("[Info] | Buffer format: %d\n", bufferFormat);

            if(SUCCEEDED(waveSizeResult) && SUCCEEDED(waveDataResult) && SUCCEEDED(waveFrequencyResult) && SUCCEEDED(waveBufferFormatResult))
            {
                // Clear any errors
                alGetError();

                // Load the data to buffer
                alBufferData(bufferId, bufferFormat, data, dataSize, frequency);

                ALenum error =  alGetError();

                if(error == AL_NO_ERROR)
                {
                    loaded = AL_TRUE;
                    log("[Info] | wav file loaded: %s", filePath);
                }
                else
                {
                    log("[Error] | Not able to load audio data, error: %d, file: %s", error, filePath);
                }
            }
            else
            {
                log("[Error]: Not able to load wav data\n");
            }

            waveLoader->DeleteWaveFile(waveId);
        }
        else
        {
            log("[Error]: Not able to load audio file '%s'\n", filePath);
        }
    }

    return loaded;
}

void AudioManager::setListenerPosition(float x, float y, float z)
{
    alListener3f(AL_POSITION, x, y, z);
}

void AudioManager::setListenerVelocity(float x, float y, float z)
{
    alListener3f(AL_VELOCITY, x, y, z);
}

ALfloat AudioManager::getBufferLength(ALuint buffer)
{
    ALint size;
    ALint bits;
    ALint channels;
    ALint freq;

    alGetBufferi(buffer, AL_SIZE, &size);
    alGetBufferi(buffer, AL_BITS, &bits);
    alGetBufferi(buffer, AL_CHANNELS, &channels);
    alGetBufferi(buffer, AL_FREQUENCY, &freq);

    ALenum error =  alGetError();

    if( != AL_NO_ERROR)
    {
        return 0.0f;
    }

    return (ALfloat)((ALuint)size / channels / (bits / 8)) / (ALfloat)freq;
}

void AudioManager::log(const char* message, ...)
{
    if(logFile != NULL)
    {
        va_list args;
        va_start(args, message );
        vfprintf(logFile, message, args );
        va_end( args );

        fflush(logFile);
    }
}

void AudioManager::cleanUp()
{
    ALCcontext *pContext = alcGetCurrentContext();

	if(pContext == context)
    {
        alcMakeContextCurrent(NULL);
    }

	alcDestroyContext(context);
	alcCloseDevice(device);

    context = NULL;
    device = NULL;

    if (waveLoader)
	{
		delete waveLoader;
		waveLoader = NULL;
        log("[Info]: Audio device destroyed.\n");
	}

    if(logFile)
    {
        log("---------- AudioManager | OpenAL Debug Logs End ----------\n");
        fclose(logFile);
        logFile = NULL;
    }
}

AudioManager::~AudioManager()
{
    cleanUp();
}
