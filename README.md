# GCPadder

GCPadder is a utility that allows you to send your GameCube controller's inputs to your computer from your modded Wii, either wirelessly or using a USB Gecko. _(Performance is best on a stable ethernet connection or a USB Gecko.)_

## [Download the Windows PC Client](https://github.com/InvoxiPlayGames/GCPadder/releases/latest) - requires [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases/latest).

## How to use:

- Please make sure you have the [ViGEmBus driver](https://github.com/ViGEm/ViGEmBus/releases/latest) installed.
- If you are using Windows 7, please make sure you have the [.NET Framework 4.5.2 Runtime](https://dotnet.microsoft.com/download/dotnet-framework/net452) installed.
    - If you intend on using a USB Gecko, make sure you have [.NET Framework 4.0 Runtime](https://dotnet.microsoft.com/download/dotnet-framework/net40) installed as well as 4.5.2.
    - Windows 8.1 and Windows 10 should come with these runtimes pre-installed, or should offer a download if they are not.

1. Download the latest version of the [Wii Homebrew application and Windows PC client](https://github.com/InvoxiPlayGames/GCPadder/releases/latest).
2. Launch the homebrew application and press the button corresponding with the connection method you want to use. Wait until it says 'Listening on...'.
3. In the PC client, type in the IP address you see on your Wii's screen. The IP may already be filled in for you. The port is filled in automatically, in most cases there is no reason to touch this.
   - If using a USB Gecko, select the USB Gecko option in the PC client.
4. Click 'Connect' on the PC client, and if all goes well, an Xbox 360 Controller will appear to applications and games running on your PC with the inputs of your GameCube controller.

## For developers:

The "protocol" used is very simple, send 0x09AD09AD to the UDP server/USB Gecko from your application, and the Wii will start reporting inputs at ~200Hz. The controller input format is shown below as a C structure:
```c
struct NETPADData {
	uint16_t buttons;
	int8_t stickX;
	int8_t stickY;
	int8_t substickX;
	int8_t substickY;
	uint8_t triggerL;
	uint8_t triggerR;
} paddata;
```