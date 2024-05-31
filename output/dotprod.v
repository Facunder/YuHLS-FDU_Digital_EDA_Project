module dotprod
(
	input [31:0] n,
	input clk, 
	input rst_, 
	output [31:0] dotprod_res,
	output dotprod_finish
);

reg [31: 0] dotprod_res_reg;
assigndotprod_res=dotprod_res_reg;
reg [31: 0] reg_0;
reg [31: 0] reg_1;
reg [31: 0] reg_2;
reg [31: 0] reg_3;
reg [31: 0] reg_4;

reg [31:0] reg_mem_c;
reg [31:0] reg_mem_i;
reg [31:0] reg_mem_cl;
reg [31:0] reg_mem_cr;
reg [31:0] reg_mem_i_inc;

\\ generate ram for array: a
wire [4:0] addr_a;
wire [31:0] din_a;
wire [31:0] dout_a;
wire ena_a;
wire wea_a;
SRAM ram_a (
	.addr(addr_a),
	.clk(clk),
	.din(din_a),
	.dout(dout_a),
	.ena(ena_a),
	.wea(wea_a)
);

\\ generate ram for array: b
wire [4:0] addr_b;
wire [31:0] din_b;
wire [31:0] dout_b;
wire ena_b;
wire wea_b;
SRAM ram_b (
	.addr(addr_b),
	.clk(clk),
	.din(din_b),
	.dout(dout_b),
	.ena(ena_b),
	.wea(wea_b)
);

reg [1:0] cur_state;
reg [1:0] last_state;
reg br_flag;
wire cond;

parameter state_0 = 2'b00;
parameter state_start = 2'b01;
parameter state_calc = 2'b11;
parameter state_ret = 2'b10;

always@(posedge clk or negedge rst_)
begin
	if(!rst_)
	begin
		cur_state <= state_0;
		last_state <= state_0;
		br_flag <= state_0;
	end
	else if(cur_state == state_0 & br_flag == 1'b1)
	begin
		last_state <= cur_state;
		cur_state <= state_start;
		br_flag <= 1'b0;
	end
	else if(cur_state == state_start & cond == 1'b1 & br_flag == 1'b1)
	begin
		last_state <= cur_state;
		cur_state <= state_ret;
		br_flag <= 1'b0;
	end
	else if(cur_state == state_start & cond == 1'b0 & br_flag == 1'b1)
	begin
		last_state <= cur_state;
		cur_state <= state_calc;
		br_flag <= 1'b0;
	end
	else if(cur_state == state_calc & br_flag == 1'b1)
	begin
		last_state <= cur_state;
		cur_state <= state_start;
		br_flag <= 1'b0;
	end
	else if(cur_state == state_ret & br_flag == 1'b1)
	begin
		last_state <= cur_state;
		cur_state <= state_0;
		br_flag <= 1'b0;
	end
end

reg [31:0] counter;
always @(posedge clk or negedge rst_)
begin
	if(!rst_)
	begin
		counter <= 0;
	end
	else if(cur_state == state_0 & counter == 1)
	begin
		counter <= 0;
	end
	else if(cur_state == state_start & counter == 3)
	begin
		counter <= 0;
	end
	else if(cur_state == state_calc & counter == 4)
	begin
		counter <= 0;
	end
	else if(cur_state == state_ret & counter == 1)
	begin
		counter <= 0;
	end
	else
		counter <= counter + 1
	end

always@ (counter) 
case(cur_state)
state_0: begin
		case(counter)
		32'd0: begin
		end
		32'd1: begin
			reg_mem_c <= reg_0;
		end
		32'd2: begin
		end
		endcase
	end
state_start: begin
		case(counter)
		32'd0: begin
		end
		32'd1: begin
			if(last_state == state_0)
				reg_mem_i <= reg_0;
			else if(last_state == state_calc)
				reg_mem_i <= reg_4;
			if(last_state == state_0)
				reg_mem_cl <= reg_1;
			else if(last_state == state_calc)
				reg_mem_cl <= reg_3;
		end
		32'd2: begin
			reg_mem_cond <= reg_0 >= reg_0;
		end
		32'd3: begin
				br_flag <= 1;
		end
		32'd4: begin
		end
		endcase
	end
state_calc: begin
		case(counter)
		32'd0: begin
		end
		32'd1: begin
			reg_mem_ai <= dout_a;
			reg_mem_bi <= dout_b;
			reg_mem_i_inc <= reg_2 + reg_0;
		end
		32'd2: begin
			reg_mem_ci <= reg_0 * reg_2;
		end
		32'd3: begin
			reg_mem_cr <= reg_1 + reg_0;
		end
		32'd4: begin
				br_flag <= 1;
		end
		32'd5: begin
		end
		endcase
	end
state_ret: begin
		case(counter)
		32'd0: begin
		end
		32'd1: begin
			dotprod_res_reg <= reg_0
		end
		32'd2: begin
		end
		endcase
	end
endcase
endmodule
