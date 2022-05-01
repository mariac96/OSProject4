#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "directory.h"
#include "file.h"
#include <fcntl.h>

extern int v,copied;

//copy everything in the directory origin to destin
int copydirectory(char*origin,char *destin){
  DIR *destptr,*origptr;
  struct dirent *ordir;
  struct stat statorg;
  char 	*neworig,*newdest;
  int counter=0,l;

  if((origptr=opendir(origin))==NULL){
    perror("Error in opendir:\n");
    return -1;
  }


  if ((destptr=opendir(destin))== NULL ) {
  	perror("Error in opendir:\n");
    return -1;
  	}

   while ((ordir = readdir(origptr)) != NULL ) {

    		if (ordir->d_ino == 0 ) continue; //it is deleted
    		neworig=(char *)malloc(strlen(origin)+strlen(ordir->d_name)+2);

        if((strcmp(".",ordir->d_name)==0) || (strcmp("..",ordir->d_name)==0)){//if it is . or ..
          free(neworig);
          neworig=NULL;
          continue;
        }
        counter++;
        newdest = (char *)malloc(strlen(destin)+strlen(ordir->d_name)+2);
        strcpy(neworig,origin);
    		strcat(neworig,"/");
    		strcat(neworig,ordir->d_name);
        strcpy(newdest,destin);
        strcat(newdest,"/");
        strcat(newdest,ordir->d_name);

        if(stat(neworig,&statorg)<0){
          perror("Error in stat:\n");
          free(neworig);
          free(newdest);
          neworig=NULL;
          newdest=NULL;
          continue;

        }

        if((statorg.st_mode & S_IFMT ) == S_IFDIR){ //if it is a directory

          if(mkdir(newdest,statorg.st_mode)==-1){
            perror("Error in mkdir:\n");
            return -1;
          }
          if(v==1){
            printf("Created directory %s\n",newdest );
          }
          if((l=copydirectory(neworig,newdest))>0){
            counter= counter+l;
          }
          copied++;
          free(neworig);
          free(newdest);
          neworig=NULL;
          newdest=NULL;

        }else if((statorg.st_mode & S_IFMT ) == S_IFREG){ //if it is a file
          if(creat(newdest,statorg.st_mode)==-1){
            perror("Error in creat:\n");
            return -1;
          }


          if(copyfile(neworig,newdest)==-1){ //copy file
            fprintf(stderr,"Error in copying %s\n",newdest );
          }else if(v==1){
            printf("%s\n",newdest );
          }
          copied++;
          free(neworig);
          free(newdest);
          neworig=NULL;
          newdest=NULL;
        }
    	}//end while
  	closedir(origptr);
    closedir(destptr);

    return counter;
}

//search and copy only the files that have changed or don't exist
int searchdir(char*origin,char*destin){
  DIR *destptr,*origptr;
  struct dirent *ordir;
  struct stat statorg,statdest;
  char 	*neworig,*newdest;
  int r,counter=0,c;

  if((origptr=opendir(origin))==NULL){
    perror("Error in opendir:\n");
    return -1;
  }


  if ((destptr=opendir(destin))== NULL ) {
    perror("Error in opendir:\n");
    return -1;
  	}

   while ((ordir = readdir(origptr)) != NULL ) {

    		if (ordir->d_ino == 0 ) continue; //it is deleted
    		neworig=(char *)malloc(strlen(origin)+strlen(ordir->d_name)+2);

        if((strcmp(".",ordir->d_name)==0) || (strcmp("..",ordir->d_name)==0)){//if it is . or ..
          free(neworig);
          neworig=NULL;
          continue;
        }
        counter ++;
        newdest = (char *)malloc(strlen(destin)+strlen(ordir->d_name)+2);
        strcpy(neworig,origin);
    		strcat(neworig,"/");
    		strcat(neworig,ordir->d_name);
        strcpy(newdest,destin);
        strcat(newdest,"/");
        strcat(newdest,ordir->d_name);

        r=stat(newdest,&statdest);

        if(r==-1 && errno==ENOENT){ //the file does not exist in the destination directory, copy it

          if(stat(neworig,&statorg)==-1){
            perror("Error in stat:\n");
            free(neworig);
            free(newdest);
            neworig=NULL;
            newdest=NULL;
            continue;

          }
          if((statorg.st_mode & S_IFMT ) == S_IFDIR){ //it is a directory

            if(mkdir(newdest,statorg.st_mode)==-1){
              perror("Error in mkdir:\n");
              return -1;
            }
            if(v==1){
              printf("Created directory %s\n",newdest );
            }
            if((c=copydirectory(neworig,newdest))>0){ //copy everything
              counter=c+counter;
            }
            copied++;
          }else if((statorg.st_mode & S_IFMT ) == S_IFREG){ //it is a file

            if(creat(newdest,statorg.st_mode)==-1){
              perror("Error in creat:\n");
              return -1;
              }


            if(copyfile(neworig,newdest)==-1){ //copy file
              fprintf(stderr,"Error in copying %s\n",newdest );
            }else if(v==1){
              printf("%s\n",newdest );
            }
            copied++;
          }
        }else if(r!=-1){ //the file exists
          if((c=comparefiles(neworig,newdest))>0){ //check if they are the same
            counter = c+counter;
          }
        }else{
          perror("Error in stat:\n");

        }
        free(neworig);
        free(newdest);
        neworig=NULL;
        newdest=NULL;
      }//end while
        closedir(origptr);
        closedir(destptr);

        return counter;
}

//search and delete the files in the destination directory that don't exist in the origin
int searchdelete(char* origin,char* dest){
  DIR *destptr,*origptr;
  struct dirent *destdir;
  struct stat statorg,statdest;
  char 	*neworig,*newdest;
  int  r,counterd=0,c;

  if((origptr=opendir(origin))==NULL){
    perror("Error in opendir:\n");
    return -1;
  }


  if ((destptr=opendir(dest))== NULL ) {
    perror("Error in opendir:\n");
    return -1;
  	}

   while ((destdir = readdir(destptr)) != NULL ) {

    		if (destdir->d_ino == 0 ) continue; //it is deleted
    		newdest=(char *)malloc(strlen(dest)+strlen(destdir->d_name)+2);

        if((strcmp(".",destdir->d_name)==0) || (strcmp("..",destdir->d_name)==0)){//if it is . or ..
          free(newdest);
          newdest=NULL;
          continue;
        }

        neworig = (char *)malloc(strlen(origin)+strlen(destdir->d_name)+2);
        strcpy(neworig,origin);
    		strcat(neworig,"/");
    		strcat(neworig,destdir->d_name);
        strcpy(newdest,dest);
        strcat(newdest,"/");
        strcat(newdest,destdir->d_name);

        if(stat(newdest,&statdest)==-1){
          perror("Error in stat:\n");
          free(neworig);
          free(newdest);
          neworig=NULL;
          newdest=NULL;
          continue;

        }
        r=stat(neworig,&statorg);
        if(r==-1 && errno==ENOENT){ //the file does not exist in the origin directory


          if((statdest.st_mode & S_IFMT ) == S_IFDIR){ //it is a directory

            if((c=deletedirectory(newdest))>0){ //delete the directory contents
              counterd = counterd +c;
            }
            if(remove(newdest)==-1){
              perror("Error in remove:\n");

            }else{
              counterd++;
              if(v==1){
                printf("Deleted %s\n",newdest );
              }
            }

          }else if((statdest.st_mode & S_IFMT ) == S_IFREG){ //it is a file

            if(remove(newdest)==-1){
              perror("Error in remove:\n");

            }else{
              counterd++;
              if(v==1){
                printf("Deleted %s\n",newdest );
              }
            }

          }
        }else if(r==-1){
          perror("Error in stat:\n");

        }else if((statdest.st_mode & S_IFMT ) == S_IFDIR){ //it exists and  it is a directory check its content

          if((c=searchdelete(neworig,newdest))>0){
            counterd = counterd +c;
          }
        }
        free(neworig);
        free(newdest);
        neworig=NULL;
        newdest=NULL;
    }//end while
        closedir(origptr);
        closedir(destptr);
        return counterd;
}

//delets the files in the directory dest
int deletedirectory(char*dest){
  DIR *destptr;
  struct dirent *destdir;
  struct stat statdest;
  char 	*newdest;
  int  counterd=0,c;


  if((destptr=opendir(dest))==NULL){
    perror("Error in opendir:\n");
    return -1;
  }




   while ((destdir = readdir(destptr)) != NULL ) {

    		if (destdir->d_ino == 0 ) continue; //it is deleted
    		newdest=(char *)malloc(strlen(dest)+strlen(destdir->d_name)+2);

        if((strcmp(".",destdir->d_name)==0) || (strcmp("..",destdir->d_name)==0)){//if it is . or ..
          free(newdest);
          newdest=NULL;
          continue;
        }


        strcpy(newdest,dest);
        strcat(newdest,"/");
        strcat(newdest,destdir->d_name);

        if(stat(newdest,&statdest)==-1){
            perror("Error in stat:\n");
            continue;

          }
        if((statdest.st_mode & S_IFMT ) == S_IFDIR){ //it is a directory

            if((c=deletedirectory(newdest))>0){
              counterd = counterd +c;
            }
            if(remove(newdest)==-1){
              perror("Error in remove:\n");

            }else{
              counterd ++;
              if(v==1){
                printf("Deleted %s\n",newdest );
              }
            }
        }else if((statdest.st_mode & S_IFMT ) == S_IFREG){ //it is a file

            if(remove(newdest)==-1){
              perror("Error in remove:\n");

        }else{
          if(v==1){
            printf("Deleted %s\n",newdest );
          }
          counterd ++;
        }

      }

        free(newdest);
        newdest=NULL;
    }//end while


        closedir(destptr);
        return counterd;
}
