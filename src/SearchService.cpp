/**
 *  TelemetryServer -- manage a bike trainer
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

#include "SearchService.h"
#include "Tools.h"
#include "HeartRateMonitor.h"
#include "FitnessEquipmentControl.h"


SearchService::SearchService(AntStick *stick, std::mutex & guard) :
    m_AntStick(stick),
    m_NumDevices(0),
    m_guard(guard)
{
    std::lock_guard<std::mutex> Guard(m_guard);
    LOG_MSG("Create search service");
    m_pDevices.resize(m_AntStick->GetMaxChannels());
}
SearchService::~SearchService()
{
    std::lock_guard<std::mutex> Guard(m_guard);
    LOG_MSG("Destroy search service");
    //TODO add cloasing channels
    for (auto it : m_pDevices)
        delete it;
    m_pDevices.shrink_to_fit();
    m_pDevices.clear();
}
void SearchService::Tick()
{
    std::lock_guard<std::mutex> Guard(m_guard);
    TickAntStick(m_AntStick);
    CheckActiveDevices();
}
std::vector<AntChannel*> SearchService::GetActiveDevices()
{
    // non block call, so no mutex protection
    return m_pDevices;
}
int SearchService::AddDeviceForSearch(AntDeviceType type)
{
    std::lock_guard<std::mutex> Guard(m_guard);
    if (m_NumDevices >= m_pDevices.size())
        return -1;

    switch (type)
    {
    case HRM_Type:
        m_pDevices[m_NumDevices] = new HeartRateMonitor(m_AntStick);
        break;
    case BIKE_Type:
        m_pDevices[m_NumDevices] = new FitnessEquipmentControl(m_AntStick);
        break;
    case NONE_Type:
    default:
        return -1;
    }
    m_NumDevices++;
}
void SearchService::CheckActiveDevices()
{
    for (auto & it : m_pDevices)
    {
        if (it && it->ChannelState() == AntChannel::CH_CLOSED) {
            if (it->ChannelId().DeviceType == HRM::ANT_DEVICE_TYPE)
            {
                LOG_MSG("Re Creating HRM channel");
                delete it;
                it = new HeartRateMonitor(m_AntStick);
            }
            else if (it->ChannelId().DeviceType ==  BIKE::ANT_DEVICE_TYPE)
            {
                LOG_MSG("Re Creating bike channel");
                delete it;
                it = new FitnessEquipmentControl(m_AntStick);
            }
        }
    }
}