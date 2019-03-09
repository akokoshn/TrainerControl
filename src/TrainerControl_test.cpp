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
#include "test_suites.h"

#define CHECK_RES(res) if (res != 0) return res

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
    SessionInit test_session_init;
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
    }
#endif// ENABLE_UNIT_TESTS
    return res;
}

int run_service()
{
    void * ant_handle;
    int max_channels = 0;
    CHECK_RES(InitAntService(&ant_handle, max_channels));

    std::thread search_thread;
    std::mutex guard;
    void * search_service;
    CHECK_RES(RunSearch(ant_handle, &search_service, search_thread, guard));

    void ** hrm_list = new void*[max_channels];
    memset(hrm_list, 0, max_channels);
    void ** bike_list = new void*[max_channels];
    memset(bike_list, 0, max_channels);

    int num_hrm = 0, num_bike = 0;
    while (num_hrm == 0 || num_bike == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        GetHRMList(search_service, hrm_list, num_hrm);
        GetBikeList(search_service, bike_list, num_bike);
    }

    AntSession ant_session = InitSession(ant_handle, hrm_list[0], bike_list[0], guard);
    std::thread server_thread;
    CHECK_RES(Run(ant_session, server_thread));
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        Telemetry t = GetTelemetry(ant_session);
        printf("HR = %lf, CAD = %lf, POWER = %lf, SPEED = %lf\n", t.hr, t.cad, t.pwr, t.spd);
        char key[2];
        printf("continue? [y/n]\n");
        /*scanf_s("%1s", key, (unsigned)_countof(key));
        if (0 == strcmp(key, "n"))
            break;*/
    }
    CHECK_RES(Stop(ant_session, server_thread));
    CHECK_RES(CloseSession(ant_session));
    CHECK_RES(StopSearch(&search_service, search_thread));
    CHECK_RES(CloseAntService());
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