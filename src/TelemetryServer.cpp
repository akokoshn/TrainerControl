/**
 *  TelemetryServer -- manage a bike trainer
 *  Copyright (C) 2017 - 2018 Alex Harsanyi (AlexHarsanyi@gmail.com),
 *                            Alexey Kokoshnikov (alexeikokoshnikov@gmail.com)
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
#include "TelemetryServer.h"
#include "Tools.h"

std::ostream& operator<<(std::ostream &out, const Telemetry &t)
{
    if (t.hr >= 0)
        out << "HR: " << t.hr;
    if (t.cad >= 0)
        out << ";CAD: " << t.cad;
    if (t.pwr >= 0)
        out << ";PWR: " << t.pwr;
    if (t.spd >= 0)
        out << ";SPD: " << t.spd;
    return out;
}

TelemetryServer::TelemetryServer (AntStick *stick, int port)
    : m_AntStick (stick),
      m_Hrm (nullptr),
      m_Fec (nullptr)
{
    try {
        LOG_MSG("Started server");
        m_Hrm = new HeartRateMonitor (m_AntStick);
        m_Fec = new FitnessEquipmentControl (m_AntStick);
    }
    catch (...) {
        delete m_Hrm;
        delete m_Fec;
    }
}

TelemetryServer::~TelemetryServer()
{
    delete m_Hrm;
    delete m_Fec;
}

void TelemetryServer::Tick()
{
#if 1
    std::lock_guard<std::mutex> Guard(guard);
    TickAntStick (m_AntStick);
    CheckSensorHealth();
#endif
    Telemetry t;
#if 1
    CollectTelemetry (t);
#else
    t.cad = 78;
    t.hr = 146;
    t.spd = 4.2;
    t.pwr = 214;
#endif
}

bool TelemetryServer::WaitConnection()
{
    int num_try = 0;
    while (num_try < 50)// wait for 5 sec
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));// wait 0.1 sec
        std::lock_guard<std::mutex> Guard(guard);
        if ((m_Hrm && m_Hrm->ChannelState() == AntChannel::CH_OPEN) &&
            (m_Fec && m_Fec->ChannelState() == AntChannel::CH_OPEN))
            return true;
        num_try++;
    }
    return false;
}

void TelemetryServer::CheckSensorHealth()
{
    if (m_Hrm && m_Hrm->ChannelState() == AntChannel::CH_CLOSED) {
        LOG_MSG("Creating new HRM channel");
        auto device_number = m_Hrm->ChannelId().DeviceNumber;
        delete m_Hrm;
        m_Hrm = nullptr;
        // Try to connect again, but we now look for the same device, don't
        // change HRM sensors mid-simulation.
        m_Hrm = new HeartRateMonitor (m_AntStick, device_number);
    }

    if (m_Fec && m_Fec->ChannelState() == AntChannel::CH_CLOSED) {
        auto device_number = m_Fec->ChannelId().DeviceNumber;
        delete m_Fec;
        m_Fec = nullptr;
        m_Fec = new FitnessEquipmentControl (m_AntStick, device_number);
    }
}

void TelemetryServer::CollectTelemetry (Telemetry &out)
{
    if (m_Hrm && m_Hrm->ChannelState() == AntChannel::CH_OPEN)
        out.hr = m_Hrm->InstantHeartRate();
    
    if (m_Fec && m_Fec->ChannelState() == AntChannel::CH_OPEN) {
        out.cad = m_Fec->InstantCadence();
        out.pwr = m_Fec->InstantPower();
        out.spd = m_Fec->InstantSpeed();
    }
}

void TelemetryServer::ProcessMessage(const std::string &message)
{
    std::istringstream input(message);
    std::string command;
    double param;
    input >> command >> param;
    if(command == "SET-SLOPE" && m_Fec) {
        m_Fec->SetSlope(param);
    }
}
