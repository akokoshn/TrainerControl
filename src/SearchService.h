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
#pragma once
#include <iostream>
#include <mutex>
#include "structures.h"
#include "AntStick.h"

class SearchService {
public:
    SearchService(AntStick *stick, std::mutex & guard);
    ~SearchService();

    void Tick();
    std::vector<AntChannel*> GetActiveDevices();
    int AddDeviceForSearch(AntDeviceType type);

private:

    void CheckActiveDevices();

    AntStick *m_AntStick;
    std::vector<AntChannel*> m_pDevices;
    unsigned int m_NumDevices;

    std::mutex & m_guard;
};