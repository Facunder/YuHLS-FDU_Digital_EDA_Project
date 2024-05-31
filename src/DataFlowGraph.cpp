#include "DataFlowGraph.h"

void DataFlowGraph::initGraph(basic_block &bb) {
    block_label_ = bb.get_label_name();
    std::vector<statement> ss = bb.get_statements();
    num_nodes_ = ss.size();
    data_flow_nodes_.resize(num_nodes_+2);
    data_flow_nodes_[0].setIndex(0); // start NOP node
    data_flow_nodes_[0].op_type_ = -2; // start NOP node
    std::cout<<"    initing DFG/block: "<<block_label_<<std::endl;
    for (int i = 0; i < num_nodes_; ++i) {
        int idx = i + 1;
        data_flow_nodes_[idx].setIndex(idx);
        data_flow_nodes_[idx].op_type_ = ss[i].get_type();
        if (ss[i].get_var() != "" && (ss[i].get_type() != OP_STORE || ss[i].get_type() != OP_RET)){
            data_flow_nodes_[idx].var_ = ss[i].get_var();
        } else {
            data_flow_nodes_[idx].var_ = " ";
        }   
        for (int k = 0; k < ss[i].get_num_oprands(); ++k){
            data_flow_nodes_[idx].oprands_.emplace_back(ss[i].get_oprand(k));
        }
        data_flow_nodes_[0].childs_idx_.emplace_back(idx);
        data_flow_nodes_[idx].indegree_++;
        // add DFG br info
        if (data_flow_nodes_[idx].op_type_ == 7) { //br
            if(data_flow_nodes_[idx].oprands_.size() > 1) {
                br_condition_ = data_flow_nodes_[idx].oprands_[0];
                next_block_label_.emplace_back(data_flow_nodes_[idx].oprands_[1]);
                next_block_label_.emplace_back(data_flow_nodes_[idx].oprands_[2]);
            } else {
                br_condition_ = " ";
                next_block_label_.emplace_back(data_flow_nodes_[idx].oprands_[0]);
            }
        }
    }
    data_flow_nodes_[num_nodes_+1].setIndex(num_nodes_+1); // end NOP node
    data_flow_nodes_[num_nodes_+1].op_type_ = -1; // end NOP node
    for (int i = 1; i <= num_nodes_; i++) {
        data_flow_nodes_[i].childs_idx_.emplace_back(num_nodes_+1);
        data_flow_nodes_[num_nodes_+1].indegree_++;
        std::string cur_var = data_flow_nodes_[i].var_;
        for (int j = i+1; j<= num_nodes_; j++) {
            if(data_flow_nodes_[j].op_type_ == 7 || //op type: br
            std::find(data_flow_nodes_[j].oprands_.begin(), data_flow_nodes_[j].oprands_.end(), cur_var)!=data_flow_nodes_[j].oprands_.end()){
                data_flow_nodes_[i].childs_idx_.emplace_back(j); //node_j depends on node_i
                data_flow_nodes_[j].indegree_++;
            }
        }
    }
    // finish DAG generation
    return;
}