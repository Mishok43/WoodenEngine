#pragma once

#include "pch.h"
#include "Common\DeviceResources.h"
#include "Common\StepTimer.h"
#include "GameMain.h"

namespace WoodenEngine
{
	// Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
	ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
	{
	public:
		App();

		// IFrameworkView methods.
		virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
		virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
		virtual void Load(Platform::String^ entryPoint);
		virtual void Run();
		virtual void Uninitialize();

		// Application lifecycle event handlers.
	protected:
		void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);

		// Window event handlers.
		void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
		void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
		void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

		// DisplayInformation event handlers.
		void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
		void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

		void OnMouseMoved(Windows::Devices::Input::MouseDevice^ mouseDevice, Windows::Devices::Input::MouseEventArgs^ args);
		void OnPointerPressed(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::PointerEventArgs^ pointerEventArgs);
		void OnPointerReleased(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::PointerEventArgs^ pointerEventArgs);

	private:
		std::unique_ptr<FGameMain> m_main;
		bool m_windowClosed;
		bool m_windowVisible;
		bool m_trackingPointer = false;

		DX::StepTimer timer;
	};
}

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
