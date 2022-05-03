#include <gnuradio/io_signature.h>
#include "bb60c_source_impl.h"



namespace gr {
namespace shbb60 {

    bb60c_source::sptr
    bb60c_source::make(
        float center_freq,
        float ref_level,
        int decimation,
        float filter_bw)
    {
        return gnuradio::make_block_sptr<bb60c_source_impl>(
            center_freq,
            ref_level,
            decimation,
            filter_bw);
    }


    bb60c_source_impl::bb60c_source_impl(
        float center_freq,
        float ref_level,
        int decimation,
        float filter_bw)
        : gr::block(
            "bb60c_source",
            gr::io_signature::make(0, 0, 0),
            gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
        printf("[*] bb60c_source_ipml: center_freq =    %0.f\n", center_freq);
        printf("[*] bb60c_source_ipml: ref_level =      %0.f\n", ref_level);
        printf("[*] bb60c_source_ipml: decimation =     %d\n", decimation);
        printf("[*] bb60c_source_ipml: filter_bw =      %0.f\n", filter_bw);

        bbStatus status;
        m_handle = -1;
        int serials[8] = {0};
        int count = 0;

        printf("[*] bb60c_source_ipml: Signal Hound BB60C API version: %s\n", bbGetAPIVersion());

        bbGetSerialNumberList(serials, &count);
        if (count < 1)
        { 
            fprintf(stderr, "[!] no BB60 devices found\n");
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: attempting to open device with serial number %d\n", serials[0]);

        status = bbOpenDeviceBySerialNumber(&m_handle, serials[0]);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbOpenDeviceBySerialNumber() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: opened device\n");

        status = bbConfigureIQCenter(m_handle, center_freq);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQCenter() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: set center frequency\n");

        status = bbConfigureLevel(m_handle, ref_level, BB_AUTO_ATTEN);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureLevel() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: configured reference level\n");

        status = bbConfigureIO(m_handle, 0, 0);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIO() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: configured BNC ports\n");

        status = bbConfigureIQ(m_handle, decimation, filter_bw);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQ() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: configured sampling rate and bandwidth\n");

        status = bbConfigureIQDataType(m_handle, bbDataType32fc);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbConfigureIQDataType() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: configured sample format\n");

        status = bbInitiate(m_handle, BB_STREAMING, BB_STREAM_IQ);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bb60c_source_ipml: bbInitiate() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        printf("[*] bb60c_source_ipml: initiated IQ sampling\n");
    }


    bb60c_source_impl::~bb60c_source_impl()
    {
    }


    bool bb60c_source_impl::stop()
    {
        bbCloseDevice(m_handle);
        return true;
    }


    int bb60c_source_impl::general_work(
        int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
        bbIQPacket pkt;
        pkt.iqData = (void*)output_items[0];
        pkt.iqCount = noutput_items;
        pkt.triggers = NULL;
        pkt.triggerCount = 0;
        pkt.purge = BB_TRUE;

        bbStatus status = bbGetIQ(m_handle, &pkt);
        if (status != bbNoError)
        {
            fprintf(stderr, "[!] bbGetIQ() failed\n");
            fprintf(stderr, "%s\n", bbGetErrorString(status));
            exit(EXIT_FAILURE);
        }

        return noutput_items;
    }

} /* namespace shbb60 */
} /* namespace gr */