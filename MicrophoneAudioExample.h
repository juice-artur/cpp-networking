#pragma comment(lib, "ole32.lib")
#include <stdio.h>
#include <Windows.h>
#include <initguid.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <assert.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

int main()
{
    HRESULT hr;
    IMMDeviceEnumerator* enumerator = NULL;
    IMMDevice* recorder = NULL;
    IMMDevice* renderer = NULL;
    IAudioClient* recorderClient = NULL;
    IAudioClient* renderClient = NULL;
    IAudioRenderClient* renderService = NULL;
    IAudioCaptureClient* captureService = NULL;
    WAVEFORMATEX* format = NULL;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&enumerator
    );
    assert(SUCCEEDED(hr));

    hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &recorder);
    assert(SUCCEEDED(hr));

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &renderer);
    assert(SUCCEEDED(hr));

    hr = enumerator->Release();
    assert(SUCCEEDED(hr));

    hr = recorder->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&recorderClient);
    assert(SUCCEEDED(hr));

    hr = renderer->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&renderClient);
    assert(SUCCEEDED(hr));

    hr = recorderClient->GetMixFormat(&format);
    assert(SUCCEEDED(hr));

    printf("Mix format:\n");
    printf("  Frame size     : %d\n", format->nBlockAlign);
    printf("  Channels       : %d\n", format->nChannels);
    printf("  Bits per second: %d\n", format->wBitsPerSample);
    printf("  Sample rate:   : %d\n", format->nSamplesPerSec);


    hr = recorderClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, format, NULL);
    assert(SUCCEEDED(hr));

    hr = renderClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000000, 0, format, NULL);
    assert(SUCCEEDED(hr));

    hr = renderClient->GetService(IID_IAudioRenderClient, (void**)&renderService);
    assert(SUCCEEDED(hr));

    hr = recorderClient->GetService(IID_IAudioCaptureClient, (void**)&captureService);
    assert(SUCCEEDED(hr));

    UINT32 nFrames;
    DWORD flags;
    BYTE* captureBuffer;
    BYTE* renderBuffer;

    hr = recorderClient->Start();
    assert(SUCCEEDED(hr));

    hr = renderClient->Start();
    assert(SUCCEEDED(hr));

    while (true)
    {
        hr = captureService->GetBuffer(&captureBuffer, &nFrames, &flags, NULL, NULL);
        assert(SUCCEEDED(hr));

        hr = captureService->ReleaseBuffer(nFrames);
        assert(SUCCEEDED(hr));

        hr = renderService->GetBuffer(nFrames, &renderBuffer);
        assert(SUCCEEDED(hr));


        memcpy(renderBuffer, captureBuffer, format->nBlockAlign * nFrames);

        hr = renderService->ReleaseBuffer(nFrames, 0);
        assert(SUCCEEDED(hr));
    }
    

    recorderClient->Stop();
    renderClient->Stop();

    captureService->Release();
    renderService->Release();

    recorderClient->Release();
    renderClient->Release();

    recorder->Release();
    renderer->Release();

    CoUninitialize();
}