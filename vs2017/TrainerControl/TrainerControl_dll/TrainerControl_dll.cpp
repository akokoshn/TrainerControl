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
#include "stdafx.h"

#include "TrainerControl.h"

#include "AntStick.h"
#include "TelemetryServer.h"
#include "Tools.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>

int InitAntService(void ** ant_instanance)
{
    LOG_MSG("InitAntService()+\n");
    if (ant_instanance == nullptr)
        return -1;
    try {
        int r = libusb_init(NULL);
        if (r < 0)
            throw LibusbError("libusb_init", r);
        *ant_instanance = new AntStick();
        LOG_MSG(" USB Stick: Serial#: "); LOG_D(((AntStick*)*ant_instanance)->GetSerialNumber());
        LOG_MSG(", version "); LOG_MSG(((AntStick*)*ant_instanance)->GetVersion().c_str());
        LOG_MSG(", max "); LOG_D(((AntStick*)*ant_instanance)->GetMaxNetworks());
        LOG_MSG(" networks, max "); LOG_D(((AntStick*)*ant_instanance)->GetMaxChannels());
        LOG_MSG(" channels\n");
        ((AntStick*)*ant_instanance)->SetNetworkKey(AntStick::g_AntPlusNetworkKey);
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        return -1;
    }
    LOG_MSG("InitAntService() - OK\n");
    return 0;
}
int CloseAntService()
{
    LOG_MSG("CloseAntService()+\n");
    LOG_MSG("CloseAntService() - OK\n");
    return 0;
}
AntSession InitSession(void * ant_instanance)
{
    LOG_MSG("InitSession()+\n");
    AntSession session = {};
    if (ant_instanance == nullptr)
        return session;
    session.m_AntStick = ant_instanance;
    try {
        session.m_TelemtryServer = new TelemetryServer((AntStick*)ant_instanance);
        // run separate thread for wait connection
        std::thread wait_thread = std::thread([session]() { ((TelemetryServer*)session.m_TelemtryServer)->Tick(); });
        if (((TelemetryServer*)session.m_TelemtryServer)->WaitConnection() == false)
        {
            LOG_MSG("\nCONNECTION FAIL\n");
#if !defined(FAKE_CALL)
            if (wait_thread.joinable())
                wait_thread.join();
            return session;
#endif
        }
        if (wait_thread.joinable())
            wait_thread.join();
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        session.m_TelemtryServer = nullptr;
        return session;
    }
    LOG_MSG("InitSession() - OK\n");
    return session;
}
int CloseSession(AntSession & session)
{
    LOG_MSG("CloseSession()+\n");
    delete session.m_AntStick;
    session.m_AntStick = nullptr;
    delete session.m_TelemtryServer;
    session.m_TelemtryServer = nullptr;
    session.m_bIsRun = false;
    LOG_MSG("CloseSession() - OK\n");
    return 0;
}
int Run(AntSession & session, std::thread & thread)
{
    LOG_MSG("Run()+\n");
    if (session.m_bIsRun)
        return -1;
    if (session.m_AntStick == nullptr ||
        session.m_TelemtryServer == nullptr)
        return -1;
    try {
        thread = std::thread([session]() { ((TelemetryServer*)session.m_TelemtryServer)->Tick(); });
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        return -1;
    }
    session.m_bIsRun = true;
    LOG_MSG("Run() - OK\n");
    return 0;
}
int Stop(AntSession & session, std::thread & thread)
{
    LOG_MSG("Stop()+\n");
    if (!session.m_bIsRun)
        return -1;
    if (session.m_AntStick == nullptr ||
        session.m_TelemtryServer == nullptr)
        return -1;
    if (!thread.joinable())
        return -1;
    thread.join();
    session.m_bIsRun = false;
    LOG_MSG("Stop() - OK\n");
    return 0;
}
Telemetry GetTelemetry(AntSession & session)
{
    LOG_MSG("GetTelemetry()+\n");
    Telemetry t = {};
    if (!session.m_bIsRun)
        return t;
    if (session.m_AntStick == nullptr ||
        session.m_TelemtryServer == nullptr)
        return t;
    LOG_MSG("GetTelemetry() - OK\n");
    return ((TelemetryServer*)session.m_TelemtryServer)->GetTelemetry();
}
