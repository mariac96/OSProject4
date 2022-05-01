#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "file.h"
#include <fcntl.h>
#include "directory.h"
#define BUFFSIZE 1024

extern int v,copied;
long int written;

//copy the file with path origin to the file with path destin
int copyfile(char*origin,char*destin){
    int orig,dest;
    long int w;
    char buffer[BUFFSIZE];
    ssize_t nread;



    if((orig=open(origin,O_RDONLY))==-1){
      perror("Error in open:\n");
      printf("%s\n",origin );
      return -1;
    }

    if((dest=open(destin,O_WRONLY | O_TRUNC))==-1){
      perror("Error in open:\n");

      return -1;
    }

    while(( nread = read(orig, buffer,sizeof(buffer))) > 0){ //copy the file
      if((w=write(dest,buffer,nread))==-1){
        perror("Error in write:\n");
        return -1;
      }
      written =w +written;
    }
    close(orig);
    close(dest);
    return 0;
}

//compares the files neworig and newdest
int comparefiles(char*neworig,char*newdest){
  struct stat statorg,statdest;
  int typed=0,typeo=0,c,counter=0;

  if(stat(newdest,&statdest)<0){
    perror("Error in stat:\n");
    return -1;
  }
  if(stat(neworig,&statorg)<0){
    perror("Error in stat:\n");
    return -1;
  }
  if((statorg.st_mode & S_IFMT ) == S_IFDIR){ //it is a directory
    typeo=1;
  }else if((statorg.st_mode & S_IFMT ) == S_IFREG){ //it is a file
    typeo=2;
  }
  if((statdest.st_mode & S_IFMT ) == S_IFDIR){
    typed=1;
  }else if((statdest.st_mode & S_IFMT ) == S_IFREG){
    typed=2;
  }

  if(typeo!=typed){ //one is a file the other a directory

    if(typeo==1){  //it is a directory
      if(mkdir(newdest,statorg.st_mode)==-1){
        perror("Error in mkdir:");
        return -1;
      }
      if(v==1){
        printf("Created directory %s\n",newdest );
      }
      if((c=copydirectory(neworig,newdest))>0){
        counter = counter +c;
      }
      copied++;
    }else {
      if(creat(newdest,statorg.st_mode)==-1){
        perror("Error in creat:");
        return -1;
      }

      if(copyfile(neworig,newdest)==-1){
        fprintf(stderr,"Error in copying %s\n",newdest );
      }else if(v==1){
        printf("%s\n",newdest );
      }
      copied++;
    }
  }else if(typeo==1 && typed==1){ //both directories

    if((c=searchdir(neworig,newdest))>0){ //search their content
      counter= counter+c;
    }
  }else{ //both files
    if(statdest.st_size != statorg.st_size){ //different sizes
      if(copyfile(neworig,newdest)==-1){
        fprintf(stderr,"Error in copying %s\n",newdest );
      }else if(v==1){
        printf("%s\n",newdest );
      }
      copied++;

   }else if(statdest.st_mtime < statorg.st_mtime){   //has been modified since
     if(copyfile(neworig,newdest)==-1){
       fprintf(stderr,"Error in copying %s\n",newdest );
     }else if(v==1){
       printf("%s\n",newdest );
     }
     copied++;
    }
  }

  return counter;
}
