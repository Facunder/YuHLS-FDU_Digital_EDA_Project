#include "ControlFlowGraph.h"

#define BIND_WIDTH 32
#define RAM_DEPTH 32

class parser;
class HLS {
public:
    HLS(parser &p) : llvm_parser_(p){    }
    void genCDGF();
    // Scheduling
	void scheduling();
	// Resource binding
	void registerAllocationAndBinding();
	// void calculateAllocationAndBinding();
	void controlLogicSynthesis();
    // Write out RTL code
	void writeOutFile();
    
private:
    parser& llvm_parser_;
    ControlFlowGraph control_flow_graph_;
    std::vector<std::vector<std::pair<std::string, int>>> register_map_;


};