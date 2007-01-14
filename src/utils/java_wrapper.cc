/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark, service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
*********************************************************************/

#include "sbsat.h"
#ifdef HAVE_JNI_H
 
extern "C" {

#include <jni.h>
#include "sat_server_ServerImpl.h"
int main(int argc, char *argv[]);

#define PROBLEMFILE "problem.dat"
#define ERRORFILE "error.dat"
#define RESULTFILE "result.dat"
#define PUSH_SIGNAL 133
#define PULL_SIGNAL 134

/* Number of arguements */
#define SIZE 20

JNIEnv * global_env;
jobject global_obj;


JNIEXPORT jobject JNICALL Java_sat_server_ServerImpl_process
                (JNIEnv * env, jobject jo, jobject ii, jstring confpath)
{
  char buf[128];
  const char *str;
  const char *path;
  FILE *fp;
  FILE *fp_r; /* result */
  FILE *fp_e; /* errors */
  FILE *fp_i; /* input */
  FILE * fp_conf;  /* file pointer for configuration file */

  FILE  *stdout_backup, *stderr_backup;
  char* argv[SIZE];
  int pid,pid2, status;
  const char *temp;

  char temp_str[1000];
  char* t;
  int ik, count =0;
  char * chrpt;

  /*  printf("From Inside the native method");  */
  system("");

  /* For some crazy reasone, the program seems to need random system calls
     or else, it sometimes freezes up when invoked from inside java. */

  /* Store method arguements in global variables */
  global_env = env;
  global_obj = jo;

  /*******************************/
#ifdef __cplusplus
  path =(env)->GetStringUTFChars( (jstring) confpath, 0);
#else
  path = (*env)->GetStringUTFChars(env, confpath, 0);
#endif

  for(ik=0;ik<SIZE;ik++)
    argv[ik] = (char *) malloc(sizeof(char)*128);
 
  fp_conf = fopen(path , "r");
  fgets(temp_str , 1000, fp_conf);

  /* remove newline character and replace it with blank space*/
  chrpt = strchr(temp_str, '\n') ;
  *chrpt = ' ';

  fflush(fp_conf);
  fclose(fp_conf);

#ifdef __cplusplus
  (env)->ReleaseStringUTFChars( (jstring)confpath, path);
#else
  (*env)->ReleaseStringUTFChars( env,  confpath, path);
#endif

  t = (char *) malloc(sizeof(char)*128);
  t = strtok(temp_str , " ");
  if(t==NULL) {printf("Empty File: conf"); exit(-1); }
  else argv[count] = t;

  while (count < SIZE && t!=NULL) {
    count ++;
    t = strtok(NULL," ");
    argv[count] = t;
  }
  
  /*******************************/
  
  fp = fopen(PROBLEMFILE , "w");
  
#ifdef __cplusplus
  str =(env)->GetStringUTFChars( (jstring) ii, 0);
#else
  str = (*env)->GetStringUTFChars(env, ii, 0);
#endif
  
  fprintf(fp, str);
  fflush(fp);
  fclose(fp);
  
#ifdef __cplusplus
  (env)->ReleaseStringUTFChars( (jstring)ii, str);
#else
  (*env)->ReleaseStringUTFChars( env,  ii, str);
#endif


  /* Done writing the input problem to a file */
  /* Backup the values of stdout and stderr */
  stdout_backup = stdout;
  stderr_backup = stderr;
  
  fp_r = fopen(RESULTFILE, "w");
  fp_e = fopen(ERRORFILE, "w");
  fp_i = fopen(PROBLEMFILE, "r");

  /* change stdout and stderr */
  stderr = fp_e;
  stdout = fp_r;
  stdin = fp_i;


#ifdef __cplusplus  
  ofstream outfile( RESULTFILE );
  ofstream errfile( ERRORFILE );
  ifstream infile ( PROBLEMFILE );
  cout = outfile;
  cerr = errfile;
  cin = infile;
#endif


  pid = fork();

  if(pid != 0) {
    /* parent code */
    pid2 = fork();

    /***************************************/
    if (pid2 !=0) {
      // waitpid(-1, &status, 0);
      wait( &status );
   
   printf("Child Terminated with status: ");
   printf("%d", status ); printf("\n");

   fflush(fp_r);
   fclose(fp_r);

   fflush(fp_e);
   fclose(fp_e);
   
   stdout = stdout_backup;
   stderr = stderr_backup ;
   
   if(status == 0) {
     /* return the name of result.dat */
     //    printf("exit status is 0");
     strcpy(buf, RESULTFILE);
   }
   else {
     /* return the name of error.dat */
     //    printf("exit status is not 0");
     strcpy(buf, ERRORFILE);
   }
   sleep(1);
   kill(pid2, 15);

#ifdef __cplusplus
   return (env)->NewStringUTF( buf);
#else
   return (*env)->NewStringUTF(env, buf);
#endif


    }
    
    else {
      /* signal the pid process (process running main) at intervals */
      while(1) {
   sleep (2);
   temp =  getenv("PUSHSIG_AVAILABLE") ;
   if (temp!= NULL &&  strcmp(temp, "set") == 0 )  kill(pid , PUSH_SIGNAL);
      }

    }

    /***************************************/

  }
  else {
    /* child code */
    /*    sigaction(PUSH_SIGNAL , &act , &oaction);    */
    sigaction(PUSH_SIGNAL , NULL, NULL);

    system(""); /* This is required for some weird reasone !! */

#ifdef __cplusplus
    ios::sync_with_stdio(0);
#endif
    
    main(count, argv);
    fflush(fp_i);
    fclose(fp_i);
    exit(0);
  }

}

/****************************/
void broadcast(const char * msg) {
  jmethodID mid;
  jclass cls;
  jstring jss ;
  char buf[128];

  strcpy(buf ,msg);

#ifdef __cplusplus
  jss = (global_env)->NewStringUTF( buf);
  cls = (global_env)->GetObjectClass(  global_obj);
  mid = (global_env)->GetMethodID( cls, "broadcastMessege", "(Ljava/lang/String;)V");
#else
  jss = (*global_env)->NewStringUTF(global_env, buf);
  cls = (*global_env)->GetObjectClass(global_env,  global_obj);
  mid = (*global_env)->GetMethodID(global_env, cls, "broadcastMessege", "(Ljava/lang/String;)V");
#endif

  if (mid == 0) {
    printf("could not find method broadcstMessege");
    return;
  }
#ifdef __cplusplus
  (global_env)->CallVoidMethod(global_obj, mid, jss);
#else 
  (*global_env)->CallVoidMethod(global_env, global_obj, mid, jss);
#endif

}

} /* extern "C" */
#endif
/****************************/
