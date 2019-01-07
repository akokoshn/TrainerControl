/**
 *  TrainServer -- prototype bike trainer application
 *  Copyright (C) 2019 Alexey Kokoshnikov (alexeikokoshnikov@gmail.com)
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

#include <vector>
#include "Mock.h"
#include "TrainerControl.h"

#define CHECK_EQ(val1, val2) if (val1 != val2) { printf("NOT EQUAL\n"); return -1;}
#define CHECK_NOT_EQ(val1, val2) if (val1 == val2) { printf("EQUAL\n"); return -1;}
template <typename T>
inline bool IS_EQ(T val1, T val2) { return (val1 == val2) ? true : false; }

enum test_case_type
{
    VALID,
    BAD_PARAM,
    BAD_STATE
};

struct test_case
{
    test_case_type type;
    char description[128];
    int expected;
};

class test_suite
{
public:
    virtual bool run_case(int num_case = -1)
    {
        if (num_case >= 0)
        {
            printf("start case #%d:\n", num_case);
            test_case _case = test_cases.at(num_case);
            if (0 != prepare(_case))
            {
                printf("finish case #%d - FAIL\n", num_case);
                return false;
            }
            if (0 != execute(_case))
            {
                printf("finish case #%d - FAIL\n", num_case);
                return false;
            }
            if (0 != complete(_case))
            {
                printf("finish case #%d - FAIL\n", num_case);
                return false;
            }
            printf("finish case #%d - OK\n", num_case);
            return true;
        }

        bool result = true;
        for (int i = 0; i < test_cases.size(); i++)
        {
            if (false == run_case(i))
            {
                result = false;
            }
        }
        return result;
    }
protected:
    virtual int prepare(const test_case _case) { return 0; }
    virtual int execute(const test_case _case) { return 0; }
    virtual int complete(const test_case _case) { return 0; }

    std::vector<test_case> test_cases;
};

class ServiceInit : public test_suite
{
public:
    ServiceInit()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {VALID, "reinit", 0},
            {BAD_PARAM, "null ptr", -1}
        };
        printf("test init service [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        if (0 != strcmp("null ptr", _case.description))
            ant_handle = new void*;
        else
            ant_handle = nullptr;
        if (0 == strcmp("reinit", _case.description))
        {
            CHECK_EQ(0, InitAntService(ant_handle));
            CHECK_EQ(0, CloseAntService());
        }
        if (0 == strcmp("double init", _case.description))
            CHECK_EQ(0, InitAntService(ant_handle));
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        CHECK_EQ(_case.expected, InitAntService(ant_handle));
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        CHECK_EQ(0, CloseAntService());
        return 0;
    }

    void ** ant_handle;
};

class ServiceClose : public test_suite
{
public:
    ServiceClose()
    {
        test_cases =
        {
            {VALID, "none", 0}
            //{BAD_STATE, "not init", -1}
        };
        printf("test close service [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        ant_handle = new void*;
        if (0 != strcmp("not init", _case.description))
            CHECK_EQ(0, InitAntService(ant_handle));
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        CHECK_EQ(_case.expected, CloseAntService());
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        return 0;
    }

    void ** ant_handle;
};

class SessionInit : public test_suite
{
public:
    SessionInit()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {BAD_PARAM, "null ptr", -1},
            //{BAD_PARAM, "wrong ptr", -1},
        };
        printf("test init session [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        ant_session = {};
        if (0 == strcmp("null ptr", _case.description))
            ant_handle = nullptr;
        else if (0 == strcmp("wrong ptr", _case.description))
            ant_handle = (void*)0xff;
        else
            CHECK_EQ(0, InitAntService(&ant_handle));
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        ant_session = InitSession(ant_handle);
        if (_case.expected == 0)
        {
            CHECK_EQ(false, ant_session.m_bIsRun);
            CHECK_NOT_EQ(nullptr, ant_session.m_AntStick);
            CHECK_NOT_EQ(nullptr, ant_session.m_TelemtryServer);
        }
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        if (_case.expected == 0)
            CHECK_EQ(0, CloseSession(ant_session));
        CHECK_EQ(0, CloseAntService());
        return 0;
    }

    void * ant_handle;
    AntSession ant_session;
};

class SessionClose : public test_suite
{
public:
    SessionClose()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {BAD_STATE, "not init service", 0},
            {BAD_STATE, "not init session", 0},
        };
        printf("test close session [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        ant_session = {};
        if (0 == strcmp("not init service", _case.description))
            return 0;
        CHECK_EQ(0, InitAntService(&ant_handle));
        if (0 != strcmp("not init session", _case.description))
            ant_session = InitSession(ant_handle);
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        CloseSession(ant_session);
        CHECK_EQ(false, ant_session.m_bIsRun);
        CHECK_EQ(nullptr, ant_session.m_AntStick);
        CHECK_EQ(nullptr, ant_session.m_TelemtryServer);
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        return 0;
    }

    void * ant_handle;
    AntSession ant_session;
};

class SessionRun : public test_suite
{
public:
    SessionRun()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {BAD_PARAM, "alredy run", -1},
            {BAD_PARAM, "no ant stick", -1},
            {BAD_PARAM, "no service", -1},
        };
        printf("test run session [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        CHECK_EQ(0, InitAntService(&ant_handle));
        ant_session = InitSession(ant_handle);
        if (0 == strcmp("alredy run", _case.description))
            ant_session.m_bIsRun = true;
        if (0 == strcmp("no ant stick", _case.description))
            ant_session.m_AntStick = nullptr;
        if (0 == strcmp("no service", _case.description))
            ant_session.m_TelemtryServer = nullptr;
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        CHECK_EQ(_case.expected, Run(ant_session, server_thread));

        return 0;
    }
    virtual int complete(const test_case _case)
    {
        if (_case.expected == 0)
        {
            CHECK_EQ(0, Stop(ant_session, server_thread));
            CHECK_EQ(0, CloseSession(ant_session));
        }
        CHECK_EQ(0, CloseAntService());
        return 0;
    }

    void * ant_handle;
    AntSession ant_session;
    std::thread server_thread;
};

class SessionStop : public test_suite
{
public:
    SessionStop()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {BAD_PARAM, "not init", -1},
            {BAD_PARAM, "not run", -1},
            {BAD_PARAM, "wrong thread", -1},
        };
        printf("test stop session [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        CHECK_EQ(0, InitAntService(&ant_handle));
        if (0 != strcmp("not init", _case.description))
        {
            ant_session = InitSession(ant_handle);
            if (0 != strcmp("not run", _case.description))
                CHECK_EQ(0, Run(ant_session, server_thread));
        }
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        if (0 == strcmp("wrong thread", _case.description))
        {
            std::thread wrong_thread;
            CHECK_EQ(_case.expected, Stop(ant_session, wrong_thread));
        }
        else
            CHECK_EQ(_case.expected, Stop(ant_session, server_thread));
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        if (server_thread.joinable())
            server_thread.join();
        CHECK_EQ(0, CloseSession(ant_session));
        CHECK_EQ(0, CloseAntService());
        return 0;
    }

    void * ant_handle;
    AntSession ant_session;
    std::thread server_thread;
};

class ServiceGetTelemetry : public test_suite
{
public:
    ServiceGetTelemetry()
    {
        test_cases =
        {
            {VALID, "none", 0},
            {BAD_PARAM, "not init", -1},
            {BAD_PARAM, "not run", -1},
        };
        printf("test get telemetry [%d]\n", test_cases.size());
    }
protected:
    virtual int prepare(const test_case _case)
    {
        CHECK_EQ(0, InitAntService(&ant_handle));
        if (0 != strcmp("not init", _case.description))
        {
            ant_session = InitSession(ant_handle);
            if (0 != strcmp("not run", _case.description))
                CHECK_EQ(0, Run(ant_session, server_thread));
        }
        return 0;
    }
    virtual int execute(const test_case _case)
    {
        Telemetry t = GetTelemetry(ant_session);
#if !defined(FAKE_CALL)
        if (_case.expected == 0)
        {
            CHECK_NOT_EQ(-1, t.hr);
            CHECK_NOT_EQ(-1, t.cad);
            CHECK_NOT_EQ(-1, t.pwr);
            CHECK_NOT_EQ(-1, t.spd);
        }
        else
#endif
        {
            CHECK_EQ(-1, t.hr);
            CHECK_EQ(-1, t.cad);
            CHECK_EQ(-1, t.pwr);
            CHECK_EQ(-1, t.spd);
        }
        return 0;
    }
    virtual int complete(const test_case _case)
    {
        if (0 != strcmp("not init", _case.description) &&
            0 != strcmp("not run", _case.description))
        {
            CHECK_EQ(0, Stop(ant_session, server_thread));
        }
        CHECK_EQ(0, CloseSession(ant_session));
        CHECK_EQ(0, CloseAntService());
        return 0;
    }

    void * ant_handle;
    AntSession ant_session;
    std::thread server_thread;
};