#include "HLS.h"

void HLS::genCDGF(){
    std::cout<<"Start generate CGF......"<<std::endl;
    control_flow_graph_.init(llvm_parser_);
}

void HLS::scheduling(){
    std::cout<<"Start scheduling......"<<std::endl;
    std::vector<DataFlowGraph>& data_flow_graphs = control_flow_graph_.getDataFlowGraphs();
    //std::vector<DataFlowGraph>& data_flow_graphs = control_flow_graph_.data_flow_graphs_;
    for (int i = 0; i < data_flow_graphs.size(); i++) {
        //ASAP
        int num_total_nodes = data_flow_graphs[i].numNodes() + 2;
        std::vector<int> visited_mark_vec(num_total_nodes, 0);
        std::queue<int> asap_node_idx_queue;
        std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].getDataFlowNodes();
        //std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].data_flow_nodes_;
        for (int k = 0; k < data_flow_nodes.size(); k++){
            if (data_flow_nodes[k].indegree_ == 0) {
                asap_node_idx_queue.push(data_flow_nodes[k].index());
                data_flow_nodes[k].sched_ = -1; // start NOP node
            }
        }
       
        while (!asap_node_idx_queue.empty()){
            int cur_node_idx = asap_node_idx_queue.front();
            asap_node_idx_queue.pop();
            visited_mark_vec[cur_node_idx] = 1;
            std::vector<int>& cur_childs_idx = data_flow_nodes[cur_node_idx].childs_idx_;          
            for (int j = 0; j < cur_childs_idx.size(); j++) {
                DataFlowNode& cur_node = data_flow_nodes[cur_childs_idx[j]];
                cur_node.indegree_--;
                if (cur_node.indegree_ == 0 && visited_mark_vec[cur_node.index()] == 0) {// TODO: check != 0 / == 0{
                    asap_node_idx_queue.push(cur_node.index());
                    cur_node.sched_ = data_flow_nodes[cur_node_idx].sched_ + 1;
                }
            }
        }
        // test print
        std::cout<<"block_label: "<<data_flow_graphs[i].blockLabel()<<std::endl;
        int latency = 0;
        for (int k = 0; k < data_flow_nodes.size(); k++){
            latency = latency > data_flow_nodes[k].sched_ ? latency : data_flow_nodes[k].sched_;
            std::cout << "  op type: " << data_flow_nodes[k].op_type_ 
                << " in sched No. " << (data_flow_graphs[i].getDataFlowNodes())[k].sched_ << std::endl;
        }
        data_flow_graphs[i].setLatency(latency);
        std::cout<<"  latency: "<<data_flow_graphs[i].latency()<<std::endl;
        data_flow_graphs[i].sched_res_.resize(latency);
        for(int k = 0; k < data_flow_nodes.size(); k++) {
            if(data_flow_nodes[k].sched_ >= 0 && data_flow_nodes[k].sched_ < latency)
                data_flow_graphs[i].sched_res_[data_flow_nodes[k].sched_].emplace_back(data_flow_nodes[k].index());
        }
        // print sched res
        for(int k = 0; k < data_flow_graphs[i].sched_res_.size(); k++){
            std::cout << "  cycle " << k + 1 << ": ";
            for(int kk = 0; kk < data_flow_graphs[i].sched_res_[k].size(); kk++) {
                std::cout << data_flow_graphs[i].sched_res_[k][kk] << " ";
            }
            std::cout<<std::endl;
        }
    }
}

void HLS::registerAllocationAndBinding()
{
    //1. DataFlowGraph to period-schedule-struct
    //2. in period-schedule-struct do left-edge algorithm
    std::cout<<"\n\n"<<"Start binding......"<<std::endl;
    std::vector<DataFlowGraph>& data_flow_graphs = control_flow_graph_.getDataFlowGraphs();
    for (int i = 0; i < data_flow_graphs.size(); i++) {
        std::cout<<"  block_label: "<<data_flow_graphs[i].blockLabel()<<std::endl;
        int latency = data_flow_graphs[i].latency();
        data_flow_graphs[i].register_binding_.resize(latency);
        data_flow_graphs[i].register_mapping_.resize(latency);
        int num_total_nodes = data_flow_graphs[i].numNodes() + 2;
        std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].getDataFlowNodes();
        std::vector<std::vector<int>> idxs_in_each_cycle(latency);
        for (int k = 1; k < num_total_nodes - 1; k++){
            int sched = data_flow_nodes[k].sched_;
            idxs_in_each_cycle[sched].emplace_back(k);
        }
        for (int l = latency - 1; l >= 0; l--){
            std::cout << "   in cycle " << l << ": ";
            if (l+1 < latency) {
                data_flow_graphs[i].register_binding_[l] = data_flow_graphs[i].register_binding_[l+1];
            }
            for (int sidx_cnt = 0; sidx_cnt < idxs_in_each_cycle[l].size(); sidx_cnt++) {
                int sidx = idxs_in_each_cycle[l][sidx_cnt];
                std::cout<<sidx<<" ";
                if(data_flow_nodes[sidx].op_type_ == 7) {
                    if(data_flow_nodes[sidx].oprands_.size() > 1) // br realized by logic sythesis
                        data_flow_graphs[i].register_binding_[l].insert(data_flow_nodes[sidx].oprands_[0]);
                    continue;
                }
                for(int c = 0; c < data_flow_nodes[sidx].oprands_.size(); c++){
                    std::string opr_str = data_flow_nodes[sidx].oprands_[c];
                    std::cout<<opr_str<<" ";
                    int input_include_flag = 0;
                    for(int t_c = 0; t_c < control_flow_graph_.getInputParams().size(); t_c++){
                        if(control_flow_graph_.getInputParams()[t_c].first == opr_str) {
                            input_include_flag = 1;
                            break;
                        }
                    }
                    if(!input_include_flag){
                        data_flow_graphs[i].register_binding_[l].insert(opr_str);
                    }
                }
                std::cout<<"||";
                if(data_flow_nodes[sidx].var_ != " "){
                    std::string var_str = data_flow_nodes[sidx].var_;
                    std::cout<<var_str<<" ";
                    auto it = data_flow_graphs[i].register_binding_[l].find(var_str);
                    if(it != data_flow_graphs[i].register_binding_[l].end()) {
                        it = data_flow_graphs[i].register_binding_[l].erase(it);
                    }
                }
            }
            std::cout<<std::endl;
        }
        int tmp_cnt = 1;
        int max_reg_num = 0;
        for(auto it : data_flow_graphs[i].register_binding_) {
            std::cout<<"    cycle "<<tmp_cnt<<": ";
            for(auto i : it){
                std::cout<<i<<" ";
            }
            std::cout<<std::endl;
            tmp_cnt++;
            max_reg_num = max_reg_num < it.size() ? it.size() : max_reg_num;
        }
        // register mapping
        for(int j = latency - 1; j >= 0; j--) {
            if(j != latency - 1) {
                data_flow_graphs[i].register_mapping_[j] = data_flow_graphs[i].register_mapping_[j+1];
                // for(int t = 0; t < data_flow_graphs[i].register_binding_[j]; t++) {
                for(auto itt : data_flow_graphs[i].register_binding_[j]) {
                    std::string tmp_str = itt;
                    if(!data_flow_graphs[i].register_mapping_[j].count(tmp_str)){
                        for(int c = 0; c < max_reg_num; c++){
                            int flag = 0;
                            int flag_1 = 0;
                            for(auto it : data_flow_graphs[i].register_mapping_[j]){
                                if(it.second == c){
                                    flag_1 = 1;
                                    if(std::find(data_flow_graphs[i].register_binding_[j].begin(), data_flow_graphs[i].register_binding_[j].end(), it.first) == data_flow_graphs[i].register_binding_[j].end()) {
                                        flag = 1;
                                        data_flow_graphs[i].register_mapping_[j].erase(it.first);
                                        break;
                                    }
                                } 
                            }
                            // no use or used
                            if (!flag_1 || flag) {
                                data_flow_graphs[i].register_mapping_[j].insert(std::pair<std::string, int>(tmp_str, c));
                                break;
                            } else
                                continue;
                        }
                    }
                }
            } else {
                int lt = 0;
                for(auto itt : data_flow_graphs[i].register_binding_[j]){
                    std::string tmp_strr = itt;
                    data_flow_graphs[i].register_mapping_[j].insert(std::pair<std::string, int>(tmp_strr, lt));
                    lt++;
                }
            } 
        }
        for(int l = 0; l < latency; l++){
            std::cout<<"in cycle "<<l+1<<":"<<std::endl;
            for(auto it : data_flow_graphs[i].register_mapping_[l]){
                std::cout<<"\t"<<it.first<<"->"<<it.second<<std::endl;
            }
        }
    }
    // block output values
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].getDataFlowNodes();
        for(int j = 0; j < data_flow_nodes.size(); j++) {
            std::string var = data_flow_nodes[j].var_;
            std::vector<std::string> next_block_label = data_flow_graphs[i].nextBlockLabel();
            for(int k = 0; k < data_flow_graphs.size(); k++) {
                std::string tmp_label = data_flow_graphs[k].blockLabel();
                if(std::find(next_block_label.begin(), next_block_label.end(), tmp_label) == next_block_label.end()){
                    continue;
                } else {
                    for(int a = 0; a < data_flow_graphs[k].register_binding_.size(); a++){
                        std::set<std::string>& values_in_cur_cyele = data_flow_graphs[k].register_binding_[a];
                        if (std::find(values_in_cur_cyele.begin(), values_in_cur_cyele.end(), var) != values_in_cur_cyele.end() &&
                            std::find(data_flow_graphs[i].output_values_.begin(), data_flow_graphs[i].output_values_.end(), var) == data_flow_graphs[i].output_values_.end()) {
                            data_flow_graphs[i].output_values_.emplace_back(var);
                            break;
                        }
                    }
                }
            }

        }
    }
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        std::vector<std::string> next_block_label = data_flow_graphs[i].nextBlockLabel();
        for(int k = 0; k < data_flow_graphs.size(); k++) {
                std::string tmp_label = data_flow_graphs[k].blockLabel();
                if(std::find(next_block_label.begin(), next_block_label.end(), tmp_label) == next_block_label.end()){
                    continue;
                } else {
                    data_flow_graphs[k].input_values_ = data_flow_graphs[i].output_values_;
                }
            }
    }
    control_flow_graph_.regsBinding();  //for RTL generation
}

// void HLS::calculateAllocationAndBinding()
// {
// }

void HLS::controlLogicSynthesis()
{
    // CFG
    auto generateGreyCode = [](int n){
        std::vector<std::string> res;
        int width = log2(n);
        res.emplace_back("0");
        res.emplace_back("1");
        for (int i = 1; i < width; i++) {
            std::vector<std::string> tmp_res;
            for(int j = 0; j < res.size(); j++){
                if(j%2){
                    tmp_res.emplace_back(res[j]+"1");
                    tmp_res.emplace_back(res[j]+"0");
                } else {
                    tmp_res.emplace_back(res[j]+"0");
                    tmp_res.emplace_back(res[j]+"1");
                }
            }
            res = tmp_res;
        }
        return res;
    };
    std::vector<std::string> greyCodeCFG;
    greyCodeCFG = generateGreyCode(control_flow_graph_.getBlocksNum());
    std::vector<DataFlowGraph>& data_flow_graphs = control_flow_graph_.getDataFlowGraphs();
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        data_flow_graphs[i].setStateCode(greyCodeCFG[i]);
    }
    // DFG

}

void HLS::writeOutFile()
{
    std::vector<DataFlowGraph>& data_flow_graphs = control_flow_graph_.getDataFlowGraphs();

    // mkdir
    std::string rmCommand = "rm -rf ./output";
    system(rmCommand.c_str());
	std::string mkdirCommand = "mkdir ./output";
    system(mkdirCommand.c_str());

    // open file
    std::string module_name = llvm_parser_.get_function_name();
    std::fstream output_verilog_file;
    output_verilog_file.open("./output/" + module_name+".v", std::ios::out);
    
    // regarding name
    std::string return_name = module_name + "_res";
    std::string finish_signal = module_name + "_finish";
    std::string output_reg = module_name + "_res_reg";

    // module declare
    output_verilog_file<<"module "<<module_name<<std::endl;
    output_verilog_file<<"("<<std::endl;
    std::vector<var> vars = llvm_parser_.get_function_params();
    std::vector<int> to_ram_idx;
    for(int i = 0; i < vars.size(); ++i)
    {
        if(vars[i]._array_flag)
            to_ram_idx.emplace_back(i);
        else
            output_verilog_file << "\t" << "input [" << BIND_WIDTH - 1 << ":0] " << vars[i]._name << "," << std::endl;
    }
    output_verilog_file << "\t" << "input clk, " << std::endl;
    output_verilog_file << "\t" << "input rst_, " << std::endl;
    if(control_flow_graph_.getRetType() == RET_INT) {
        output_verilog_file << "\t" << "output [" << BIND_WIDTH - 1 << ":0] " << return_name << "," << std::endl;
    }
    output_verilog_file << "\t" << "output " << finish_signal << std::endl;
    output_verilog_file<<");"<<std::endl;

    // generate register
    output_verilog_file << std::endl;
    output_verilog_file << "reg" << " [" << BIND_WIDTH - 1 << ": 0] " << output_reg << ";" << std::endl;
    output_verilog_file << "assign" << return_name << "=" << output_reg << ";" << std::endl;
    for (int i = 0; i < control_flow_graph_.getRegsNum(); i++) {
        output_verilog_file << "reg" << " [" << BIND_WIDTH - 1 << ": 0] " << "reg_" << i << ";" <<std::endl;
    }

    // block output register
    output_verilog_file << std::endl;
    for (int i = 0; i < data_flow_graphs.size(); i++) {
        for(auto it : data_flow_graphs[i].output_values_) {
            output_verilog_file << "reg" << " [" << BIND_WIDTH - 1 << ":0] " << "reg_mem_" << it << ";" << std::endl;
        }
    }

    // generate ram
    for(int i = 0; i < to_ram_idx.size(); i++) {
        output_verilog_file << std::endl;
        std::string array_name = vars[to_ram_idx[i]]._name;
        output_verilog_file << "\\\\ generate ram for array: " << array_name << std::endl;
        output_verilog_file << "wire [" << log2(RAM_DEPTH) - 1 << ":0] " << "addr_" + array_name << ";" << std::endl;
        output_verilog_file << "wire [" << BIND_WIDTH - 1 << ":0] " << "din_" + array_name << ";" << std::endl;
        output_verilog_file << "wire [" << BIND_WIDTH - 1 << ":0] " << "dout_" + array_name << ";" << std::endl;
        output_verilog_file << "wire" << " ena_" + array_name << ";" << std::endl;
        output_verilog_file << "wire" << " wea_" + array_name << ";" << std::endl;
        output_verilog_file << "SRAM " << "ram_" + array_name << " (" << std::endl;
        output_verilog_file << "\t" << ".addr(" << "addr_" + array_name << ")," << std::endl;
        output_verilog_file << "\t" << ".clk(" << "clk" << ")," << std::endl;
        output_verilog_file << "\t" << ".din(" << "din_" + array_name << ")," << std::endl;
        output_verilog_file << "\t" << ".dout(" << "dout_" + array_name << ")," << std::endl;
        output_verilog_file << "\t" << ".ena(" << "ena_" + array_name << ")," << std::endl;
        output_verilog_file << "\t" << ".wea(" << "wea_" + array_name << ")" << std::endl;
        output_verilog_file << ");" << std::endl;
    }

    // generate FSM for controlFlowGraph
    std::vector<std::string> state_parameters;
    int state_bit_width = log2(control_flow_graph_.getBlocksNum());
    output_verilog_file << std::endl;
    output_verilog_file << "reg [" << state_bit_width - 1 << ":0] " << "cur_state;" << std::endl;
    output_verilog_file << "reg [" << state_bit_width - 1 << ":0] " << "last_state;" << std::endl;
    if (control_flow_graph_.getBlocksNum() > 1) {
        output_verilog_file << "reg " << "br_flag;" << std::endl;
    }
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].getDataFlowNodes();
        std::string cond_str = data_flow_graphs[i].brCondition();
        if(cond_str != " "){
            output_verilog_file << "wire " << cond_str << ";" << std::endl;
        }
    }
    
    output_verilog_file << std::endl;
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        state_parameters.emplace_back("state_"+data_flow_graphs[i].blockLabel());
        output_verilog_file << "parameter " << state_parameters[i] << " = " << state_bit_width <<"'b" << data_flow_graphs[i].stateCode() << ";" << std::endl;
    }
    output_verilog_file << std::endl;
    output_verilog_file << "always@(posedge clk or negedge rst_)" << std::endl;
    output_verilog_file << "begin" << std::endl;
    output_verilog_file << "\t" << "if(!rst_)" << std::endl;
    output_verilog_file << "\t" << "begin" << std::endl;
    output_verilog_file << "\t" << "\t" << "cur_state <= " << state_parameters[0] << ";" << std::endl;
    output_verilog_file << "\t" << "\t" << "last_state <= " << state_parameters[0] << ";" << std::endl;
    output_verilog_file << "\t" << "\t" << "br_flag <= " << state_parameters[0] << ";" << std::endl;
    output_verilog_file << "\t" << "end" << std::endl;
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        if(data_flow_graphs[i].brCondition() != " ") {
            for(int c = 1; c >= 0; c--) {
                output_verilog_file << "\t" << "else if(cur_state == " << state_parameters[i] <<
                    " & " << data_flow_graphs[i].brCondition() << " == 1'b" << c << " & br_flag == 1'b1)" << std::endl; 
                output_verilog_file << "\t" << "begin" << std::endl;
                output_verilog_file << "\t" << "\t" << "last_state <= cur_state;" << std::endl;
                output_verilog_file << "\t" << "\t" << "cur_state <= " << "state_"+data_flow_graphs[i].nextBlockLabel()[1-c] << ";" << std::endl;
                output_verilog_file << "\t" << "\t" << "br_flag <= 1'b0;" << std::endl;
                output_verilog_file << "\t" << "end" << std::endl;
            }
        } else {
            output_verilog_file << "\t" << "else if(cur_state == " << state_parameters[i] << " & br_flag == 1'b1)" << std::endl; 
            output_verilog_file << "\t" << "begin" << std::endl;
            output_verilog_file << "\t" << "\t" << "last_state <= cur_state;" << std::endl;
            output_verilog_file << "\t" << "\t" << "cur_state <= " << "state_"+data_flow_graphs[i].nextBlockLabel()[0] << ";" << std::endl;
            output_verilog_file << "\t" << "\t" << "br_flag <= 1'b0;" << std::endl;
            output_verilog_file << "\t" << "end" << std::endl;
        }
    }
    output_verilog_file << "end" << std::endl;
     
    // counter reset
    output_verilog_file << std::endl;
    output_verilog_file << "reg [" << BIND_WIDTH - 1 << ":0] " << "counter" << ";" << std::endl;
    output_verilog_file << "always @(posedge clk or negedge rst_)" << std::endl;
    output_verilog_file << "begin" << std::endl;
    output_verilog_file << "\t" << "if(!rst_)" << std::endl;
    output_verilog_file << "\t" << "begin" << std::endl;
    output_verilog_file << "\t" << "\t" << "counter <= 0;" << std::endl;
    output_verilog_file << "\t" << "end" << std::endl;
    for(int i = 0; i < data_flow_graphs.size(); i++) {
        output_verilog_file << "\t" << "else if(cur_state == " << "state_"+data_flow_graphs[i].blockLabel() << " & counter == " << data_flow_graphs[i].latency() << ")" <<std::endl;
        output_verilog_file << "\t" << "begin" << std::endl;
        output_verilog_file << "\t" << "\t" << "counter <= 0;" <<std::endl;
        output_verilog_file << "\t" << "end" << std::endl;
    }
    output_verilog_file << "\t" << "else" << std::endl;
    output_verilog_file << "\t" << "\t" << "counter <= counter + 1" << std::endl;
    output_verilog_file << "\t" << "end" << std::endl;
    
    // all DFG control logic: realized by counter 
    output_verilog_file << std::endl;
    output_verilog_file << "always@ (counter) " << std::endl;
    output_verilog_file << "case(cur_state)" << std::endl;
    for(int i = 0; i < state_parameters.size(); i++){
        output_verilog_file << state_parameters[i] << ": begin" << std::endl;
        output_verilog_file << "\t" << "\t" << "case(counter)" << std::endl;
        std::vector<DataFlowNode>& data_flow_nodes = data_flow_graphs[i].getDataFlowNodes();
        std::vector<std::map<std::string, int>>& register_mapping = data_flow_graphs[i].register_mapping_;
        for(int cycle_cnt = 0; cycle_cnt < data_flow_graphs[i].latency() + 2; cycle_cnt++){ // prepare data -> process -> store data
            output_verilog_file << "\t" << "\t" << BIND_WIDTH << "'d" << cycle_cnt << ": begin" << std::endl;
            if(cycle_cnt == 0){ // prepare

            } else if(cycle_cnt == data_flow_graphs[i].latency() + 1) { // block output

            } else {
                int pc = cycle_cnt - 1;
                for(int l = 0; l < data_flow_graphs[i].sched_res_[pc].size(); l++){
                    int statement_idx = data_flow_graphs[i].sched_res_[pc][l];
                    DataFlowNode& cur_node = data_flow_nodes[statement_idx];
                    int op_type = cur_node.op_type_;
                    if(op_type == 14) {    // ret
                        output_verilog_file << "\t" << "\t" << "\t" << output_reg << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << std::endl;
                    } else if(op_type == 13) { // phi
                        output_verilog_file << "\t" << "\t" << "\t" << "if(last_state == " << "state_" << cur_node.oprands_[1] << ")" << std::endl;
                        if(register_mapping[pc].count(cur_node.var_))
                            output_verilog_file << "\t" << "\t" << "\t" << "\t" << "reg_" << register_mapping[pc][cur_node.var_] << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << ";" << std::endl;
                        else
                             output_verilog_file << "\t" << "\t" << "\t" << "\t" << "reg_mem_" << cur_node.var_ << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << ";" << std::endl;
                        output_verilog_file << "\t" << "\t" << "\t" << "else if(last_state == " << "state_" << cur_node.oprands_[3] << ")" << std::endl;
                        if(register_mapping[pc].count(cur_node.var_))
                            output_verilog_file << "\t" << "\t" << "\t" << "\t" << "reg_" << register_mapping[pc][cur_node.var_] << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[2]] << ";" << std::endl;
                        else
                             output_verilog_file << "\t" << "\t" << "\t" << "\t" << "reg_mem_" << cur_node.var_ << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[2]] << ";" << std::endl;     
                    } else if(op_type == 7) { // br
                        output_verilog_file << "\t" << "\t" << "\t" << "\t" << "br_flag <= 1;" << std::endl;
                    } else if(op_type == 0) { // assign
                        if(register_mapping[pc].count(cur_node.var_))
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_" << register_mapping[pc][cur_node.var_] << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << ";" << std::endl;
                        else
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_mem_" << cur_node.var_ << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << ";" << std::endl;
                    } else if(op_type == 5) { // LOAD
                        if(register_mapping[pc].count(cur_node.var_))
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_" << register_mapping[pc][cur_node.var_] << " <= " << "dout_" << cur_node.oprands_[0] << ";" << std::endl;
                        else
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_mem_" << cur_node.var_ << " <= " << "dout_" << cur_node.oprands_[0] << ";" << std::endl;
                    } else {
                        std::string typical_op_type = " ";
                        if(op_type == 10) // LE
                            typical_op_type = " <= ";
                        else if(op_type == 11) // GE
                            typical_op_type = " >= ";
                        else if(op_type == 4) // DIV
                            typical_op_type = " / ";
                        else if(op_type == 3) // MUL
                            typical_op_type = " * ";
                        else if(op_type == 2) // SUB
                            typical_op_type = " - "; 
                        else if(op_type == 1) // ADD
                            typical_op_type = " + ";
                        if(register_mapping[pc].count(cur_node.var_))
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_" << register_mapping[pc][cur_node.var_] << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << typical_op_type << "reg_" << register_mapping[pc][cur_node.oprands_[1]] << ";" << std::endl;
                        else
                            output_verilog_file << "\t" << "\t" << "\t" << "reg_mem_" << cur_node.var_ << " <= " << "reg_" << register_mapping[pc][cur_node.oprands_[0]] << typical_op_type << "reg_" << register_mapping[pc][cur_node.oprands_[1]] << ";" << std::endl;
                    }
                }
            }
            output_verilog_file << "\t" << "\t" << "end" <<std::endl;
        }
        output_verilog_file << "\t" << "\t" << "endcase" << std::endl;
        output_verilog_file << "\t" << "end" << std::endl;
    }
    output_verilog_file << "endcase" << std::endl;

    output_verilog_file << "endmodule" << std::endl;
    output_verilog_file.close();
}
