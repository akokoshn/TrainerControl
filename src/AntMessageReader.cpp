/**
 *  AntStickReader -- communicate with an ANT+ USB stick
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

bool IsGoodChecksum(const Buffer &message)
{
#if !defined(FAKE_CALL)
    uint8_t c = 0;
    std::for_each(message.begin(), message.end(), [&](uint8_t e) { c ^= e; });
    return c == 0;
#else
    return true;
#endif
}

AntMessageReader::AntMessageReader(libusb_device_handle *dh, uint8_t endpoint)
    : m_DeviceHandle(dh),
    m_Endpoint(endpoint),
    m_Transfer(nullptr),
    m_Mark(0),
    m_Active(false)
{
    m_Buffer.reserve(1024);
    m_Transfer = libusb_alloc_transfer(0);
}

AntMessageReader::~AntMessageReader()
{
    if (m_Active)
        libusb_cancel_transfer(m_Transfer);
    while (m_Active) {
        libusb_handle_events(nullptr);
    }
    libusb_free_transfer(m_Transfer);
}

/** Fill `message' with the next available message.  If no message is received
    * within a small amount of time, an empty buffer is returned.  If a message
    * is returned, it is a valid message (good header, length and checksum).
    */
void AntMessageReader::MaybeGetNextMessage(Buffer &message)
{
    message.clear();

    if (m_Active) {
        // Finish the transfer, wait 2 seconds for it
#if !defined(FAKE_CALL)
        struct timeval tv;
        tv.tv_sec = 2; tv.tv_usec = 0;
        int r = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
        if (r < 0)
            throw LibusbError("libusb_handle_events", r);
#else
        m_Transfer->status = LIBUSB_TRANSFER_COMPLETED;
        m_Transfer->actual_length = 4;
        m_Buffer[0] = SYNC_BYTE;
        m_Buffer[1] = 0;
        CompleteUsbTransfer(m_Transfer);
#endif
    }

    if (m_Active)
        return;

    GetNextMessage1(message);
}


/** Fill `message' with the next available message.  If a message is returned,
    * it is a valid message (good header, length and checksum).  If no message is
    * received within a small amount of time, a timeout exception will be
    * thrown.
    */
void AntMessageReader::GetNextMessage(Buffer &message)
{
    MaybeGetNextMessage(message);
    if (message.empty())
        throw std::runtime_error("AntMessageReader -- timed out");
}

void AntMessageReader::GetNextMessage1(Buffer &message)
{
    // Cannot operate on the buffer while a transfer is active
    assert(!m_Active);

    // Look for the sync byte which starts a message
    while ((!m_Buffer.empty()) && m_Buffer[0] != SYNC_BYTE) {
        m_Buffer.erase(m_Buffer.begin());
        m_Mark--;
    }

    // An ANT message has the following sequence: SYNC, LEN, MSGID, DATA,
    // CHECKSUM.  An empty message has at least 4 bytes in it.
    if (m_Mark < 4) {
        SubmitUsbTransfer();
        return MaybeGetNextMessage(message);
    }

    // LEN is the length of the data, actual message length is LEN + 4.
    unsigned len = m_Buffer[1] + 4;

    if (m_Mark < len) {
        SubmitUsbTransfer();
        return MaybeGetNextMessage(message);
    }

    std::copy(m_Buffer.begin(), m_Buffer.begin() + len, std::back_inserter(message));
    // Remove the message from the buffer.
    m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + len);
    m_Mark -= len;

    if (!IsGoodChecksum(message))
        throw std::runtime_error("AntMessageReader -- bad checksum");
}

void LIBUSB_CALL AntMessageReader::Trampoline(libusb_transfer *t)
{
    AntMessageReader *a = reinterpret_cast<AntMessageReader*>(t->user_data);
    a->CompleteUsbTransfer(t);
}

void AntMessageReader::SubmitUsbTransfer()
{
    assert(!m_Active);

    const int read_size = 128;

    // Make sure we have enough space in the buffer.
    m_Buffer.resize(m_Mark + read_size);
#if !defined(FAKE_CALL)
    libusb_fill_bulk_transfer(
        m_Transfer, m_DeviceHandle, m_Endpoint,
        &m_Buffer[m_Mark], read_size, Trampoline, this, TIMEOUT);

    int r = libusb_submit_transfer(m_Transfer);
    if (r < 0)
        throw LibusbError("libusb_submit_transfer", r);
#endif
    m_Active = true;
}

void AntMessageReader::CompleteUsbTransfer(const libusb_transfer *t)
{
    assert(t == m_Transfer);

    m_Active = false;

    bool ok = (m_Transfer->status == LIBUSB_TRANSFER_COMPLETED);

    m_Mark += ok ? m_Transfer->actual_length : 0;
    m_Buffer.erase(m_Buffer.begin() + m_Mark, m_Buffer.end());
}
