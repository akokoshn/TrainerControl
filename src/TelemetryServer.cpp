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
    return out;
}

TelemetryServer::TelemetryServer (AntStick * stick, HeartRateMonitor * hrm, std::mutex & guard)
    : m_AntStick (stick),
      m_Hrm (hrm),
      m_current_telemetry(),
      m_guard(guard)
{
    LOG_MSG("Started server");
}

TelemetryServer::~TelemetryServer()
{}

void TelemetryServer::Tick()
{
    std::lock_guard<std::mutex> Guard(m_guard);
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
}

Telemetry TelemetryServer::GetTelemetry()
{
    return m_current_telemetry;
}
