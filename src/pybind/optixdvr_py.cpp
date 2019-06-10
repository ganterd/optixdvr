#include <string>
#include <fstream>
#include <streambuf>
#include <pybind11/embed.h>

namespace py = pybind11;

int main(int argc, char** argv)
{

    py::scoped_interpreter guard{};

    std::string pyscript = argv[1];
    std::ifstream t(pyscript);
    std::string pystring((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    py::exec(pystring);
}