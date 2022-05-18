#define WIN32
#ifdef WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"
#include <stdint.h>
#define MAX_STR 255
#include <locale.h>
#include <string.h>
#include <wchar.h>
union union_type
    {
    unsigned char elem16[2];
    uint16_t elem10;
    };
void MakeMaximumBrightness(hid_device *handle, unsigned char *buf);
void MakeMinimumBrightness(hid_device *handle, unsigned char *buf);
void PaintOverTheScreen(hid_device *handle, unsigned char *buf);
void MeasureVoltageAndChangeRGB(hid_device *handle, unsigned char *buf, union_type * diod_color, int res);

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

	int res;
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

    union_type diod_color;

	struct hid_device_info *devs, *cur_dev;

	printf("hidapi test/example tool. Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
	if (hid_version()->major == HID_API_VERSION_MAJOR && hid_version()->minor == HID_API_VERSION_MINOR && hid_version()->patch == HID_API_VERSION_PATCH) {
		printf("Compile-time version matches runtime version of hidapi.\n\n");
	}
	else {
		printf("Compile-time version is different than runtime version of hidapi.\n]n");
	}

	if (hid_init())
		return -1;

    // находим все устройства USB HID, печатаем содержимое дескриптора устройства, доступное через драйвер
    devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("  Usage (page): 0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x1234, 0x0001, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}


	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);




    // LEDs lights
        buf[0] = 0x02; // descriptor number
        buf[1] = 0xff; //
        buf[2] = 0xff; // 2 byte = uint16_t = power of light color 1
        buf[3] = 0x00; //
        buf[4] = 0x00; // 2 byte = uint16_t = power of light color 2
        buf[5] = 0xff; //
        buf[6] = 0xff; // 2 byte = uint16_t = power of light color 3


        res = hid_send_feature_report(handle,buf,7); // send report, 7 byte

        if(res == -1) {
            printf("hid_write error.\n");
        }



     // keys
     // Read a Feature Report from the device
	buf[0] = 0x1;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}



    while(1)
{
	buf[1] = 0x00;
    buf[0] = 0x01;
	hid_get_feature_report(handle,buf,2);
	switch(buf[1])
		{
			case 0x01:
			{
				MakeMaximumBrightness(handle, buf);
			}
			break;
			case 0x02:
			{
				MakeMinimumBrightness(handle, buf);
			}
			break;
			case 0x04://закрашиваем экран
			{
				PaintOverTheScreen(handle, buf);
			}
			break;
			case 0x08://измеряем напряжение и меняем цвет диода
			{
				MeasureVoltageAndChangeRGB(handle, buf, &diod_color, res);
			}
			break;
		}
}
    return 0;
}
void MakeMaximumBrightness(hid_device *handle, unsigned char *buf)
{
	buf[0] = 0x02;
	for (int i=1; i<7;i++)
	buf[i] = 0xff;
	hid_send_feature_report(handle,buf,7);
}
void MakeMinimumBrightness(hid_device *handle, unsigned char *buf)
{
	buf[0] = 0x02;
	for (int i=1; i<7;i++)
	buf[i] = 0x00;
	hid_send_feature_report(handle,buf,7);
}
void PaintOverTheScreen(hid_device *handle, unsigned char *buf)
	{
		static int color = 1;
		buf[0] = 0x04;
				for (int i =0;i<64;i++)
					for (int j=0;j<128;j++)
					{
                        buf[1] = j;
                        buf[2] = i;
                        buf[3] = color;
                        hid_send_feature_report(handle,buf,4);
					}
                if (color) color = 0;
                    else color = 1;
	}

void MeasureVoltageAndChangeRGB(hid_device *handle, unsigned char *buf, union_type * diod_color, int res)
{
	buf[0] = 0x03;
	hid_get_feature_report(handle,buf,7); //измеряем напряжение
	memcpy(diod_color,&buf[1],2); // копируем данные
	res = diod_color->elem10; //приводим к инту для вывода в консоль
	Sleep(200);
	printf("%d \n",res);
	for (int i=0; i++;i<3)
	{
		buf[1+i*2] = diod_color->elem16[0];
		buf[2+i*2] = diod_color->elem16[1];
	}
	buf[0] = 0x02;
	hid_send_feature_report(handle,buf,7); // меняем RGB
}
