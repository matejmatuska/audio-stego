#ifndef EMBEDDER_H
#define EMBEDDER_H

#include <istream>
#include <vector>

template<typename T>
class Embedder {
    public:
        Embedder(std::istream &data)
            : data(data), in_frame(4096), out_frame(4096){};

        virtual void embed() = 0;

        const std::vector<T> &input() const { return in_frame; }
        const std::vector<T> &output() const { return out_frame; }
        std::vector<T> &input() { return in_frame; }
        std::vector<T> &output() { return out_frame; }

    protected:
        virtual char get_bit() {
            if (bit_idx == 0) {
                if (!this->data.get(c))
                    return 0;
            }

            char bit = (c >> bit_idx) & (unsigned) 1;
            bit_idx = (bit_idx + 1) % 8;
            return bit;
        }

        std::istream& data;
        std::vector<T> in_frame;
        std::vector<T> out_frame;
    private:
        int bit_idx = 0;
        char c = 0;
};

#endif
