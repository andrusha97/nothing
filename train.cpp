#include "model.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <iostream>
#include <fstream>

int
main(int argc, char *argv[]) {
    std::ifstream input;
    std::ofstream output;
    unsigned int order = 0;

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <training-file> <chain-order> <output-model-file>" << std::endl;
        return 1;
    } else {
        input.open(argv[1]);

        if (!input) {
            std::cerr << "Unable to open the input file: " << argv[1] << std::endl;
            return 1;
        }

        try {
            order = boost::lexical_cast<unsigned int>(argv[2]);
        } catch (const boost::bad_lexical_cast&) {
            std::cerr << "Invalid chain order: " << argv[2] << std::endl;
            return 1;
        }

        if (order == 0) {
            std::cerr << "Chain order must be positive." << std::endl;
            return 1;
        }

        output.open(argv[3]);

        if (!output) {
            std::cerr << "Unable to open the output file: " << argv[3] << std::endl;
            return 1;
        }
    }

    markov::model_t model(order);

    model.train(input);

    boost::archive::text_oarchive serializator(output);

    serializator << model;

    return 0;
}
