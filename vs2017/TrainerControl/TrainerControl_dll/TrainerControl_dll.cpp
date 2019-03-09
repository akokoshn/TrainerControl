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
#include "SearchService.h"
#include "Tools.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>

int InitAntService(void ** ant_instanance, int & max_channels)
{
    LOG_MSG("InitAntService()+\n");
    if (ant_instanance == nullptr)
        return -1;
    try {
        libusb_context *ctx = nullptr;
        int r = libusb_init(&ctx);
        if (r < 0)
            throw LibusbError("libusb_init", r);
        libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK);
        *ant_instanance = new AntStick();
        LOG_MSG(" USB Stick: Serial#: "); LOG_D(((AntStick*)*ant_instanance)->GetSerialNumber());
        LOG_MSG(", version "); LOG_MSG(((AntStick*)*ant_instanance)->GetVersion().c_str());
        LOG_MSG(", max "); LOG_D(((AntStick*)*ant_instanance)->GetMaxNetworks());
        LOG_MSG(" networks, max "); LOG_D(((AntStick*)*ant_instanance)->GetMaxChannels());
        LOG_MSG(" channels\n");
        ((AntStick*)*ant_instanance)->SetNetworkKey(AntStick::g_AntPlusNetworkKey);
        max_channels = ((AntStick*)*ant_instanance)->GetMaxChannels();
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
AntSession InitSession(void * ant_instanance, void * hrm_device, void * bike_device, std::mutex & guard)
{
    LOG_MSG("InitSession()+\n");
    AntSession session = {};
    if (ant_instanance == nullptr)
        return session;
    session.m_AntStick = ant_instanance;
    try {
        session.m_TelemtryServer = new TelemetryServer((AntStick*)ant_instanance, (HeartRateMonitor*)hrm_device, (FitnessEquipmentControl*)bike_device, guard);
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
int RunSearch(void * ant_instanance, void ** pp_search_service, std::thread & thread, std::mutex & guard)
{
    LOG_MSG("RunSearch()+\n");
    if (ant_instanance == nullptr)
        return -1;
    if (pp_search_service == nullptr)
        return -1;
    *pp_search_service = new SearchService((AntStick*)ant_instanance, guard);
    try {
        thread = std::thread([ant_instanance, pp_search_service]()
        {
            while (true)
                ((SearchService*)*pp_search_service)->Tick();
        });
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        delete pp_search_service;
        *pp_search_service = nullptr;
        return -1;
    }
    LOG_MSG("RunSearch() - OK\n");
    return 0;
}
int StopSearch(void ** pp_search_service, std::thread & thread)
{
    LOG_MSG("StopSearch()+\n");
    if (pp_search_service == nullptr)
        return -1;
    if (!thread.joinable())
        return -1;
    thread.join();
    delete *pp_search_service;
    *pp_search_service = nullptr;
    LOG_MSG("StopSearch() - OK\n");
    return 0;
}

// internal function
inline int GetChannelList(void * p_search_service, void ** devices, int & num_hrm, uint8_t type)
{
    if (p_search_service == nullptr)
        return -1;
    if (devices == nullptr)
        return -1;
    try {
        std::vector<AntChannel*> active_devices = ((SearchService*)p_search_service)->GetActiveDevices();
        num_hrm = 0;
        for (auto it : active_devices)
        {
            if (it &&
                it->ChannelId().DeviceType == type &&
                it->ChannelState() == AntChannel::CH_OPEN)
            {
                devices[num_hrm] = (void*)it;
                num_hrm++;
            }
        }
        return 0;
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        return -1;
    }
}

int GetHRMList(void * p_search_service, void ** devices, int & num_hrm)
{
    LOG_MSG("GetHRMList()+\n");
    if (GetChannelList(p_search_service, devices, num_hrm, hrm::ANT_DEVICE_TYPE) != 0)
        return -1;
    return 0;
    LOG_MSG("GetHRMList() - OK\n");
    return 0;
}
int GetBikeList(void * p_search_service, void ** devices, int & num_hrm)
{
    LOG_MSG("GetBikeList()+\n");
    if (GetChannelList(p_search_service, devices, num_hrm, bike::ANT_DEVICE_TYPE) != 0)
        return -1;
    return 0;
    LOG_MSG("GetBikeList() - OK\n");
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
        thread = std::thread([session]()
        {
            while(true)
                ((TelemetryServer*)session.m_TelemtryServer)->Tick();
        });
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
