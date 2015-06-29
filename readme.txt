Tested with recent clang and recent boost.

Build:
mkdir build && cd build && cmake .. && make

Run `./train <text-file> <chain-order> <output-model-file>` to train the model.
`./generate <text-file> <number-of-words-to-generate> <model-file>` to generate new text.
