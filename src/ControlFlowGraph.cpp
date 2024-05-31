#include "ControlFlowGraph.h"

void ControlFlowGraph::init(parser&p){
    std::vector<basic_block> bbs = p.get_basic_blocks();
    std::vector<var> vars = p.get_function_params();
    for(int i = 0; i < vars.size(); i++) {
        input_parameters_.emplace_back(std::pair<std::string, int>(vars[i]._name, vars[i]._array_flag));
    }
    for(int i = 0; i < bbs.size(); i++) {
        DataFlowGraph tmp_graph;
        tmp_graph.initGraph(bbs[i]);
        data_flow_graphs_.emplace_back(tmp_graph);
        if(i > 0 && data_flow_graphs_[i-1].nextBlockLabel().empty()) {
            data_flow_graphs_[i-1].nextBlockLabel().emplace_back(data_flow_graphs_[i].blockLabel());
        }
    }     
    if(data_flow_graphs_[bbs.size()-1].nextBlockLabel().empty()) {
        data_flow_graphs_[bbs.size()-1].nextBlockLabel().emplace_back(data_flow_graphs_[0].blockLabel());
    }
    ret_type_ = p.get_ret_type();
    blocks_num_ = bbs.size();
}

void ControlFlowGraph::regsBinding(){
    all_regs_num_ = 0;
    block_output_regs_.clear();
    for(int i = 0; i < data_flow_graphs_.size(); i++) {
        for(int j = 0 ;j < data_flow_graphs_[i].register_binding_.size(); j++){
            all_regs_num_ = all_regs_num_ < data_flow_graphs_[i].register_binding_[j].size() ? data_flow_graphs_[i].register_binding_[j].size() : all_regs_num_;
        }
        int latency = data_flow_graphs_[i].latency();
        for(auto it : data_flow_graphs_[i].register_binding_[latency-1]) {
            block_output_regs_.insert(it);
        }
    }
    std::cout<<"all_regs_num_: "<<all_regs_num_<<std::endl;
    for(auto it : block_output_regs_) {
        std::cout<<" "<<it;
    }
    std::cout<<std::endl;
}