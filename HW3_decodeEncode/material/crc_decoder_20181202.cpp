// g++ -Wall -o crc_decoder_20181202 crc_decoder_20181202.cpp
// ./crc_decoder_20181202 codestream.tx datastream.rx result.txt 1101 8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int gen_size, dword_size, cword_size, input_size, padding_size;
int cword_num, error_num = 0;

int modulo2(int *dividend, int *divisor, int r_size){
  int *quota = (int*)malloc(sizeof(int)*gen_size);
  int divid_idx = 0;
  int fin = cword_size-gen_size+1;
  int break_flag = 1;

  while(dividend[divid_idx]==0){
    divid_idx++;
    if(divid_idx==fin) break;
  }

  if(divid_idx!=fin){
    for(int i=0; i<gen_size; i++){
      if(dividend[divid_idx]==divisor[i]) quota[i]=0;
      else quota[i]=1;
      divid_idx++;
    }
    while(quota[0]==0) {
      for(int i=0; i<gen_size-1; i++) quota[i] = quota[i+1];
	  if(divid_idx==cword_size) {
        quota[gen_size-1] = 0;
		divid_idx++;
        break;
      }
      quota[gen_size-1] = dividend[divid_idx];
	  divid_idx++;
    }

    /*printf("<quota>\n");
    for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
    printf("\n");*/
  }

  if(divid_idx<=cword_size){
    while(break_flag){
      for(int i=0; i<gen_size; i++){
        if(quota[i]==divisor[i]) quota[i]=0;
        else quota[i]=1;
      }
      while(quota[0]==0) {
        for(int i=0; i<gen_size-1; i++) quota[i] = quota[i+1];
		if(divid_idx==cword_size) {
          quota[gen_size-1] = 0;
          break_flag = 0;
          break;
        }
        quota[gen_size-1] = dividend[divid_idx];
		divid_idx++;
      }

      /*for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
      printf("\n");*/
    }
  }

  /*printf("fin:");
  for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
  printf("\n");*/

  int index = 0;
  for(int i=dword_size; i<cword_size; i++){
    dividend[i] = quota[index];
    index++;
  }
  /*for(int i=0; i<cword_size; i++) printf("%d", dividend[i]);
  printf("\n");*/

  return 0;
}

int main(int argc, char *argv[]) {
	// -------- Check Arguments --------
    if(argc != 6) {
	  fprintf(stderr, "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
	  exit(1);
	}

	/*argv[1] //input file
    argv[2] //output file
    argv[3] //result file
    argv[4] //generator
	argv[5] //dataword_size(4 or 8)*/

	// ---------- Open Files ----------
	FILE *input, *output, *result;
	input = fopen(argv[1], "r");
    if (input == NULL) {
		fprintf(stderr, "input file open error.\n");
		exit(1); }
	fseek(input, 0, SEEK_END);
    input_size = ftell(input);
	input = fopen(argv[1], "r");

	output = fopen(argv[2], "wt");
    if (output == NULL) {
		fprintf(stderr, "output file open error.\n");
		exit(1); }
	
	result = fopen(argv[3], "wt");
    if (result == NULL) {
		fprintf(stderr, "result file open error.\n");
		exit(1); }
	
	if(strcmp(argv[5], "4")!=0 && strcmp(argv[5], "8")!=0) {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1); }

	// ----------- Save Data -----------
	gen_size = (int)strlen(argv[4]);
	dword_size = atoi(argv[5]);
	cword_size = dword_size+gen_size-1;
	//printf("generator_size : %d\n",gen_size);

    int *generator = (int*)malloc(sizeof(int)*gen_size);
	for(int i=0; i<gen_size; i++) generator[i] = ((int)(argv[4][i]))-48;
	//printf("generator : ");
    //for(int i=0; i<gen_size; i++) printf("%d", generator[i]);
    //printf("\n");

	//printf("dataword_size : %d\n",dword_size);
    //printf("codeword_size : %d\n",cword_size);

	int **context;
	context = (int**)malloc(sizeof(int*)*(input_size-1));
    for(int i=0; i<input_size; i++) context[i] = (int*)malloc(sizeof(int)*8);

	char buffer[1]; int dividend;
	int index = 0;
	int total_size = (input_size-1)*8;
	int *total = (int*)malloc(sizeof(int)*total_size);
	int total_idx = 0;

	fread(buffer, sizeof(buffer), 1, input);
	padding_size = (int)buffer[0];
	//printf("padding_size : %d\n", padding_size);

	cword_num = ((input_size-1)*8 - padding_size)/cword_size;
	//printf("cword_num : %d\n", cword_num);
	int **codeword;
	codeword = (int**)malloc(sizeof(int*)*cword_num);
	for(int i=0; i<cword_num; i++) codeword[i] = (int*)malloc(sizeof(int)*cword_size);

	while(fread(buffer, sizeof(buffer), 1, input) == 1){
		dividend = (int)buffer[0];
		if(dividend < 0){
          dividend = dividend*(-1)-1;
          for(int i=7; i>=0; i--){
			  context[index][i] = (0 == dividend%2);
			  dividend = dividend/2;
			}
        }
		else{
			for(int i=7; i>=0; i--){
				context[index][i] = dividend%2;
				dividend = dividend/2;
			}
		}
        index++;
	}

	//printf("<bytes>\n");
	for(int i=0; i<input_size-1; i++){
		for(int j=0; j<8; j++){
			//printf("%d",context[i][j]);
			total[total_idx] = context[i][j];
			total_idx++;
		} //printf("\n");
	}

	/*printf("<total>\n");
	for(int i=0; i<total_size; i++){
		printf("%d",total[i]);
	}printf("\n");*/
	
	int new_total_size = total_size - padding_size;
	int *new_total = (int*)malloc(sizeof(int)*new_total_size);
	for(int i=padding_size; i<total_size; i++){
		new_total[i-padding_size] = total[i];
	}

	/*printf("<new total>\n");
	for(int i=0; i<new_total_size; i++){
		printf("%d",new_total[i]);
	}printf("\n");*/
    
	int cword_idx1 = 0;
	int cword_idx2 = 0;
	for(int i=0; i<new_total_size; i++){
		if(i!=0 && i%(cword_size)==0){ cword_idx1++; cword_idx2 = 0; }
		codeword[cword_idx1][cword_idx2] = new_total[i];
		cword_idx2++;
	}

	/*printf("<codewords>\n");
	for(int i=0; i<cword_num; i++){
		for(int j=0; j<cword_size; j++){
			printf("%d",codeword[i][j]);
		}printf("\n");
	}*/

	int error_flag = 0;
	//printf("<error codewords>\n");
	for(int i=0; i<cword_num; i++){
		modulo2(codeword[i], generator, gen_size-1);
		int cword_idx3 = cword_size-1;
		for(int j=0; j<cword_size-dword_size; j++){
			if(codeword[i][cword_idx3]!=0) error_flag = 1;
			cword_idx3--;
		}
		if(error_flag){
			error_num++;
			/*printf("error! : ");
			for(int j=0; j<cword_size; j++){
				printf("%d",codeword[i][j]);
			}printf("(i=%d)\n",i);*/
		}
		error_flag = 0;
	}

	//printf("error_num : %d\n", error_num);
	fprintf(result, "%d %d", cword_num, error_num);

	// -------- Restore file --------
	int **restore;
    int restore_num = cword_num;
	int r_idx1 = 0, r_idx2 = 0;

    if(dword_size==4) restore_num/=2;
    restore = (int**)malloc(sizeof(int*)*(restore_num));
    for(int i=0; i<restore_num; i++) restore[i] = (int*)malloc(sizeof(int)*8);

    if(dword_size==4){
		for(int i=0; i<cword_num; i++){
			if(i!=0 && i%2==0){ r_idx1++; r_idx2 = 0; }
			for(int j=0; j<dword_size; j++){
				restore[r_idx1][r_idx2] = codeword[i][j];
				r_idx2++;
			}
		}
	}
	else{
		for(int i=0; i<cword_num; i++){
			for(int j=0; j<8; j++){ restore[i][j] = codeword[i][j]; }
		}
	}

   /* printf("<restore>\n");
    for(int i=0; i<restore_num; i++){
      for(int j=0; j<8; j++){
        printf("%d",restore[i][j]);
      }printf("\n");
    }*/

	// -------- Make Output file --------
	char a[1];
    int binary = 128, sum = 0;
    for(int i=0; i<restore_num; i++){
		for(int j=0; j<8; j++){
			sum += restore[i][j] * binary;
			binary/=2;
		}
		//printf("sum : %d ",sum);
		a[0] = (char)(sum);
		fwrite(a,sizeof(a),1,output); //printf("(written)\n");
		binary = 128;
		sum = 0;
    }
    return 0;
}