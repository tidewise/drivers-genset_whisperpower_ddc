#ifndef _GENSET_WHISPERPOWER_DDC_VARIABLE_SPEED_HPP_
#define _GENSET_WHISPERPOWER_DDC_VARIABLE_SPEED_HPP_

#include <array>

#include <genset_whisperpower_ddc/Frame.hpp>

namespace genset_whisperpower_ddc {
    /**
     * Implementation of the protocol for variable speed gensets –GV-4 (230V/50Hz)
     */
    namespace variable_speed {
        /** Address for the monitor panel
         */
        static const int TARGET_ADDRESS = 0x0081;
        
        /** Address for the DDC power controller
         */
        static const int SOURCE_ADDRESS = 0x0088;

        /** Maximum total frame size
         */
        static const int FRAME_MAX_SIZE = 16;

        /** Total frame size of sent packet
         */
        static const int SENT_FRAME_SIZE = 16;

        /** Total frame size of received packet
         */
        static const int RECEIVED_FRAME_SIZE = 10;

        /** Number of bytes in a frame on top of the frame payload itself
         */
        static const int FRAME_OVERHEAD_SIZE = 6;

        /** Number of bytes in a frame header */
        static const int FRAME_HEADER_SIZE = 5;

        /** @overload
         */
        uint8_t* formatFrame(uint8_t* buffer, uint16_t targetID, uint16_t sourceID, uint8_t command,
                     std::vector<uint8_t> const& payload);

        /** Fill a byte buffer with a valid frame (incl. checksum)
         */
        uint8_t* formatFrame(uint8_t* buffer, uint16_t targetID, uint16_t sourceID, uint8_t command,
                                uint8_t const* payloadStart, uint8_t const* payloadEnd);

        /**
         * Exception thrown when the given strings of bytes do not match
         * the received frame expected size
         */
        struct WrongSize : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        /** Exception thrown by parseFrame if the given buffer does not contain
         * a valid frame
         */
        struct InvalidChecksum : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        /** Validates a set of bytes and converts it into a Frame
         *
         * @throw InvalidChecksum if the buffer does not contain a valid frame
         */
        Frame parseFrame(uint8_t const* start, uint8_t const* end);

        /** @overload parseFrame version that allows the use of a preallocated
         *      payload
         */
        void parseFrame(Frame& frame, uint8_t const* start, uint8_t const* end);

        /** Computes the expected checksum by taking the lowest byte
         *  of the sum of the first 15 bytes in the packet
         */
        uint8_t checksum(uint8_t const* start, uint8_t const* end);

        /** Validates the checksum contained at the end of the frame
         */
        bool isChecksumValid(uint8_t const* start, uint8_t const* end);

        /** Format payload for command 02
         * 
         * @arg RPM
         * @arg Udc start battery: value x 0.01 V
         * @arg Status bits (A)
         * @arg Status bits (B)
         * @arg Status bits (C)
         * @arg Generator status
         * @arg Generator type
         */
        std::vector<uint8_t> formatCommand02Data(
            uint16_t rpm, uint16_t udcStartBattery, uint8_t statusA,
            uint8_t statusB, uint8_t statusC, uint8_t generatorStatus,
            uint8_t generatorType
        );

        /** Format payload for command 14
         * 
         * @arg Total run time to be reset after maintenance (Minutes)
         * @arg Total run time to be reset after maintenance (Hours)
         * @arg Historical run time (Minutes)
         * @arg Historical run time (Hours)
         */
        std::vector<uint8_t> formatCommand14Data(
            uint8_t totalMinutes, uint32_t totalHours,
            uint8_t historicalMinutes, uint32_t historicalHours
        );
    }

}

#endif
