// distvec_20181202.cc
#include <stdio.h>
#include <stdlib.h>
int node_num, t_node1, t_node2, t_cost; // topology file
int source, dest; // messages file
char message[1000]; // messages file
char *message_line; // messages file
int c_node1, c_node2, c_cost; // changes file

/* ----------------- Declared functions ----------------- */
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

void fill_table_1(FILE *fp, int ***r_table){
	while(!feof(fp)){
		fscanf(fp, "%d%d%d", &t_node1, &t_node2, &t_cost);
		r_table[t_node1][t_node2][0] = t_node2;
		r_table[t_node1][t_node2][1] = t_cost;
		r_table[t_node2][t_node1][0] = t_node1;
		r_table[t_node2][t_node1][1] = t_cost;
	} fclose(fp);
}

void fill_table_2(int ***r_table){
	int update_flag;
	do {
		update_flag = 0;
		for(int i=0; i<node_num; i++){
			for(int j=0; j<node_num; j++){
				if(j==r_table[i][j][0] && r_table[i][j][1]!=0){ // If neighboring node
					for(int k=0; k<node_num; k++){ // Check rather can be updated
						if(r_table[i][k][1] == -999 || r_table[i][j][1]+r_table[j][k][1] < r_table[i][k][1]){
							if(r_table[i][j][1]+r_table[j][k][1] < 0){ continue; }
							r_table[i][k][0] = j;
							r_table[i][k][1] = r_table[i][j][1]+r_table[j][k][1];
							update_flag = 1; }
					}
				}
			}
		}
	} while(update_flag==1);
}

void change_table(int ***r_table){
	r_table[c_node1][c_node2][0] = c_node2;
	r_table[c_node1][c_node2][1] = c_cost;
	r_table[c_node2][c_node1][0] = c_node1;
	r_table[c_node2][c_node1][1] = c_cost;
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

void print_message(FILE *fp, FILE *out, int ***r_table){
	while(!feof(fp)){
		fscanf(fp, "%d%d", &source, &dest);
		message_line = fgets(message, 1000, fp);
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
	} fprintf(out, "\n"); fclose(fp);
}

/* ----------------- Main function ----------------- */
int main(int argc, char *argv[]) {
	// 1. Get and check arguments.
	if(argc != 4) {
		fprintf(stderr, "usage: distvec topologyfile messagesfile changesfile\n");
		exit(1);
	}

	// 2. Open three txt files.
	FILE *fp1, *fp2, *fp3; // topology, messages, changes
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

	// 3. Make routing table.
	fscanf(fp1, "%d", &node_num);
	int ***r_table;
	r_table = (int***)malloc(sizeof(int**)*node_num); // number of routing table : node_num
	make_table(r_table); // Make routing Table node Array
	init_table(r_table); // Initialize routing table
	fill_table_1(fp1, r_table); // Neighboring table
	fill_table_2(r_table); // Exchange table

	// 4. Print output file.
	FILE *out;
	out = fopen("output_dv.txt", "wt");
	if (out == NULL) {
		fprintf(stderr, "Error: open output file. \n");
		exit(1); }
	print_table(out, r_table);
	print_message(fp2, out, r_table);

	// 5. Read changes file and update routing table.
	while(1){
		init_table(r_table); // init routing table
		fp1 = fopen(argv[1], "r");
		if (fp1 == NULL) {
			fprintf(stderr, "Error: open input file.\n");
			exit(1); } fscanf(fp1, "%d", &node_num);
			
		while(!feof(fp1)){ // fill routing table
			fscanf(fp1, "%d%d%d", &t_node1, &t_node2, &t_cost);
			r_table[t_node1][t_node2][0] = t_node2;
			r_table[t_node1][t_node2][1] = t_cost;
			r_table[t_node2][t_node1][0] = t_node1;
			r_table[t_node2][t_node1][1] = t_cost;
		} fclose(fp1);

		fscanf(fp3, "%d%d%d", &c_node1, &c_node2, &c_cost);
		if(feof(fp3)) break;
		change_table(r_table); // change routing table
		fill_table_2(r_table); // exchange routing table
		print_table(out, r_table); // print changed routing table

		fp2 = fopen(argv[2], "r");
		if (fp2 == NULL) {
		fprintf(stderr, "Error: open input file.\n");
		exit(1); }

		while(!feof(fp2)){ // print message
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

	// 6. Print out complete message.
	printf("Complete. Output file written to output_dv.txt\n");

	fclose(fp3);
	fclose(out);
	return 0;
}
