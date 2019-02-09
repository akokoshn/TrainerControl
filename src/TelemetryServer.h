/**
 *  TelemetryServer -- manage a bike trainer
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
#pragma once
#include <iostream>
#include <mutex>
#include "structures.h"
#include "FitnessEquipmentControl.h"
#include "HeartRateMonitor.h"

std::ostream& operator<<(std::ostream &out, const Telemetry &t);

class TelemetryServer {
public:
    TelemetryServer (AntStick *stick, HeartRateMonitor *hrm, FitnessEquipmentControl *fec);
    ~TelemetryServer();

    void Tick();
    Telemetry GetTelemetry();
    
private:

    void CheckSensorHealth();
    void CollectTelemetry ();
    void ProcessMessage(const std::string &message);

    AntStick *m_AntStick;
    HeartRateMonitor *m_Hrm;
    FitnessEquipmentControl *m_Fec;
    Telemetry m_current_telemetry;

    std::mutex guard;
};
