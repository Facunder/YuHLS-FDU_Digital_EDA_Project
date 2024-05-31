#include "DataFlowGraph.h"

class ControlFlowGraph {
public:
    ControlFlowGraph(){};
    void init(parser&p);
    void regsBinding();

    int getRegsNum() { return all_regs_num_; }
    int getRetType() { return ret_type_; }
    int getBlocksNum() { return blocks_num_; }
    std::set<std::string> getBlockOutputRegsName() { return block_output_regs_; }
    std::vector<DataFlowGraph> &getDataFlowGraphs() { return data_flow_graphs_; }
    std::vector<std::string> getStateCodes() { return state_codes_; }
    std::vector<std::pair<std::string, int>> getInputParams() { return input_parameters_; }

private:
    int all_regs_num_;
    int ret_type_;
    int blocks_num_;
    std::vector<std::pair<std::string, int>> input_parameters_; // <name, array_flag>
    std::set<std::string> block_output_regs_;
    std::vector<DataFlowGraph> data_flow_graphs_;
    std::vector<std::string> state_codes_;
};