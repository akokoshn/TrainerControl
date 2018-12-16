/**
 *  TrainServer -- prototype bike trainer application
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
#include "AntStick.h"
#include "NetTools.h"
#include "TelemetryServer.h"
#include "Tools.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <iostream>

void ProcessChannels(AntStick &s)
{
    try {
        TelemetryServer server (&s);
        while (true) {
            server.Tick();
        }
    }
    catch (std::exception &e) {
        LOG_MSG(e.what());
    }
}

void ProcessAntSticks()
{
    while (true) {
        try {
            AntStick a;
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            LOG_MSG(" USB Stick: Serial#: "); LOG_D(a.GetSerialNumber());
            LOG_MSG(", version "); LOG_MSG(a.GetVersion().c_str());
            LOG_MSG(", max "); LOG_D(a.GetMaxNetworks()); 
            LOG_MSG(" networks, max "); LOG_D(a.GetMaxChannels());
            LOG_MSG(" channels\n");
            a.SetNetworkKey(AntStick::g_AntPlusNetworkKey);
            ProcessChannels(a);
        }
        catch (const AntStickNotFound &e) {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            LOG_MSG(e.what());
            return;
        }
        catch (std::exception &e) {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            LOG_MSG(e.what());
        }
    }
}

int main()
{
    try {
        int r = libusb_init(NULL);
        if (r < 0)
            throw LibusbError("libusb_init", r);
        ProcessAntSticks();
    }
    catch (const std::exception &e) {
        LOG_MSG(e.what()); LOG_MSG("\n");
        return 1;
    }
    return 0;
}

