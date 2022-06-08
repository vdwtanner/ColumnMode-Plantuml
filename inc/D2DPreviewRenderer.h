#pragma once

namespace CMPlantuml
{
	class D2DPreviewRenderer
	{
	public:
		D2DPreviewRenderer();
		void Init(HWND hwnd);

        void Draw();
        void OnResize();

        bool LoadPreviewImage(std::filesystem::path path, bool needToReset = true);

        void Zoom(float deltaZoom);
        void SetPosition(D2D1_VECTOR_2F newPos);
        D2D1_VECTOR_2F GetPosition() { return m_previewPos; }
        void ResetPreview();

    protected:
        void CreateDeviceIndependentResources();
        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();
        void SetTargetToBackBuffer();
        void UpdateZoomState();
        

    private:
        const float m_padding = 20.0f;

        HWND m_hwnd;
        D2D1_SIZE_U m_imageSize;
        D2D1_SIZE_F m_viewportSize;
        D2D1_SIZE_F m_paddedViewportSize;
        D2D1_VECTOR_2F m_previewPos;
        float m_zoom = 1;
        float m_zoomMin = .05f;
        float m_zoomMax = 10.0f;
        std::mutex m_imageMutex;

        //Device independent
        ComPtr<IWICImagingFactory> m_wicImagingFactory;
        ComPtr<IWICColorContext> m_wicColorContext;

        //Device Dependent
        ComPtr<ID2D1Factory6> m_d2dFactory;
        ComPtr<IDXGISwapChain> m_swapChain;
        ComPtr<ID2D1Device5> m_d2dDevice;
        ComPtr<ID2D1DeviceContext5> m_deviceContext;
        ComPtr<ID2D1ImageSourceFromWic> m_imageSource;
        ComPtr<ID2D1TransformedImageSource> m_scaledImage;
        ComPtr<ID2D1Effect> m_outputEffect;
	};
}