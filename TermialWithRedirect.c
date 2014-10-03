#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<malloc.h>
#include<unistd.h>
#include<signal.h>
#include<wait.h>
#include<fcntl.h>
#include<pthread.h>
#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>
#include<limits.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>

/////////////////////////////////////////////////////
struct array
{
	int pid;
	char pname[100];
	int x;
};
////////////////////////////////////////////////////////////
int tog=-100;
char *in;
char *out;
int inflag;
int outflag;
void redirect(char *emrg);
//int fd[10][2];
typedef struct array proc;
int history_legnth=0;//Total Number of History Stored
int arg_count=0;//Counts No of Argument Given
char hisbuff[100][15];//Stores History.
void  parse(char *line, char **argv);//Splits String in Command and Arguments
void execute(char **argv);//Executes Commands like ps using execv
proc* arr[100];//Stores processes executed
proc* bgarr[100];//stores BG processes
int counter=0;
int bgcounter=0;
char command[100];
void changecommand(char *command,char **argv)
{
	int i=0;
	int x=0;
	while(1)
	{
		if(command[i]==' ')
		{
			command[i]='\0';
			strcpy(argv[x],command);
			command=&command[i+1];
			i=0;	
			x++;
			tog=-100;
		}
		if(command[i]=='\0')
		{
			if(strcmp(command,"&")==0)
			{
				tog=10;
			}
			else
			{
				strcpy(argv[x],command);
				tog=-100;
				x++;
			}
			break;
		}
		i++;
	}
	argv[x]=NULL;
}	
void signal_handler()//Handles Signal
{
	signal(SIGINT, signal_handler);
}
////////////////////////////////////////////////////////////////
void prompt()//Prints Prompt Each Time a Command is Executed
{
	char hostname[1024];
	hostname[1023]='\0';
	gethostname(hostname, 1023);
	char *machinename=getenv("USER");
	char cwd[1024];
	char *cw = getcwd(cwd, 1024*sizeof(char));
	char *rem=getenv("HOME");
	int remov=strlen(rem);
	cw[remov-1]='~';
	cw=cw+remov-1;
	printf("%s@%s:%s>",machinename,hostname,cw);
	fflush(stdout);
}
///////////////////////////////////////////////////////////////////
void child_handler()
{

	int pid = waitpid(WAIT_ANY, NULL, WNOHANG);
	int i;
	for(i=0;i<bgcounter;i++)
	{
		if(bgarr[i]->pid==pid)
		{
			bgarr[i]->x=0;
			printf("%s  %d exited normally\n",arr[i]->pname,arr[i]->pid);
			prompt();
			break;
		}
	}
	fflush(stdout);
}
///////////////////////////////////////////////////////////////
void printhistory(int len)//Prints History 
{
	int i;
	if(len<history_legnth)
	{
		for(i=history_legnth-len;i<history_legnth;i++)
		{
			printf("%s\n",hisbuff[i]);
		}
	}
	else
	{
		for(i=0;i<history_legnth;i++)
		{
			printf("%s\n",hisbuff[i]);
		}
	}
}
//////////////////////////////////////////////////////////////
int writehis(char *emrg)//stores history
{
	strcpy(hisbuff[history_legnth],emrg);
	history_legnth++;
}
int checkpipe(char *emrg)
{
	int i;
	for(i=0;emrg[i]!='\0';i++)
	{
		if(emrg[i]=='|')
		{
			return 1;
		}
	}
	return 0;
}
void execute1st(char *arg,int fd[10][2]);
void executelast(char str[],int fd[10][2],int x);
void executepipes(char pipes[20][100],int no_pipes)
{
	int fd[10][2];
	int i;
	int status;
	pid_t pid;
	char str[100];
	for(i=0;i<=no_pipes+1;i++)
	{
		pipe(fd[i]);
	}
	execute1st(pipes[0],fd);
//	printf("done1st");
//	fflush(stdout);
	for(i=1;i<no_pipes-1;i++)
	{
		strcpy(str,pipes[i]);
		pid=fork();
		if(pid==0)
		{			
			char *pos[100];
			char *token=strtok(str," ");
			int count=0;
			while(token!=NULL)
			{
				pos[count++]=token;
				token=strtok(NULL, " ");
			}
			pos[count++]=token;
			dup2(fd[i-1][0],0);
			close(fd[i-1][1]);
			dup2(fd[i][1],1);	//Commented For Debugging
			close(fd[i][0]);
			printf("debug");
			fflush(stdout);
			execvp(*pos,pos);
		//	close(fd[i][1]);
		//	close(fd[i-1][0]);
		//	continue;
		}
		else
		{
//			close(fd[i-1][0]);
			waitpid(pid,NULL,0);
			close(fd[i-1][0]);
			close(fd[i][1]);
//			return;
			printf("waiting for child\n");
			fflush(stdout);
		}
	}
	executelast(pipes[no_pipes-1],fd,no_pipes-2);
//	scanf("*c");
}
void executelast(char str[],int fd[10][2],int x)
{
	inflag=0;
	outflag=0;
	printf("%s\n",str);
	in=strchr(str,'<');
	out=strchr(str,'>');
	if(in!=NULL)
	{
		inflag=1;
		in[0]='\0';
		*in++;
	}
	if(out!=NULL)
	{
		outflag=1;
		out[0]='\0';
		*out++;
	}
	if((inflag==1)||(outflag==1))
	{
		printf("%d %d",inflag,outflag);
		pid_t pid;		
		char *pos[100];
		char *token=strtok(str," ");
		int count=0;
		while(token!=NULL)
		{
			pos[count++]=token;
			token=strtok(NULL, " ");
		}
		pos[count++]=token;	
		int infd,outfd;
		pid=fork();
		if(pid==0)
		{
			dup2(fd[x][0],0);
			close(fd[x][1]);
//			execvp(*pos ,pos);
			if(inflag==1)
			{
				infd=open(in,O_RDONLY);
			}
			if(outflag==1)
			{
				outfd=open(out,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR |S_IRGRP | S_IWGRP |S_IWUSR);
			}
			if(inflag==1)
			{
				dup2(infd,0);
			}
			if(outflag==1)
			{
				dup2(outfd,1);
			}
			if(outflag==1)
			{
				close(outfd);
			}
			if(inflag==1)
			{
				close(infd);
			}
			execvp(*pos,pos);
		}
		else
		{
			waitpid(pid,NULL,0);
			close(fd[x][0]);
			if(inflag==1)
			{
				close(infd);
			}
			if(outflag==1)
			{
				close(outfd);
			}
		}
		//		redirect(emrg);
	}
	else
	{
		pid_t pid;
		pid=fork();
		if(pid==0)
		{		 
			char *pos[100];
			char *token=strtok(str, " ");
			int count=0;
			while(token!=NULL)
			{
				pos[count++]=token;
				token=strtok(NULL, " ");
			}
			pos[count++]=token;
			dup2(fd[x][0],0);
			close(fd[x][1]);
			execvp(*pos ,pos);
			close(fd[x][0]);
		}
		else
		{
			waitpid(pid,NULL,0);
			close(fd[x][0]);
		}
	}
}
void execute1st(char str[],int fd[10][2])
{
	inflag=0;
	outflag=0;
	printf("%s\n",str);
	in=strchr(str,'<');
	out=strchr(str,'>');
	if(in!=NULL)
	{
		inflag=1;
		in[0]='\0';
		*in++;
	}
	if(out!=NULL)
	{
		outflag=1;
		out[0]='\0';
		*out++;
	}
	if((inflag==1)||(outflag==1))
	{
		printf("%d %d",inflag,outflag);
		pid_t pid;		
		char *pos[100];
		char *token=strtok(str," ");
		int count=0;
		while(token!=NULL)
		{
			pos[count++]=token;
			token=strtok(NULL, " ");
		}
		pos[count++]=token;	
		pos[count++]=token;
		dup2(fd[0][1],1);
		int infd,outfd;
		pid=fork();
		if(pid==0)
		{
			if(inflag==1)
			{
				infd=open(in,O_RDONLY);
			}
			if(outflag==1)
			{
				outfd=open(out,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR |S_IRGRP | S_IWGRP |S_IWUSR);
			}
			if(inflag==1)
			{
				dup2(infd,0);
			}
			if(outflag==1)
			{
				dup2(outfd,1);
			}
			if(outflag==1)
			{
				close(outfd);
			}
			if(inflag==1)
			{
				close(infd);
			}
			execvp(*pos,pos);
		}
		else
		{
			waitpid(pid,NULL,0);
			if(inflag==1)
			{
				close(infd);
			}
			if(outflag==1)
			{
				close(outfd);
			}
		}
		//		redirect(emrg);
	}
	else
	{
		pid_t pid;
		pid=fork();
		if(pid==0)
		{
			char *pos[100];
			char *token=strtok(str, " ");
			int count=0;
			while(token!=NULL)
			{
				pos[count++]=token;
				token=strtok(NULL, " ");
			}
			pos[count++]=token;
			dup2(fd[0][1],1);
			close(fd[0][0]);

			execvp(*pos,pos);
			close(fd[0][1]);
		}
		else
		{
			//	close(fd[0][1]);
			waitpid(pid,NULL,0);
			close(fd[0][1]);
		}
	}
}

void userdefined(char **argv,char *emrg,int no_arg);
int parsepipe(char *emrg,char pipe[20][100]);
int no_pipes;
///////////////////////////////////////////////////////////
int main()
{
	char nullchar;
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, signal_handler);
	signal(SIGCHLD, child_handler);
	int i;
	char pipe[20][100];
	int tempor;
	char *myname=getenv("USER");
	char emrg[100];
	chdir("/home");//takes back to home each time shell is executes
	chdir(myname);
	char *argv[64];
	while(1)
	{
		inflag=0;
		outflag=0;
		int no_arg=0;
		prompt();
		//printf("heya!!\n");
		for(i=0;i<64;i++)
		{
			argv[i]=NULL;
		}
		fflush(stdout);

		tog=-100;

		gets(command);
	//	if(command[0]=='\0'||command[0]=='\n')//exception for blank command
	//		continue;
		for(i=0;command[i]!='\0';i++)//checks if process is background
		{
			if(command[i]==' ')
			{
				no_arg++;
			}
			if(command[i]=='&')
			{
				tog=10;
				command[i]='\0';
				break;
			}
		}
		strcpy(emrg,command);


		if(checkpipe(emrg)==1)
		{
			printf("here");
			fflush(stdout);
			writehis(emrg);
			no_pipes=parsepipe(emrg,pipe);
			printf("%d\n",no_pipes);
			executepipes(pipe,no_pipes);
//			goto tp;
//			command[0]='\0';
//			gets(command);
//			scanf("%c",&nullchar);
		}
		else
		{
			if(tog!=10)
			{
				emrg[i]='\0';
			}
		inflag=0;
		outflag=0;
		printf("%s\n",emrg);
		in=strchr(emrg,'<');
		out=strchr(emrg,'>');
		if(in!=NULL)
		{
			inflag=1;
			in[0]='\0';
			*in++;
		}
		if(out!=NULL)
		{
			outflag=1;
			out[0]='\0';
			*out++;
		}
		if((inflag==1)||(outflag==1))
		{
			printf("%d %d",inflag,outflag);
			redirect(emrg);
		}
		else
		{
		printf("reached here");
		fflush(stdout);
		writehis(emrg);
		userdefined(argv,emrg,no_arg);
		}
		}
	}

	return 0;
}
void redirect(char *emrg)
{
	printf("IN REDIRECTTION");
	printf("%s\n",emrg);
	printf("%s\n",out);
	printf("%s\n",in);
	fflush(stdout);
	char str[100];
//	strcpy(temp,emrg);
	strcpy(str,emrg);
	pid_t pid;		
	char *pos[100];
	char *token=strtok(str," ");
	int count=0;
	while(token!=NULL)
	{
		pos[count++]=token;
		token=strtok(NULL, " ");
	}
	pos[count++]=token;	
	int infd,outfd;
	pid=fork();
	if(pid==0)
	{
		if(inflag==1)
		{
			infd=open(in,O_RDONLY);
		}
		if(outflag==1)
		{
			outfd=open(out,O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR |S_IRGRP | S_IWGRP |S_IWUSR);
		}
		if(inflag==1)
		{
			dup2(infd,0);
		}
		if(outflag==1)
		{
			dup2(outfd,1);
		}
		if(outflag==1)
		{
			close(outfd);
		}
		if(inflag==1)
		{
			close(infd);
		}
		execvp(*pos,pos);
	}
	else
	{
		waitpid(pid,NULL,0);
		if(inflag==1)
		{
			close(infd);
		}
		if(outflag==1)
		{
			close(outfd);
		}
	}

		//			dup2(fd[i-1][0],0);

}
//////////////////////////////////////////////////////////////////////////////////////	
int parsepipe(char *emrg,char pipe[20][100])
{
	int i;
	int j=0;
	int x=0;//Curr Position
	for(i=0;emrg[i]!='\0';i++)
	{
		if(emrg[i]=='|')
		{
			pipe[j][x]='\0';
			j++;
			x=0;
		}
		else
		{
			pipe[j][x]=emrg[i];
			x++;
		}
	}
	pipe[j][x]='\0';
	j++;
	pipe[j][0]='\0';
	return j;
/*	for(i=0;i<j;i++)
	{
		printf("%s\n",pipe[i]);
	}
*/
}
void userdefined(char **argv,char *emrg,int no_arg)
{

	int i;
	parse(command,argv);
	if(tog==10)
	{
		argv[no_arg]=NULL;
	}
	if(strcmp(emrg,"quit")==0)//Exits the Program
	{
		exit(1);
	}
	////////////////////////////Prints History////////////////
	if(strncmp(emrg,"hist",4)==0)//Prints History            
	{							
		if(emrg[4]=='\0')		
		{
			printhistory(1000);
		}
		else
		{
			printhistory(emrg[4]-'0');
		}
	}
	else if(strncmp(emrg,"!hist",5)==0)
	{
		changecommand(hisbuff[emrg[5]-1-'0'],argv);
		execute(argv);
	}
	/////////////////////////////////////////////////////////
	else if(strcmp(argv[0],"cd")==0)//Execute CD command
	{
		if(chdir(argv[1])<0)
		{
			printf("Can't Find Direcory\n");
		}
		if(argv[0][0]=='~')
		{
			char *myname=getenv("USER");
			chdir("/home");
			chdir(myname);
			chdir(&argv[0][2]);
		}
	}
	////////////////////////////////////////////////////////
	else if(strcmp(emrg,"pid")==0)//Prints Pid
	{
		printf("command name: ./a.out process id: %d\n",getpid());
	}
	else if(strcmp(emrg,"pid current")==0)
	{
		printf(" List of currently executing processes spawned from this shell:\n");
		for(i=0;i<bgcounter;i++)
		{
			if(bgarr[i]->x!=0)
			{
				printf("command name: %s process id: %d\n",bgarr[i]->pname,bgarr[i]->pid);
			}
		}
	}
	else if(strcmp(emrg,"pid all")==0)
	{
		printf(" List of all processes spawned from this shell:\n");
		fflush(stdout);
		for(i=0;i<counter;i++)
		{
			printf("command name: %s process id: %d\n",arr[i]->pname,arr[i]->pid);
		}
	}
	///////////////////////////////////////////////////////
	else
	{
		execute(argv);//ForMiscellaneous Commands
	}

}
void execute(char **argv)//Executes remaining commands
{
	arr[counter]=malloc(sizeof(proc));
	pid_t pid;
	int status;
	int temp;
	int i;
	pid=fork();
	if(pid==0)
	{
		if(tog==10)
		{
		}
		if((execvp(*argv , argv))<0)
		{
			printf("failed Command Not found\n");
			fflush(stdout);
			_exit(0);
		}
	}
	else
	{
		arr[counter]->pid=pid;
		strcpy(arr[counter]->pname,argv[0]);
		counter++;
		if(tog==-100)
		{
//			while(wait(&status)!=pid);
			wait(&status);
		}
		if(tog==10)
		{
			bgarr[bgcounter]=malloc(sizeof(proc));//Stores PRocess iDs and Names in structures
			bgarr[bgcounter]->pid=pid;
			strcpy(bgarr[bgcounter]->pname,argv[0]);
			bgarr[bgcounter]->x=10;
			bgcounter++;
			printf("command %s pid %d\n",argv[0],pid);
			return;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
void  parse(char *line, char **argv)//Divides Command into Command and arguments
{
	arg_count=0;
	char **temp=argv;
	while (*line != '\0') 
	{       /* if not the end of line ....... */ 
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';     /* replace white spaces with 0    */
		*argv++ = line;          /* save the argument position     */
		arg_count++;
		while (*line != '\0' && *line != ' ' && 
				*line != '\t' && *line != '\n') 
			line++;             /* skip the argument until ...    */
	}
	*argv = '\0';                 /* mark the end of argument list  */
}
