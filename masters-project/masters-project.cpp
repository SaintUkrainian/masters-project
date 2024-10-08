// masters-project.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "masters-project.h"
#include <stdio.h>
#include <memory.h>
#include "kupyna.h"
#include "Resource.h"
#include <CommCtrl.h>  // Необхідно для використання Common Controls, як ComboBox і Button
#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>  
#include <sstream>  // Для роботи з потоками строк
#include <random>
#include <algorithm>
#include <fstream>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hText;  // Глобальна змінна для текстового поля

void append_text_to_edit_control(HWND hWnd, const wchar_t* new_text);

void calculateHashCode();

// Глобальний м'ютекс для синхронізації доступу до файлу
std::mutex fileMutex;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MASTERSPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MASTERSPROJECT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MASTERSPROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MASTERSPROJECT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;  // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 200, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // Створення ComboBox

    HWND hReadAndHashButton = CreateWindowW(L"BUTTON", L"Read and Generate QRNG Hash",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        260, 10, 250, 30, hWnd, (HMENU)(LONG_PTR)IDC_READ_AND_HASH_BUTTON, hInstance, NULL);


    // Додавання опцій до ComboBox
    //const wchar_t* sizes[] = { L"512", L"1024", L"2048", L"8", L"760", L"0", L"510", L"655" };
//for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
     //   SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)sizes[i]);
    //}
    //SendMessage(hComboBox, CB_SETCURSEL, 0, 0);  // Встановлення вибраного елементу на перший

    // Додавання ComboBox для вибору режиму Купини
    HWND hComboBoxMode = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        10, 10, 130, 300, hWnd, (HMENU)IDC_COMBO_MODE, hInstance, NULL);
    SendMessage(hComboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Kupyna-256");
    SendMessage(hComboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Kupyna-384");
    SendMessage(hComboBoxMode, CB_ADDSTRING, 0, (LPARAM)L"Kupyna-512");
    SendMessage(hComboBoxMode, CB_SETCURSEL, 0, 0);  // Set default selection

    // Додавання ComboBox для вибору розміру блоку
    HWND hComboBoxBlockSize = CreateWindowW(L"COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        150, 10, 100, 300, hWnd, (HMENU)IDC_COMBO_BLOCK_SIZE, hInstance, NULL);
    SendMessage(hComboBoxBlockSize, CB_ADDSTRING, 0, (LPARAM)L"256");
    SendMessage(hComboBoxBlockSize, CB_ADDSTRING, 0, (LPARAM)L"512");
    SendMessage(hComboBoxBlockSize, CB_ADDSTRING, 0, (LPARAM)L"1024");
    SendMessage(hComboBoxBlockSize, CB_SETCURSEL, 0, 0);  // Set default selection


    // Створення кнопки
   // HWND hButton = CreateWindowW(L"BUTTON", L"Generate Dummy Hash",
     //   WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
     //   170, 10, 250, 30, hWnd, (HMENU)IDC_MYBUTTON, hInstance, NULL);

    // Створення текстового поля для виведення результатів
   // hText = CreateWindowW(L"EDIT", NULL,
    //    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
    //    10, 50, 780, 580, hWnd, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void GenerateAndDisplayHash(int size, HWND hWnd) {
    kupyna_t ctx;
    uint8_t hash_code[512 / 8];  // Максимальний розмір для Kupyna-512
    wchar_t message[4096];     // Буфер для повідомлень

    // Ініціалізація Kupyna контексту
    KupynaInit(256, &ctx);  // Припустимо, ви використовуєте 256-бітний хеш

    // Генерація випадкових даних (або тестових даних) для хешування
    uint8_t* test_data = new uint8_t[size];
    srand((unsigned)time(NULL));  // Ініціалізація генератора випадкових чисел
    for (int i = 0; i < size; i++) {
        test_data[i] = rand() % 256;  // Випадкові дані
    }

    // Очистка текстового поля перед виведенням нового хешу
    SetWindowText(hText, L"");  // Очистити вміст

    // Виконання хешування
    KupynaHash(&ctx, test_data, size, hash_code);

    // Форматування результату для виведення
    swprintf(message, 4096, L"Hash for %d bytes of data:\n", size);
    for (int i = 0; i < sizeof(hash_code); i++) {
        wchar_t temp[10];
        swprintf(temp, 10, L"%02X ", hash_code[i]);
        wcscat_s(message, 4096, temp);
        if ((i + 1) % 16 == 0) wcscat_s(message, 4096, L"\n");
    }

    // Відображення результату у текстовому полі
    SetWindowText(hText, message);

    // Очищення пам'яті
    delete[] test_data;
}

bool ReadFileData(const std::string& filename, std::vector<uint8_t>& data) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    // Читання даних з файлу
    data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return true;
}

std::vector<uint8_t> HashKupyna(std::vector<uint8_t>& block, int hashMode) {
    kupyna_t ctx;
    KupynaInit(hashMode, &ctx);
    std::vector<uint8_t> hash_code(hashMode / 8);
    KupynaHash(&ctx, block.data(), block.size(), hash_code.data());
    return hash_code;
}

void processBlock(std::vector<uint8_t> block, int hashMode, std::ofstream& output) {
    std::vector<uint8_t> hash = HashKupyna(block, hashMode);

    // Забезпечуємо синхронізований доступ до файлу для запису
    std::lock_guard<std::mutex> lock(fileMutex);
    for (uint8_t byte : hash) {
        output << std::hex << std::setfill('0') << std::setw(2) << (int)byte;
    }
    output << std::endl;
}

void OnGenerateHash(HWND hWnd) {
    MessageBox(hWnd, L"Хешування розпочато. Будь ласка, зачекайте...", L"Інформація", MB_ICONINFORMATION | MB_OK);
    int modeIndex = (int)SendMessage(GetDlgItem(hWnd, IDC_COMBO_MODE), CB_GETCURSEL, 0, 0);
    int blockSizeIndex = (int)SendMessage(GetDlgItem(hWnd, IDC_COMBO_BLOCK_SIZE), CB_GETCURSEL, 0, 0);
    int hashMode = (modeIndex == 0 ? 256 : (modeIndex == 1 ? 384 : 512));
    int blockSize = (blockSizeIndex == 0 ? 256 : (blockSizeIndex == 1 ? 512 : 1024));

    std::vector<uint8_t> data;
    ReadFileData("C:/Users/idanc/Downloads/QRNG.txt", data);

    std::ofstream output("C:/Users/idanc/Downloads/Output.txt");
    std::vector<std::thread> threads;

    for (size_t i = 0; i < data.size(); i += blockSize) {
        size_t currentBlockSize = min(blockSize, data.size() - i);
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + currentBlockSize);

        // Створення нового потоку для кожного блоку даних
        threads.emplace_back(processBlock, block, hashMode, std::ref(output));
    }

    // Очікуємо завершення всіх потоків
    for (auto& thread : threads) {
        thread.join();
    }

    output.close();
    MessageBox(hWnd, L"Хешування завершено. Результати збережено у файлі 'Output.txt'.", L"Інформація", MB_ICONINFORMATION | MB_OK);
}

void OnGenerateHashOld(HWND hWnd) {
    int modeIndex = (int)SendMessage(GetDlgItem(hWnd, IDC_COMBO_MODE), CB_GETCURSEL, 0, 0);
    int blockSizeIndex = (int)SendMessage(GetDlgItem(hWnd, IDC_COMBO_BLOCK_SIZE), CB_GETCURSEL, 0, 0);
    int hashMode = (modeIndex == 0 ? 256 : (modeIndex == 1 ? 384 : 512));
    int blockSize = (blockSizeIndex == 0 ? 256 : (blockSizeIndex == 1 ? 512 : 1024));

    std::vector<uint8_t> data;
    ReadFileData("C:/Users/idanc/Downloads/QRNG.txt", data);

    std::ofstream output("C:/Users/idanc/Downloads/Output.txt");
    for (size_t i = 0; i < data.size(); i += blockSize) {
        size_t currentBlockSize = min(blockSize, data.size() - i);
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + currentBlockSize);
        std::vector<uint8_t> hash = HashKupyna(block, hashMode);

        for (uint8_t byte : hash) {
            output << std::hex << std::setfill('0') << std::setw(2) << (int)byte;
        }
        output << std::endl;
    }
    output.close();
}

void HashRandomBlock(const std::vector<uint8_t>& data, int blockSize, HWND hEdit) {
    if (data.size() < blockSize) {
        std::cerr << "Data size is too small for the given block size." << std::endl;
        return;
    }

    int numBlocks = data.size() / blockSize;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, numBlocks - 1);

    int blockIndex = dis(gen);
    std::vector<uint8_t> block(data.begin() + blockIndex * blockSize, data.begin() + (blockIndex + 1) * blockSize);

    kupyna_t ctx;
    KupynaInit(256, &ctx);
    uint8_t hash_code[256 / 8];
    KupynaHash(&ctx, block.data(), blockSize, hash_code);

    std::wstringstream ss;
    ss << L"Hash for block " << blockIndex << L" of data:\n";
    for (int i = 0; i < sizeof(hash_code); i++) {
        ss << std::hex << std::setfill(L'0') << std::setw(2) << static_cast<int>(hash_code[i]);
    }

    // Вивід у текстове поле
    SetWindowText(hEdit, ss.str().c_str());
}



void OnButtonPress(HWND hEdit) {
    std::vector<uint8_t> data;
    std::string filename = "C:/Users/idanc/Downloads/QRNG.txt";
    if (ReadFileData(filename, data)) {
        HashRandomBlock(data, 512, hEdit);  // Припустимо, блоки по 512 байт
    }
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDC_READ_AND_HASH_BUTTON:
            OnGenerateHash(hWnd);
            break;
        case IDM_ABOUT:
            MessageBox(hWnd, L"Розробив студент групи КБ-61, Маценко Денис Віталійович", L"Про", MB_OK);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
                   break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // Тут можна додати малювання
        EndPaint(hWnd, &ps);
    }
                 break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


#define CHECK(expected, hash_nbytes) \
    if (memcmp(hash_code, (expected), (hash_nbytes)) == 0) { \
        printf("Success hashing"); \
    } else { \
        printf("Failed hashing!"); \
    }printf ("\n\n");\

void print(int data_len, uint8_t data[]);


void calculateHashCode() {
    kupyna_t ctx;
    uint8_t hash_code[512 / 8]; // Буфер для хешу
    wchar_t message[4096];    // Більший буфер для зберігання форматованого тексту

    // Очищення текстового поля перед виведенням нових результатів
    SetWindowTextW(hText, L"");  // Очистити вміст

    // Встановлення шрифту фіксованої ширини
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FIXED_PITCH | FF_MODERN, L"Consolas");
    SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Початкове повідомлення
    wcscpy_s(message, 4096, L"Starting Kupyna-256 tests:\n-----------------------------\n");
    append_text_to_edit_control(hText, message);

    const int test_sizes[] = { 512, 1024, 2048, 8, 760, 0, 510, 655 };
    for (int test = 0; test < sizeof(test_sizes) / sizeof(test_sizes[0]); ++test) {
        int size = test_sizes[test];
        KupynaInit(256, &ctx); // Ініціалізація контексту хешування

        // Формування повідомлення про тест
        swprintf(message, 4096, L"\nTest Size: %d bits\nData:\n", size);
        append_text_to_edit_control(hText, message);

        // Генерація і виведення хешу
        KupynaHash(&ctx, hash_code, sizeof(hash_code), hash_code);
        swprintf(message, 4096, L"Hash Output:\n");
        for (int i = 0; i < sizeof(hash_code); ++i) {
            wchar_t temp[10];
            swprintf(temp, 10, L"%02X ", hash_code[i]);
            wcscat_s(message, 4096, temp);
            if ((i + 1) % 16 == 0) wcscat_s(message, 4096, L"\n");
        }
        wcscat_s(message, 4096, L"\n-----------------------------\n");
        append_text_to_edit_control(hText, message);
    }

    swprintf(message, 4096, L"All tests completed.\n");
    append_text_to_edit_control(hText, message);

    // Очищення ресурсів шрифту
    DeleteObject(hFont);
}


void append_text_to_edit_control(HWND hWnd, const wchar_t* new_text)
{
    // Отримання поточного тексту
    int len = GetWindowTextLengthW(hWnd);
    wchar_t* buf = (wchar_t*)GlobalAlloc(GPTR, (len + wcslen(new_text) + 1) * sizeof(wchar_t));
    GetWindowTextW(hWnd, buf, len + 1);
    wcscat_s(buf, len + wcslen(new_text) + 1, new_text);
    SetWindowTextW(hWnd, buf);
    GlobalFree(buf);
}



void print(int data_len, uint8_t data[])
{
	wchar_t buffer[1024] = { 0 };
	int i = 0;
	int data_size = data_len / 8;  // Assuming BITS_IN_BYTE defined as 8
	for (i = 0; i < data_size; i++)
	{
		if (!(i % 16)) wcscat_s(buffer, 1024, L"    ");
		wchar_t temp[10];
		swprintf(temp, 10, L"%02X ", (unsigned int)data[i]);
		wcscat_s(buffer, 1024, temp);
		if (!((i + 1) % 16)) wcscat_s(buffer, 1024, L"\n");
	}
	wcscat_s(buffer, 1024, L"\n");

	// Додаємо результат у текстове поле
	append_text_to_edit_control(hText, buffer);
}


