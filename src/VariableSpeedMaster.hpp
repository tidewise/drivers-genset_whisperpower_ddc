#ifndef _GENSET_WHISPERPOWER_DDC_VARIABLE_SPEED_MASTER_HPP_
#define _GENSET_WHISPERPOWER_DDC_VARIABLE_SPEED_MASTER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <genset_whisperpower_ddc/Frame.hpp>
#include <iodrivers_base/Driver.hpp>

namespace genset_whisperpower_ddc {
    class VariableSpeedMaster : public iodrivers_base::Driver {
        /** Genset DDC packet extraction is time-based
         *  This method just throws
         */
        int extractPacket(uint8_t const* buffer, size_t bufferSize) const;

        /*
         *  
         */
        base::Time m_interframe_delay = base::Time::fromMilliseconds(250); 

        /** Internal read buffer */
        std::vector<uint8_t> m_read_buffer;

        /** Internal write buffer */
        std::vector<uint8_t> m_write_buffer;

        /** Internal frame object
         *
         * This is used to avoid unnecessary memory allocation
         */
        Frame m_frame;

        public:
            VariableSpeedMaster();

            /** Wait for one frame on the bus and read it
             */
            Frame readFrame();

            /** Wait for one frame on the bus and read it
             */
            void readFrame(Frame& frame);

            /** Send a frame
             *
             * Does not wait for the turnaround delay
             */
            void writeFrame(uint8_t command, std::vector<uint8_t> const& payload);

            /** Send a command 02 frame
             * 
             * @arg RPM
             * @arg Udc start battery: value x 0.01 V
             * @arg Status bits (A)
             * @arg Status bits (B)
             * @arg Status bits (C)
             * @arg Generator status
             * @arg Generator type
             */
            void Command02(
                uint16_t rpm, uint16_t udcStartBattery, uint8_t statusA,
                uint8_t statusB, uint8_t statusC, uint8_t generatorStatus,
                uint8_t generatorType
            );

            /** Send a command 14 frame
             * 
             * @arg Total run time to be reset after maintenance (Minutes)
             * @arg Total run time to be reset after maintenance (Hours)
             * @arg Historical run time (Minutes)
             * @arg Historical run time (Hours)
             */
            void Command14(
                uint8_t totalMinutes, uint32_t totalHours,
                uint8_t historicalMinutes, uint32_t historicalHours
            );
    };

}

#endif
