/**
 *  HeartRateMonitor -- communicate with an ANT+ HRM 
 *  Copyright (C) 2017 Alex Harsanyi (AlexHarsanyi@gmail.com)
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
#include "HeartRateMonitor.h"
#include "Tools.h"
#include <iostream>

/** IMPLEMENTATION NOTE 
 * 
 * Implementation of the ANT+ Heart Rate Device profile is based on the
 * "D00000693_-_ANT+_Device_Profile_-_Heart_Rate_Rev_2.1.pdf" document
 * available from https://www.thisisant.com
 */

using namespace hrm;

HeartRateMonitor::HeartRateMonitor (AntStick *stick, uint32_t device_number)
    : AntChannel(
        stick, 
        AntChannel::Id(ANT_DEVICE_TYPE, device_number),
        CHANNEL_PERIOD, SEARCH_TIMEOUT, CHANNEL_FREQUENCY)
{
    m_LastMeasurementTime = 0;
    m_MeasurementTime = 0;
    m_HeartBeats = 0;
    m_InstantHeartRate = 0;
    m_InstantHeartRateTimestamp = 0;
    LOG_MSG("Created instance of HR Monitor\n");
}

void HeartRateMonitor::OnMessageReceived(const unsigned char *data, int size)
{
    LOG_MSG("OnMessageReceived for HRM\n");
    if (data[2] != BROADCAST_DATA)
        return;

    // NOTE: the last 3 values in the payload are always the same regardless
    // of the data page.  Also for the data page, we need to observe the
    // highest bit toggle, as old HRM's don't have data pages.
    m_LastMeasurementTime = m_MeasurementTime;
    m_MeasurementTime = data[8] + (data[9] << 8);
    m_HeartBeats = data[10];
    m_InstantHeartRate = data[11];
    m_InstantHeartRateTimestamp = CurrentMilliseconds();
}

double HeartRateMonitor::InstantHeartRate() const 
{
    if ((CurrentMilliseconds() - m_InstantHeartRateTimestamp) > STALE_TIMEOUT) {
        return 0;
    } else {
        return m_InstantHeartRate;
    }
}

void HeartRateMonitor::OnStateChanged (
    AntChannel::State old_state, AntChannel::State new_state)
{
    if (new_state == AntChannel::CH_OPEN) {
        LOG_MSG("Connected to HRM with serial "); LOG_D(ChannelId().DeviceNumber);
     } else {
        m_LastMeasurementTime = 0;
        m_MeasurementTime = 0;
        m_HeartBeats = 0;
        m_InstantHeartRate = 0;
        m_InstantHeartRateTimestamp = 0;
     }
}
