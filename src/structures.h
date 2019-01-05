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
#pragma once

struct AntSession
{
    void * m_AntStick;
    void * m_TelemtryServer;
    bool m_bIsRun;
};

// Hold information about a "current" reading from the trainer.  We quote
// "current" because data comes from different sources and might not be
// completely in sync.
struct Telemetry
{
    Telemetry()
        : hr(-1), cad(-1), spd(-1), pwr(-1) {}
    double hr;
    double cad;
    double spd;
    double pwr;
};