#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <stdarg.h>

void DoEvents()
{
	/**/
	MSG msg;
	BOOL result;

	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		result = ::GetMessage(&msg, NULL, 0, 0);
		if (result == 0) // WM_QUIT
		{
			::PostQuitMessage(msg.wParam);
			break;
		}
		else if (result == -1)
		{
			// Handle errors/exit application, etc.
		}
		else
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}


extern "C"
{
	void basic();
	void basic_zero();
	void initfirmware();

	int stop_flg = 0;

	char inputbuffer[4096];
	char outputbuffer[8096];

	extern unsigned char cpos[32];
	extern int sim_x, sim_y, sim_z, sim_psd, gDistance, x_value, y_value, z_value;
	unsigned char BASIC_PROG_SPACE[];
	unsigned short eeprom_read_byte(unsigned short *p);

	extern WORD psize;

	int wsibp = 0;

	int wsgetch()
	{
		int n = -1;

		if (stop_flg) exit(1);

		if (wsibp == 0 && inputbuffer[0] == 0)
		{
			DoEvents();
			return n;
		}
		if (wsibp < 4095)
		{
			n = inputbuffer[wsibp++];
			if (n == 0) {
				wsibp = 0;
				inputbuffer[wsibp] = 0;
				return -1;
			}
		}
		return n;
	}

	void wsputch(char c)
	{
		int obl = strlen(outputbuffer);
		putchar(c);

		if (obl >= 8095) return ;
		outputbuffer[obl] = c;
		outputbuffer[obl+1] = 0;

		if (stop_flg) exit(1);
		DoEvents();
	}


	__declspec(dllexport)void  basic_setibuf(const char *s)
	{
		int i;
		for (i = 0; i < strlen(s); i++) { inputbuffer[wsibp+i] = s[i]; }
		inputbuffer[wsibp+i] = 0;
		return;
	}

	__declspec(dllexport) void basic_getobuf(char* str, int n)
	{
		int obl = strnlen_s(outputbuffer,4096);
		strcpy_s(str, n, &outputbuffer[0]);
		outputbuffer[0] = 0;
		DoEvents();
	}

	__declspec(dllexport)void  basic_api()
	{
		printf_s("Running Windows API ...\n");
		outputbuffer[0] = 0;
		stop_flg = 0;
		wsibp = 0;
		initfirmware();
		basic_zero();
		basic();
	}

	__declspec(dllexport)int read_mem(int n)
	{
		//special cases
		if (n == -1) return psize;
		return BASIC_PROG_SPACE[n];
	}

	__declspec(dllexport)int set_mem(int n, int v)
	{
		return BASIC_PROG_SPACE[n]=(unsigned char)(v&0xFF);
	}

	__declspec(dllexport)int read_servo(int n)
	{
		return (int)cpos[n];
	}

	__declspec(dllexport)int set_servo(int n, int v)
	{
		if (n>=0 && n<30)
			return (int)(cpos[n]=(unsigned char)v);

		printf_s("set servo %d %d\n", n, v);

		if (n == 32)	
			x_value = (sim_x = v);
		else if (n == 33)	
			y_value = (sim_y = v);
		else if (n == 34)	
			z_value = (sim_z = v);
		else if (n == 35)	
			gDistance = (sim_psd = v);
		return 0;
	}

	__declspec(dllexport)void basic_stop()
	{
		stop_flg = 1;
	}


}