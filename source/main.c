#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <network.h>
#include <wiiuse/wpad.h>
#include <string.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

u32 storedmagic = 0x0;
s8 selectedoutput = -1;

u16 netport = 0x9AD;
s32 sock;
u32 size;
struct sockaddr_in connected;

char localip[16] = {0};
char gateway[16] = {0};
char netmask[16] = {0};
int recv_buffer(void *buffer, int size) {
	if (selectedoutput == 2) {
		size = sizeof(connected);
		return net_recvfrom(sock, buffer, size, 0, (struct sockaddr *)&connected, (u32*)&size);
	} else {
		return usb_recvbuffer(selectedoutput, buffer, size);
	}
}
int send_buffer(void *buffer, int size) {
	if (selectedoutput == 2) {
		return net_sendto(sock, buffer, size, 0, (struct sockaddr *)&connected, size);
	} else {
		return usb_sendbuffer(selectedoutput, buffer, size);
	}
}

typedef struct NETPADData NETPADData;
struct NETPADData {
	u16 buttons;
	s8 stickX;
	s8 stickY;
	s8 substickX;
	s8 substickY;
	u8 triggerL;
	u8 triggerR;
} paddata;

struct NETPADData old;

int main(int argc, char **argv) {
	VIDEO_Init();
	WPAD_Init();
	PAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	bool geckoA;
	bool geckoB;

	printf("\x1b[2;0H");
	printf("GCPadder v1.0\n");
	printf("-------------\n\n");
	printf("Press A on GameCube Pad 1 to use the network.\n");
	if ((geckoB = usb_isgeckoalive(1))) printf("Press B on GameCube Pad 1 to use USB Gecko Slot B.\n");
	if ((geckoA = usb_isgeckoalive(0))) printf("Press X on GameCube Pad 1 to use USB Gecko Slot A.\n");
	printf("Press START, RESET or HOME to exit\n");

	while(selectedoutput == -1) {
		PAD_ScanPads();
		WPAD_ScanPads();
		u16 pressedGC = PAD_ButtonsDown(0);
		if (pressedGC & PAD_BUTTON_A) selectedoutput = 2;
		if (geckoB && pressedGC & PAD_BUTTON_B) selectedoutput = 1;
		if (geckoA && pressedGC & PAD_BUTTON_X) selectedoutput = 0;
		if (pressedGC & PAD_BUTTON_START) exit(0);
		u32 pressedRVL = WPAD_ButtonsDown(0);
		if (pressedRVL & WPAD_BUTTON_HOME) exit(0);
		if (pressedRVL & WPAD_CLASSIC_BUTTON_HOME) exit(0);
		if (SYS_ResetButtonDown()) exit(0);
	}

	if (selectedoutput == 2) {
		printf("Connecting to the network...\n");
		s32 ret;
		char localip[16] = {0};
		char gateway[16] = {0};
		char netmask[16] = {0};
		ret = if_config(localip, netmask, gateway, TRUE, 20);
		if (ret < 0) {
			printf("Failed to connect to the network. Press any button to exit.\n");
			while (1) {
				PAD_ScanPads();
				if (PAD_ButtonsDown(0) > 0) exit(0);
			}
		}
		printf("Initialising socket...\n");
		sock = net_socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		if (sock == INVALID_SOCKET) {
			printf("Failed to create a socket. Press any button to exit.\n");
			while (1) {
				PAD_ScanPads();
				if (PAD_ButtonsDown(0) > 0) exit(0);
			}
		}
		struct sockaddr_in bind;
		memset(&bind, 0, sizeof(bind));
		bind.sin_family = AF_INET;
		bind.sin_port = htons(netport);
		bind.sin_addr.s_addr = INADDR_ANY;
		ret = net_bind(sock, (struct sockaddr *)&bind, sizeof(bind));
		if (ret) {
			printf("Failed to bind socket. Press any button to exit.\n");
			while (1) {
				PAD_ScanPads();
				if (PAD_ButtonsDown(0) > 0) exit(0);
			}
		}
		printf("Listening on %s:%i...\n", localip, netport);
	} else {
		printf("Listening on USB Gecko Slot %c...\n", 65 + selectedoutput);
	}
	printf("Use the GCPadder application on your PC to connect.");
	while (storedmagic != 0x09ad09ad) {
		recv_buffer(&storedmagic, 4);
	}
	if (selectedoutput == 2) {
		uint32_t ip = connected.sin_addr.s_addr;
    	unsigned char bytes[4];
    	bytes[0] = ip & 0xFF;
    	bytes[1] = (ip >> 8) & 0xFF;
    	bytes[2] = (ip >> 16) & 0xFF;
    	bytes[3] = (ip >> 24) & 0xFF; 
		printf("\rConnected to %u.%u.%u.%u:%i!                            \n", bytes[3], bytes[2], bytes[1], bytes[0], connected.sin_port);
	} else {
		printf("\rConnected!                                         \n");
	}
	printf("Press RESET or HOME to exit.\n");
	uint8_t loopis = 0;
	while(true) {
		if (SYS_ResetButtonDown()) exit(0);
		WPAD_ScanPads();
		u32 pressedRVL = WPAD_ButtonsDown(0);
		if (pressedRVL & WPAD_BUTTON_HOME) exit(0);
		if (pressedRVL & WPAD_CLASSIC_BUTTON_HOME) exit(0);
		PAD_ScanPads();
		paddata.buttons = PAD_ButtonsHeld(0);
		paddata.stickX = PAD_StickX(0);
		paddata.stickY = PAD_StickY(0);
		paddata.substickX = PAD_SubStickX(0);
		paddata.substickY = PAD_SubStickY(0);
		paddata.triggerL = PAD_TriggerL(0);
		paddata.triggerR = PAD_TriggerR(0);
		if (selectedoutput < 2 || loopis == 255 || memcmp(&paddata, &old, 8) != 0) send_buffer(&paddata, sizeof(paddata));
		old = paddata;
		loopis++;
		usleep(5000);
	};

	return 0;
}
