/**
 *  AntStickWriter -- communicate with an ANT+ USB stick
 *  Copyright (C) 2017 - 2018 Alex Harsanyi (AlexHarsanyi@gmail.com),
 *                            Alexey Kokoshnikov (alexeikokoshnikov@gmail.com)
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
#include "Tools.h"
#include <assert.h>

#include "winsock2.h" // for struct timeval

AntMessageWriter::AntMessageWriter(libusb_device_handle *dh, uint8_t endpoint)
    : m_DeviceHandle(dh), m_Endpoint(endpoint), m_Transfer(nullptr), m_Active(false)
{
    m_Transfer = libusb_alloc_transfer(0);
}

AntMessageWriter::~AntMessageWriter()
{
    if (m_Active)
        libusb_cancel_transfer(m_Transfer);
    while (m_Active) {
        libusb_handle_events(nullptr);
    }
    libusb_free_transfer(m_Transfer);
}

/** Write `message' to the USB device.  This is presumably an ANT message, but
    * we don't check.  When this function returns, the message has been written
    * (there is no buffering on the application side). An exception is thrown if
    * there is an error or a timeout.
    */
void AntMessageWriter::WriteMessage(const Buffer &message)
{
    assert(!m_Active);
    m_Buffer = message;
    m_Active = false;
    SubmitUsbTransfer();
#if !defined(FAKE_CALL)
    // Finish the transfer, wait 2 seconds for it
    struct timeval tv;
    tv.tv_sec = 2; tv.tv_usec = 0;
    int r = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
    if (r < 0)
        throw LibusbError("libusb_handle_events", r);
#else
    CompleteUsbTransfer(m_Transfer);
    m_Transfer->status = LIBUSB_TRANSFER_COMPLETED;
#endif
    if (!m_Active && m_Transfer->status != LIBUSB_TRANSFER_COMPLETED)
        throw LibusbError("AntMessageWriter", m_Transfer->status);

    if (m_Active) {
        libusb_cancel_transfer(m_Transfer);
        struct timeval tv;
        tv.tv_sec = 0; tv.tv_usec = 10 * 1000;
        int r = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
        if (r < 0)
            throw LibusbError("libusb_handle_events", r);

        m_Active = false;               // ready or not!

        throw std::runtime_error("AntMessageWriter -- timed out");
    }
}

void LIBUSB_CALL AntMessageWriter::Trampoline(libusb_transfer *t)
{
    AntMessageWriter *a = reinterpret_cast<AntMessageWriter*>(t->user_data);
    a->CompleteUsbTransfer(t);
}

void AntMessageWriter::SubmitUsbTransfer()
{
#if !defined(FAKE_CALL)
    libusb_fill_bulk_transfer(
        m_Transfer, m_DeviceHandle, m_Endpoint,
        &m_Buffer[0], (int)m_Buffer.size(), Trampoline, this, TIMEOUT);

    int r = libusb_submit_transfer(m_Transfer);
    if (r < 0)
        throw LibusbError("libusb_submit_transfer", r);
#endif
    m_Active = true;
}

void AntMessageWriter::CompleteUsbTransfer(const libusb_transfer *t)
{
    assert(t == m_Transfer);
    m_Active = false;
}