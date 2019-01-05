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
#include "TrainerControl.h"

#define CHECK_RES(res) if (res != 0) return res

int main(int argc, const char** argv)
{
    void * ant_handle = nullptr;
    int res = InitAntService(&ant_handle);
    printf("InitAntService(0x%x) return %d\n", ant_handle, res);
    CHECK_RES(res);
    AntSession ant_session = InitSession(ant_handle);
    printf("InitSession(0x%x):\n session.m_AntStick = 0x%x\n session.m_TelemtryServer = 0x%x\n session.m_bIsRun = %d\n",
        ant_handle, ant_session.m_AntStick,
        ant_session.m_TelemtryServer,
        ant_session.m_bIsRun);
    std::thread server_thread;
    res = Run(ant_session, server_thread);
    printf("Run(session = 0x%x, thread = 0x%x) return %d\n", ant_handle, &server_thread, res);
    CHECK_RES(res);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));// wait 5 sec
    Telemetry t = GetTelemetry(ant_session);
    printf("GetTelemetry(session = 0x%x): HR = %lf, CAD = %lf, PWR = %lf, SPEED = %lf\n", ant_handle, t.hr, t.cad, t.pwr, t.spd);
    res = Stop(ant_session, server_thread);
    printf("Stop(session = 0x%x, thread = 0x%x) return %d\n", ant_handle, &server_thread, res);
    CHECK_RES(res);
    res = CloseSession(ant_session);
    printf("CloseSession(session = 0x%x) return %d\n", ant_handle, res);
    CHECK_RES(res);
    res = CloseAntService();
    printf("CloseAntService() return %d\n", res);
    CHECK_RES(res);
    return 0;
}