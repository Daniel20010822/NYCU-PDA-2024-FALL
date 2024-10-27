#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include <random>
using namespace std;


#define DISPLAY_PROGRESS 1
#define DRAW             0

#define DEBUG 0  // Set to 0 to disable debugging
#if DEBUG == 1
    #define debug std::cout
#elif DEBUG == 2
    std::ofstream debugFile("debug.txt");
    #define debug debugFile
#else
    #define debug if (false) std::cout
#endif


typedef default_random_engine RandomEngine;
// unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
unsigned seed = 964175062;
RandomEngine engine(seed);

class XYCoord {
public:
    int x = 0;
    int y = 0;
    XYCoord (int inX = 0, int inY = 0) : x(inX), y(inY) {}
};

class Terminal {
public:
    string name;
    XYCoord pos = {0,0};
    Terminal (string name, int x, int y) : name(name), pos(x,y) {}
};

class Macro {
public:
    string name;
    XYCoord LB;
    XYCoord UR;
    int width;
    int height;
    Macro(string name, int width = 0, int height = 0, int x = 0, int y = 0) :
        name(name), LB(x,y), width(width), height(height) { UR = {x + width, y + height}; }
};

class Net {
public:
    vector<Macro *> macros;
    vector<Terminal *> terminals;
    int HPWL = 0;
    Net() {}
};

class SequencePair {
private:
    int oW, oH;
    int numBlocks = 0, numTerminals = 0, numNets = 0;
    double alpha;
    int    blockArea = 0;
    int    AREA = 0;
    int    HPWL = 0;
    int    COST = 0;
    int    minX, minY, maxX, maxY;



    vector<Net *>             nets;
    vector<Terminal *>        terminals;
    vector<Macro *>           macros;
    vector<string>            posLoci;
    vector<string>            negLoci;

    unordered_map<string, Terminal *>   name2terminal;
    unordered_map<string, Macro *>      name2macro;
    unordered_map<string, int>          posName2negIdx;

    void parse_block(string filename);
    void parse_nets(string filename);
    void OP1(int idx1, int idx2);
    void OP2(int idx1, int idx2);
    void OP3(int idx1, int idx2);
    void OP4(int idx1);
    void SA_Init();
    void SA_FitOutline(chrono::time_point<chrono::high_resolution_clock> start);
    void SA_Optimize(chrono::time_point<chrono::high_resolution_clock> start);
    int get_Area();
    int get_DeadSpace();
    int get_HPWL();
    int get_RealCost(int area, int HPWL);
    double get_Cost(int area, int deadSpace, int HPWL, int initArea, int initDeadSpace, int initHPWL);
    void seq_to_FP();

public:
    SequencePair(double alpha, string f_block, string f_nets);
    void solve();
    void write_output(string filename, double runtime);
    void draw_all_macros(string filename);
};

SequencePair::SequencePair(double alpha, string f_block, string f_nets) {
    this->alpha = alpha;
    this->parse_block(f_block);
    this->parse_nets(f_nets);
}
void SequencePair::parse_block(string filename) {
    ifstream blockFile(filename);
    string temp;

    blockFile >> temp >> this->oW >> this->oH;
    blockFile >> temp >> this->numBlocks;
    blockFile >> temp >> this->numTerminals;

    this->macros.resize(this->numBlocks);
    this->posLoci.resize(this->numBlocks);
    this->negLoci.resize(this->numBlocks);
    this->terminals.resize(this->numTerminals);

    for (int i = 0; i < this->numBlocks; i++) {
        string name;
        int w, h;
        blockFile >> name >> w >> h;
        Macro *newMacro = new Macro(name, w, h);
        this->macros[i]  = newMacro;
        this->posLoci[i] = name;
        this->negLoci[i] = name;
        this->name2macro[name] = newMacro;
        this->blockArea += w*h;
    }

    for (int i = 0; i < this->numTerminals; i++) {
        string name;
        int x, y;
        blockFile >> name >> temp >> x >> y;
        Terminal *newTerminal = new Terminal(name, x, y);
        this->terminals[i] = newTerminal;
        this->name2terminal[name] = newTerminal;
    }

    blockFile.close();
}
void SequencePair::parse_nets(string filename) {
    ifstream netsFile(filename);

    string temp;
    netsFile >> temp >> this->numNets;
    for (int i = 0; i < this->numNets; i++) {
        int netDegree;
        netsFile >> temp >> netDegree;
        Net *newNet = new Net();
        for (int j = 0; j < netDegree; j++) {
            string net;
            netsFile >> net;

            bool found = false;
            for (auto it = this->name2macro.begin(); it != name2macro.end(); it++) {
                if (it->first == net) {
                    newNet->macros.emplace_back(it->second);
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (auto it = this->name2terminal.begin(); it != name2terminal.end(); it++) {
                    if (it->first == net) {
                        newNet->terminals.emplace_back(it->second);
                        break;
                    }
                }
            }
        }
        nets.emplace_back(newNet);
    }
    netsFile.close();
}
void SequencePair::solve() {
    auto start = chrono::high_resolution_clock::now();
    SA_Init();
    if (DRAW) {
        draw_all_macros("final_result.txt");
    }
    SA_FitOutline(start);
    cout << endl;
    SA_Optimize(start);
    cout << endl;
    if (DRAW) {
        draw_all_macros("final_result.txt");
    }
}
void SequencePair::SA_Init() {
    // SA_Init: Shuffle the vectors with different engines
    shuffle(posLoci.begin(), posLoci.end(), engine);
    shuffle(negLoci.begin(), negLoci.end(), engine);

    // auto widthComparator = [this](const string &a, const string &b) {
    //     Macro *macroA = name2macro[a];
    //     Macro *macroB = name2macro[b];
    //     return macroA->width > macroB->width;
    // };
    // auto heightComparator = [this](const string &a, const string &b) {
    //     Macro *macroA = name2macro[a];
    //     Macro *macroB = name2macro[b];
    //     return macroA->height > macroB->height;
    // };
    // sort(posLoci.begin(), posLoci.end(), widthComparator);
    // sort(negLoci.begin(), negLoci.end(), heightComparator);

    // Update posName2negIdx
    for (int i = 0; i < numBlocks; i++) {
        string target = posLoci[i];
        for (int j = 0; j < numBlocks; j++) {
            if (negLoci[j] == target) {
                posName2negIdx[target] = j;
                break;
            }
        }
    }

    seq_to_FP();
    this->AREA = get_Area();
    this->HPWL = get_HPWL();
    this->COST = get_RealCost(this->AREA, this->HPWL);
}
void SequencePair::SA_FitOutline(chrono::time_point<chrono::high_resolution_clock> start) {
    // Total time limit
    int maxTime      = 295;

    // SA parameters
    int maxIteration = 200;
    double r         = 0.95;
    double T         = 100;
    // double Tmin      = 1e-6;

    // Initial values
    bool isCurrentBest = false;
    int totalRnd     = 0;
    int failRnd      = 0;
    int successRnd   = 0;

    // Calculate Exceed area (Cost)
    int exceedW = (this->maxX > this->oW) ? (this->maxX - this->oW) : 0;
    int exceedH = (this->maxY > this->oH) ? (this->maxY - this->oH) : 0;
    int exceedArea = this->oH*exceedW + this->oW*exceedH;

    // Create a uniform distributions
    uniform_int_distribution<int> randAction(1, 4);
    uniform_int_distribution<int> randCand1(0, numBlocks - 1);
    uniform_int_distribution<int> randCand2(0, numBlocks - 1);
    uniform_real_distribution<>   prob(0.0, 1.0);

    // Create another macro vector to backup those data
    vector<Macro *> macrosClone(numBlocks);
    for (int i = 0; i < numBlocks; i++) {
        macrosClone[i] = new Macro(
            this->macros[i]->name,
            this->macros[i]->width,
            this->macros[i]->height,
            this->macros[i]->LB.x,
            this->macros[i]->LB.y
        );
    }

    // while (T > Tmin) {
    while (exceedArea > 0) {
        for (int iter = 0; iter < maxIteration; iter++) {
            // Generate a neighbor
            int action = randAction(engine);
            int idx1   = randCand1(engine);
            int idx2   = randCand2(engine);

            // Clone it if previous is downhill move for later backup
            if (isCurrentBest) {
                for (int i = 0; i < numBlocks; i++) {
                    macrosClone[i]->name   = this->macros[i]->name;
                    macrosClone[i]->LB     = this->macros[i]->LB;
                    macrosClone[i]->UR     = this->macros[i]->UR;
                    macrosClone[i]->width  = this->macros[i]->width;
                    macrosClone[i]->height = this->macros[i]->height;
                }
                isCurrentBest = false;
            }

            switch (action) {
            case 1:
                debug << "OP 1: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                OP1(idx1, idx2);
                break;
            case 2:
                debug << "OP 2: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                OP2(idx1, idx2);
                break;
            case 3:
                debug << "OP 3" << endl;
                OP3(idx1, idx2);
                break;
            case 4:
                debug << "OP 4" << endl;
                OP4(idx1);
                break;
            default:
                break;
            }

            // Calculate new cost
            seq_to_FP();
            int newExceedW = (this->maxX > this->oW) ? (this->maxX - this->oW) : 0;
            int newExceedH = (this->maxY > this->oH) ? (this->maxY - this->oH) : 0;
            int newExceedArea = this->oH*newExceedW + this->oW*newExceedH;
            double deltaCost = newExceedArea - exceedArea;

            // Downhill move & Uphill move (accepted)
            if (deltaCost < 0 || exp(-deltaCost / T) > prob(engine)) {
                this->AREA = this->get_Area();
                this->HPWL = this->get_HPWL();
                this->COST = get_RealCost(this->AREA, this->HPWL);
                exceedArea = newExceedArea;
                isCurrentBest = true;
                successRnd += 1;
                debug << "Current cost = " << this->COST << endl;
            }
            // Uphill move (not accepted)
            else {
                // Revert to previous solution
                switch (action) {
                case 1:
                    debug << "Reverse 1: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                    OP1(idx1, idx2);
                    break;
                case 2:
                    debug << "Reverse 2: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                    OP2(idx1, idx2);
                    break;
                case 3:
                    debug << "Reverse 3" << endl;
                    OP3(idx1, idx2);
                    break;
                case 4:
                    debug << "Reverse 4" << endl;
                    OP4(idx1);
                    break;
                default:
                    break;
                }

                // Restore previous state
                for (int i = 0; i < numBlocks; i++) {
                    this->macros[i]->name   = macrosClone[i]->name;
                    this->macros[i]->LB     = macrosClone[i]->LB;
                    this->macros[i]->UR     = macrosClone[i]->UR;
                    this->macros[i]->width  = macrosClone[i]->width;
                    this->macros[i]->height = macrosClone[i]->height;
                }

                failRnd += 1;

            }
            totalRnd += 1;

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            if (elapsed.count() > maxTime)
                return;

            // system("clear");
            if (DISPLAY_PROGRESS == 1 && totalRnd % 10 == 0) {
                cout << "\r"
                     << "T = " << setw(8) << setprecision(6) << T
                     << " | Cost = " << setw(10) << setprecision(4) << fixed << exceedArea
                     << " | " << failRnd << "/" << totalRnd
                     << " | " << (int)elapsed.count() << " sec" << flush;
            }

        }

        if (exceedArea == 0) {
            break;
        }

        // Cool down the temperature
        T *= r;
    }
}
void SequencePair::SA_Optimize(chrono::time_point<chrono::high_resolution_clock> start) {
    // Total time limit
    int maxTime      = 297;

    // SA parameters
    int maxIteration       = 1000;
    const double r         = 0.95;
    double T               = 1;
    const double Tmin      = 1e-7;

    // Initial values
    bool isCurrentBest = false;
    int totalRnd     = 0;
    int failRnd      = 0;
    int successRnd   = 0;

    // Initial Cost
    int initArea = get_Area();
    int initHPWL = get_HPWL();
    int initDeadSpace = get_DeadSpace();
    double currCost = get_Cost(initArea, initDeadSpace, initHPWL, initArea, initDeadSpace, initHPWL);

    // Create a uniform distributions
    uniform_int_distribution<int> randAction(1, 4);
    uniform_int_distribution<int> randCand1(0, numBlocks - 1);
    uniform_int_distribution<int> randCand2(0, numBlocks - 1);
    uniform_real_distribution<>   prob(0.0, 1.0);

    // Create another macro vector to backup those data
    vector<Macro *> macrosClone(numBlocks);
    for (int i = 0; i < numBlocks; i++) {
        macrosClone[i] = new Macro(
            this->macros[i]->name,
            this->macros[i]->width,
            this->macros[i]->height,
            this->macros[i]->LB.x,
            this->macros[i]->LB.y
        );
    }


    while (T > Tmin) {
        for (int iter = 0; iter < maxIteration; iter++) {
            // Generate a neighbor
            int action = randAction(engine);
            int idx1   = randCand1(engine);
            int idx2   = randCand2(engine);

            // Clone it if previous is downhill move for later backup
            if (isCurrentBest) {
                for (int i = 0; i < numBlocks; i++) {
                    macrosClone[i]->name   = this->macros[i]->name;
                    macrosClone[i]->LB     = this->macros[i]->LB;
                    macrosClone[i]->UR     = this->macros[i]->UR;
                    macrosClone[i]->width  = this->macros[i]->width;
                    macrosClone[i]->height = this->macros[i]->height;
                }
                isCurrentBest = false;
            }

            switch (action) {
            case 1:
                debug << "OP 1: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                OP1(idx1, idx2);
                break;
            case 2:
                debug << "OP 2: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                OP2(idx1, idx2);
                break;
            case 3:
                debug << "OP 3" << endl;
                OP3(idx1, idx2);
                break;
            case 4:
                debug << "OP 4" << endl;
                OP4(idx1);
                break;
            default:
                break;
            }

            // Calculate new cost
            seq_to_FP();
            int newArea = get_Area();
            int newHPWL = get_HPWL();
            int newDeadSpace = get_DeadSpace();
            double newCost = get_Cost(newArea, newDeadSpace, newHPWL, initArea, initDeadSpace, initHPWL);
            double deltaCost = newCost - currCost;

            // Downhill move & Uphill move (accepted)
            if ((deltaCost < 0 || exp(-deltaCost*1000 / T) > prob(engine)) && (this->maxX < this->oW && this->maxY < this->oH)) {
                this->AREA = newArea;
                this->HPWL = newHPWL;
                this->COST = get_RealCost(newArea, newHPWL);
                currCost = newCost;
                isCurrentBest = true;
                successRnd += 1;
                debug << "Current cost = " << this->COST << endl;
            }
            // Uphill move (not accepted)
            else {
                // Revert to previous solution
                switch (action) {
                case 1:
                    debug << "Reverse 1: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                    OP1(idx1, idx2);
                    break;
                case 2:
                    debug << "Reverse 2: " << posLoci[idx1] << " " << posLoci[idx2] << endl;
                    OP2(idx1, idx2);
                    break;
                case 3:
                    debug << "Reverse 3" << endl;
                    OP3(idx1, idx2);
                    break;
                case 4:
                    debug << "Reverse 4" << endl;
                    OP4(idx1);
                    break;
                default:
                    break;
                }

                // Restore previous state
                for (int i = 0; i < numBlocks; i++) {
                    this->macros[i]->name   = macrosClone[i]->name;
                    this->macros[i]->LB     = macrosClone[i]->LB;
                    this->macros[i]->UR     = macrosClone[i]->UR;
                    this->macros[i]->width  = macrosClone[i]->width;
                    this->macros[i]->height = macrosClone[i]->height;
                }

                failRnd += 1;

            }
            totalRnd += 1;

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = end - start;

            if (elapsed.count() > maxTime)
                return;

            // system("clear");
            if (DISPLAY_PROGRESS == 1 && totalRnd % 10 == 0) {
                cout << "\r"
                     << "T = " << setw(9) << setprecision(7) << T
                     << " | Cost = " << setw(10) << setprecision(4) << fixed << currCost
                     << " | " << failRnd << "/" << totalRnd
                     << " | " << (int)elapsed.count() << " sec" << flush;
            }
        }

        // Cool down the temperature
        T *= r;
    }
}
void SequencePair::OP1(int idx1, int idx2) {
    string temp = posLoci[idx1];
    posLoci[idx1] = posLoci[idx2];
    posLoci[idx2] = temp;
}
void SequencePair::OP2(int idx1, int idx2) {
    string macro1 = posLoci[idx1];
    string macro2 = posLoci[idx2];

    // Swap negLoci
    int nidx1 = 0, nidx2 = 0;
    for (int i = 0; i < numBlocks; i++) {
        if (negLoci[i] == macro1) nidx1 = i;
        if (negLoci[i] == macro2) nidx2 = i;
    }
    string temp = negLoci[nidx1];
    negLoci[nidx1] = negLoci[nidx2];
    negLoci[nidx2] = temp;

    // Swap map
    posName2negIdx[macro1] = nidx2;
    posName2negIdx[macro2] = nidx1;
}
void SequencePair::OP3(int idx1, int idx2) {
    string macro1 = posLoci[idx1];
    string macro2 = posLoci[idx2];

    // Swap posLoci
    string temp = posLoci[idx1];
    posLoci[idx1] = posLoci[idx2];
    posLoci[idx2] = temp;

    // Swap negLoci
    int nidx1 = 0, nidx2 = 0;
    for (int i = 0; i < numBlocks; i++) {
        if (negLoci[i] == macro1) nidx1 = i;
        if (negLoci[i] == macro2) nidx2 = i;
    }
    temp = negLoci[nidx1];
    negLoci[nidx1] = negLoci[nidx2];
    negLoci[nidx2] = temp;

    // Swap map
    posName2negIdx[macro1] = nidx2;
    posName2negIdx[macro2] = nidx1;
}
void SequencePair::OP4(int idx1) {
    string name = posLoci[idx1];
    Macro *macro = name2macro[name];
    int w = macro->width;
    int h = macro->height;
    macro->width = h;
    macro->height = w;
    macro->UR.x = macro->LB.x + macro->width;
    macro->UR.y = macro->LB.y + macro->height;
}

void SequencePair::seq_to_FP() {
    // Initialize every macro: place at the origin
    for (auto macro: macros) {
        macro->LB.x = 0;
        macro->LB.y = 0;
        macro->UR.x = macro->width;
        macro->UR.y = macro->height;
    }
    // Initialize boundaries
    this->maxX = 0;
    this->maxY = 0;
    this->minX = numeric_limits<int>::max();
    this->minY = numeric_limits<int>::max();

    // (posLoci, negLoci) -> Floorplan
    for (int p_idx0 = 0; p_idx0 < numBlocks - 1; p_idx0++) {
        for (int p_idx1 = p_idx0 + 1; p_idx1 < numBlocks; p_idx1++) {
            Macro *macro0 = this->name2macro[this->posLoci[p_idx0]];
            Macro *macro1 = this->name2macro[this->posLoci[p_idx1]];
            int n_idx0 = this->posName2negIdx[macro0->name];
            int n_idx1 = this->posName2negIdx[macro1->name];
            if (n_idx0 < n_idx1 && macro1->LB.x < macro0->UR.x) {
                macro1->LB.x = macro0->UR.x;
                macro1->UR.x = macro0->UR.x + macro1->width;
            }
            else if (n_idx0 > n_idx1 && macro1->LB.y < macro0->UR.y) {
                macro1->LB.y = macro0->UR.y;
                macro1->UR.y = macro0->UR.y + macro1->height;
            }
        }
    }

    // Update boundaries
    for (auto macro: this->macros) {
        if (macro->UR.x > this->maxX) this->maxX = macro->UR.x;
        if (macro->UR.y > this->maxY) this->maxY = macro->UR.y;
        if (macro->LB.x < this->minX) this->minX = macro->LB.x;
        if (macro->LB.y < this->minY) this->minY = macro->LB.y;
    }
}
int SequencePair::get_RealCost(int area, int HPWL) {
    return this->alpha*area + (1 - this->alpha)*HPWL;
}
double SequencePair::get_Cost(int area, int deadSpace, int HPWL, int initArea, int initDeadSpace, int initHPWL) {
    double normArea       = (area > 0)       ? (double)area       / (double)initArea : 0;
    double normHPWL       = (HPWL > 0)       ? (double)HPWL       / (double)initHPWL : 0;
    double normDeadSpace  = (deadSpace > 0)  ? (double)deadSpace  / (double)initDeadSpace : 0;
    return this->alpha*normArea + (1 - this->alpha)*normHPWL + normDeadSpace;
}
int SequencePair::get_Area() {
    return (this->maxX - this->minX)*(this->maxY - this->minY);
}
int SequencePair::get_DeadSpace() {
    int deadSpace = (this->maxX*this->maxY) - this->blockArea;
    if (deadSpace < 0) deadSpace = 0;
    return deadSpace;
}
int SequencePair::get_HPWL() {
    int totalHPWL = 0;
    for (int i = 0; i < numNets; i++) {
        int maxX = 0;
        int maxY = 0;
        int minX = numeric_limits<int>::max();
        int minY = numeric_limits<int>::max();
        Net *currNet = this->nets[i];
        for (auto macro: currNet->macros) {
            int xc = macro->LB.x + macro->width  / 2;
            int yc = macro->LB.y + macro->height / 2;
            if (xc < minX) minX = xc;
            if (xc > maxX) maxX = xc;
            if (yc < minY) minY = yc;
            if (yc > maxY) maxY = yc;
        }
        for (auto terminal: currNet->terminals) {
            if (terminal->pos.x < minX) minX = terminal->pos.x;
            if (terminal->pos.x > maxX) maxX = terminal->pos.x;
            if (terminal->pos.y < minY) minY = terminal->pos.y;
            if (terminal->pos.y > maxY) maxY = terminal->pos.y;
        }
        currNet->HPWL  = ((int)maxX - (int)minX) + ((int)maxY - (int)minY);
        totalHPWL     += ((int)maxX - (int)minX) + ((int)maxY - (int)minY);
    }
    return totalHPWL;
}
void SequencePair::write_output(string filename, double runtime) {
    ofstream f_output(filename);

    f_output << this->COST << endl
             << this->HPWL << endl
             << this->AREA << endl
             << this->oW << " " << this->oH << endl
             << runtime << endl;
    for (auto macro: this->macros) {
        f_output << macro->name << " "
                 << macro->LB.x << " "
                 << macro->LB.y << " "
                 << macro->UR.x << " "
                 << macro->UR.y << endl;
    }

    f_output.close();
}

void SequencePair::draw_all_macros(string filename) {
    ofstream f_layout(filename);
    f_layout << this->oW << " " << this->oH << endl;
    f_layout << this->numBlocks << endl;
    for (int i = 0; i < this->numBlocks; i++) {
        f_layout << this->macros[i]->name << " "
                 << this->macros[i]->LB.x << " "
                 << this->macros[i]->LB.y << " "
                 << this->macros[i]->width << " "
                 << this->macros[i]->height << endl;
    }
    f_layout << this->numTerminals << endl;
    for (int i = 0; i < this->numTerminals; i++) {
        f_layout << this->terminals[i]->name << " "
                 << this->terminals[i]->pos.x << " "
                 << this->terminals[i]->pos.y << endl;
    }
    f_layout.close();
}



int main (int argc, char** argv) {

    if (argc < 5) {
        cerr << "Invalid input arguments!" << endl;
        return 1;
    }

    SequencePair SP(stod(argv[1]), argv[2], argv[3]);

    auto start = chrono::high_resolution_clock::now();
    SP.solve();
    // cout << "SEED = " << seed << endl;

    auto end   = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    SP.write_output(argv[4], elapsed.count());

    return 0;
}
