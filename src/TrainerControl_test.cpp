/**
 *  TrainServer -- prototype bike trainer application
 *  Copyright (C) 2018 Alexey Kokoshnikov (alexeikokoshnikov@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <chrono>
#include <string>
#include "test_suites.h"

#define CHECK_RES(res) if (res != 0) return res

bool STOP_ALL;

int run_unit_tests()
{
    int res = 0;
#if defined(ENABLE_UNIT_TESTS)
    ServiceInit test_init;
    if (false == test_init.run_case())
    {
        printf("test_init FAILED\n");
        res = -1;
    }
    ServiceClose test_close;
    if (false == test_close.run_case())
    {
        printf("test_close FAILED\n");
        res = -1;
    }
    SearchRun test_run_search;
    if (false == test_run_search.run_case())
    {
        printf("test_run_search FAILED\n");
        res = -1;
    }
    /*SessionInit test_session_init;
    if (false == test_session_init.run_case())
    {
        printf("test_session_init FAILED\n");
        res = -1;
    }
    SessionClose test_session_close;
    if (false == test_session_close.run_case())
    {
        printf("test_session_close FAILED\n");
        res = -1;
    }
    SessionRun test_session_run;
    if (false == test_session_run.run_case())
    {
        printf("test_session_run FAILED\n");
        res = -1;
    }
    SessionStop test_session_stop;
    if (false == test_session_stop.run_case())
    {
        printf("test_session_stop FAILED\n");
        res = -1;
    }
    ServiceGetTelemetry test_get_telemetry;
    if (false == test_get_telemetry.run_case())
    {
        printf("test_get_telemetry FAILED\n");
        res = -1;
    }*/
#endif// ENABLE_UNIT_TESTS
    return res;
}

#if defined _WIN32 || _WIN64
#include "windows.h"

void _CreateWindow(WNDCLASS &wc, HWND &hWind, LPCSTR name)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    wc.lpfnWndProc = DefWindowProc;// WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = (LPCSTR)CLASS_NAME;
    RegisterClass(&wc);

    hWind = CreateWindowEx(
        0,                              // Optional window styles.
        (LPCSTR)CLASS_NAME,                     // Window class
        (LPCSTR)L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        10, 10, 310, 160,

        NULL,       // Parent window    
        NULL,       // Menu
        GetModuleHandle(NULL),  // Instance handle
        NULL        // Additional application data
    );
    SetWindowTextA(hWind, name);
    ShowWindow(hWind, SW_SHOWDEFAULT);
    UpdateWindow(hWind);
}
#endif
int run_service()
{
    STOP_ALL = false;
    void * ant_handle;
    int max_channels = 0;
    
    // reinit AntStick
    CHECK_RES(InitAntService(&ant_handle, max_channels));
    CHECK_RES(CloseAntService());

    CHECK_RES(InitAntService(&ant_handle, max_channels));

    std::thread search_thread;
    std::mutex guard;
    std::mutex local_guard;
    void * search_service;
    CHECK_RES(RunSearch(ant_handle, &search_service, search_thread, guard));

    CHECK_RES(AddDeviceForSearch(search_service, HRM_Type));

    AntDevice ** device_list = new AntDevice*[max_channels];
    for (int i = 0; i < max_channels; i++)
        device_list[i] = new AntDevice();
    
    std::vector<bool> is_assigned;
    is_assigned.resize(max_channels);
    std::fill(is_assigned.begin(), is_assigned.end(), false);

    std::vector< FILE *> files;
    files.resize(max_channels);
    std::fill(files.begin(), files.end(), nullptr);

    std::vector<std::thread> client_threads;
    client_threads.resize(max_channels);

    uint32_t num_hrm = 0;
    while (true)
    {
        unsigned int num_devices = 0;
        unsigned int num_active_devices = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        GetDeviceList(search_service, device_list, num_devices, num_active_devices);
        printf("Found HRMs:\n");
        for (int i = 0; i < num_active_devices; i++)
            if (device_list[i]->m_type == HRM_Type)
            {
                num_hrm++;
                printf("    (%d) %d\, is assigned %s\n", i, device_list[i]->m_device_number, is_assigned[i] ? "true" : "false");
            }
        if (num_active_devices == 0)
            continue;
        uint32_t device_for_assign = 0;
        scanf_s("%d", &device_for_assign, sizeof(device_for_assign));
        if (device_for_assign < num_hrm && !is_assigned[device_for_assign])
        {
            is_assigned[device_for_assign] = true;
            std::string file_name = std::to_string(device_list[device_for_assign]->m_device_number);
            fopen_s(&files[device_for_assign], file_name.c_str(), "w");
            auto funk = [ant_handle, device_list, files, device_for_assign, file_name](std::mutex & guard, std::mutex & local_guard)
            {
#if defined _WIN32 || _WIN64
                HWND hWind = nullptr;
                WNDCLASS wc = {};
                _CreateWindow(wc, hWind, (LPCSTR)file_name.c_str());
#endif
                AntSession ant_session = InitSession(ant_handle, &device_list[device_for_assign], 1, guard);
                std::thread server_thread;
                CHECK_RES(Run(ant_session, server_thread));
                while (!STOP_ALL)
                {
                    std::unique_lock<std::mutex> Guard(local_guard);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    Telemetry t = GetTelemetry(ant_session);
                    if (files[device_for_assign] != nullptr && t.hr > 0)
                        fprintf(files[device_for_assign], "%lf\n", t.hr);
#if defined _WIN32 || _WIN64
                    if (hWind != nullptr)
                    {
                        UpdateWindow(hWind);
                        RECT rect;
                        GetClientRect(hWind, &rect);

                        std::string hr;
                        hr = std::to_string((unsigned int)t.hr);

                        HDC dc = GetWindowDC(hWind);
                        int res = DrawText(dc, hr.c_str(), hr.length(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        DeleteDC(dc);

                        MSG Msg;
                        if (GetMessage(&Msg, NULL, 0, 0) > 0)
                        {
                            TranslateMessage(&Msg);
                            DispatchMessage(&Msg);
                        }
                    }
#endif
                }
#if defined _WIN32 || _WIN64
                if (hWind != nullptr)
                    DestroyWindow(hWind);
#endif
                CHECK_RES(Stop(ant_session, server_thread));
                CHECK_RES(CloseSession(ant_session));
            };
            client_threads[device_for_assign] = std::thread(funk, std::ref(guard), std::ref(local_guard));
        }
        else
            printf("ERR: can't assign device %d\n", device_for_assign);
        
        char key[2];
        printf("continue? [y/n]\n");
        scanf_s("%1s", key, (unsigned)_countof(key));
        if (0 == strcmp(key, "n"))
            break;
        if (num_devices == num_active_devices)
            CHECK_RES(AddDeviceForSearch(search_service, HRM_Type));
    }

    STOP_ALL = true;
    CHECK_RES(StopSearch(&search_service, search_thread));
    CHECK_RES(CloseAntService());

    for (auto & it : client_threads)
        if (it.joinable())
            it.join();
    local_guard.lock();
    for (auto & it : files)
        if (it != nullptr)
        {
            fclose(it);
            it = nullptr;
        }
    local_guard.unlock();
    return 0;
}


int main(int argc, const char** argv)
{
    if (argc == 2 &&
        0 == strcmp(argv[1], "unit_test"))
    {
        if (0 != run_unit_tests())
            printf("unit tests FAILED\n");
        else
            printf("unit tests PASSED\n");
    }
    else
    {
        printf("use \"TrainerControl unit_test\" for run unit tests\n");
        run_service();
    }
    return 0;
}