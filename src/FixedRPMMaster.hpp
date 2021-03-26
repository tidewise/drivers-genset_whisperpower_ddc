#ifndef _GENSET_WHISPERPOWER_DDC_FIXED_RPM_MASTER_HPP_
#define _GENSET_WHISPERPOWER_DDC_FIXED_RPM_MASTER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <genset_whisperpower_ddc/Frame.hpp>
#include <iodrivers_base/Driver.hpp>

namespace genset_whisperpower_ddc {
    class FixedRPMMaster : public iodrivers_base::Driver {
        /** Genset DDC packet extraction is time-based
         *  This method just throws
         */
        int extractPacket(uint8_t const* buffer, size_t bufferSize) const;

        /*
         * The DDC power print sends every 300ms data information for the panels
         */
        base::Time m_interframe_delay = base::Time::fromMilliseconds(300);

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
            FixedRPMMaster();

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
    };

}

#endif
