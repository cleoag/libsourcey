///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <http://sourcey.com>
//
// SPDX-License-Identifier:	LGPL-2.1+
//
/// @addtogroup base
/// @{


#ifndef SCY_PACKET_IO_H
#define SCY_PACKET_IO_H


#include "scy/base.h"
#include "scy/packetstream.h"
#include <fstream>
#include <iostream>


namespace scy {


///
/// Threaded stream reader class.
///
/// This class can be connected to a `PacketStream` to read input from any
/// class that derives from `std::istream`. It's most regularly used for reading
/// input files.
///
class SCY_EXTERN ThreadedStreamReader : public PacketSource, public basic::Startable
{
public:
    ThreadedStreamReader(std::istream* is)
        : PacketSource(this->emitter)
        , _istream(is)
    {
        _runner.setRepeating(true);
    }

    ~ThreadedStreamReader()
    {
        stop();

        if (_istream) {
            delete _istream;
        }
    }

    void start()
    {
        _runner.start(
            [](void* arg) {
                auto self = reinterpret_cast<ThreadedStreamReader*>(arg);
                std::string line;
                if (getline(self->stream(), line)) {
                    self->emit(line);
                }
                if (self->stream().eof()) {
                    self->emit(PacketFlags::Final);
                }
            },
            this);
    }

    void stop()
    {
        // _runner.close();
        _runner.cancel();
    }

    template <class StreamT> StreamT& stream()
    {
        auto stream = dynamic_cast<StreamT*>(_istream);
        if (!stream)
            throw std::runtime_error("Cannot cast internal stream type.");

        return *stream;
    }

    std::istream& stream() { return *_istream; }

    PacketSignal emitter;

protected:
    Thread _runner;
    std::istream* _istream;
};


///
/// Packet stream writer class.
///
/// This class can be connected to a `PacketStream` to write output to any
/// class that derives from `std::ostream`. It's most regularly used for writing
/// output files.
///
class SCY_EXTERN StreamWriter : public PacketProcessor
{
public:
    StreamWriter(std::ostream* stream)
        : PacketProcessor(this->emitter)
        , _ostream(stream)
    {
    }

    virtual ~StreamWriter()
    {
        if (_ostream) {
            auto fstream = dynamic_cast<std::ofstream*>(_ostream);
            if (fstream)
                fstream->close();
            delete _ostream;
        }
    }

    virtual void process(IPacket& packet)
    {
        Buffer buffer;
        packet.write(buffer);

        _ostream->write(buffer.data(), buffer.size());
        _ostream->flush();

        emit(packet);
    }

    template <class StreamT> StreamT& stream()
    {
        auto stream = dynamic_cast<StreamT*>(_ostream);
        if (!stream)
            throw std::runtime_error("Cannot cast internal stream type.");

        return *stream;
    }

    std::ostream& stream() { return *_ostream; }

    PacketSignal emitter;

protected:
    std::ostream* _ostream;
};


} // namespace scy


#endif // SCY_PACKET_IO_H


/// @\}
