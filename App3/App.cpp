#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace WoodenEngine;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;

// The DirectX 12 Application template is documented at https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	Windows::Devices::Input::MouseDevice::GetForCurrentView()->MouseMoved += 
		ref new TypedEventHandler<Windows::Devices::Input::MouseDevice^, Windows::Devices::Input::MouseEventArgs^>(this, &App::OnMouseMoved);

	CoreWindow::GetForCurrentThread()->PointerPressed +=
		ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerPressed);

	CoreWindow::GetForCurrentThread()->PointerReleased +=
		ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerReleased);

	CoreWindow::GetForCurrentThread()->KeyDown +=
		ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::KeyEventArgs^>(this, &App::OnKeyPressed);

	CoreWindow::GetForCurrentThread()->KeyUp +=
		ref new TypedEventHandler<Windows::UI::Core::CoreWindow^, Windows::UI::Core::KeyEventArgs^>(this, &App::OnKeyReleased);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<FGameMain>(new FGameMain());
		m_main->Initialize(CoreWindow::GetForCurrentThread());
	}
}

// This method is called after the window becomes active.
void App::Run()
{
	
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			timer.Tick([&](float Delta)
			{
				m_main->Update(Delta);
			});

//			if (timer.GetFrameCount() == 0)
//			{
//				continue;
//			}
			
			m_main->Render();
		//	if (m_main->Render())
		//	{
				//GetDeviceResources()->Present();
		//	}
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
//		m_main->OnSuspending();
		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

//	m_main->OnResuming();
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
//	m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
//	m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
//	m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}

void WoodenEngine::App::OnMouseMoved(Windows::Devices::Input::MouseDevice ^ mouseDevice, Windows::Devices::Input::MouseEventArgs ^ args)
{
	if (!m_trackingPointer)
	{
		return;
	}

	const auto dx = static_cast<float>(args->MouseDelta.X);
	const auto dy = static_cast<float>(args->MouseDelta.Y);

	m_main->InputMouseMoved(dx, dy);
}

void WoodenEngine::App::OnKeyPressed(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::KeyEventArgs^ args)
{
	char key;
	switch(args->VirtualKey)
	{
	case Windows::System::VirtualKey::W:
		key = 'w';
		break;
	case Windows::System::VirtualKey::D:
		key = 'd';
		break;
	case Windows::System::VirtualKey::A:
		key = 'a';
		break;
	case Windows::System::VirtualKey::S:
		key = 's';
		break;
	case Windows::System::VirtualKey::Number1:
		key = '1';
		break;
	case Windows::System::VirtualKey::Number2:
		key = '2';
		break;
	case Windows::System::VirtualKey::Number3:
		key = '3';
		break;
	default:
		return;
	}
	m_main->InputKeyPressed(key);
}

void WoodenEngine::App::OnKeyReleased(Windows::UI::Core::CoreWindow^ coreWindow, Windows::UI::Core::KeyEventArgs^ args)
{

	char key;
	switch (args->VirtualKey)
	{
	case Windows::System::VirtualKey::W:
		key = 'w';
		break;
	case Windows::System::VirtualKey::D:
		key = 'd';
		break;
	case Windows::System::VirtualKey::A:
		key = 'a';
		break;
	case Windows::System::VirtualKey::S:
		key = 's';
		break;
	default:
		return;
	}
	m_main->InputKeyReleased(key);
}

void WoodenEngine::App::OnPointerPressed(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::PointerEventArgs ^ pointerEventArgs)
{
	m_trackingPointer = true;
}

void WoodenEngine::App::OnPointerReleased(Windows::UI::Core::CoreWindow ^ coreWindow, Windows::UI::Core::PointerEventArgs ^ pointerEventArgs)
{
	m_trackingPointer = false;
}
