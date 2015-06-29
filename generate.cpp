#include "model.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

markov::model_t
load_model(std::istream &input) {
    markov::model_t model;

    boost::archive::text_iarchive deserializator(input);

    deserializator >> model;

    return model;
}

std::string
read_file(std::istream &input) {
    std::stringstream buffer;
    buffer << input.rdbuf();

    return buffer.str();
}

void
print_words(std::ostream &output, const std::vector<std::string> &words) {
    for (const auto& word: words) {
        output << word << " ";
    }
}

int
main(int argc, char *argv[]) {
    std::ifstream input;
    std::ifstream model_file;
    unsigned int to_generate = 0;

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <text-file> <words-to-generate> <model-file>" << std::endl;
        return 1;
    } else {
        input.open(argv[1]);

        if (!input) {
            std::cerr << "Unable to open the input file: " << argv[1] << std::endl;
            return 1;
        }

        try {
            to_generate = boost::lexical_cast<unsigned int>(argv[2]);
        } catch (const boost::bad_lexical_cast&) {
            std::cerr << "Invalid number of words: " << argv[2] << std::endl;
            return 1;
        }

        model_file.open(argv[3]);

        if (!model_file) {
            std::cerr << "Unable to open the model file: " << argv[3] << std::endl;
            return 1;
        }
    }

    markov::model_t model = load_model(model_file);
    auto text = markov::extract_words(input);

    std::vector<std::string> generated;

    if (!model.extend(text, to_generate, generated)) {
        std::cerr << "Cannot continue the text using the given model. "
                  << "Consider using bigger training text with more complete vocabulary."
                  << std::endl;
        return 1;
    }

    print_words(std::cout, text);
    print_words(std::cout, generated);

    std::cout << std::endl;

    return 0;
}
