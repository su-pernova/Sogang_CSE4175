// linkstate_20181202.cc
#include <stdio.h>
#include <stdlib.h>
int node_num, t_node1, t_node2, t_cost; // topology file
int source, dest; // messages file
char message[1000]; // messages file
char *message_line; // messages file
int c_node1, c_node2, c_cost; // changes file
int links[5000][3];
int link_num = 0;

struct SPT_node {
	int distance;
	int parent;
};

void make_table(int ***r_table){
	for(int i=0; i<node_num; i++){
		// number of table row : node_num
		r_table[i] = (int**)malloc(sizeof(int*)*node_num);
		for(int j=0; j<node_num; j++){
			// number of table col : 2
			r_table[i][j] = (int*)malloc(sizeof(int)*2);
		}
	}
}

void init_table(int ***r_table){
	for(int i=0; i<node_num; i++){
		for(int j=0; j<node_num; j++){
			if(i==j) {
				r_table[i][j][0] = j;
				r_table[i][j][1] = 0;
			}
			else{
				r_table[i][j][0] = -1; // next node
				r_table[i][j][1] = -999; // distance
			} }
	}
}

void print_table(FILE *out, int ***r_table){
	int x, y;
	for(x=0; x<node_num; x++){
		for(y=0; y<node_num; y++){
			if(r_table[x][y][1] == -999) continue;
			fprintf(out, "%d %d %d\n", y, r_table[x][y][0], r_table[x][y][1]);
		}
		fprintf(out, "\n");
	}
}

/* ----------------- Main function ----------------- */
int main(int argc, char *argv[]) {
	// 1. Get and check arguments.
	if(argc != 4) {
	  fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
	  exit(1);
	}

	// 2. Open three txt files.
	FILE *fp1, *fp2, *fp3, *out; // topology, messages, changes
	fp1 = fopen(argv[1], "r");
	if (fp1 == NULL) {
		fprintf(stderr, "Error: open input file.\n");
		exit(1); }
	fp2 = fopen(argv[2], "r");
	if (fp2 == NULL) {
		fprintf(stderr, "Error: open input file.\n");
		exit(1); }
	fp3 = fopen(argv[3], "r");
	if (fp3 == NULL) {
		fprintf(stderr, "Error: open input file.\n");
		exit(1); }
	out = fopen("output_ls.txt", "wt");
	if (out == NULL) {
		fprintf(stderr, "Error: open output file. \n");
		exit(1); }

	// 3. Make SPT table.
	int ***r_table;
	fscanf(fp1, "%d", &node_num);
	
	r_table = (int***)malloc(sizeof(int**)*node_num);
	make_table(r_table);
	init_table(r_table);

	while(!feof(fp1)){ // save link info
		fscanf(fp1, "%d%d%d", &t_node1, &t_node2, &t_cost);
		if(feof(fp1)) break;
		links[link_num][0] = t_node1;
		links[link_num][1] = t_node2;
		links[link_num][2] = t_cost;
		link_num++;
	} fclose(fp1);

	int node_index = 0;
	while(node_index != node_num){
		struct SPT_node *SPT = (struct SPT_node*)malloc(sizeof(struct SPT_node)*(node_num));
		for(int i=0; i<node_num; i++){ // init table
			if(i==node_index){
				SPT[i].distance = 0;
				SPT[i].parent = i; }
			else {
				SPT[i].distance = -999;
				SPT[i].parent = -1; }
		}
		// (1) Fill table : Neighboring
		for(int i=0; i<link_num; i++){
			if(links[i][0] == node_index){
				SPT[links[i][1]].distance = links[i][2];
				SPT[links[i][1]].parent = node_index;
			}
			else if(links[i][1] == node_index){
				SPT[links[i][0]].distance = links[i][2];
				SPT[links[i][0]].parent = node_index;
			}
		}

		// (2) Fill table : Exchange
		int min = 999;
		int selected_node;
		int selected_list[100];
		selected_list[0] = node_index;
		int selected_num = 1; // 총 노드개수와 같아지면 끝난다
		int continue_flag;

		while(1){ 
			min = 999;
			for(int i=0; i<node_num; i++){
				if(SPT[i].distance>0 && SPT[i].distance < min){
					continue_flag = 0;
					for(int j=0; j<selected_num; j++){
						if(i==selected_list[j]) {
							continue_flag = 1;
							break;
						}
					}
					if(continue_flag==1) continue;
					min = SPT[i].distance;
					selected_node = i;
				}
			}
			selected_list[selected_num] = selected_node;
			selected_num++;
			if(selected_num == node_num) break;

			// (3) Update table
			for(int i=0; i<link_num; i++){
				if(links[i][0] == selected_node){
					if(SPT[links[i][1]].distance == -999 || SPT[selected_node].distance + links[i][2] < SPT[links[i][1]].distance){
						if(SPT[selected_node].distance + links[i][2] < 0) continue;
						SPT[links[i][1]].distance = SPT[selected_node].distance + links[i][2];
						SPT[links[i][1]].parent = selected_node;
					}
				}
				else if(links[i][1] == selected_node){
					if(SPT[links[i][0]].distance == -999 || SPT[selected_node].distance + links[i][2] < SPT[links[i][0]].distance){
						if(SPT[selected_node].distance + links[i][2] < 0) continue;
						SPT[links[i][0]].distance = SPT[selected_node].distance + links[i][2];
						SPT[links[i][0]].parent = selected_node;
					}
				}
			}
		}

		// 4. Make routing table.
		int temp;
		for(int i=0; i<node_num; i++){
			if(i!=node_index){
				if(SPT[i].distance < 0) continue;
				temp = i; // init variables
				while(1){
				if(SPT[temp].parent == node_index) break;
				temp = SPT[temp].parent;
				}
				r_table[node_index][i][0] = temp;
				r_table[node_index][i][1] = SPT[i].distance;
			}
		}
		node_index++;
	}

	// 5. Print routing table.
	print_table(out, r_table);

	// 6. Print message.
	while(!feof(fp2)){
		fscanf(fp2, "%d%d", &source, &dest);
		message_line = fgets(message, 1000, fp2);
		if(message_line == NULL) break;
		if(r_table[source][dest][1] != -999){
			fprintf(out, "from %d to %d cost %d hops ", source, dest, r_table[source][dest][1]);
			while(source!=dest){
				fprintf(out, "%d ", source);
				source = r_table[source][dest][0];
			} fprintf(out, "message%s", message_line);
		}
		else if(r_table[source][dest][1] == -999){
			fprintf(out, "from %d to %d cost infinite hops unreachable message%s", source, dest, message_line);
		}
	} fprintf(out, "\n"); fclose(fp2);

	// 7. Read changes file and update routing table.
	init_table(r_table); // init routing table
	while(1){
		fscanf(fp3, "%d%d%d", &c_node1, &c_node2, &c_cost);
		if(feof(fp3)) break;

		// Update changes
		for(int i=0; i<link_num; i++){
			if(links[i][0]==c_node1 && links[i][1]==c_node2)
				links[i][2] = c_cost;
			else if(links[i][0]==c_node2 && links[i][1]==c_node1)
				links[i][2] = c_cost;
			else {
				links[link_num][0] = c_node1;
				links[link_num][1] = c_node2;
				links[link_num][2] = c_cost;
				link_num++;
			}
		}

		// Make things again
		node_index = 0;
		while(node_index != node_num){
			struct SPT_node *SPT = (struct SPT_node*)malloc(sizeof(struct SPT_node)*(node_num));
			for(int i=0; i<node_num; i++){ // init table
				if(i==node_index){
					SPT[i].distance = 0;
					SPT[i].parent = i; }
				else {
					SPT[i].distance = -999;
					SPT[i].parent = -1; }
			}
			// (1) Fill table : Neighboring
			for(int i=0; i<link_num; i++){
				if(links[i][0] == node_index){
					SPT[links[i][1]].distance = links[i][2];
					SPT[links[i][1]].parent = node_index;
				}
				else if(links[i][1] == node_index){
					SPT[links[i][0]].distance = links[i][2];
					SPT[links[i][0]].parent = node_index;
				}
			}

			// (2) Fill table : Exchange
			int min = 999;
			int selected_node;
			int selected_list[100];
			selected_list[0] = node_index;
			int selected_num = 1; // 총 노드개수와 같아지면 끝난다
			int continue_flag;

			while(1){ 
				min = 999;
				for(int i=0; i<node_num; i++){
					if(SPT[i].distance>0 && SPT[i].distance < min){
						continue_flag = 0;
						for(int j=0; j<selected_num; j++){
							if(i==selected_list[j]) {
								continue_flag = 1;
								break;
							}
						}
						if(continue_flag==1) continue;
						min = SPT[i].distance;
						selected_node = i;
					}
				}
				selected_list[selected_num] = selected_node;
				selected_num++;
				if(selected_num == node_num) break;

				// (3) Update table
				for(int i=0; i<link_num; i++){
					if(links[i][0] == selected_node){
						if(SPT[links[i][1]].distance == -999 || SPT[selected_node].distance + links[i][2] < SPT[links[i][1]].distance){
							if(SPT[selected_node].distance + links[i][2] < 0) continue;
							SPT[links[i][1]].distance = SPT[selected_node].distance + links[i][2];
							SPT[links[i][1]].parent = selected_node;
						}
					}
					else if(links[i][1] == selected_node){
						if(SPT[links[i][0]].distance == -999 || SPT[selected_node].distance + links[i][2] < SPT[links[i][0]].distance){
							if(SPT[selected_node].distance + links[i][2] < 0) continue;
							SPT[links[i][0]].distance = SPT[selected_node].distance + links[i][2];
							SPT[links[i][0]].parent = selected_node;
						}
					}
				}
			}

			// Make routing table.
			int temp;
			for(int i=0; i<node_num; i++){
				if(i!=node_index){
					if(SPT[i].distance < 0) continue;
					temp = i; // init variables
					while(1){
					if(SPT[temp].parent == node_index) break;
					temp = SPT[temp].parent;
					}
					r_table[node_index][i][0] = temp;
					r_table[node_index][i][1] = SPT[i].distance;
				}
			}
			node_index++;
		}

		// Print routing table.
		print_table(out, r_table);

		// Print message.
		fp2 = fopen(argv[2], "r");
		if (fp2 == NULL) {
		fprintf(stderr, "Error: open input file.\n");
		exit(1); }

		while(!feof(fp2)){ 
			fscanf(fp2, "%d%d", &source, &dest);
			message_line = fgets(message, 1000, fp2);
			if(message_line == NULL) break;
			if(r_table[source][dest][1] != -999){
				fprintf(out, "from %d to %d cost %d hops ", source, dest, r_table[source][dest][1]);
				while(source!=dest){
					fprintf(out, "%d ", source);
					source = r_table[source][dest][0];
				} fprintf(out, "message%s", message_line);
			}
			else if(r_table[source][dest][1] == -999){
				fprintf(out, "from %d to %d cost infinite hops unreachable message%s", source, dest, message_line);
			}
		} fprintf(out, "\n"); fclose(fp2);
	}

	// 0. Print out complete message.
	printf("Complete. Output file written to output_ls.txt\n");

	fclose(fp3);
	fclose(out);
	return 0;
}
