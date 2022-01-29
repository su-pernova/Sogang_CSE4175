// g++ -Wall -o crc_encoder_20181202 crc_encoder_20181202.cpp
// ./crc_encoder_20181202 datastream.tx codestream.tx 1101 8
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int gen_size, dword_size, cword_size, input_size, padding_size;

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

    /*printf("<quota>\n");
    for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
    printf("\n");*/

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

    printf("-> ");
    for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
    printf("\n");
  }

  

  if(divid_idx<=cword_size){
    while(break_flag){
      for(int i=0; i<gen_size; i++){
        if(quota[i]==divisor[i]) quota[i]=0;
        else quota[i]=1;
      }

      for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
      printf("\n");

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
      printf("-> ");
      for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
      printf("\n");
    }
  }

  printf("fin:");
  for(int i=0; i<gen_size; i++) printf("%d",quota[i]);
  printf("\n");

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
    if(argc != 5) {
	  fprintf(stderr, "usage: ./crc_encoder input_file output_file generator dataword_size\n");
	  exit(1); }

    // ---------- Open Files ----------
    FILE *input, *output;
    input = fopen(argv[1], "rb");
    if (input == NULL) {
		fprintf(stderr, "input file open error.\n");
		exit(1); }

    output = fopen(argv[2], "wb");
    if (output == NULL) {
		fprintf(stderr, "output file open error.\n");
		exit(1); }

    if(strcmp(argv[4], "4")!=0 && strcmp(argv[4], "8")!=0) {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1); }

    // 1. save input-file size
    fseek(input, 0, SEEK_END);
    input_size = ftell(input);
    printf("input_size : %d\n", input_size);
    input = fopen(argv[1], "rb");
    int **context;
    context = (int**)malloc(sizeof(int*)*input_size);
    for(int i=0; i<input_size; i++) context[i] = (int*)malloc(sizeof(int)*8);

    // 2. save generator, dataword, codeword size
    gen_size = (int)strlen(argv[3]);
    dword_size = atoi(argv[4]);
    cword_size = dword_size+gen_size-1;
    printf("generator_size : %d\n",gen_size);

    int *generator = (int*)malloc(sizeof(int)*gen_size);
    for(int i=0; i<gen_size; i++) generator[i] = ((int)(argv[3][i]))-48;
    printf("generator : ");
    for(int i=0; i<gen_size; i++) printf("%d", generator[i]);
    printf("\n");

    printf("dataword_size : %d\n",dword_size);
    printf("codeword_size : %d\n",cword_size);

    int cword_num = input_size;
    if(dword_size == 4) cword_num*=2;
    padding_size = ((cword_size*cword_num)/8+1)*8 - (cword_size*cword_num);
    if(padding_size==8) padding_size =0;
    printf("padding_size : %d\n",padding_size);

    // 3. save input-file context
    char buffer[1]; int dividend;
    int index = 0;
    while(fread(buffer, sizeof(buffer), 1, input) == 1){
        //fwrite(buffer, sizeof(buffer),1, output);
        dividend = (int)buffer[0];
        // ASCII 코드가 음수인 경우
        if(dividend < 0){
          dividend = dividend*(-1)-1;
          for(int i=7; i>=0; i--){ context[index][i] = (0 == dividend%2); dividend = dividend/2; }
        }
        // ASCII 코드가 양수인 경우
        else{ for(int i=7; i>=0; i--){ context[index][i] = dividend%2; dividend = dividend/2; }}
        index++;
    }

    if(dword_size == 8){
      printf("<datawords>\n");
      for(int i=0; i<input_size; i++){
        for(int j=0; j<8; j++){
          printf("%d",context[i][j]);
        } printf("\n");
      }
    }

    // 4. make array for dwordsize 4
    int **context_half;
    if(dword_size == 4){
      context_half = (int**)malloc(sizeof(int*)*(input_size*2));
      for(int i=0; i<input_size*2; i++) context_half[i] = (int*)malloc(sizeof(int)*4);

      int temp1 = 0, temp2 = 0;
      for(int i=0; i<input_size; i++){
        for(int j=0; j<8; j++){
          context_half[temp1][temp2] = context[i][j];
          temp2++;
          if(temp2==4) { temp1++; temp2=0; }
        }
      }

      printf("<datawords>\n");
      for(int i=0; i<input_size*2; i++){
        for(int j=0; j<4; j++){
          printf("%d",context_half[i][j]);
        }printf("\n");
      }
    }

    // 5. make codeword buffers
    int **cword_buffer;
    cword_buffer = (int**)malloc(sizeof(int*)*(cword_num));
    for(int i=0; i<cword_num; i++) cword_buffer[i] = (int*)malloc(sizeof(int)*cword_size);

    int **temp;
    index = 0;
    temp = context;
    if(dword_size == 4) temp = context_half;

    for(int i=0; i<cword_num; i++){
      for(int j=0; j<cword_size; j++){
        if(j>=dword_size) { cword_buffer[i][j] = 0; continue; }
        cword_buffer[i][j] = temp[i][j];
      }
    }

    printf("<codewords(1)>\n");
    for(int i=0; i<cword_num; i++){
      for(int j=0; j<cword_size; j++){
        printf("%d", cword_buffer[i][j]);
      }printf("\n");
    }

    // 6. encode codewords
    int *final_int = (int*)malloc(sizeof(int)*(cword_size*cword_num+padding_size));
    int final_idx = 0;
    for(int i=0;i<padding_size;i++) {
      final_int[final_idx] = 0;
      final_idx++;
    }

    printf("<codewords(2)>\n");
    for(int i=0; i<cword_num; i++){
      modulo2(cword_buffer[i], generator, gen_size-1);
      for(int j=0; j<cword_size; j++) {
        printf("%d", cword_buffer[i][j]);
        final_int[final_idx] = cword_buffer[i][j];
        final_idx++;
      }
      printf("\n");
    }

    printf("<final>\n");
    for(int i=0; i<cword_size*cword_num+padding_size; i++){
      printf("%d", final_int[i]);
    }printf("\n");

    // -------- Make Output file --------
    char a[1]; a[0] = (char)(padding_size);
    fwrite(a, sizeof(a),1,output); //printf("(written)\n");
    int binary = 128, sum = 0;
    for(int i=0; i<=cword_size*cword_num+padding_size; i++){
      if(i!=0 && i%8==0) {
        //printf("sum : %d ",sum);
        a[0] = (char)(sum);
        fwrite(a,sizeof(a),1,output); //printf("(written)\n");
        if(i == cword_size*cword_num+padding_size) break;
        binary = 128;
        sum = 0;
      }
      sum += final_int[i] * binary;
      binary/=2;
    }

    fclose(input);
    fclose(output);
    return 0;
}