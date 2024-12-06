#ifndef _NET_H_
#define _NET_H_

#include "XYCoord.h"
#include <string>
#include <vector>

class Net {
private:
    std::string name;
    std::vector<std::string> paths;
public:
    Net(std::string inName) { this->name = inName; }
    ~Net();

    void setName(std::string inName) { this->name = inName; }
    std::string getName() const { return this->name; }

    void addPath(std::string inPath) { this->paths.push_back(inPath); }
    std::vector<std::string> getPaths() const { return this->paths; }
};

#endif