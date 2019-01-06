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


int main(int argc, const char** argv)
{
    int res = 0;
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
    return res;
}