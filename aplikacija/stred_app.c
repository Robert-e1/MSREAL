#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_SIZE 100;

int main()
{
	FILE* fp;
	size_t command_bytes = 2;
	size_t str_bytes = MAX_SIZE;
	char *str;
	char *command = (char*)malloc(command_bytes);

	
    while(1)
      {
	printf("1: Procitaj trenutno stanje stringa\n");
	printf("2: Upisi novi string\n");
	printf("3: Konkataniraj string na trenutni\n");
	printf("4: Izbrisi citav string\n");
	printf("5: Izbrisi vodece i pratece space karaktere\n");
	printf("6: Izbrisi izraz iz stringa\n");
	printf("7: Izbrisi poslednjih n karaktera iz stringa\n");
	printf("Q: Exit app\n");
	
	getline(&command, &command_bytes, stdin);

	switch(*command - 48){

	default:
	  printf("No such command!\n");
	  break;

	case 'Q' - 48:
	  return 0;
	  
	case 1:  //Procitaj stanje stringa
   	  fp = fopen("/dev/stred","r");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }
	  str = (char*)malloc(str_bytes);
	  getline(&str,&str_bytes,fp);
	  getline(&str,&str_bytes,fp);
	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  printf("Sadrzaj stringa je: %s \n\n\n", str);
	  
	  free(str);
	  
	  break;  

	case 2: //Upisi novi string
	  str = (char*)malloc(str_bytes);
	  printf("Unesite string koji zelite da upisete:\n");
	  getline(&str, &str_bytes, stdin);
	  
	  fp = fopen("/dev/stred","w");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }

	  fprintf(fp, "string=%s", str);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  free(str);

	  break;

	case 3: //konkataniraj na string
	  str = (char*)malloc(str_bytes);
	  printf("Unesite string koji zelite da doajete postojecem:\n");
	  getline(&str,&str_bytes,stdin);

	  fp = fopen("/dev/stred","w");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }

	  fprintf(fp, "append=%s",str);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  free(str);

	  break;

	case 4: //brisanje sadrzaja stringa
   
	  fp = fopen("/dev/stred","w");
	  if(fp == NULL)
	    {
	      printf("FAiled to open /dev/stred\n");
	      return -1;
	    }

	  fputs("clear\n", fp);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  break;

	case 5://Izbrisi space karaktere
	  
	  fp = fopen("/dev/stred", "w");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }

	  fputs("shrink\n",fp);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  
	  break;

	case 6: //Izbrisi izraz iz stringa
	  str = (char *)malloc(str_bytes);
	  printf("Unesite izraz koji zelite da brisete iz stringa: \n");
	  getline(&str, &str_bytes, stdin);

	  fp = fopen("/dev/stred","w");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }

	  fprintf(fp, "remove=%s",str);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  free(str);

	  break;

	case 7:  //brisi poslednjih n karaktera iz stringa
	  str = (char *)malloc(str_bytes);
	  printf("Unesite broj karaktera koji zelite da brisete:\n");
	  getline(&str,&str_bytes,stdin);
	  
	  fp = fopen("/dev/stred","w");
	  if(fp == NULL)
	    {
	      printf("Failed to open /dev/stred\n");
	      return -1;
	    }

	  fprintf(fp,"truncate= %s",str);

	  if(fclose(fp))
	    {
	      printf("Failed to close /dev/stred\n");
	      return -1;
	    }
	  free(str);
	  
	  break;
	}//kraj switch-a
      }//kraj while-a
	return 0;
}

