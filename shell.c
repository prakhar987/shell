#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <sys/wait.h>


    char c[100][100];     // contains one command in each row (PARSED WITH ';')
    char c1[100][100];    // contains parts of commands seperated by spaces (PARSED WITH ';')
    char cwd[100];        // CONTAINS CURRENT WORKING DIRECTORY
    char home[100];       // remembers the path from where shell was invoked
    char *ptr[100];       // THIS IS PASSED TO execvp()
    char *user_name;      // points to user name
    char machine_name[20];  // contians machine name
    char input[100];           // stores input
    int bk=0;
    int back=0; // 1 if & encountered
    int CTRL_C =0;
    int tx=-1;

pid_t shell_pgid;     //shell's process group id
int interactive,shell_terminal,redirection;
pid_t child_pid;      //pid of the foreground running process (0 if shell is in foreground)

struct bak_ground {      //queue storing the details of the processes
  char pname[200];
  pid_t pid,pgid;
} b[100],tempo;



void init_shell()  // ALL PROCESS WITH GROUP ID shell_pgid HAVE ACCESS TO TERMINAL 
{
  shell_terminal=STDERR_FILENO;back=0; // Shell_Terminal has FD of Terminal       
  interactive=isatty(shell_terminal);   // TESTS Wether FD refers to teminal 1 : Terminal 0: Not
  if(interactive)
  {
    while(tcgetpgrp(shell_terminal) != (shell_pgid=getpgrp()))
      kill(- shell_pgid,SIGTTIN);
  }
  signal (SIGINT, SIG_IGN);  // IGNORE ALL SIGNALS
  signal (SIGTSTP, SIG_IGN);
  signal (SIGQUIT, SIG_IGN);
  signal (SIGTTIN, SIG_IGN);
  signal (SIGTTOU, SIG_IGN);
  shell_pgid=getpid();           // Create a group with SHELL as Leader
  if(setpgid(shell_pgid,shell_pgid)<0)
  {
    perror("Can't make shell a member of it's own process group");
    _exit(1);
  }
  tcsetpgrp(shell_terminal,shell_pgid);  // all processes with pgid of shell have terminal
}
/*insert a new process into the jobs table*/
void insert_process(char name[200],pid_t pid,pid_t pgid)
{
  
  strcpy(tempo.pname,name);
  tempo.pid=pid;
  tempo.pgid=pgid;
   tx++;
    b[tx]=tempo;
      
  
}
/*remove a process from jobs table on termination*/
void remove_process(pid_t pid)
{
  if(tx!= -1 )
  {
      int i;
      for (i=0;i<=tx;i++)
      {
        if (b[i].pid==pid)
          break;
      }
      char ty[100];pid_t qw,we;
      for (; i <tx; ++i)
      {  strcpy(ty,b[i+1].pname);qw=b[i+1].pid;we=b[i+1].pgid;
         strcpy(b[i].pname,ty);b[i].pid=qw;b[i].pgid=we;
      }
      b[tx].pname[0]='\0';
      b[tx].pid=0;b[tx].pgid=0;
      tx--;
  
}
}

void sig_handler(int signo) // WE HANDLE TWO SIGNALS - SIGINT & SIGCHLD
{
  if(signo==SIGINT)   // here #@!#@!#@!$!
  {
    fprintf(stderr,"\n");CTRL_C=1;
    main();
  }
  else if(signo==SIGCHLD)
  {
    int status,i;
    pid_t pid;
    while((pid=waitpid(-1,&status,WNOHANG))>0)                    //return if no child has changed state
    {
      if(pid>0)
      {
        if(WIFEXITED(status))  // Terminates Normally     
        {
          for(i=0;i<=tx;i++)
            if(b[i].pid==pid)
              break;
          if(b[i].pid!=0)
          {
            fprintf(stdout,"%s :Process ID %d Exited Normally\n",b[i].pname,pid);
            remove_process(b[i].pid);     
          }
        }
        else if(WIFSIGNALED(status))  // EXITED DUE TO SIGNAL
        {
          for(i=0;i<=tx;i++)
             if(b[i].pid==pid)
               break;
          if(b[i].pid!=0)
          {
            fprintf(stdout,"\n%s with Process ID %d was signalled to Exit\n",b[i].pname,pid);
            remove_process(b[i].pid);
          }
        }
      }
    }
  }
}



void parser(char input[],int j)
    {          
             int i=0;
            char escape[]=  {'\t',' '};
            char *p,*q,dup[100][100];
            for (p = strtok(input,";"); p != NULL; p = strtok(NULL, ";"),i++)  // PARSING WITH semicolon
            {strcpy(&c[i][0],p);strcpy(&dup[i][0],p);}
            i=0;
            for (p = strtok(&dup[j][0],escape); p != NULL; p = strtok(NULL, escape),i++) // PARSING WITH spaces
                        strcpy(&c1[i][0],p);

    }

// CLEANS THE ARRAYS WHICH HOLD COMMANDS
void cleaner()
    {
            int i,j;
            for(i=0;i<100;i++)
            { ptr[i]=NULL;
            for(j=0;j<100;j++)
            { c[i][j]='\0';c1[i][j]='\0';}}
    }


// ROUTINE TO HANDLE COMMANDS WHICH ARE not INBUILT
void external_cmd()
  { int p=0,run;
    pid_t pid;

    pid=fork();           //fork the child
    if(pid<0)
      perror("Child process not created");
    else if(pid==0)   
    { 
      
      /*inside child*/
      prctl(PR_SET_PDEATHSIG, SIGHUP);    //kill the child when parent dies.(prevents formation of zombie process)
      setpgid(getpid(),getpid());     // BY DEFAULT WE MAKE BACKGROUND PROCESSES

      if(back==0)
        tcsetpgrp(shell_terminal,getpid()); // IF FOREGROUND , ASSIGN SAME GROUP AS SHELL
      // ALL signals of child are handled as default
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
      run=execvp(ptr[0],ptr);
      if(run<0)
      {
        perror("Error exectuing command");
        _exit(-1);
      }
      _exit(0);
    }
    if(back==0) // if the child just created was FOREGROUND
    { 
      tcsetpgrp(shell_terminal,pid);        //GROUPS ARE ALWAYS SET IN BOTH PARENT AND CHILD TO AVOID RACES
      insert_process(ptr[0],pid,pid);
      int status;
      child_pid=pid; // THis means child is controlling the terminal
      waitpid(pid,&status,WUNTRACED);       //wait for child till it terminates or stops
      if(!WIFSTOPPED(status))
        remove_process(pid);
      else
        fprintf(stderr,"\n[%d]+ stopped %s\n",child_pid,ptr[0]);
      tcsetpgrp(shell_terminal,shell_pgid);         //return control of terminal to the shell
    }
    else // BACKGROUND PROCESS IS PUT IN THE LIST
      insert_process(ptr[0],pid,pid);

   
    fflush(stdout);
  }

void filehandler()
  {
    char *pin,*pout=NULL;
              int i,j,fd,index;
              char command[100],dup[100],*pp[100],*p;
              char escape[]= {'<','>'};
              pid_t pid;
              for(i=0;i<100;i++)
                    { command[i]='\0'; pp[i]=NULL; }
              pin=strchr(&c[bk][0],'<');
              pout=strchr(&c[bk][0],'>');
              // to handle >>
              int appender=0;
              if(pout!=NULL)
                  {   index=(int)(pout-&c[bk][0]);
                      if (c[bk][index+1]=='>')
                      appender=1;
                  }
    

              
              if (pin !=NULL || pout!=NULL)
                    {
                      strcpy(dup,&c[bk][0]);  // command is in dup[]
                      i=0;
                      for (p = strtok(dup,escape); p != NULL; p = strtok(NULL, escape),i++) 
                          {   if (i==0)
                                   strcpy(command,p);
                              if(i==1 && pin==NULL)
                                  pout=p;
                              if(i==1 && pin !=NULL)
                                pin=p;
                              if (i==2)
                                pout=p;
                          }
                       //trim pin pout
                             char temp[100],temp1[100];
                             if(pin!=NULL)
                                {strcpy(temp,pin);pin=strtok(temp," ");}
                             if (pout!=NULL)
                                {strcpy(temp1,pout);pout=strtok(temp1," ");} 
                                
                       // create pointer of array of command
                      i=0;
                      for (p = strtok(command," "); p != NULL; p = strtok(NULL," "),i++) 
                           pp[i]=p;  
                      // begin execution
                      pid = fork();
                      if (pid==0)
                          {                   
                                              // child
                                              if(pin!=NULL)
                                                  {
                                                  fd = open(pin, O_RDONLY);
                                                  dup2(fd, 0);  
                                                  close(fd);
                                                  }
                                              if(pout!=NULL && appender==0)
                                                  {  
                                                  fd = open(pout,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                                                  dup2(fd, 1);  
                                                  close(fd);
                                                  }
                                              if(pout!=NULL && appender==1)
                                                  {  
                                                  fd = open(pout,O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                                                  dup2(fd, 1);  
                                                  close(fd);
                                                  }
   
                                                  execvp(pp[0],pp);
                                                  _exit(0);
                                                  }
                      else
                        waitpid(pid,NULL,WUNTRACED);
                      
                    }
  }

// search for & | < > << >>
 int search()
      {       
              int rt_val=0;
              int pd[2];
              int k=0,fd,j,i;
              char *pin,*pout,*p;
              char dup[100],*pp[100];
              // Check Backgrounding
              char *ptr;
              int index;
              ptr=strchr(&c[bk][0],'&');
              if(ptr!=NULL)  // & DETECTED
              {
                index=(int)(ptr-&c[bk][0]);
                input[index]='\0';
                parser(input,bk);
                back=1;
              }  
      // ################ INPUT OUTPUT REDIRECTION ##################
              pin=strchr(&c[bk][0],'<');
              pout=strchr(&c[bk][0],'>');
              if (pin !=NULL || pout != NULL)
              {
                rt_val=1;
                filehandler();
              }
    

              
        // #####################################check for pipelines ################################
              pin=strchr(&c[bk][0],'|');
              if(pin != NULL)
              {
                  pid_t pid;
                  char duplicate[50][50];
                  strcpy(dup,&c[bk][0]);
                  i=0;
                  for (p = strtok(dup,"|"); p != NULL; p = strtok(NULL,"|"), i++) 
                    strcpy(&duplicate[i][0],p);  
                  j=i;
                  for( i=0; i<j; i++)
                  {   
        // tokenizing 
                      pipe(pd);
                      for(k=0;k<100;k++)
                       pp[k]=NULL;
                      k=0;
                      for (p = strtok(&duplicate[i][0]," "); p != NULL; p = strtok(NULL," "),k++) 
                       pp[k]=p;                      
    pid=fork();           //fork the child
    if(pid==0)   
    { 
        tcsetpgrp(shell_terminal,getpid()); // IF FOREGROUND , ASSIGN SAME GROUP AS SHELL
      // ALL signals of child are handled as default
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
      if(i!=(j-1))
      dup2(pd[1], 1); // remap output back to parent
      execvp(pp[0],pp);
      _exit(0);
    }
     
      tcsetpgrp(shell_terminal,pid);        //GROUPS ARE ALWAYS SET IN BOTH PARENT AND CHILD TO AVOID RACES
      insert_process(pp[0],pid,pid);
      int status;
      child_pid=pid; // THis means child is controlling the terminal
      waitpid(pid,&status,WUNTRACED);       //wait for child till it terminates or stops
      if(!WIFSTOPPED(status))
        remove_process(pid);
      else
        fprintf(stderr,"\n[%d]+ stopped %s\n",child_pid,pp[0]);
      tcsetpgrp(shell_terminal,shell_pgid);         //return control of terminal to the shell
    
    //else // BACKGROUND PROCESS IS PUT IN THE LIST
     // insert_process(ptr[0],pid,pid);
                    
                 
      // remap output from previous child to input
                    dup2(pd[0], 0);
                    close(pd[1]);
                  }
                  /*for(k=0;k<100;k++)
                    pp[k]=NULL;
                  k=0;
                  for (p = strtok(&duplicate[i][0]," "); p != NULL; p = strtok(NULL," "),k++) 
                    pp[k]=p;    
                  execvp(pp[0], pp);
                  _exit(0);
                    */     
                  rt_val=2;
                  close(pd[0]);close(pd[1]);dup2(STDERR_FILENO,0);dup2(STDERR_FILENO,1);
              }
                   
          return rt_val;
             }
// THIS ROUTINE HANDLES THE COMMANDS INBUILT COMMANDS
void handler()
      {      //C1[][] IS THE ARRAY CONTAING THE TOKENISED COMMAND TO EXECUTE
            int i;char *x;char temp[100];char xyz[100];int len1;int len2;pid_t pid;
            // CHECK FOR & | < > << >>
            if(search()) {
                 return;
            }
            if(strcmp(&c1[0][0],"echo")==0)                // handle echo
            {  i=1;
            while(c1[i][0]!='\0') {
               printf("%s ",&c1[i][0]);
                 i++;
                  }
                printf("\n");
            }
            else if (strcmp(&c1[0][0],"jobs")==0)                // #!@#!@#$!$!
            {      int i;
                for (i = 0; i <= tx; ++i)
                {
                  fprintf(stdout, "%d %s %d\n",(i+1),b[i].pname,b[i].pid );
                }
            }
            else if(strcmp(&c1[0][0],"kjob")==0)     //  #@!$!$@#@!#$            
            {
              
            if(c1[3][0]=='\0')                 
             {  int pid,n;
                n=atoi(&c1[1][0]);
                n--;
                
                
                if(n<=tx)
                    pid= b[n].pid;
                else
                    pid= -1;

                if(pid>=0)
                {
                   if(killpg(getpgid(pid),atoi(&c1[2][0]))<0)
                      perror("Signal not sent!!");
                }
                else
                fprintf(stderr,"No such process !!\n");
             }
            else
                fprintf(stderr,"Enter 2 Argument!!\n");
            }
            else if(strcmp(&c1[0][0],"fg")==0)                //make the job args[1] the foreground process
            {
    
    
      int status;
      pid_t pgid,pid;
      int n;
                n=atoi(&c1[1][0]);
                n--;
                if(n<=tx)
                    pid= b[n].pid;
                else
                    pid= -1;
      if(pid>=0)               //get the pgid of the job
      {  

        fprintf(stderr,"%s\n",b[n].pname);                                        
        pgid=getpgid(pid);
        tcsetpgrp(shell_terminal,pgid);           //give control of the terminal to the process
        child_pid=pgid;
        if(killpg(pgid,SIGCONT)<0)                //send a SIGCONT signal
          perror("Can not be killed!");
        waitpid(pid,&status,WUNTRACED);           //wait for it to stop or terminate
        if(!WIFSTOPPED(status))
        {
          remove_process(pid);          //if terminated remove from jobs table
          child_pid=0;
        }
        else
         fprintf(stderr,"\n%d + process stopped :%s\n",child_pid,b[n].pname);   //if stopped print message
         tcsetpgrp(shell_terminal,shell_pgid);             //return control of terminal to the shell
      }
      else
        fprintf(stderr,"No Process !!\n");
    }    
        else if(strcmp(&c1[0][0],"overkill")==0)              //          kill all processs
   {
                
                for(i=0;i<=tx;i++)
                  {
                 if(killpg(getpgid(b[i].pid),9)<0)
                perror("Error killing process");
    }
   }
  
 
            else if (strcmp(&c1[0][0],"pwd")==0)         // handle pwd
            { if(getcwd(xyz, sizeof(xyz)) == NULL)
                        perror("getcwd() error");
              printf("%s\n",xyz);
            }

            else if   (strcmp(&c1[0][0],"cd")==0 )          // handle cd
            {
            if(c1[1][0]=='\0')
            strcpy(&c1[1][0],home);
            if(chdir(&c1[1][0])==-1)
             perror("");
            if(getcwd(cwd, sizeof(cwd)) == NULL)
                        perror("getcwd() error");
            len1=strlen(home);len2=strlen(cwd);
            if(len1>len2)
            return;
            i=strlen(home);
            x=&(cwd[i]);
            strcpy(temp,x);
            strcpy(cwd,temp);



            }
            else if (strcmp(&c1[0][0],"quit")==0)       // handle exit
            {
                 _exit(0);
            }
            else if (strcmp(&c1[0][0],"pinfo")==0)
            { pid=getpid();
            printf("PROCESS ID=%d\n",pid);

            }
            else if (strcmp(&c1[0][0],"clear")==0)         // handle clear
            {   printf("\e[1;1H\e[2J");
            }
            else                                        //  to handle commands which are not inbuilt
            {
            for(i=0;c1[i][0]!='\0';i++)
            { ptr[i]=&c1[i][0];
            }
                external_cmd();                         // calls to external_cmd() to handle external commands
            }
}
//main

int main()
        {

                
                int i,j;
                char dup[100],x;
                if (CTRL_C==0)
                printf("\e[1;1H\e[2J"); // clear the screen
                  CTRL_C=0;
                if(getcwd(home, sizeof(home)) == NULL)      // gets Present working directory
                perror("getcwd() error");              // and stores to home
                init_shell();
                signal(SIGINT,sig_handler);
                signal(SIGCHLD,sig_handler);
                user_name=getlogin();                     // get username
                gethostname(machine_name,20);            // get machine name

                while(1)                                 // THE MAIN LOOP WHICH RUNS TILL exit IS TYPED
                {back=0;
                printf("<%s@%s:~%s> ",user_name,machine_name,cwd);
                fgets(input,100,stdin);
                input[strlen(input)-1]='\0';
                strcpy(dup,input);
                // ALL COMMANDS ARE IN input
                i=0;
                parser(dup,i);
                while(c[i][0]!='\0')
                {
                bk=i;
                handler();   // CALL MAIN FUNCTION handler()
                cleaner();
                i++;
                strcpy(dup,input);
                parser(dup,i);  // ready the next command to process
                }
                i=0;
                cleaner();

                }
                return 0;
                }
