#include <wimp_log.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wimp_server.h>

int get_char_width_dec(int var)
{
	int no_sign_var = var;
	if (var < 0)
	{
		no_sign_var *= -1;
	}

	//Do stupid boolean hack
	return
		1 +					//Minumum char
		(var < 0) +			//Sign
		(no_sign_var >= 10) +
		(no_sign_var >= 100) +
		(no_sign_var >= 1000) +
		(no_sign_var >= 10000) +
		(no_sign_var >= 100000) +
		(no_sign_var >= 1000000) +
		(no_sign_var >= 10000000) +
		(no_sign_var >= 100000000) +
		(no_sign_var >= 1000000000)
		;
}

int get_char_width_oct(int var)
{
	int no_sign_var = var;
	if (var < 0)
	{
		return 11; //If negative gets formatted with leading figures
	}

	//Do stupid boolean hack
	return
		1 +					//Minumum char
		(var >= 8) +
		(var >= 8*8) +
		(var >= 8*8*8) +
		(var >= 8*8*8*8) +
		(var >= 8*8*8*8*8) +
		(var >= 8*8*8*8*8*8) +
		(var >= 8*8*8*8*8*8*8) +
		(var >= 8*8*8*8*8*8*8*8) +
		(var >= 8*8*8*8*8*8*8*8*8) +
		(var >= 8*8*8*8*8*8*8*8*8*8)
		;
}

int get_char_width_hex(int var)
{
	int no_sign_var = var;
	if (var < 0)
	{
		return 8; //If negative gets formatted with leading figures
	}

	//Do stupid boolean hack
	return
		1 +					//Minumum char
		(no_sign_var >= 16) +
		(no_sign_var >= 16*16) +
		(no_sign_var >= 16*16*16) +
		(no_sign_var >= 16*16*16*16) +
		(no_sign_var >= 16*16*16*16*16) +
		(no_sign_var >= 16*16*16*16*16*16) +
		(no_sign_var >= 16*16*16*16*16*16*16)
		;
}

size_t format_string(char* str, char* format, va_list arg)
{
	size_t string_offset = 0;
	while (*format != '\0')
	{
		if (*format == '%')
		{
			format++;
			if (*format == '%')
			{
				str[string_offset] = '%';
				string_offset++;
			}
			else if ((*format == 'd') || (*format == 'i'))
			{
				int int_prt = va_arg(arg, int);
				_itoa(int_prt, &str[string_offset], 10);
				string_offset += get_char_width_dec(int_prt);
			}
			else if (*format == 'o')
			{
				int int_prt = va_arg(arg, int);
				_itoa(int_prt, &str[string_offset], 8);
				string_offset += get_char_width_dec(int_prt);
			}
			else if ((*format == 'x') || (*format == 'X'))
			{
				int int_prt = va_arg(arg, int);
				_itoa(int_prt, &str[string_offset], 16);
				string_offset += get_char_width_hex(int_prt);
			}
			else if (*format == 'u')
			{
				int int_prt = va_arg(arg, int);
				_itoa(int_prt, &str[string_offset], 10);
				string_offset += get_char_width_dec((unsigned int)int_prt);
			}
			else if (*format == 'c')
			{
				unsigned char char_prt = (unsigned char)va_arg(arg, int);
				str[string_offset] = char_prt;
				string_offset += 1;
			}
			else if (*format == 's')
			{
				//Get the string
				const char* str_prt = va_arg(arg, const char*);

				//Get the length (don't include the null)
				size_t str_bytes = strlen(str_prt) * sizeof(char);

				//Copy
				memcpy(&str[string_offset], str_prt, str_bytes);
				string_offset += str_bytes;
			}
			else if (*format == 'f')
			{
				//Check if a precision is specified
				if (*(format + 1) != '.')
				{
					//Default is 6 dp
					double double_prt = va_arg(arg, double);
					sprintf(&str[string_offset], "%f", double_prt);

					//Get the length to the null to work out length
					size_t float_bytes = strlen(&str[string_offset]) * sizeof(char);
					string_offset += float_bytes;
				}
				else
				{
					//7 is the cutoff so just do as a single char
					char fmt[5]
						=
					{
						'%',
						'f',
						'.',
						*(format + 2),
						'\0'
					};

					double double_prt = va_arg(arg, double);
					sprintf(&str[string_offset], fmt, double_prt);

					//Get the length to the null to work out length
					size_t float_bytes = strlen(&str[string_offset]) * sizeof(char);
					string_offset += float_bytes;
				}
			}
			else if (*format == 'p')
			{
				void* ptr = va_arg(arg, void*);
				sprintf(&str[string_offset], "%p", ptr);

				//Get the length to the null to work out length
				size_t ptr_bytes = strlen(&str[string_offset]) * sizeof(char);
				string_offset += ptr_bytes;
			}
		}
		else
		{
			str[string_offset] = (char)*format;
			string_offset++;
		}
		format++;
	}
	return strlen(str);
}

//For now just log if debug, otherwise don't
//Will definately make this less shit and awful later
void wimp_log(const char* format, ...)
{
#ifdef _DEBUG
	//_DEBUG set uses printf behavior, but ensures all is printed from the master 

	va_list arg;
	int done;

	WimpServer* server = wimp_get_local_server();
	if (server == NULL || server->server_type == WIMP_SERVERTYPE_MASTER)
	{
		//Default to printf behavior is is the master, or there is no server locally
		va_start (arg, format);
		done = vfprintf(stdout, format, arg);
		va_end (arg);
		return;
	}

	//If a child, create the log string and send that to the master
	//Basically do the work of the printf modifiers here
	//Have a thread local static char array to push to
	//Should mean there aren't race conditions
	thread_local static char str[MAXIMUM_LOG_BYTES];

	va_start(arg, format);
	size_t size = (format_string(str, format, arg) + 1) * sizeof(char);
	wimp_add_local_server("master", WIMP_INSTRUCTION_LOG, str, size);
	va_end(arg);

	//Zero buffer
	memset(str, 0, MAXIMUM_LOG_BYTES);
#endif
}