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
#include "Tools.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>

int InitAntService(void ** ant_instanance)
{
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
    return 0;
}
int CloseAntService()
{
    return 0;
}
