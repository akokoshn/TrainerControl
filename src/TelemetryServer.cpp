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

TelemetryServer::TelemetryServer (AntStick * stick, HeartRateMonitor * hrm, FitnessEquipmentControl * fec)
    : m_AntStick (stick),
      m_Hrm (hrm),
      m_Fec (fec),
      m_current_telemetry()
{
    LOG_MSG("Started server");
}

TelemetryServer::~TelemetryServer()
{}

void TelemetryServer::Tick()
{
    std::lock_guard<std::mutex> Guard(guard);
    //TickAntStick(m_AntStick);
    CheckSensorHealth();
    CollectTelemetry();
}

void TelemetryServer::CheckSensorHealth()
{
    //TODO implement reconnect to device from searching service's pool
}

void TelemetryServer::CollectTelemetry ()
{
    double hr = 0;
    if (m_Hrm && m_Hrm->ChannelState() == AntChannel::CH_OPEN)
        hr = m_Hrm->InstantHeartRate();
    m_current_telemetry.hr = hr ? hr : m_current_telemetry.hr;
    
    if (m_Fec && m_Fec->ChannelState() == AntChannel::CH_OPEN) {
        m_current_telemetry.cad = m_Fec->InstantCadence();
        m_current_telemetry.pwr = m_Fec->InstantPower();
        m_current_telemetry.spd = m_Fec->InstantSpeed();
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

Telemetry TelemetryServer::GetTelemetry()
{
    return m_current_telemetry;
}
