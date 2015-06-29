#ifndef MODEL_HPP
#define MODEL_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <deque>
#include <vector>

#include <boost/container/map.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>

// Boost.Serialization doesn't support Boost.Containers :(
namespace boost { namespace serialization {

template<class Archive, class Key, class Value>
void
save(Archive &ar, const boost::container::map<Key, Value> &m, const unsigned int /* version */) {
    size_t size = m.size();
    ar << size;

    for (const auto& kv: m) {
        ar << kv.first;
        ar << kv.second;
    }
}

template<class Archive, class Key, class Value>
void
load(Archive &ar, boost::container::map<Key, Value> &m, const unsigned int /* version */) {
    size_t size = 0;
    ar >> size;

    m.clear();

    for (size_t i = 0; i < size; ++i) {
        Key key;
        Value value;

        ar >> key;
        ar >> value;

        m[key] = value;
    }
}

template<class Archive, class Key, class Value>
inline void
serialize(Archive &ar, boost::container::map<Key, Value> &m, const unsigned int version) {
    split_free(ar, m, version);
}

}} // namespace boost::serialization

namespace markov {

std::vector<std::string>
extract_words(std::istream& input) {
    std::vector<std::string> result;

    std::string current_word;

    while (true) {
        // May be EOF.
        std::istream::int_type current_char = input.get();

        if (input.eof()) {
            if (!current_word.empty()) {
                result.push_back(current_word);
            }
            break;
        }

        if (!std::isspace(current_char)) {
            current_word.push_back(std::tolower(current_char));
        } else if (!current_word.empty()) {
            result.push_back(current_word);
            current_word.clear();
        }
    }

    return result;
}

class model_t {
    struct node_t {
        node_t() :
            count(0)
        { }

        template<class Archive>
        void
        serialize(Archive &ar, const unsigned int /* version */) {
            ar & count;
            ar & children;
        }

        unsigned int count;

        // We use boost::container::map to create a recursive data structure.
        // The standard containers cannot be instantiated with incomplete types.
        boost::container::map<std::string, node_t> children;
    };

public:
    explicit
    model_t(unsigned int order = 1) :
        m_order(order)
    { }

    void
    train(const std::vector<std::string>& text) {
        for (auto sample_start = text.begin(); sample_start != text.end(); ++sample_start) {
            node_t *current_node = &m_frequences;
            ++current_node->count;

            for (auto sample_it = sample_start;
                 sample_it - sample_start < m_order + 1 && sample_it != text.end();
                 ++sample_it)
            {
                current_node = &current_node->children[*sample_it];
                ++current_node->count;
            }
        }
    }

    void
    train(std::istream& input) {
        train(extract_words(input));
    }

    template<class Iterator>
    bool
    find_next_word(Iterator begin, Iterator end, std::string &result) const {
        const node_t *current_node = &m_frequences;

        for (auto it = begin; it != end; ++it) {
            if (current_node->children.count(*it) == 0) {
                return false;
            } else {
                current_node = &current_node->children.at(*it);
            }
        }

        if (current_node->children.empty()) {
            return false;
        }

        unsigned int max_count = 0;

        for (auto it = current_node->children.begin(); it != current_node->children.end(); ++it) {
            if (it->second.count > max_count) {
                result = it->first;
                max_count = it->second.count;
            }
        }

        return true;
    }

    bool
    extend(const std::vector<std::string> &text,
           size_t extension_size,
           std::vector<std::string> &result) const
    {
        auto chain_start = text.end() - std::min<size_t>(m_order, text.size());
        std::deque<std::string> chain(chain_start, text.end());

        unsigned int generated = 0;

        while (generated < extension_size) {
            if (chain.empty()) {
                return false;
            } else {
                std::string next_word;

                if (find_next_word(chain.begin(), chain.end(), next_word)) {
                    result.push_back(next_word);
                    chain.push_back(next_word);
                    ++generated;

                    if (chain.size() > m_order) {
                        chain.pop_front();
                    }
                } else {
                    chain.pop_front();
                }
            }
        }

        return true;
    }

    bool
    extend(std::istream& input, size_t extension_size, std::vector<std::string> &result) const {
        return extend(extract_words(input), extension_size, result);
    }

    template<class Archive>
    void
    serialize(Archive &ar, const unsigned int /* version */) {
        ar & m_order;
        ar & m_frequences;
    }

    void
    print() const {
        print_impl(&m_frequences, 0);
    }

private:
    void
    print_impl(const node_t *current_node, unsigned int indent) const {
        std::string indent_string(indent, ' ');

        std::cout << indent_string << current_node->count << std::endl;

        for (const auto& kv_pair: current_node->children) {
            std::cout << indent_string << kv_pair.first << ": {" << std::endl;
            print_impl(&kv_pair.second, indent + 2);
            std::cout << indent_string << "}" << std::endl;
        }
    }

private:
    unsigned int m_order;
    node_t m_frequences;
};

} // namespace markov

#endif
