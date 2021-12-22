/*
 * thread.c - simple example demonstrating the creation of threads
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(){

   float a=0, b=0, c=0;
   
   pid_t pid, cpid;
   int status;
   
   
   pid = fork();
   if(pid == 0) {
      a=1+1;
      printf("a: (in Proc)= %.2f\n",a);
      exit(0);
   }else {
      if ((cpid=wait(&status)) == pid){
         printf("Child %d returned\n",pid);
      }
      sleep(5);
      b=2+2;
      printf("a: (in Parent)=%.2f\n", a);
      printf("b=%.2f\n", b);
      c=a+b;
      printf("c=%.2f\n", c);
   }
   return 0; 
}