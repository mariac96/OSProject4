#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "directory.h"
#include <sys/times.h>
#include <unistd.h>


int v=0,copied=0;
long int written=0;

int main (int argc, char* argv[]){
  int d=0,i,r;
  struct stat statorg,statdest;
  struct dirent *destp;
  char *origin,*dest;
  DIR *destptr;
  int c;
  double t1, t2,t;
  struct tms tb1, tb2;
  double ticspersec;

  ticspersec = (double) sysconf(_SC_CLK_TCK); //start time
  t1 = (double) times(&tb1);

  if(argc<3){
    fprintf(stderr, "Not enough arguments.\n" );
    return-1;
  }

  origin =(char *) malloc( (strlen(argv[argc-2])+1) * sizeof(char) );
  if(origin ==NULL){
    fprintf(stderr, "Error - unable to allocate required memory.\n");
    return-1;
  }
  strcpy(origin,argv[argc-2]);

  dest = (char *) malloc( (strlen(argv[argc-1])+1) * sizeof(char) );
  if(dest ==NULL){
    fprintf(stderr, "Error - unable to allocate required memory.\n");
    return-1;
  }
  strcpy(dest,argv[argc-1]);

  for(i=0;i<argc-2;i++){
    if(strcmp(argv[i],"-v")==0){
      v=1;

    }
    if(strcmp(argv[i],"-d")==0){
      d=1;

    }
  }


    r=stat(dest,&statdest);
    if(r==-1 && errno==ENOENT){  //if the destination directory does not exist, copy everything
      if(stat(origin,&statorg)<0){
        perror("Error in stat:\n");
        return -1;
      }
      if(mkdir(dest,statorg.st_mode)==-1){
        perror("Error in mkdir:\n");
        return -1;
      }
      if(v==1){
        printf("Created directory %s\n",dest );
      }
      if((c=copydirectory(origin,dest))==-1){
        fprintf(stderr, "Error in copydirectory.\n" );
        return-1;
      }
      copied++;  //number of copied entities

    }
    else if(r!=-1){ //if the destination directory exists
        if ((destptr=opendir(dest))== NULL ) {
        	perror("Error in opendir:\n");
          return -1;
        	}

        i=0;
         while ((destp = readdir(destptr)) != NULL ) {  //first check if it is empty
           if((strcmp(".",destp->d_name)!=0) && (strcmp("..",destp->d_name)!=0)){//if it is . or ..
            i++;
            break;
           }
        }
        closedir(destptr);
        if(i>0){ //if it is not empty
          if((c=searchdir(origin,dest))>0){ //find which files to copy

          }
        }
        else{  //if the destination directory exists but it is empty, copy everything

          if((c=copydirectory(origin,dest))==-1){
            fprintf(stderr, "Error in copydirectory.\n" );
            return-1;
          }

        }
    }else{
      perror("Error in stat:\n");
      return -1;
    }

    c++;  //count the origin directory
    printf("There are %d files/directories in the hierarchy.\n",c );


    if(d==1){

      if((c=searchdelete(origin,dest))>=0){  //search if there are any files to delete
        printf("%d entities deleted\n", c);
      }
    }
    printf("%d entities copied\n",copied );




    free(dest);
    free(origin);

    t2 = (double) times(&tb2);
    t=(t2 - t1) / ticspersec; //run time
    printf("copied %ld bytes, total run time of the program %lf sec, at %f bytes/sec\n",written,  t , written/t);


}
