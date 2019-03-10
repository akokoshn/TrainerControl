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

#include <thread>
#include <mutex>
#include "structures.h"

#ifdef TRAINERCONTROLDLL_EXPORTS
#define TRAINERCONTROLDLL_API __declspec(dllexport)
#else
#define TRAINERCONTROLDLL_API __declspec(dllimport)
#endif

extern "C" TRAINERCONTROLDLL_API int InitAntService(void ** ant_instanance, int & max_channels);
extern "C" TRAINERCONTROLDLL_API int CloseAntService();
extern "C" TRAINERCONTROLDLL_API int RunSearch(void * ant_instanance, void ** pp_search_service, std::thread & thread, std::mutex & guard);
extern "C" TRAINERCONTROLDLL_API int StopSearch(void ** pp_search_service, std::thread & thread);
extern "C" TRAINERCONTROLDLL_API AntSession InitSession(void * ant_instanance, void * hrm_device, void * bike_device, std::mutex & guard);
extern "C" TRAINERCONTROLDLL_API int GetDeviceList(void * p_search_service, void ** devices, int & num_devices);
extern "C" TRAINERCONTROLDLL_API int CloseSession(AntSession & session);
/*create separate thread assign with session*/
extern "C" TRAINERCONTROLDLL_API int Run(AntSession & session, std::thread & thread);
extern "C" TRAINERCONTROLDLL_API int Stop(AntSession & session, std::thread & thread);
extern "C" TRAINERCONTROLDLL_API Telemetry GetTelemetry(AntSession & session);