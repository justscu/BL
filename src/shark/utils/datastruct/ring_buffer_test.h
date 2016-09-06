#include "ring_buffer.h"
#include <stdio.h>
#include <string>

#pragma once

namespace DS {
class RingBuffer_test {
public:
    // rPos <= wPos
    void test_read_write() {
        RingBuffer rb(8);
        assert(rb.used_size() == 0);
        assert(rb.free_size() == 8);
        assert(rb.buf_size()  == 8);
        // 写
        int32_t wLen = rb.write("abcd", 4);
        assert(wLen == 4);
        char buf[16];
        int32_t rLen = rb.read(buf, 2);
        assert(rLen == 2);
        assert(rb.used_size() == 2);
        assert(rb.free_size() == 6);
        assert(std::string(buf, 2) == "ab");
        // write
        wLen = rb.write("123456", 6);
        assert(wLen == 6);
        // assert(std::string(rb.buf_, 8) == "56cd1234");
        wLen = rb.write("111", 3);
        assert(wLen == 0);

        // read
        rLen = rb.read(buf, sizeof(buf));
        assert(rLen == 8);
        assert(std::string(buf, rLen) == "cd123456");

        assert(rb.used_size() == 0);
        assert(rb.free_size() == 8);
        assert(rb.buf_size()  == 8);

        // write
        wLen = rb.write("123456", 6);
        assert(wLen == 6);
        assert(rb.free_size() == 2);
        assert(rb.used_size() == 6);
        // read
        rLen = rb.read(buf, 2);
        assert(rLen == 2);
        assert(rb.free_size() == 4);
        assert(rb.used_size() == 4);

        wLen = rb.write("ab", 2);
        assert(wLen == 2);
        assert(rb.free_size() == 2);
        assert(rb.used_size() == 6);
    }
    // rPos > wPos
    void test_read_write2() {
        RingBuffer rb(8);
        assert(rb.used_size() == 0);
        assert(rb.free_size() == 8);
        assert(rb.buf_size()  == 8);
        int32_t wLen = rb.write("123456", 6);
        assert(wLen == 6);
        char buf[8];
        int32_t rLen = rb.read(buf, 4);
        assert(rLen == 4);
        assert(std::string(buf, 4) == "1234");
        wLen = rb.write("abc", 3);
        assert(wLen == 3);

        //
        if (0) {
            rLen = rb.read(buf, 8);
            assert(rLen == 5);
            assert(std::string(buf, 5) == "56abc");
        }

        if (1) {
            assert(rb.used_size() == 5);
            wLen = rb.write("xy", 2);
            assert(rb.used_size() == 7);
            assert(rb.free_size() == 1);
            rLen = rb.read(buf, sizeof(buf));
            assert(rLen == 7);
            assert(std::string(buf, 7) == "56abcxy");
        }
    }

    void test_rand() {
        const std::string paper("Leaders of major economies reached consensus during the G20 "
                " Summit on seeking workable solutions for global growth and development, "
                "a consensus that one analyst described as having China's distinctive "
                "stamp. In his concluding remarks, President Xi Jinping said summit "
                " participants reached important consensus on such G20 tasks as "
                "strengthening policy coordination, breaking a new path for growth, "
                "achieving more efficient and effective global economic and financial "
                "governance, boosting international trade and investment and enhancing "
                "anti-graft efforts. Besides tackling regular challenges, such as "
                "promoting innovation to provide new engines for global economic growth,"
                "world leaders at the summit, which ended on Monday in Hangzhou, Zhejiang "
                " province, emphasized inclusive growth and development of less-developed"
                " countries. Xi called this progress groundbreaking. For the first time,"
                " we have given priority to development in the global macro-policy "
                " framework, a move that will help reduce inequality and imbalance in "
                "global development, deliver tangible benefits to people of the developing "
                "world, make important progress toward realizing the (United Nations')"
                " Sustainable Development Goals by 2030 and contribute to the common "
                "development of mankind, the president said.Su Ge, president of the China "
                " Institute of International Studies, said the consensus shows that China,"
                " as the host and a developing country, has played a significant role in "
                "contributing its ideas to global governance. It has the distinctive stamp "
                " of China, although the outcomes result from contributions of all G20 "
                " members and international institutions, Su said, adding that the summit "
                " was not a Chinese solo or duet, but rather a symphony. He said the "
                "outcomes of the G20 Summit reflect the direction of the G20 reform in "
                " serving the common interests of developed and developing countries. "
                "The rise of the G20 comes from the failure of traditional organizations, "
                "such as the IMF and the World Bank, in handling global economic and "
                "financial crises, said Zhu Jiejin, a researcher of global governance "
                "studies at Fudan University in Shanghai. The G20 mechanism is quite "
                "flexible, and since the world economic situation often changes, "
                "flexibility is the biggest advantage of the G20 in handling crisis, "
                "Zhu said. Analysts said the summit reflects a change of China's role in "
                "global governance from a participator to a lead reformer. China has given "
                " the prescription that the world economies should not form small interest "
                " groups, but should use the G20 as a common governance platform to step "
                "toward a community of common destiny, said Wei Jianguo, vice-president "
                "of the China Center for International Economic Exchanges. Contact the "
                "writer at zhangyunbi@chinadaily.com.cn");
        const int32_t paperLen = paper.length();
        fprintf(stdout, "%s\n\n", paper.c_str());

#define SIZE 1024
        RingBuffer rb(SIZE);
        std::string result;
        for (int times = 0; times < 10; ++times) {
            rb.reset();
            result.clear();

            for (int32_t i = 0; i < paperLen;) {
                int32_t w = rand() % SIZE;
                if (w == 0) w = 1;
                if (i+w >= paperLen) w = paperLen-i;
                int wLen = rb.write(paper.c_str()+i, w);
                i += wLen;

                int32_t r = rand() % SIZE;
                if (r == 0) r = 6;
                static char buf[SIZE];
                int rLen = rb.read(buf, r);
                result += std::string(buf, rLen);
            }
            char last[SIZE] = {0};
            int lastReadLen = rb.read(last, SIZE);
            result += std::string(last, lastReadLen);
            if (result != paper) {
                fprintf(stdout, "%s\n", result.c_str());
                assert(0);
            }
        }
    }
};

} // namespace DS
