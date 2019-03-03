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
#include "HeartRateMonitor.h"
#include "FitnessEquipmentControl.h"
#include "Tools.h"

SearchService::SearchService(AntStick *stick, std::mutex & guard) :
    m_AntStick(stick),
    m_guard(guard)
{
    std::lock_guard<std::mutex> Guard(m_guard);
    LOG_MSG("Create search service");
    m_pDevices.resize(m_AntStick->GetMaxChannels());
    uint8_t max_num_channels = m_AntStick->GetMaxChannels();
    for (uint8_t i = 0; i < max_num_channels/2; i++)
        m_pDevices[i] = new HeartRateMonitor(m_AntStick);
    for (uint8_t i = 0; i < max_num_channels / 2; i++)
        m_pDevices[max_num_channels / 2 + i] = new FitnessEquipmentControl(m_AntStick);
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
void SearchService::CheckActiveDevices()
{
    for (auto & it : m_pDevices)
    {
        if (it && it->ChannelState() == AntChannel::CH_CLOSED) {
            if (it->ChannelId().DeviceType == hrm::ANT_DEVICE_TYPE)
            {
                LOG_MSG("Re Creating HRM channel");
                delete it;
                it = new HeartRateMonitor(m_AntStick);
            }
            else if (it->ChannelId().DeviceType == bike::ANT_DEVICE_TYPE)
            {
                LOG_MSG("Re Creating bike channel");
                delete it;
                it = new FitnessEquipmentControl(m_AntStick);
            }
        }
    }
}