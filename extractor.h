#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <ostream>
#include <vector>

template<typename T>
class Extractor {
    public:
        Extractor() : in_frame(4096) {};

        /**
         * @return True if should continue else false
         */
        virtual bool extract(std::ostream &data) = 0;

        const std::vector<T>& input() const { return in_frame; }
        std::vector<T>& input() { return in_frame; }

    protected:
        virtual void output_bit(std::ostream& out, char bit) {
            c |= bit << bit_idx++;

            if (bit_idx == 8) {
                if (c == '\0')
                    return;

                out.put(c);
                c = 0;
                bit_idx = 0;
            }
        }

        std::vector<T> in_frame;
        //std::ostream& data;
    private:
        int bit_idx = 0;
        char c = 0;
};

#endif
