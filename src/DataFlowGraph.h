#include "parser.h"

// class DataFlowEdge{
// public:
//     DataFlowEdge* next(){
//         return next_edge_;
//     }
// private:
//     int end_node_index_;
//     DataFlowEdge * next_edge_;
// }

class DataFlowNode{
public:
    DataFlowNode(){
        index_ = -1;
        childs_idx_.clear();
    }
    void setIndex(int i){index_ = i;}
    int index(){return index_;}
    // void setFirstEdge(DataFlowEdge* e){first_edge_ = e;}

    int op_type_;
    std::string var_;
    std::vector<std::string> oprands_;
    int sched_ = -1;
    int indegree_ = 0;
    std::vector<int> childs_idx_;
private:
    int index_; 
};

class DataFlowGraph{
public:
    DataFlowGraph() {
        num_nodes_ = 0;
        num_edges_ = 0;
        latency_ = 0;
        data_flow_nodes_.clear();
    }
    void initGraph(basic_block &bb);
    int numNodes() { return num_nodes_; }
    int numEdges() { return num_edges_; }
    int latency() { return latency_; }
    void setLatency(int l) { latency_ = l; }
    std::string stateCode() { return state_code_; }
    void setStateCode(std::string s) { state_code_ = s; }
    std::string blockLabel() { return block_label_; }
    std::string brCondition() { return br_condition_; }
    std::vector<std::string>& nextBlockLabel() { return next_block_label_; }
    std::vector<DataFlowNode>& getDataFlowNodes() { return data_flow_nodes_; }
    std::vector<std::set<std::string>> register_binding_;
    // name to register number
    std::vector<std::map<std::string, int>> register_mapping_;
    // if some vars are required as oprands in next blocks
    std::vector<std::string> output_values_;
    std::vector<std::string> input_values_;
    // sched result
    std::vector<std::vector<int>> sched_res_;
private:
    std::string block_label_;
    std::string state_code_;
    std::string br_condition_ = " ";
    std::vector<std::string> next_block_label_;
    int num_nodes_, num_edges_;
    int latency_;
    std::vector<DataFlowNode> data_flow_nodes_; 
};